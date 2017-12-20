#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <systemd/sd-bus.h>
#include <linux/i2c-dev-user.h>
#include <log.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define I2C_CLIENT_PEC          0x04    /* Use Packet Error Checking */
#define I2C_M_RECV_LEN          0x0400  /* length will be first received byte */

#define MAX_PATH_LEN 200
#define MAX_SENSOR_NUM 50
#define SAMPLING_N  20

#define CMD_OUTPUT_PORT_0 2

#define FAN_SHM_KEY  (4320)
#define FAN_SHM_PATH "skeleton/fan_algorithm2"
#define MAX_WAIT_TIMEOUT  (50000)
#define MAX_SENSOR_READING (9999)
#define MAX_CLOSELOOP_RECORD (10)
#define MAX_CLOSELOOP_SENSOR_NUM (8)
#define MAX_CLOSELOOP_PROFILE_NUM (8)


struct st_closeloop_obj_data {
	int index;
	int sensor_tracking;
	int warning_temp;
	int sensor_reading;
	int integral_error[100];
	int integral_i;
	int last_error;
	double Kp;
	double Ki;
	double Kd;
	int critical_temp;
};

struct st_fan_obj_path_info {
	char service_bus[MAX_PATH_LEN];
	char service_inf[MAX_PATH_LEN];
	char ext_service_bus[MAX_PATH_LEN]; //for pwm0~2 mapping xyz.openbmc_project.Hwmon.hwmon4 Bus
	                                    //   pwm3~5 mapping xyz.openbmc_project.Hwmon.hwmon3 Bus
	char ext_service_inf[MAX_PATH_LEN];
	char path[MAX_SENSOR_NUM][MAX_PATH_LEN];
	int  sensor_number_list[MAX_SENSOR_NUM];
	int  sensor_exist[MAX_SENSOR_NUM];
	int size_sensor_list;
	int size;
	void *obj_data;
	struct st_fan_obj_path_info *next;
};

struct st_fan_closeloop_par {
	double Kp;
	double Ki;
	double Kd;
	int sensor_tracking;
	int warning_temp;
	double pid_value;
	double closeloop_speed;
	int closeloop_sensor_reading;
	int sample_n;

	double current_fanspeed;
	int total_integral_error;
	int cur_integral_error;
	int last_integral_error;
	int groups_sensor_reading[MAX_CLOSELOOP_SENSOR_NUM];
};

struct st_fan_parameter {
	int flag_closeloop; //0: init ; 1:do nothing ; 2: changed; 3:lock waiting
	int closeloop_count;
	struct st_fan_closeloop_par closeloop_param[MAX_CLOSELOOP_PROFILE_NUM];

	int flag_openloop; //0: init ; 1:do nothing ; 2: changed; 3:lock waiting
	float g_ParamA;
	float g_ParamB;
	float g_ParamC;
	int g_LowAmb;
	int g_UpAmb;
	int g_LowSpeed;
	int g_HighSpeed;
	int openloop_speed;
	double openloop_sensor_reading;
	int openloop_sensor_offset;
	int openloop_warning_upper;
	int openloop_critical_upper;

	int current_speed;
	int max_fanspeed;
	int min_fanspeed;
	int current_power_state;
	int debug_msg_info_en; //0:close fan alogrithm debug message; 1: open fan alogrithm debug message
};

enum FAN_ALGO_TYPE {
	EM_CLOSELOOP = 0,
	EM_OPENLOOP
};


static struct st_fan_obj_path_info g_FanInputObjPath = {0};
static struct st_fan_obj_path_info g_CloseloopG1_ObjPath = {0};
static struct st_fan_obj_path_info g_CloseloopG3_ObjPath = {0};
static struct st_fan_obj_path_info g_CloseloopG2_ObjPath = {0};
static struct st_fan_obj_path_info g_AmbientObjPath = {0};
static struct st_fan_obj_path_info g_FanSpeedObjPath = {0};
static struct st_fan_obj_path_info g_SetFanSpeedObjPath = {0};
static struct st_fan_obj_path_info g_FanModuleObjPath = {0};
static struct st_fan_obj_path_info g_PowerObjPath = {0};
static struct st_fan_obj_path_info *g_Closeloop_Header = NULL;
static struct st_fan_obj_path_info *g_Openloop_Header = NULL;

static struct st_fan_parameter *g_fan_para_shm = NULL;
static int g_shm_id;
static char *g_shm_addr = NULL;

struct st_closeloop_record {
	int sensor_reading;
	double pid_val;
	double cal_speed;
	int current_error;
};

static struct st_closeloop_record g_closeloop_record[MAX_CLOSELOOP_RECORD];
static int g_closeloop_record_count = 0;



//OpenLoop config parameters
static float g_ParamA= 0;
static float g_ParamB= 0;
static float g_ParamC= 0;
static int g_LowAmb = 0;
static int g_UpAmb = 0;
static int g_LowSpeed = 0;
static int g_HighSpeed = 0;
static int g_Openloop_Warning_Upper = 0;
static int g_Openloop_Critical_Upper = 0;



//Fan LED command & register mask
static int FAN_LED_OFF   =  0;
static int FAN_LED_PORT0_ALL_BLUE = 0;
static int FAN_LED_PORT1_ALL_BLUE = 0;
static int FAN_LED_PORT0_ALL_RED = 0;
static int FAN_LED_PORT1_ALL_RED = 0;
static int PORT0_FAN_LED_RED_MASK = 0;
static int PORT0_FAN_LED_BLUE_MASK = 0;
static int PORT1_FAN_LED_RED_MASK = 0;
static int PORT1_FAN_LED_BLUE_MASK = 0;
static int g_fanled_speed_limit = 0;
static int g_fanled_aux_speed_limit = 0;
static unsigned int g_trigger_system_event = 0;


//Fan LED I2C bus
static char g_FanLed_I2CBus [MAX_PATH_LEN] = {0};
//Fan LED I2C Slave Address
static unsigned char g_FanLed_SlaveAddr = 0;

static double g_FanSpeed = 0;
static int g_Openloopspeed = 0;
static double g_Closeloopspeed = 0;
static unsigned int closeloop_first_time = 0;

static int initial_fan_config(sd_bus *bus);
static int get_dbus_hwmon_mapping_parameters(sd_bus *bus , char *request_param , char *response_data, int response_len);


static int push_fan_obj(struct st_fan_obj_path_info **header, struct st_fan_obj_path_info *item)
{
	struct st_fan_obj_path_info *t_header;

	if (*header == NULL) {
		*header = item;
		return 1;
	}

	t_header = *header;
	while (t_header->next != NULL)
		t_header = t_header->next;

	t_header->next = item;
	return 1;
}

static int freeall_fan_obj(struct st_fan_obj_path_info **header)
{
	struct st_fan_obj_path_info *t_header = NULL, *t_next = NULL;
	t_header = *header;
	while(t_header != NULL) {
		if (t_header->obj_data != NULL)
			free(t_header->obj_data);
		t_next = t_header->next;
		free(t_header);
		t_header = t_next;
	}
}

static int i2c_open(int bus)
{
	int rc = 0, fd = -1;
	char fn[32];

	snprintf(fn, sizeof(fn), "/dev/i2c-%d", bus);
	fd = open(fn, O_RDWR);
	if (fd == -1) {
		printf("--> Set Fan: Failed to open i2c device %s", fn);
		return -1;
	}
	return fd;
}

