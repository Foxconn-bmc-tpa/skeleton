## System states
##   state can change to next state in 2 ways:
##   - a process emits a GotoSystemState signal with state name to goto
##   - objects specified in EXIT_STATE_DEPEND have started
SYSTEM_STATES = [
    'BASE_APPS',
    'BMC_STARTING',
    'BMC_READY',
    'HOST_POWERING_ON',
    'HOST_POWERED_ON',
    'HOST_BOOTING',
    'HOST_BOOTED',
    'HOST_POWERED_OFF',
]

EXIT_STATE_DEPEND = {
    'BASE_APPS' : {
        '/org/openbmc/sensors': 0,
    },
    'BMC_STARTING' : {
        '/org/openbmc/control/chassis0': 0,
        '/org/openbmc/control/power0' : 0,
        '/org/openbmc/control/flash/bios' : 0,
    },
}

FRU_INSTANCES = {
    '<inventory_root>/system' : { 'fru_type' : 'SYSTEM','is_fru' : True, 'present' : "True" },
    '<inventory_root>/system/bios' : { 'fru_type' : 'SYSTEM','is_fru' : True, 'present' : "True" },
    '<inventory_root>/system/misc' : { 'fru_type' : 'SYSTEM','is_fru' : False, },

    '<inventory_root>/system/chassis' : { 'fru_type' : 'SYSTEM','is_fru' : True, 'present' : "True" },

    '<inventory_root>/system/chassis/motherboard' : { 'fru_type' : 'MAIN_PLANAR','is_fru' : True, 'present' : "True"},

    '<inventory_root>/system/systemevent'                  : { 'fru_type' : 'SYSTEM_EVENT', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/refclock' : { 'fru_type' : 'MAIN_PLANAR', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/pcieclock': { 'fru_type' : 'MAIN_PLANAR', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/todclock' : { 'fru_type' : 'MAIN_PLANAR', 'is_fru' : False, },

    '<inventory_root>/system/chassis/fan0' : { 'fru_type' : 'FAN','is_fru' : True, },
    '<inventory_root>/system/chassis/fan1' : { 'fru_type' : 'FAN','is_fru' : True, },
    '<inventory_root>/system/chassis/fan2' : { 'fru_type' : 'FAN','is_fru' : True, },
    '<inventory_root>/system/chassis/fan3' : { 'fru_type' : 'FAN','is_fru' : True, },
    '<inventory_root>/system/chassis/fan4' : { 'fru_type' : 'FAN','is_fru' : True, },
    '<inventory_root>/system/chassis/fan5' : { 'fru_type' : 'FAN','is_fru' : True, },

    '<inventory_root>/system/chassis/motherboard/bmc' : { 'fru_type' : 'BMC','is_fru' : False, 'manufacturer' : 'ASPEED', 'present' : "True" },

    '<inventory_root>/system/chassis/motherboard/cpu0' : { 'fru_type' : 'CPU', 'is_fru' : True, },
    '<inventory_root>/system/chassis/motherboard/cpu1' : { 'fru_type' : 'CPU', 'is_fru' : True, },

    '<inventory_root>/system/chassis/motherboard/cpu0/core0' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core1' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core2' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core3' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core4' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core5' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core6' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core7' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core8' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core9' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core10': { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core11': { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core12' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core13' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core14' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core15' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core16' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core17' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core18' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core19' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core20' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core21' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core22': { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core23': { 'fru_type' : 'CORE', 'is_fru' : False, },

    '<inventory_root>/system/chassis/motherboard/cpu1/core0' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core1' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core2' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core3' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core4' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core5' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core6' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core7' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core8' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core9' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core10' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core11' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core12' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core13' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core14' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core15' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core16' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core17' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core18' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core19' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core20' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core21' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core22' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core23' : { 'fru_type' : 'CORE', 'is_fru' : False, },

    '<inventory_root>/system/chassis/motherboard/dimm0' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm1' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm2' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm3' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm4' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm5' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm6' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm7' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm8' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm9' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm10' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm11' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm12' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm13' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm14' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm15' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm16' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm17' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm18' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm19' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm20' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm21' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm22' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm23' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm24' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm25' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm26' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm27' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm28' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm29' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm30' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm31' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
}

ID_LOOKUP = {
    'FRU' : {
        0x01 : '<inventory_root>/system/chassis/motherboard/cpu0',
        0x02 : '<inventory_root>/system/chassis/motherboard/cpu1',
        0x03 : '<inventory_root>/system/chassis/motherboard',
        0x04 : '<inventory_root>/system/chassis/motherboard/membuf0',
        0x05 : '<inventory_root>/system/chassis/motherboard/membuf1',
        0x06 : '<inventory_root>/system/chassis/motherboard/membuf2',
        0x07 : '<inventory_root>/system/chassis/motherboard/membuf3',
        0x08 : '<inventory_root>/system/chassis/motherboard/membuf4',
        0x09 : '<inventory_root>/system/chassis/motherboard/membuf5',
        0x0c : '<inventory_root>/system/chassis/motherboard/dimm0',
        0x0d : '<inventory_root>/system/chassis/motherboard/dimm1',
        0x0e : '<inventory_root>/system/chassis/motherboard/dimm2',
        0x0f : '<inventory_root>/system/chassis/motherboard/dimm3',
        0x10 : '<inventory_root>/system/chassis/motherboard/dimm4',
        0x11 : '<inventory_root>/system/chassis/motherboard/dimm5',
        0x12 : '<inventory_root>/system/chassis/motherboard/dimm6',
        0x13 : '<inventory_root>/system/chassis/motherboard/dimm7',
        0x14 : '<inventory_root>/system/chassis/motherboard/dimm8',
        0x15 : '<inventory_root>/system/chassis/motherboard/dimm9',
        0x16 : '<inventory_root>/system/chassis/motherboard/dimm10',
        0x17 : '<inventory_root>/system/chassis/motherboard/dimm11',
        0x18 : '<inventory_root>/system/chassis/motherboard/dimm12',
        0x19 : '<inventory_root>/system/chassis/motherboard/dimm13',
        0x1a : '<inventory_root>/system/chassis/motherboard/dimm14',
        0x1b : '<inventory_root>/system/chassis/motherboard/dimm15',
        0x1c : '<inventory_root>/system/chassis/motherboard/dimm16',
        0x1d : '<inventory_root>/system/chassis/motherboard/dimm17',
        0x1e : '<inventory_root>/system/chassis/motherboard/dimm18',
        0x1f : '<inventory_root>/system/chassis/motherboard/dimm19',
        0x20 : '<inventory_root>/system/chassis/motherboard/dimm20',
        0x21 : '<inventory_root>/system/chassis/motherboard/dimm21',
        0x22 : '<inventory_root>/system/chassis/motherboard/dimm22',
        0x23 : '<inventory_root>/system/chassis/motherboard/dimm23',
        0x24 : '<inventory_root>/system/chassis/motherboard/dimm24',
        0x25 : '<inventory_root>/system/chassis/motherboard/dimm25',
        0x26 : '<inventory_root>/system/chassis/motherboard/dimm26',
        0x27 : '<inventory_root>/system/chassis/motherboard/dimm27',
        0x28 : '<inventory_root>/system/chassis/motherboard/dimm28',
        0x29 : '<inventory_root>/system/chassis/motherboard/dimm29',
        0x2a : '<inventory_root>/system/chassis/motherboard/dimm30',
        0x2b : '<inventory_root>/system/chassis/motherboard/dimm31',
    },
    'FRU_STR' : {
        'PRODUCT_0'  : '<inventory_root>/system/bios',
        'BOARD_1'    : '<inventory_root>/system/chassis/motherboard/cpu0',
        'BOARD_2'    : '<inventory_root>/system/chassis/motherboard/cpu1',
        'CHASSIS_3'  : '<inventory_root>/system/chassis/motherboard',
        'BOARD_3'    : '<inventory_root>/system/misc',
        'PRODUCT_12'   : '<inventory_root>/system/chassis/motherboard/dimm0',
        'PRODUCT_13'   : '<inventory_root>/system/chassis/motherboard/dimm1',
        'PRODUCT_14'   : '<inventory_root>/system/chassis/motherboard/dimm2',
        'PRODUCT_15'   : '<inventory_root>/system/chassis/motherboard/dimm3',
        'PRODUCT_16'   : '<inventory_root>/system/chassis/motherboard/dimm4',
        'PRODUCT_17'   : '<inventory_root>/system/chassis/motherboard/dimm5',
        'PRODUCT_18'   : '<inventory_root>/system/chassis/motherboard/dimm6',
        'PRODUCT_19'   : '<inventory_root>/system/chassis/motherboard/dimm7',
        'PRODUCT_20'   : '<inventory_root>/system/chassis/motherboard/dimm8',
        'PRODUCT_21'   : '<inventory_root>/system/chassis/motherboard/dimm9',
        'PRODUCT_22'   : '<inventory_root>/system/chassis/motherboard/dimm10',
        'PRODUCT_23'   : '<inventory_root>/system/chassis/motherboard/dimm11',
        'PRODUCT_24'   : '<inventory_root>/system/chassis/motherboard/dimm12',
        'PRODUCT_25'   : '<inventory_root>/system/chassis/motherboard/dimm13',
        'PRODUCT_26'   : '<inventory_root>/system/chassis/motherboard/dimm14',
        'PRODUCT_27'   : '<inventory_root>/system/chassis/motherboard/dimm15',
        'PRODUCT_28'   : '<inventory_root>/system/chassis/motherboard/dimm16',
        'PRODUCT_29'   : '<inventory_root>/system/chassis/motherboard/dimm17',
        'PRODUCT_30'   : '<inventory_root>/system/chassis/motherboard/dimm18',
        'PRODUCT_31'   : '<inventory_root>/system/chassis/motherboard/dimm19',
        'PRODUCT_32'   : '<inventory_root>/system/chassis/motherboard/dimm20',
        'PRODUCT_33'   : '<inventory_root>/system/chassis/motherboard/dimm21',
        'PRODUCT_34'   : '<inventory_root>/system/chassis/motherboard/dimm22',
        'PRODUCT_35'   : '<inventory_root>/system/chassis/motherboard/dimm23',
        'PRODUCT_36'   : '<inventory_root>/system/chassis/motherboard/dimm24',
        'PRODUCT_37'   : '<inventory_root>/system/chassis/motherboard/dimm25',
        'PRODUCT_38'   : '<inventory_root>/system/chassis/motherboard/dimm26',
        'PRODUCT_39'   : '<inventory_root>/system/chassis/motherboard/dimm27',
        'PRODUCT_40'   : '<inventory_root>/system/chassis/motherboard/dimm28',
        'PRODUCT_41'   : '<inventory_root>/system/chassis/motherboard/dimm29',
        'PRODUCT_42'   : '<inventory_root>/system/chassis/motherboard/dimm30',
        'PRODUCT_43'   : '<inventory_root>/system/chassis/motherboard/dimm31',
        'PRODUCT_47'   : '<inventory_root>/system/misc',
    },
    'SENSOR' : {
        0x02 : '/org/openbmc/sensors/host/HostStatus',
        0x03 : '/org/openbmc/sensors/host/BootProgress',
        0x21 : '<inventory_root>/system/chassis/motherboard/cpu0',
        0x71 : '<inventory_root>/system/chassis/motherboard/cpu1',
        0xc7 : '<inventory_root>/system/chassis/motherboard/dimm3',
        0xc5 : '<inventory_root>/system/chassis/motherboard/dimm2',
        0xc3 : '<inventory_root>/system/chassis/motherboard/dimm1',
        0xc1 : '<inventory_root>/system/chassis/motherboard/dimm0',
        0xcf : '<inventory_root>/system/chassis/motherboard/dimm7',
        0xcd : '<inventory_root>/system/chassis/motherboard/dimm6',
        0xcb : '<inventory_root>/system/chassis/motherboard/dimm5',
        0xc9 : '<inventory_root>/system/chassis/motherboard/dimm4',
        0xd7 : '<inventory_root>/system/chassis/motherboard/dimm11',
        0xd5 : '<inventory_root>/system/chassis/motherboard/dimm10',
        0xd3 : '<inventory_root>/system/chassis/motherboard/dimm9',
        0xd1 : '<inventory_root>/system/chassis/motherboard/dimm8',
        0xdf : '<inventory_root>/system/chassis/motherboard/dimm15',
        0xdd : '<inventory_root>/system/chassis/motherboard/dimm14',
        0xdb : '<inventory_root>/system/chassis/motherboard/dimm13',
        0xd9 : '<inventory_root>/system/chassis/motherboard/dimm12',
        0xe7 : '<inventory_root>/system/chassis/motherboard/dimm19',
        0xe5 : '<inventory_root>/system/chassis/motherboard/dimm18',
        0xe3 : '<inventory_root>/system/chassis/motherboard/dimm17',
        0xe1 : '<inventory_root>/system/chassis/motherboard/dimm16',
        0xef : '<inventory_root>/system/chassis/motherboard/dimm23',
        0xed : '<inventory_root>/system/chassis/motherboard/dimm22',
        0xeb : '<inventory_root>/system/chassis/motherboard/dimm21',
        0xe9 : '<inventory_root>/system/chassis/motherboard/dimm20',
        0xf7 : '<inventory_root>/system/chassis/motherboard/dimm27',
        0xf5 : '<inventory_root>/system/chassis/motherboard/dimm26',
        0xf3 : '<inventory_root>/system/chassis/motherboard/dimm25',
        0xf1 : '<inventory_root>/system/chassis/motherboard/dimm24',
        0xbf : '<inventory_root>/system/chassis/motherboard/dimm31',
        0xfd : '<inventory_root>/system/chassis/motherboard/dimm30',
        0xfb : '<inventory_root>/system/chassis/motherboard/dimm29',
        0xf9 : '<inventory_root>/system/chassis/motherboard/dimm28',
        0x23 : '<inventory_root>/system/chassis/motherboard/cpu0/core0',
        0x26 : '<inventory_root>/system/chassis/motherboard/cpu0/core1',
        0x29 : '<inventory_root>/system/chassis/motherboard/cpu0/core2',
        0x2c : '<inventory_root>/system/chassis/motherboard/cpu0/core3',
        0x2f : '<inventory_root>/system/chassis/motherboard/cpu0/core4',
        0x32 : '<inventory_root>/system/chassis/motherboard/cpu0/core5',
        0x35 : '<inventory_root>/system/chassis/motherboard/cpu0/core6',
        0x38 : '<inventory_root>/system/chassis/motherboard/cpu0/core7',
        0x3b : '<inventory_root>/system/chassis/motherboard/cpu0/core8',
        0x3e : '<inventory_root>/system/chassis/motherboard/cpu0/core9',
        0x41 : '<inventory_root>/system/chassis/motherboard/cpu0/core10',
        0x44 : '<inventory_root>/system/chassis/motherboard/cpu0/core11',
        0x47 : '<inventory_root>/system/chassis/motherboard/cpu0/core12',
        0x4a : '<inventory_root>/system/chassis/motherboard/cpu0/core13',
        0x4d : '<inventory_root>/system/chassis/motherboard/cpu0/core14',
        0x50 : '<inventory_root>/system/chassis/motherboard/cpu0/core15',
        0x53 : '<inventory_root>/system/chassis/motherboard/cpu0/core16',
        0x56 : '<inventory_root>/system/chassis/motherboard/cpu0/core17',
        0x59 : '<inventory_root>/system/chassis/motherboard/cpu0/core18',
        0x5c : '<inventory_root>/system/chassis/motherboard/cpu0/core19',
        0x5f : '<inventory_root>/system/chassis/motherboard/cpu0/core20',
        0x62 : '<inventory_root>/system/chassis/motherboard/cpu0/core21',
        0x65 : '<inventory_root>/system/chassis/motherboard/cpu1/core22',
        0x68 : '<inventory_root>/system/chassis/motherboard/cpu1/core23',
        0x73 : '<inventory_root>/system/chassis/motherboard/cpu1/core0',
        0x76 : '<inventory_root>/system/chassis/motherboard/cpu1/core1',
        0x79 : '<inventory_root>/system/chassis/motherboard/cpu1/core2',
        0x7c : '<inventory_root>/system/chassis/motherboard/cpu1/core3',
        0x7f : '<inventory_root>/system/chassis/motherboard/cpu1/core4',
        0x82 : '<inventory_root>/system/chassis/motherboard/cpu1/core5',
        0x85 : '<inventory_root>/system/chassis/motherboard/cpu1/core6',
        0x88 : '<inventory_root>/system/chassis/motherboard/cpu1/core7',
        0x8b : '<inventory_root>/system/chassis/motherboard/cpu1/core8',
        0x8e : '<inventory_root>/system/chassis/motherboard/cpu1/core9',
        0x91 : '<inventory_root>/system/chassis/motherboard/cpu1/core10',
        0x94 : '<inventory_root>/system/chassis/motherboard/cpu1/core11',
        0x97 : '<inventory_root>/system/chassis/motherboard/cpu1/core12',
        0x9a : '<inventory_root>/system/chassis/motherboard/cpu1/core13',
        0x9d : '<inventory_root>/system/chassis/motherboard/cpu1/core14',
        0xa0 : '<inventory_root>/system/chassis/motherboard/cpu1/core15',
        0xa3 : '<inventory_root>/system/chassis/motherboard/cpu1/core16',
        0xa6 : '<inventory_root>/system/chassis/motherboard/cpu1/core17',
        0xa9 : '<inventory_root>/system/chassis/motherboard/cpu1/core18',
        0xac : '<inventory_root>/system/chassis/motherboard/cpu1/core19',
        0xaf : '<inventory_root>/system/chassis/motherboard/cpu1/core20',
        0xb2 : '<inventory_root>/system/chassis/motherboard/cpu1/core21',
        0xb5 : '<inventory_root>/system/chassis/motherboard/cpu1/core22',
        0xb8 : '<inventory_root>/system/chassis/motherboard/cpu1/core23',
        0x07 : '/org/openbmc/sensors/host/BootCount',
        0x10 : '<inventory_root>/system/chassis/motherboard',
        0x01 : '<inventory_root>/system/systemevent',
        0x11 : '<inventory_root>/system/chassis/motherboard/refclock',
        0x12 : '<inventory_root>/system/chassis/motherboard/pcieclock',
        0x13 : '<inventory_root>/system/chassis/motherboard/todclock',
        0x05 : '/org/openbmc/sensors/host/OperatingSystemStatus',
        0x04 : '<inventory_root>/system/chassis/motherboard/pcielink',
    },
    'GPIO_PRESENT' : {}
}

GPIO_CONFIG = {}
GPIO_CONFIG['SOFTWARE_PGOOD'] = \
        {'gpio_pin': 'R1', 'direction': 'out'}
GPIO_CONFIG['BMC_POWER_UP'] = \
        {'gpio_pin': 'D1', 'direction': 'out'}
GPIO_CONFIG['SYS_PWROK_BUFF'] = \
        {'gpio_pin': 'D2', 'direction': 'in'}
GPIO_CONFIG['PHY_RST_N'] = \
        {'gpio_pin': 'D3', 'direction': 'out'}
GPIO_CONFIG['HDD_PWR_EN'] = \
        {'gpio_pin': 'D4', 'direction': 'out'}
GPIO_CONFIG['CP0_DEVICES_RESET_N'] = \
        {'gpio_pin': 'A1', 'direction': 'out'}
GPIO_CONFIG['BMC_CP0_PERST_ENABLE'] = \
        {'gpio_pin': 'A3', 'direction': 'out'}
GPIO_CONFIG['BMC_UCD_LATCH_LE'] = \
        {'gpio_pin': 'B4', 'direction': 'out'}
GPIO_CONFIG['FSI_ENABLE'] = \
        {'gpio_pin': 'D0', 'direction': 'out'}
GPIO_CONFIG['CRONUS_SEL'] = \
        {'gpio_pin': 'P6', 'direction': 'out'}
# FIXME: net name is FP_PWR_BTN_N in schematic
GPIO_CONFIG['POWER_BUTTON'] = \
        {'gpio_pin': 'I3', 'direction': 'both'}
# FIXME: net name is FP_RST_BTN_N in schematic
GPIO_CONFIG['RESET_BUTTON'] = \
        {'gpio_pin': 'AA0', 'direction': 'both'}
GPIO_CONFIG['PE_MEZZB_PRSNT_N'] = \
        {'gpio_pin': 'P7', 'direction': 'in'}
GPIO_CONFIG['WOL_CONTROL'] = \
        {'gpio_pin': 'F6', 'direction': 'falling'}
GPIO_CONFIG['CHECKSTOP'] = \
        {'gpio_pin': 'F7', 'direction': 'falling'}

GPIO_CONFIGS = {
    'power_config' : {
        'latch_out': 'BMC_UCD_LATCH_LE',
        'power_good_in' : 'SYS_PWROK_BUFF',
        'power_up_outs' : [
            ('SOFTWARE_PGOOD', True),
            ('BMC_POWER_UP', True),
        ],
        'reset_outs' : [
        ],
    },
    'hostctl_config' : {
        'fsi_data' : 'FSI_DATA',
        'fsi_clk' : 'FSI_CLK',
        'fsi_enable' : 'FSI_ENABLE',
        'cronus_sel' : 'CRONUS_SEL',
        'optionals' : [
        ],
    },
}

# Miscellaneous non-poll sensor with system specific properties.
# The sensor id is the same as those defined in ID_LOOKUP['SENSOR'].
MISC_SENSORS = {
    0x07 : { 'class' : 'BootCountSensor' },
    0x03 : { 'class' : 'BootProgressSensor' },
    0x05 : { 'class' : 'OperatingSystemStatusSensor' },
}

# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4

# Set fan_algorithm config
#   FAN_DBUS_INTF_LOOKUP: {keys:[dbus interface name]}
#   CHASSIS_POWER_STATE: set chassis power state object path
#   FAN_INPUT_OBJ: set fan input object path
#   FAN_OUTPUT_OBJ: set fan out object path , eg: hwmon control fan speed or directly use pwm control pwm
#   OPEN_LOOP_PARAM: set openloop parameter
#       - paramA
#       - paramB
#       - paramC
#       - Low_Amb
#       - Up_Amb
#       - Low_Speed
#       - High_Speed
#   OPEN_LOOP_GROUPS_1: set openloop object path
#   CLOSE_LOOP_PARAM_1: set closeloop parameter
#       - g_Kp
#       - g_Ki
#       - g_Kd
#       - sensor_tracking
#       - highest warning temp
#       - highest critical temp
#       - sensor reading scale
#   CLOSE_LOOP_GROUPS_1: set closeloop  group001 object path, eg: CPU0/CPU1
#   CLOSE_LOOP_PARAM_2: set closeloop parameter
#       - g_Kp
#       - g_Ki
#       - g_Kd
#       - sensor_tracking
#       - highest warning temp
#       - highest critical temp
#       - sensor reading scale
#   CLOSE_LOOP_GROUPS_2: set closeloop  group002 object path, eg: DIMM
#   FAN_LED_OFF: set fan led command: off
#   FAN_LED_PORT0_ALL_BLUE: set fan led port0 command: all blue
#   FAN_LED_PORT1_ALL_BLUE: set fan led port1 command: all blue
#   FAN_LED_PORT0_ALL_RED: set fan led port0 command: all red
#   FAN_LED_PORT1_ALL_RED: set fan led port1 command: all red
#   PORT0_FAN_LED_RED_MASK: set fan led port0 register mask with red
#   PORT0_FAN_LED_BLUE_MASK: set fan led port0 register mask with blue
#   PORT1_FAN_LED_RED_MASK: set fan led port1 register mask with red
#   PORT1_FAN_LED_BLUE_MASK: set fan led port1 register mask with blue
#   FAN_LED_I2C_BUS: set fan led i2c bus
#   FAN_LED_I2C_SLAVE_ADDRESS: set fan led i2c slave address
FAN_ALGORITHM_CONFIG = {
    'FAN_DBUS_INTF_LOOKUP':
    {
        'CHASSIS_POWER_STATE': ['org.openbmc.control.Chassis'],
        'FAN_INPUT_OBJ' : ['xyz.openbmc_project.Sensor.Value'],
        'FAN_OUTPUT_OBJ' : ['xyz.openbmc_project.Sensor.Value'],
        'OPEN_LOOP_GROUPS_1' :  ['xyz.openbmc_project.Sensor.Value'],
        'CLOSE_LOOP_GROUPS_1' :  ['xyz.openbmc_project.Sensor.Value'],
        'CLOSE_LOOP_GROUPS_2' :  ['xyz.openbmc_project.Sensor.Value'],
        'CLOSE_LOOP_GROUPS_3' :  ['xyz.openbmc_project.Sensor.Value'],
        'CLOSE_LOOP_GROUPS_4' :  ['xyz.openbmc_project.Sensor.Value'],
    },

    'CHASSIS_POWER_STATE': ['/org/openbmc/control/chassis0'],
    'FAN_INPUT_OBJ':
        [
            "/xyz/openbmc_project/sensors/fan_tach/tach/fan0L",
            "/xyz/openbmc_project/sensors/fan_tach/tach/fan0H",
            "/xyz/openbmc_project/sensors/fan_tach/tach/fan1L",
            "/xyz/openbmc_project/sensors/fan_tach/tach/fan1H",
            "/xyz/openbmc_project/sensors/fan_tach/tach/fan2L",
            "/xyz/openbmc_project/sensors/fan_tach/tach/fan2H",
            "/xyz/openbmc_project/sensors/fan_tach/tach/fan3L",
            "/xyz/openbmc_project/sensors/fan_tach/tach/fan3H",
            "/xyz/openbmc_project/sensors/fan_tach/tach/fan4L",
            "/xyz/openbmc_project/sensors/fan_tach/tach/fan4H",
            "/xyz/openbmc_project/sensors/fan_tach/tach/fan5L",
            "/xyz/openbmc_project/sensors/fan_tach/tach/fan5H",
        ],
    'FAN_OUTPUT_OBJ':
        [
            "/xyz/openbmc_project/sensors/pwm/speed/fan0",
            "/xyz/openbmc_project/sensors/pwm/speed/fan1",
            "/xyz/openbmc_project/sensors/pwm/speed/fan2",
            "/xyz/openbmc_project/sensors/pwm/speed/fan3",
            "/xyz/openbmc_project/sensors/pwm/speed/fan4",
            "/xyz/openbmc_project/sensors/pwm/speed/fan5",
        ],
    'SET_FAN_OUTPUT_OBJ':
        [
            "9-002e:pwm3",
            "9-002e:pwm2",
            "9-002e:pwm1",
            "9-002d:pwm3",
            "9-002d:pwm2",
            "9-002d:pwm1",
        ],
#Open Loop Profile# Amb_inlet =System ambient + Ambient sensor Offset
    'OPEN_LOOP_PARAM':
        [
            '0.2113',
            '-11.009',
            '179.4',
            '24',
            '44',
            '36',
            '100',
            '43', # Warning upper
            '46', # Cirtical upper
        ],
    'OPEN_LOOP_GROUPS_1':
        [
            "/xyz/openbmc_project/sensors/temperature/ambient_mb",
        ],
#Close Loop Profile#1: CPU thermal
    'CLOSE_LOOP_PARAM_1' :
        [
            '0.12',
            '-0.0032',
            '0.2',
            '80',
            '85',
            '90',
            '1000',
        ],
    'CLOSE_LOOP_GROUPS_1':
        [
        	"OCC_DEVICE",
        	"/run/initramfs/max_occ_p0_core_temp",
        	"/run/initramfs/max_occ_p1_core_temp",
        ],
#Close Loop Profile#2: DIMM thermal
    'CLOSE_LOOP_PARAM_2' :
        [
            '0.16',
            '-0.004',
            '0.1',
            '80',
            '85',
            '90',
            '1000',
        ],
    'CLOSE_LOOP_GROUPS_2':
        [
            "OCC_DEVICE",
            "/run/initramfs/max_occ_p0_dimm_temp",
            "/run/initramfs/max_occ_p1_dimm_temp",
        ],
#Close Loop Profile#3: HDD thermal
    'CLOSE_LOOP_PARAM_3' :
        [
            '0.16',
            '-0.004',
            '0.1',
            '60',
            '65',
            '71',
            '1000',
        ],
    'CLOSE_LOOP_GROUPS_3':
        [
           "SENSORS_DEVICE_NODE",
           "/sys/bus/i2c/devices/9-0071/hwmon", "hdd_max_temp",
           "/sys/bus/i2c/devices/9-0072/hwmon", "hdd_max_temp",
           "/sys/bus/i2c/devices/9-0073/hwmon", "hdd_max_temp",
           "/sys/bus/i2c/devices/9-0074/hwmon", "hdd_max_temp",
           "/sys/bus/i2c/devices/9-005f/hwmon", "max_temp",
           "PRESENT_GROUP_DEVICE_NODE",
           "/sys/bus/i2c/devices/9-005f/hwmon", "present_status",
           "/sys/bus/i2c/devices/9-005d/hwmon", "nvme_hdd_present_status",
        ],
#Close Loop Profile#4: GPU thermal
    'CLOSE_LOOP_PARAM_4' :
        [
            '0.16',
            '-0.006',
            '0.5',
            '76',
            '82',
            '85',
            '1000',
        ],
    'CLOSE_LOOP_GROUPS_4':
        [
           "SENSORS_DEVICE_NODE",
           "/sys/bus/i2c/devices/217-004c/hwmon", "temp1_input",
           "/sys/bus/i2c/devices/220-004c/hwmon", "temp1_input",
           "PRESENT_GROUP_DEVICE_NODE",
           "/sys/bus/i2c/devices/217-004c/hwmon", "present_status",
           "/sys/bus/i2c/devices/220-004c/hwmon", "present_status",
        ],

    'FAN_LED_OFF': ["0xFF"],
    'FAN_LED_PORT0_ALL_BLUE': ["0xAA"],
    'FAN_LED_PORT1_ALL_BLUE': ["0x55"],
    'FAN_LED_PORT0_ALL_RED': ["0x55"],
    'FAN_LED_PORT1_ALL_RED': ["0xAA"],
    'PORT0_FAN_LED_RED_MASK': ["0x01"],
    'PORT0_FAN_LED_BLUE_MASK': ["0x02"],
    'PORT1_FAN_LED_RED_MASK': ["0x02"],
    'PORT1_FAN_LED_BLUE_MASK': ["0x01"],
    
    'FAN_LED_LIMIT': ["3000"],
    'FAN_LED_AUX_LIMIT': ["-1"],

    'FAN_LED_I2C_BUS': ["/dev/i2c-9"],
    'FAN_LED_I2C_SLAVE_ADDRESS': [0x20],
}