static void add_system_event_thermal_shutdown(sd_bus *bus, enum FAN_ALGO_TYPE fan_algo_type)
{
	int rc;
	sd_bus_error bus_error = SD_BUS_ERROR_NULL;
	sd_bus_message *response = NULL;

	if (g_trigger_system_event != 0) //even trigger thermal shutdown event
		return ;

	rc = sd_bus_call_method(bus,
				"xyz.openbmc_project.Logging",
				"/xyz/openbmc_project/logging/internal/manager",
				"xyz.openbmc_project.Logging.Internal.Manager",
				"Commit",
				&bus_error,
				&response,
				"ts",
				(unsigned long long) 0, "xyz.openbmc_project.State.Shutdown.ThermalEvent.Error.Processor");
	if(rc < 0)
		if (g_fan_para_shm->debug_msg_info_en == 1)
			fprintf(stderr, "add_system_event_thermal_shutdown  ERROR: %s!!!!\n",  bus_error.message);
	else {
		g_trigger_system_event |= (1<<((int)fan_algo_type));
	}
	sd_bus_error_free(&bus_error);
	response = sd_bus_message_unref(response);
}

static void system_shut_down(sd_bus *bus, enum FAN_ALGO_TYPE fan_algo_type) {
	add_system_event_thermal_shutdown(bus, fan_algo_type);
	if (g_fan_para_shm->debug_msg_info_en == 1)
		printf("system_shut_down to prepare poweroff!!!! \n");
	system("obmcutil  poweroff");
}


#define PCA9535_ADDR 0x20
static int set_fanled( uint8_t port0, uint8_t port1)
{
	struct i2c_rdwr_ioctl_data data;
	struct i2c_msg msg[1];
	int rc = 0, use_pec = 0;
	uint8_t write_bytes[3];
	int fd;
	static int flag_fanled_enable = 0;

	fd = i2c_open(9);
	if (fd == -1)
	{
		printf("%s, %d  fd Error: %d\n", __FUNCTION__, __LINE__, fd);
		return -1;
	}

//	fprintf(stderr,"SetFanLed: port0 = %02X,port1 = %02X\n",port0,port1);

	memset(&msg, 0, sizeof(msg));

	write_bytes[0] = CMD_OUTPUT_PORT_0;
	write_bytes[1] = port0;
	write_bytes[2] = port1;
  
	msg[0].addr = PCA9535_ADDR;
	msg[0].flags = (use_pec) ? I2C_CLIENT_PEC : 0;
	msg[0].len = sizeof(write_bytes);
	msg[0].buf = write_bytes;

	data.msgs = msg;
	data.nmsgs = 1;
	rc = ioctl(fd, I2C_RDWR, &data);
	if (rc < 0) {
		printf("SetFanLed: Failed to do raw io");
		close(fd);
		return -1;
	}

	if (flag_fanled_enable == 0) {
		memset(&msg, 0, sizeof(msg));
		write_bytes[0] = 6;
		write_bytes[1] = 0;
		write_bytes[2] = 0;
	  
		msg[0].addr = PCA9535_ADDR;
		msg[0].flags = (use_pec) ? I2C_CLIENT_PEC : 0;
		msg[0].len = 3;
		msg[0].buf = write_bytes;

		data.msgs = msg;
		data.nmsgs = 1;
		rc = ioctl(fd, I2C_RDWR, &data);
		if (rc < 0) {
			printf("SetFanLed: Failed to do raw io: flag_fanled_enable");
			close(fd);
			return -1;
		}

		system("i2cset -y 9 0x20 6 0 w");
		
		flag_fanled_enable = 1;
	}

	close(fd);

	return 0;
}

static int calculate_closeloop(struct st_closeloop_obj_data *sensor_data, double current_fanspeed, sd_bus *bus)
{
	int total_integral_error;
	int i;
	double pid_value;
	double pwm_speed;
	double Kp = 0, Ki = 0, Kd = 0;
	int cur_integral_error = 0;
	int sample_n = 0;
	int index;

	if (sensor_data == NULL)
		return 0;

	index = sensor_data->index;

	sensor_data->sensor_reading = g_fan_para_shm->closeloop_param[index].closeloop_sensor_reading;

	if (sensor_data->sensor_reading <=0) {
		pwm_speed = 100;
		g_closeloop_record[g_closeloop_record_count].cal_speed = 100;
		g_closeloop_record[g_closeloop_record_count].current_error = 0;
		g_closeloop_record[g_closeloop_record_count].pid_val = 0;
		g_closeloop_record[g_closeloop_record_count].sensor_reading = sensor_data->sensor_reading;
		g_closeloop_record_count++;


		g_fan_para_shm->closeloop_param[index].pid_value = 0;
		g_fan_para_shm->closeloop_param[index].closeloop_speed = 100;
		g_fan_para_shm->closeloop_param[index].current_fanspeed = current_fanspeed;
		g_Closeloopspeed = 100;

		return 1;
	}

	Kp = sensor_data->Kp;
	Ki = sensor_data->Ki;
	Kd = sensor_data->Kd;
	if (g_fan_para_shm != NULL)
		sample_n = g_fan_para_shm->closeloop_param[index].sample_n;
	else
		sample_n = SAMPLING_N;

	if (g_fan_para_shm->debug_msg_info_en == 1)
		printf("[FAN_ALGORITHM][%s, %d] [PID value] kp:%f, Ki:%f, Kd:%f, target: %d\n", __FUNCTION__, __LINE__, Kp, Ki, Kd, sensor_data->sensor_tracking);

	cur_integral_error =(int) (sensor_data->sensor_reading - sensor_data->sensor_tracking);
	sensor_data->integral_i = sensor_data->integral_i % sample_n;
	sensor_data->integral_error[sensor_data->integral_i] = cur_integral_error;
	sensor_data->integral_i=(sensor_data->integral_i+1) % sample_n;
	total_integral_error = 0;

	for(i=0; i<sample_n; i++) {
		total_integral_error += sensor_data->integral_error[i] ;
		if (g_fan_para_shm->debug_msg_info_en == 1)
			printf("[FAN_ALGORITHM][%s, %d]  integral_error[%d]:%d\n", __FUNCTION__, __LINE__, i, sensor_data->integral_error[i]);
	}

	pid_value = ((double) Kp * cur_integral_error +  (double)Ki * total_integral_error + (double)Kd * (cur_integral_error - sensor_data->last_error));
	if (g_fan_para_shm->debug_msg_info_en == 1)
		printf("[FAN_ALGORITHM][%s, %d] cur_integral_error:%d, total_integral_error:%d, last_error:%d, pid_value:%f\n", __FUNCTION__, __LINE__, cur_integral_error,
		       total_integral_error, sensor_data->last_error, pid_value);
	//pwm_speed = pid_value + g_FanSpeed;

	if (closeloop_first_time <= 10) {
		if (current_fanspeed == 100)
			current_fanspeed = 40;
		else {
			closeloop_first_time +=1;
		}
	}

	pwm_speed = pid_value + (double) current_fanspeed;

	g_fan_para_shm->closeloop_param[index].pid_value = pid_value;
	g_fan_para_shm->closeloop_param[index].closeloop_speed = pwm_speed;

	g_fan_para_shm->closeloop_param[index].current_fanspeed = current_fanspeed;
	g_fan_para_shm->closeloop_param[index].total_integral_error = total_integral_error;
	g_fan_para_shm->closeloop_param[index].cur_integral_error  = cur_integral_error;
	g_fan_para_shm->closeloop_param[index].last_integral_error  = sensor_data->last_error;

	if (g_fan_para_shm->debug_msg_info_en == 1)
		printf("[FAN_ALGORITHM][%s, %d] [Closeloop pid_value] %f; [Closeloop Calculate Fan Speed]: %f\n", __FUNCTION__, __LINE__, pid_value, pwm_speed);

	if(pwm_speed > 100)
		pwm_speed = 100;

	if(pwm_speed < 0)
		pwm_speed = 0;

	sensor_data->last_error = (int) cur_integral_error;

	g_closeloop_record[g_closeloop_record_count].cal_speed = pwm_speed;
	g_closeloop_record[g_closeloop_record_count].current_error = cur_integral_error;
	g_closeloop_record[g_closeloop_record_count].pid_val = pid_value;
	g_closeloop_record[g_closeloop_record_count].sensor_reading = sensor_data->sensor_reading;
	g_closeloop_record_count++;

	if (g_Closeloopspeed < pwm_speed)
		g_Closeloopspeed = pwm_speed;

	if (sensor_data->sensor_reading>=sensor_data->critical_temp) {
		system_shut_down(bus, EM_CLOSELOOP);
		g_Closeloopspeed = 100;
		if (g_fan_para_shm->debug_msg_info_en == 1)
			printf("%s, %d, upper Critical !!!! \n", __FUNCTION__, __LINE__);
	} else if (sensor_data->sensor_reading>=sensor_data->warning_temp)
		g_Closeloopspeed = 100;
		if (g_fan_para_shm->debug_msg_info_en == 1)
			printf("%s, %d, upper Warning !!!! \n", __FUNCTION__, __LINE__);
	else {
		g_trigger_system_event &=~(1<<((int)EM_CLOSELOOP));
	}

	return 1;
}

static int calculate_openloop (double sensorreading, sd_bus *bus)
{
	int speed = 0;

	if (sensorreading<=0) {
		g_Openloopspeed = 100; // As inlet temperature is not available, set fan speed to 100% and warn
		g_fan_para_shm->openloop_speed = 100;
		return 1;
	}

	sensorreading=sensorreading+g_fan_para_shm->openloop_sensor_offset;

	if (sensorreading > (double)g_Openloop_Critical_Upper) {
		system_shut_down(bus, EM_OPENLOOP);
		g_Openloopspeed = 100; // as openloop reading > warning upper, then Fan Speed 100%
		g_fan_para_shm->openloop_speed = 100;
		if (g_fan_para_shm->debug_msg_info_en == 1)
			printf("%s, %d, upper Critical !!!! \n", __FUNCTION__, __LINE__);
		return 1;
	} else if (sensorreading > (double) g_Openloop_Warning_Upper) {
		g_Openloopspeed = 100; // as openloop reading > warning upper, then Fan Speed 100%
		g_fan_para_shm->openloop_speed = 100;
		if (g_fan_para_shm->debug_msg_info_en == 1)
			printf("%s, %d, upper Warning !!!! \n", __FUNCTION__, __LINE__);
		return 1;
	}
	g_trigger_system_event &=~(1<<((int)EM_OPENLOOP));

	if (sensorreading > g_UpAmb) {
		speed = g_HighSpeed;
	} else if (sensorreading <= g_LowAmb) {
		speed = g_LowSpeed;
	} else {
		speed =(int) (( (double)g_ParamA * sensorreading * sensorreading ) + ((double) g_ParamB * sensorreading ) + (double)g_ParamC);
		speed = (speed > g_HighSpeed)? g_HighSpeed : ((speed < g_LowSpeed)? g_LowSpeed : speed);
	}

	g_fan_para_shm->openloop_speed = speed;

	if (g_fan_para_shm->debug_msg_info_en == 1)
		printf("[FAN_ALGORITHM][%s, %d] [Openloop Parameters: g_UpAmb, g_LowAmb, A, B, C] %d ,%d, %f, %f, %f; [Openloop Calculate Fan Speed]: %d\n", __FUNCTION__, __LINE__,
		       g_UpAmb, g_LowAmb, g_ParamA, g_ParamB, g_ParamC, speed);

	if (g_Openloopspeed < speed)
		g_Openloopspeed = speed;
	return 1;
}

static int get_sensor_reading(sd_bus *bus, char *obj_path, int *sensor_reading, struct st_fan_obj_path_info *fan_obj)
{
	sd_bus_error bus_error = SD_BUS_ERROR_NULL;
	sd_bus_message *response = NULL;
	int rc;

	*sensor_reading = 0;

	if (strlen(fan_obj->service_bus) <= 0 || strlen(fan_obj->service_inf) <= 0 || strlen(obj_path) <= 0)
		return -1;

	rc = sd_bus_call_method(bus,
				fan_obj->service_bus,
				obj_path,
				fan_obj->service_inf,
				"getValue",
				&bus_error,
				&response,
				NULL);

	if(rc < 0) {
		if (g_fan_para_shm->debug_msg_info_en == 1)
			fprintf(stderr, "obj_path: %s Failed to get temperature from dbus: %s\n", obj_path, bus_error.message);
	} else {
		rc = sd_bus_message_read(response,"v", "i", sensor_reading);
		if (rc < 0)
			fprintf(stderr, "obj_path: %s Failed to parse response message:[%s]\n",obj_path, strerror(-rc));
	}

	sd_bus_error_free(&bus_error);
	response = sd_bus_message_unref(response);

	return rc;
}

static int get_sensor_reading_with_bus(sd_bus *bus, char *obj_path, int *sensor_reading, struct st_fan_obj_path_info *fan_obj, char *service_bus, char *service_inf)
{
	sd_bus_error bus_error = SD_BUS_ERROR_NULL;
	sd_bus_message *response = NULL;
	int rc;
	char *ptr_service_bus;
	char temp_service_bus[100];
	int flag_error = 0;

	*sensor_reading = 0;


	ptr_service_bus = service_bus;
retry_sensor_reading_with_bus:

	if (ptr_service_bus == NULL || service_inf == NULL || strlen(ptr_service_bus) <= 0 || strlen(service_inf) <= 0 || strlen(obj_path) <= 0)
		return -1;

	rc = sd_bus_call_method(bus,
				ptr_service_bus,
				obj_path,
				"org.freedesktop.DBus.Properties",
				"Get",
				&bus_error,
				&response,
				"ss",
				service_inf, "Value");
	
	if(rc < 0) {
		if (g_fan_para_shm->debug_msg_info_en == 1)
			fprintf(stderr, "obj_path: %s Failed to get temperature from dbus: %s, busname:%s\n", obj_path, bus_error.message, ptr_service_bus);
		if (flag_error==0 && get_dbus_hwmon_mapping_parameters(bus, ptr_service_bus, temp_service_bus, sizeof(temp_service_bus)) >= 0) {
			ptr_service_bus = temp_service_bus;
			if (strcmp(fan_obj->service_bus, service_bus) == 0)
				strcpy(fan_obj->service_bus, temp_service_bus);
			else
				strcpy(fan_obj->ext_service_bus, temp_service_bus);
			bus_error = SD_BUS_ERROR_NULL;
			response = NULL;
			flag_error =1;
			goto retry_sensor_reading_with_bus;
		}
	} else {
		rc = sd_bus_message_read(response,"v", "x", sensor_reading);
		if (rc < 0)
			fprintf(stderr, "obj_path: %s Failed to parse response message:[%s]\n",obj_path, strerror(-rc));
	}

	sd_bus_error_free(&bus_error);
	response = sd_bus_message_unref(response);

	return rc;
}

static int get_max_sensor_reading_with_bus(sd_bus *bus, struct st_fan_obj_path_info *fan_obj)
{
	int i;
	int rc;
	int sensor_reading;
	int max_value = 0;

	for(i=0; i<fan_obj->size; i++) {
		if (fan_obj->sensor_exist[i] == 1)
			continue;
		rc = -1;
		if (fan_obj->sensor_exist[i] ==2 || fan_obj->sensor_exist[i] == 0) {
			rc = get_sensor_reading_with_bus(bus, fan_obj->path[i], &sensor_reading, fan_obj, fan_obj->service_bus, fan_obj->service_inf);
			if (rc >= 0)
				fan_obj->sensor_exist[i] = 2;
		}
		if (fan_obj->sensor_exist[i] ==3 || fan_obj->sensor_exist[i] == 0) {
			if (rc < 0 && strlen(fan_obj->ext_service_bus)>0 && strlen(fan_obj->ext_service_inf)>0) {
				rc = get_sensor_reading_with_bus(bus, fan_obj->path[i], &sensor_reading, fan_obj, fan_obj->ext_service_bus, fan_obj->ext_service_inf);
				if (rc >=0)
					fan_obj->sensor_exist[i] = 3;
			}
		}
		if (rc >= 0)
			max_value = (max_value < sensor_reading)? sensor_reading : max_value;
		else {
			if (strlen(fan_obj->service_bus)>0 && strlen(fan_obj->ext_service_bus)>0 && fan_obj->sensor_exist[i]==0) {
				if(strncmp(fan_obj->service_bus, "occ-hwmon", strlen("occ-hwmon")) != 0 && strncmp(fan_obj->ext_service_bus, "occ-hwmon", strlen("occ-hwmon")) != 0)
					fan_obj->sensor_exist[i] = 1;
			}
		}
	}

	return max_value;
}


static int get_max_sensor_reading(sd_bus *bus, struct st_fan_obj_path_info *fan_obj)
{
	int i;
	int rc;
	int sensor_reading;
	int max_value = 0;

	for(i=0; i<fan_obj->size; i++) {
		rc = get_sensor_reading(bus, fan_obj->path[i], &sensor_reading, fan_obj);
		if (rc >= 0)
			max_value = (max_value < sensor_reading)? sensor_reading : max_value;
	}

	return max_value;
}


static int get_sensor_reading_Fan(sd_bus *bus, char *obj_path, int *sensor_reading, struct st_fan_obj_path_info *fan_obj)
{
	sd_bus_error bus_error = SD_BUS_ERROR_NULL;
	sd_bus_message *response = NULL;
	int rc;

	*sensor_reading = 0;

	if (strlen(fan_obj->service_bus) <= 0 || strlen(fan_obj->service_inf) <= 0 || strlen(obj_path) <= 0)
		return -1;

	rc = sd_bus_call_method(bus,
				fan_obj->service_bus,
				obj_path,
				fan_obj->service_inf,
				"getValue_Fan",
				&bus_error,
				&response,
				NULL);


	if(rc < 0) {
		fprintf(stderr, "obj_path: %s Failed to get temperature from dbus: %s\n", obj_path, bus_error.message);
	} else {
		rc = sd_bus_message_read(response, "i", sensor_reading);
		if (rc < 0)
			fprintf(stderr, "obj_path: %s Failed to parse response message:[%s]\n",obj_path, strerror(-rc));
	}

	sd_bus_error_free(&bus_error);
	response = sd_bus_message_unref(response);

	return rc;
}

static int get_max_sensor_reading_Fan(sd_bus *bus, struct st_fan_obj_path_info *fan_obj)
{
	int i;
	int rc;
	int sensor_reading;
	int max_value = 0;

	for(i=0; i<fan_obj->size; i++) {
		rc = get_sensor_reading_Fan(bus, fan_obj->path[i], &sensor_reading, fan_obj);
		if (rc >= 0)
			max_value = (max_value < sensor_reading)? sensor_reading : max_value;
	}

	return max_value;
}

static int set_sensor_value_Pwm(char *obj_path, int sensor_value)
{
	FILE *fPtr;
	
	if (obj_path == NULL)
		return -1;
	
	fPtr = fopen(obj_path,"w");
	if (fPtr == NULL)
		return -1;
	fprintf(fPtr, "%d", sensor_value);
	fclose(fPtr);
	return 0;
}


static void check_change_closeloop_params(struct st_closeloop_obj_data *sensor_data)
{
	int wait_times = 0;
	int index;

	if (g_fan_para_shm == NULL)
		return ;


	while (g_fan_para_shm->flag_closeloop == 3 && wait_times<MAX_WAIT_TIMEOUT)
		wait_times++;

	if (g_fan_para_shm->flag_closeloop == 2) {
		index = sensor_data->index;
		sensor_data->Kp = g_fan_para_shm->closeloop_param[index].Kp;
		sensor_data->Ki = g_fan_para_shm->closeloop_param[index].Ki;
		sensor_data->Kd = g_fan_para_shm->closeloop_param[index].Kd;
		sensor_data->sensor_tracking = g_fan_para_shm->closeloop_param[index].sensor_tracking;
		sensor_data->warning_temp = g_fan_para_shm->closeloop_param[index].warning_temp;
	}
}

static void check_change_openloop_params()
{
	int wait_times = 0;

	if (g_fan_para_shm == NULL)
		return ;


	while (g_fan_para_shm->flag_openloop == 3 && wait_times<MAX_WAIT_TIMEOUT)
		wait_times++;

	if (g_fan_para_shm->flag_openloop == 2) {
		g_ParamA = g_fan_para_shm->g_ParamA;
		g_ParamB = g_fan_para_shm->g_ParamB;
		g_ParamC = g_fan_para_shm->g_ParamC;
		g_LowAmb = g_fan_para_shm->g_LowAmb;
		g_UpAmb = g_fan_para_shm->g_UpAmb;
		g_LowSpeed = g_fan_para_shm->g_LowSpeed;
		g_HighSpeed = g_fan_para_shm->g_HighSpeed;
		g_Openloop_Warning_Upper = g_fan_para_shm->openloop_warning_upper;
		g_Openloop_Critical_Upper = g_fan_para_shm->openloop_critical_upper;
	}
}


static int fan_control_algorithm_monitor(void)
{
	sd_bus *bus = NULL;
	sd_bus_error bus_error = SD_BUS_ERROR_NULL;
	sd_bus_message *response = NULL;
	int rc = 0, i = 0, offset = 0;
	int Fan_tach, FinalFanSpeed = 255;
	int Power_state = 0, fan_led_port0 = 0xFF, fan_led_port1 = 0xFF;
	char fan_presence[MAX_SENSOR_NUM] = {0}, fan_presence_previous[MAX_SENSOR_NUM] = {0};
	struct st_fan_obj_path_info *t_header = NULL;
	struct st_closeloop_obj_data *t_closeloop_data = NULL;
	int closeloop_reading = 0, openloop_reading = 0;
	int current_fanspeed = 0;
	int first_time_set = 0;
	char *ptr_temp_fan_bus = NULL, *ptr_temp_fan_intf= NULL;

	double real_fanspeed = 0.0;
	do {
		/* Connect to the user bus this time */
		rc = sd_bus_open_system(&bus);
		if(rc < 0) {
			fprintf(stderr, "Failed to connect to system bus for fan_algorithm: %s\n", strerror(-rc));
			bus = sd_bus_flush_close_unref(bus);
			sleep(1);
		}
	} while (rc < 0);

	initial_fan_config(bus);

	while (1) {
		rc = sd_bus_call_method(bus,
					g_PowerObjPath.service_bus,
					g_PowerObjPath.path[0],
					g_PowerObjPath.service_inf,
					"getPowerState",
					&bus_error,
					&response,
					NULL);
		if(rc < 0) {
			fprintf(stderr, "Failed to get power state from dbus: %s\n", bus_error.message);
			goto finish;
		}

		rc = sd_bus_message_read(response, "i", &Power_state);
		if (rc < 0 ) {
			fprintf(stderr, "Failed to parse GetPowerState response message:[%s]\n", strerror(-rc));
			goto finish;
		}
		sd_bus_error_free(&bus_error);
		response = sd_bus_message_unref(response);

		g_fan_para_shm->current_power_state = Power_state;
		if (Power_state == 0) // Aux Condition
			g_trigger_system_event = 0; //Reset system event flag for next dc on system event

		current_fanspeed = 0;
		for(i=0; i<g_FanSpeedObjPath.size; i++) {
			int temp_fanspeed = 0;
			if (i<3) {
				ptr_temp_fan_bus = g_FanSpeedObjPath.service_bus;
				ptr_temp_fan_intf = g_FanSpeedObjPath.service_inf;
			} else {
				ptr_temp_fan_bus = g_FanSpeedObjPath.ext_service_bus;
				ptr_temp_fan_intf = g_FanSpeedObjPath.ext_service_inf;
			}

			rc = get_sensor_reading_with_bus(bus, g_FanSpeedObjPath.path[i], &temp_fanspeed, &g_FanSpeedObjPath, ptr_temp_fan_bus, ptr_temp_fan_intf);
			if (rc >= 0)
			{
				current_fanspeed = current_fanspeed < temp_fanspeed? temp_fanspeed: current_fanspeed;
			}
		}
		g_fan_para_shm->current_speed = current_fanspeed;
		if (g_fan_para_shm->debug_msg_info_en == 1)
			printf("[FAN_ALGORITHM][Current FanSpeed value] :%d\n", current_fanspeed);
		if (current_fanspeed <0)
			current_fanspeed = 0;
		else {
			current_fanspeed = current_fanspeed*100;
			if (real_fanspeed == 0)
				real_fanspeed = (double) current_fanspeed / 255;
			current_fanspeed =(int) current_fanspeed / 255;
		}
		if (current_fanspeed > 100)
			current_fanspeed = 100;

		if (real_fanspeed == 0 || real_fanspeed >= 100)
			real_fanspeed = (double) current_fanspeed;

		closeloop_reading = MAX_SENSOR_READING;
		openloop_reading = 0;
		t_header = g_Closeloop_Header;
		g_Closeloopspeed = 0;
		g_closeloop_record_count = 0;
		int closeloop_index = 0;
		while (t_header != NULL) {
			t_closeloop_data = (struct st_closeloop_obj_data *) t_header->obj_data;

			if (t_closeloop_data != NULL) {
				t_closeloop_data->sensor_reading = get_max_sensor_reading_with_bus(bus, t_header);
				if (t_closeloop_data->sensor_reading >= 0)
					t_closeloop_data->sensor_reading = t_closeloop_data->sensor_reading / 1000;
				g_fan_para_shm->closeloop_param[t_closeloop_data->index].closeloop_sensor_reading = t_closeloop_data->sensor_reading;
				check_change_closeloop_params(t_closeloop_data);
				calculate_closeloop(t_closeloop_data, real_fanspeed, bus);
				closeloop_reading = (closeloop_reading<t_closeloop_data->sensor_reading)? t_closeloop_data->sensor_reading:closeloop_reading;
			}
			t_header = t_header->next;
			closeloop_index = closeloop_index +1;
		}
		double adjust_closeloop_speed = -1;
		int min_current_error = 9999;
		for (i = 0; i<g_closeloop_record_count; i++)
		{
			if (g_fan_para_shm->debug_msg_info_en == 1)
					printf("[FAN_ALGORITHM][Adjust Closeloop list ] i:%d, current_error:%d, adjust speed:%f pid_valu:%f\n",
					    i, g_closeloop_record[i].current_error, g_closeloop_record[i].cal_speed,
					    g_closeloop_record[i].pid_val);
			if (g_closeloop_record[i].pid_val >=0) {
				adjust_closeloop_speed = -1;
				break;
			}
			int c_error = g_closeloop_record[i].current_error;
			if (c_error < 0)
				c_error = c_error*-1;
			if (c_error > min_current_error){
				adjust_closeloop_speed = g_closeloop_record[i].cal_speed;
				min_current_error =c_error;
				if (g_fan_para_shm->debug_msg_info_en == 1)
					printf("[FAN_ALGORITHM][Adjust Closeloop Speed:original: %f, adjust: %f \n", g_Closeloopspeed, adjust_closeloop_speed);
				g_Closeloopspeed = adjust_closeloop_speed;
			}
		}
		if (adjust_closeloop_speed >=0) {
			if (g_fan_para_shm->debug_msg_info_en == 1)
				printf("[FAN_ALGORITHM][Adjust Closeloop Speed:original: %f, adjust: %f \n", g_Closeloopspeed, adjust_closeloop_speed);
			g_Closeloopspeed = adjust_closeloop_speed;
		}

		check_change_openloop_params();
		openloop_reading = 0;
		t_header = g_Openloop_Header;
		g_Openloopspeed = 0;
		while (t_header != NULL) {
			double t_reading;
			t_reading = (double) get_max_sensor_reading_with_bus(bus, t_header);
			t_reading = t_reading>=0? (double) t_reading/1000 : t_reading;
			g_fan_para_shm->openloop_sensor_reading = t_reading;
			calculate_openloop(t_reading, bus);
			openloop_reading = (openloop_reading<t_reading? t_reading:openloop_reading);
			t_header = t_header->next;
		}

		if((double) g_Openloopspeed > g_Closeloopspeed) {
			g_FanSpeed = (double)g_Openloopspeed;
		} else {
			g_FanSpeed = (double)g_Closeloopspeed;
		}

		if (first_time_set == 0 && g_Openloopspeed>0 && g_Closeloopspeed>0) {
			if (g_Closeloopspeed == 100)
				g_FanSpeed = (double)g_Openloopspeed;
			first_time_set = 1;
		}

		real_fanspeed = g_FanSpeed;
		FinalFanSpeed =(int) ((double)g_FanSpeed * 255)/100;

		if (g_fan_para_shm != NULL) {
			if (FinalFanSpeed < g_fan_para_shm->min_fanspeed) {
				FinalFanSpeed = g_fan_para_shm->min_fanspeed;
				real_fanspeed = (double)  g_fan_para_shm->min_fanspeed/100;
			}
			else if (FinalFanSpeed > g_fan_para_shm->max_fanspeed) {
				FinalFanSpeed = g_fan_para_shm->max_fanspeed;
				real_fanspeed = (double)  g_fan_para_shm->max_fanspeed/100;
			}
	    }
		g_FanSpeed = real_fanspeed;

		if (Power_state == 0) { //AUX condition
			if (g_FanSpeed > g_fanled_aux_speed_limit) {
				fan_led_port0 = FAN_LED_PORT0_ALL_BLUE;
				fan_led_port1 = FAN_LED_PORT1_ALL_BLUE;
			}
			else {
				fan_led_port0 = FAN_LED_PORT0_ALL_RED;
				fan_led_port1 = FAN_LED_PORT1_ALL_RED;
			}
		} else if (Power_state == 1) { //Power on condition
			if(g_FanSpeed > g_fanled_speed_limit) {
				fan_led_port0 = FAN_LED_PORT0_ALL_BLUE;
				fan_led_port1 = FAN_LED_PORT1_ALL_BLUE;
			}
			else {
				fan_led_port0 = FAN_LED_PORT0_ALL_RED;
				fan_led_port1 = FAN_LED_PORT1_ALL_RED;
			}
		}

		int fan_in_index = 0;
		int fan_mask = 0x0;
		for(i=0, fan_in_index=0; fan_in_index<g_FanInputObjPath.size; i++, fan_in_index+=2) {
			if (i<6) {
				ptr_temp_fan_bus = g_FanInputObjPath.service_bus;
				ptr_temp_fan_intf = g_FanInputObjPath.service_inf;
			} else {
				ptr_temp_fan_bus = g_FanInputObjPath.ext_service_bus;
				ptr_temp_fan_intf = g_FanInputObjPath.ext_service_inf;
			}
			rc = get_sensor_reading_with_bus(bus, g_FanInputObjPath.path[fan_in_index], &Fan_tach, &g_FanInputObjPath, ptr_temp_fan_bus, ptr_temp_fan_intf);
			if (rc < 0)
				Fan_tach = 0;

			if (Fan_tach == 0) {
				//FinalFanSpeed = 255;
				if (i < 6) { //FAN1~FAN3
					offset = (i / 2)*2;
					offset = 6 - offset;
					fan_mask = 0x3<<offset;
					fan_led_port1 = fan_led_port1 & ~(fan_mask);
					fan_led_port1 = fan_led_port1 | (PORT1_FAN_LED_RED_MASK<<offset);
				} else { //FAN4~FAN6
					offset = ((i-6) / 2)*2;
					offset = offset + 2;
					fan_mask = 0x3<<offset;
					fan_led_port0 = fan_led_port0 & ~(fan_mask);
					fan_led_port0 = fan_led_port0 | (PORT0_FAN_LED_RED_MASK<<offset);
				}
			}else {
				fan_presence[i/2] = 1;
			}
		}

		set_fanled(fan_led_port0,fan_led_port1);

		for(i=0; i<g_SetFanSpeedObjPath.size; i++) {
			rc = set_sensor_value_Pwm(g_SetFanSpeedObjPath.path[i], FinalFanSpeed);
			if(rc < 0)
				fprintf(stderr, "Failed to adjust fan speed  %d:%s\n", i, g_FanSpeedObjPath.path[i]);
		}

		for(i=0; i<g_FanModuleObjPath.size; i++) {
			if (fan_presence[i] == fan_presence_previous[i])
				continue;

			rc = sd_bus_call_method(bus,
						"org.openbmc.Inventory",
						g_FanModuleObjPath.path[i],
						"org.openbmc.InventoryItem",
						"setPresent",
						&bus_error,
						&response,
						"s",
						(fan_presence[i] == 1 ? "True" : "False"));
			if(rc < 0)
				fprintf(stderr, "Failed to update fan presence via dbus: %s\n", bus_error.message);
			sd_bus_error_free(&bus_error);
			response = sd_bus_message_unref(response);
		}

finish:
		sd_bus_error_free(&bus_error);
		response = sd_bus_message_unref(response);
		sd_bus_flush(bus);
		memcpy(fan_presence_previous, fan_presence, sizeof(fan_presence));
		memset(fan_presence, 0, sizeof(fan_presence));
		usleep(200*1000);
	}
	bus = sd_bus_flush_close_unref(bus);
	freeall_fan_obj(&g_Closeloop_Header);
	freeall_fan_obj(&g_Openloop_Header);
	shmdt(g_shm_addr);
	shmctl(g_shm_id , IPC_RMID , NULL);
	return rc < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

static int get_dbus_fan_parameters(sd_bus *bus , char *request_param , int *response_len, char response_data[50][200])
{
	sd_bus_error bus_error = SD_BUS_ERROR_NULL;
	sd_bus_message *response = NULL;
	int rc = 0;
	const char*  response_param;

	*response_len = 0; //clear response_len

	rc = sd_bus_call_method(bus,
				"org.openbmc.managers.System",
				"/org/openbmc/managers/System",
				"org.openbmc.managers.System",
				"getFanControlParams",
				&bus_error,
				&response,
				"s", request_param);
	if(rc < 0) {
		printf("%s, %d response message:[%s]\n", __FUNCTION__, __LINE__, strerror(-rc));
	} else {
		rc = sd_bus_message_read(response, "s", &response_param);
		if (rc < 0 ) {
			fprintf(stderr, "Failed to parse response message:[%s]\n", strerror(-rc));
			return rc;
		}

		int stard_idx = 0, end_idx = 0;
		int str_len = 0;
		while (response_param[end_idx]!=0) {
			if (response_param[end_idx] == ';') { // ';' means to indentify every data token
				if (stard_idx < end_idx) {
					str_len = end_idx - stard_idx;
					if (str_len < MAX_PATH_LEN) {
						memcpy(response_data[*response_len], response_param+stard_idx, str_len);
						response_data[*response_len][str_len] = '\0';
						*response_len=*response_len+1;
					} else
						printf("Error:%s[%d], parse string exceds length:%d, str_len:%d\n", __FUNCTION__, __LINE__, MAX_PATH_LEN, str_len);
				}
				stard_idx=end_idx+1;
			}
			end_idx++;
		}
	}
	sd_bus_error_free(&bus_error);
	response = sd_bus_message_unref(response);
	return rc;
}


static int get_dbus_hwmon_mapping_parameters(sd_bus *bus , char *request_param , char *response_data, int response_len)
{
	sd_bus_error bus_error = SD_BUS_ERROR_NULL;
	sd_bus_message *response = NULL;
	int rc = 0;
	const char*  response_param;

	if (response_data == NULL || response_len == 0)
		return -1;

	response_data[0] = 0;
	rc = sd_bus_call_method(bus,
				"org.openbmc.managers.System",
				"/org/openbmc/managers/System",
				"org.openbmc.managers.System",
				"getHwmonMappingParams",
				&bus_error,
				&response,
				"s", request_param);
	if(rc < 0) {
		printf("%s, %d response message:[%s]\n", __FUNCTION__, __LINE__, strerror(-rc));
	} else {
		rc = sd_bus_message_read(response, "s", &response_param);
		if (rc < 0 ) {
			fprintf(stderr, "Failed to parse response message:[%s]\n", strerror(-rc));
			goto error_hwmon_mapping;
		}
		strncpy(response_data, response_param, response_len);
		if (strcmp(response_data, request_param) ==0) {
			rc = -1;
		    goto error_hwmon_mapping;
		}
	}
error_hwmon_mapping:
	sd_bus_error_free(&bus_error);
	response = sd_bus_message_unref(response);
	return rc;
}

static int initial_fan_config(sd_bus *bus)
{
	int response_len = 0;
	char response_data[50][200];
	int i;
	int obj_count = 0;
	char *p;
	int index;

	get_dbus_fan_parameters(bus, "FAN_INPUT_OBJ", &response_len, response_data);
	g_FanInputObjPath.size = response_len;
	for (i = 0; i<response_len; i+=2) {
		strcpy(g_FanInputObjPath.path[i], response_data[i]);
	}
	get_dbus_fan_parameters(bus, "FAN_DBUS_INTF_LOOKUP#FAN_INPUT_OBJ", &response_len, response_data);
	if (response_len == 2) {
		strcpy(g_FanInputObjPath.service_bus , response_data[0]);
		strcpy(g_FanInputObjPath.service_inf , response_data[1]);
	}
	get_dbus_fan_parameters(bus, "FAN_DBUS_INTF_LOOKUP#EXT_FAN_INPUT_OBJ", &response_len, response_data);
	g_FanInputObjPath.ext_service_bus[0] = 0;
	g_FanInputObjPath.ext_service_inf[0] = 0;
	if (response_len == 2) {
		strcpy(g_FanInputObjPath.ext_service_bus , response_data[0]);
		strcpy(g_FanInputObjPath.ext_service_inf , response_data[1]);
	}

	get_dbus_fan_parameters(bus, "FAN_OUTPUT_OBJ", &response_len, response_data);
	g_FanSpeedObjPath.size = response_len;
	for (i = 0; i<response_len; i++) {
		strcpy(g_FanSpeedObjPath.path[i], response_data[i]);
	}
	get_dbus_fan_parameters(bus, "FAN_DBUS_INTF_LOOKUP#FAN_OUTPUT_OBJ", &response_len, response_data);
	if (response_len == 2) {
		strcpy(g_FanSpeedObjPath.service_bus , response_data[0]);
		strcpy(g_FanSpeedObjPath.service_inf , response_data[1]);
	}
	get_dbus_fan_parameters(bus, "FAN_DBUS_INTF_LOOKUP#EXT_FAN_OUTPUT_OBJ", &response_len, response_data);
	g_FanSpeedObjPath.ext_service_bus[0] = 0;
	g_FanSpeedObjPath.ext_service_inf[0] = 0;
	if (response_len == 2) {
		strcpy(g_FanSpeedObjPath.ext_service_bus , response_data[0]);
		strcpy(g_FanSpeedObjPath.ext_service_inf , response_data[1]);
	}


	get_dbus_fan_parameters(bus, "SET_FAN_OUTPUT_OBJ", &response_len, response_data);
	g_SetFanSpeedObjPath.size = response_len;
	for (i = 0; i<response_len; i++) {
		strcpy(g_SetFanSpeedObjPath.path[i], response_data[i]);
	}

	get_dbus_fan_parameters(bus, "OPEN_LOOP_PARAM", &response_len, response_data);
	g_ParamA = atof(response_data[0]);
	g_ParamB = atof(response_data[1]);
	g_ParamC = atof(response_data[2]);
	g_LowAmb = atoi(response_data[3]);
	g_UpAmb = atoi(response_data[4]);
	g_LowSpeed = atoi(response_data[5]);
	g_HighSpeed = atoi(response_data[6]);
	g_Openloop_Warning_Upper = atoi(response_data[7]);
	g_Openloop_Critical_Upper = atoi(response_data[8]);

	g_fan_para_shm->g_ParamA= g_ParamA;
	g_fan_para_shm->g_ParamB= g_ParamB;
	g_fan_para_shm->g_ParamC= g_ParamC;
	g_fan_para_shm->g_LowAmb= g_LowAmb;
	g_fan_para_shm->g_UpAmb= g_UpAmb;
	g_fan_para_shm->g_LowSpeed= g_LowSpeed;
	g_fan_para_shm->g_HighSpeed= g_HighSpeed;
	g_fan_para_shm->flag_openloop= 1;
	g_fan_para_shm->openloop_warning_upper = g_Openloop_Warning_Upper;
	g_fan_para_shm->openloop_critical_upper = g_Openloop_Critical_Upper;


	obj_count = 1;
	struct st_closeloop_obj_data *t_closeloop_data = NULL;
	while (1) {
		char prefix_closeloop[100];
		struct st_fan_obj_path_info *t_fan_obj = NULL;

		prefix_closeloop[0] = 0;
		sprintf(prefix_closeloop, "CLOSE_LOOP_GROUPS_%d", obj_count);
		get_dbus_fan_parameters(bus, prefix_closeloop, &response_len, response_data);
		if (response_len <= 2)
			break;

		t_fan_obj =(struct st_fan_obj_path_info *) malloc(sizeof(struct st_fan_obj_path_info));

		t_fan_obj->size = response_len;
		for (i = 0; i<response_len ; i++)
			strcpy(t_fan_obj->path[i], response_data[i]);

		prefix_closeloop[0] = 0;
		sprintf(prefix_closeloop, "FAN_DBUS_INTF_LOOKUP#CLOSE_LOOP_GROUPS_%d", obj_count);
		get_dbus_fan_parameters(bus, prefix_closeloop, &response_len, response_data);
		if (response_len == 2) {
			strcpy(t_fan_obj->service_bus , response_data[0]);
			strcpy(t_fan_obj->service_inf , response_data[1]);
		} else {
			free(t_fan_obj);
			break;
		}

		prefix_closeloop[0] = 0;
		sprintf(prefix_closeloop, "FAN_DBUS_INTF_LOOKUP#EXT_CLOSE_LOOP_GROUPS_%d", obj_count);
		get_dbus_fan_parameters(bus, prefix_closeloop, &response_len, response_data);
		t_fan_obj->ext_service_bus[0] = 0;
		t_fan_obj->ext_service_inf[0] = 0;
		if (response_len == 2) {
			strcpy(t_fan_obj->ext_service_bus , response_data[0]);
			strcpy(t_fan_obj->ext_service_inf , response_data[1]);
		}

		prefix_closeloop[0] = 0;
		sprintf(prefix_closeloop, "CLOSE_LOOP_PARAM_%d", obj_count);
		get_dbus_fan_parameters(bus, prefix_closeloop, &response_len, response_data);
		if (response_len > 0) {
			t_closeloop_data = (struct st_closeloop_obj_data *) malloc(sizeof(struct st_closeloop_obj_data));
			index = obj_count -1;
			t_closeloop_data->index = index;
			t_closeloop_data->Kp = (double)atof(response_data[0]);
			t_closeloop_data->Ki = (double)atof(response_data[1]);
			t_closeloop_data->Kd = (double)atof(response_data[2]);
			t_closeloop_data->sensor_tracking = atoi(response_data[3]);
			t_closeloop_data->warning_temp = atoi(response_data[4]);
			t_closeloop_data->critical_temp = atoi(response_data[5]);

			g_fan_para_shm->closeloop_param[index].Kp = t_closeloop_data->Kp;
			g_fan_para_shm->closeloop_param[index].Ki = t_closeloop_data->Ki;
			g_fan_para_shm->closeloop_param[index].Kd = t_closeloop_data->Kd;
			g_fan_para_shm->closeloop_param[index].sensor_tracking = t_closeloop_data->sensor_tracking;
			g_fan_para_shm->closeloop_param[index].warning_temp = t_closeloop_data->warning_temp;
			g_fan_para_shm->flag_closeloop = 1;

			for (i = 0 ; i<100; i++)
				t_closeloop_data->integral_error[i] = 0;
			t_closeloop_data->last_error = 0;
			t_closeloop_data->integral_i = 0;
		}
		t_fan_obj->obj_data = (void*)t_closeloop_data;
		

		push_fan_obj(&g_Closeloop_Header, t_fan_obj);
		obj_count++;
	}
	g_fan_para_shm->closeloop_count =  obj_count-1;

	obj_count = 1;
	while (1) {
		char prefix_openloop[100];
		struct st_fan_obj_path_info *t_fan_obj = NULL;

		prefix_openloop[0] = 0;
		sprintf(prefix_openloop, "OPEN_LOOP_GROUPS_%d", obj_count);
		get_dbus_fan_parameters(bus, prefix_openloop, &response_len, response_data);
		if (response_len == 0)
			break;

		t_fan_obj =(struct st_fan_obj_path_info *) malloc(sizeof(struct st_fan_obj_path_info));

		t_fan_obj->size = response_len;
		for (i = 0; i<response_len ; i++)
			strcpy(t_fan_obj->path[i], response_data[i]);

		prefix_openloop[0] = 0;
		sprintf(prefix_openloop, "FAN_DBUS_INTF_LOOKUP#OPEN_LOOP_GROUPS_%d", obj_count);
		get_dbus_fan_parameters(bus, prefix_openloop, &response_len, response_data);
		t_fan_obj->ext_service_bus[0] = 0;
		t_fan_obj->ext_service_inf[0] = 0;
		if (response_len == 2) {
			strcpy(t_fan_obj->service_bus , response_data[0]);
			strcpy(t_fan_obj->service_inf , response_data[1]);
		} else {
			free(t_fan_obj);
			break;
		}

		push_fan_obj(&g_Openloop_Header, t_fan_obj);

		obj_count++;
	}

	get_dbus_fan_parameters(bus, "FAN_LED_OFF", &response_len, response_data);
	FAN_LED_OFF = response_len > 0? strtoul(response_data[0], &p, 16): FAN_LED_OFF;

	get_dbus_fan_parameters(bus, "FAN_LED_PORT0_ALL_BLUE", &response_len, response_data);
	FAN_LED_PORT0_ALL_BLUE = response_len > 0? strtoul(response_data[0], &p, 16): FAN_LED_PORT0_ALL_BLUE;

	get_dbus_fan_parameters(bus, "FAN_LED_PORT1_ALL_BLUE", &response_len, response_data);
	FAN_LED_PORT1_ALL_BLUE = response_len > 0? strtoul(response_data[0], &p, 16): FAN_LED_PORT1_ALL_BLUE;

	get_dbus_fan_parameters(bus, "FAN_LED_PORT0_ALL_RED", &response_len, response_data);
	FAN_LED_PORT0_ALL_RED = response_len > 0? strtoul(response_data[0], &p, 16): FAN_LED_PORT0_ALL_RED;

	get_dbus_fan_parameters(bus, "FAN_LED_PORT1_ALL_RED", &response_len, response_data);
	FAN_LED_PORT1_ALL_RED = response_len > 0? strtoul(response_data[0], &p, 16): FAN_LED_PORT1_ALL_RED;

	get_dbus_fan_parameters(bus, "PORT0_FAN_LED_RED_MASK", &response_len, response_data);
	PORT0_FAN_LED_RED_MASK = response_len > 0? strtoul(response_data[0], &p, 16): PORT0_FAN_LED_RED_MASK;

	get_dbus_fan_parameters(bus, "PORT0_FAN_LED_BLUE_MASK", &response_len, response_data);
	PORT0_FAN_LED_BLUE_MASK = response_len > 0? strtoul(response_data[0], &p, 16): PORT0_FAN_LED_BLUE_MASK;

	get_dbus_fan_parameters(bus, "PORT1_FAN_LED_RED_MASK", &response_len, response_data);
	PORT1_FAN_LED_RED_MASK = response_len > 0? strtoul(response_data[0], &p, 16): PORT1_FAN_LED_RED_MASK;

	get_dbus_fan_parameters(bus, "PORT1_FAN_LED_BLUE_MASK", &response_len, response_data);
	PORT1_FAN_LED_BLUE_MASK = response_len > 0? strtoul(response_data[0], &p, 16): PORT1_FAN_LED_BLUE_MASK;

	get_dbus_fan_parameters(bus, "FAN_LED_SPEED_LIMIT", &response_len, response_data);
	g_fanled_speed_limit = response_len > 0? atoi(response_data[0]): g_fanled_speed_limit;

	get_dbus_fan_parameters(bus, "FAN_LED_AUX_SPEED_LIMIT", &response_len, response_data);
	g_fanled_aux_speed_limit = response_len > 0? atoi(response_data[0]): g_fanled_speed_limit;

	get_dbus_fan_parameters(bus, "FAN_LED_I2C_BUS", &response_len, response_data);
	if (response_len > 0)
		strcpy(g_FanLed_I2CBus, response_data[0]);

	get_dbus_fan_parameters(bus, "FAN_LED_I2C_SLAVE_ADDRESS", &response_len, response_data);
	g_FanLed_SlaveAddr = response_len > 0? strtoul(response_data[0], &p, 16): g_FanLed_SlaveAddr;

	//Refere to FRU_INSTANCES in config/.py file to search inventory item object path with keywords: 'fan'
	get_dbus_fan_parameters(bus, "INVENTORY_FAN", &response_len, response_data);
	g_FanModuleObjPath.size = response_len;
	for (i = 0; i<response_len; i++) {
		strcpy(g_FanModuleObjPath.path[i], response_data[i]);
	}

	get_dbus_fan_parameters(bus, "CHASSIS_POWER_STATE", &response_len, response_data);
	if (response_len > 0) {
		strcpy(g_PowerObjPath.path[0], response_data[0]);
		g_PowerObjPath.size = 1;
	}
	get_dbus_fan_parameters(bus, "FAN_DBUS_INTF_LOOKUP#CHASSIS_POWER_STATE", &response_len, response_data);
	if (response_len == 2) {
		strcpy(g_PowerObjPath.service_bus , response_data[0]);
		strcpy(g_PowerObjPath.service_inf , response_data[1]);
	}
}

static void inital_fan_pid_shm()
{
	key_t key = ftok(FAN_SHM_PATH, FAN_SHM_KEY);
	int i, j;

	g_shm_id = shmget(key, sizeof(struct st_fan_parameter), (IPC_CREAT | 0666));
	if (g_shm_id < 0) {
		printf("Error: shmid \n");
		return ;
	}
	g_shm_addr =  shmat(g_shm_id, NULL, 0);
	if (g_shm_addr == (char *) -1) {
		printf("Error: shmat \n");
		return ;
	}

	g_fan_para_shm = (struct st_fan_parameter *) g_shm_addr;
	if (g_fan_para_shm != NULL) {
		g_fan_para_shm->flag_closeloop = 0;
		g_fan_para_shm->flag_openloop = 0;
		g_fan_para_shm->max_fanspeed = 255;
		g_fan_para_shm->min_fanspeed = 0;
		for (i = 0 ; i<MAX_CLOSELOOP_PROFILE_NUM; i++) {
			g_fan_para_shm->closeloop_param[i].closeloop_sensor_reading = 0;
			g_fan_para_shm->closeloop_param[i].sample_n = SAMPLING_N;
			for (j = 0; j<MAX_CLOSELOOP_SENSOR_NUM; j++)
				g_fan_para_shm->closeloop_param[i].groups_sensor_reading[j] = 0;
		}
		g_fan_para_shm->closeloop_count = 0;
		g_fan_para_shm->openloop_sensor_offset = -2;
		g_fan_para_shm->debug_msg_info_en = 0;
		g_fan_para_shm->current_power_state = -1;
		g_fan_para_shm->openloop_warning_upper = MAX_SENSOR_READING;
		g_fan_para_shm->openloop_critical_upper = MAX_SENSOR_READING;
	}
}

int main(int argc, char *argv[])
{
         //TODO: workaround for fan algorithm service starting time
        //      to aviod fan algorithm to get dbus fail
	sleep(90);
	inital_fan_pid_shm();
	return fan_control_algorithm_monitor();
}


