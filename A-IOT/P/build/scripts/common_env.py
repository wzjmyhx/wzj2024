#!/usr/bin/env python3
# coding=utf-8

import os
import sys

from copy import deepcopy
from scons_utils import scons_usr_string_option
from scons_utils import scons_pre_check
from scons_utils import select_added_cfg
from scons_utils import scons_get_cfg_val
from scons_utils import condition_str
from scons_utils import scons_usr_bool_option
from scons_utils import scons_get_cfg_val
from scons_utils import traverse_subdir

#Module to be built by default.
compile_module = ['drv', 'sys', 'os', 'wpa', 'mbedtls', 'lwip', 'at']

#Get settings from menuconfig
product_soft_ver_str = scons_usr_string_option('CONFIG_TARGET_SOFT_VER')
log_output_flag = False

#library path for linking
os_lib_path = [
    os.path.join('build', 'scripts'),
    os.path.join('build', 'libs'),
]

#library path for boot linking
os_boot_path = [
    os.path.join('build', 'libs'),
]
"""
module_dir dictionary. Format as
module name : relative directory where the module is located
"""
module_dir = {
    'boot': os.path.join('boot', 'flashboot'),
    'loaderboot': os.path.join('boot', 'loaderboot'),
    'drv': os.path.join('platform', 'drivers'),
    'sys': os.path.join('platform', 'system'),
    'os': os.path.join('platform', 'os', 'Huawei_LiteOS'),
    'mqtt': os.path.join('third_party', 'paho.mqtt.c'),
    'mbedtls': os.path.join('third_party', 'mbedtls'),
    'coap': os.path.join('third_party', 'libcoap'),
    'histreaming': os.path.join('components', 'histreaming'),
    'at': os.path.join('components', 'at'),
    'iperf': os.path.join('components', 'iperf2'),
    'cjson': os.path.join('third_party', 'cjson'),
    'lwip': os.path.join('components', 'lwip_sack'),
    'wpa': os.path.join('platform', 'os', 'Huawei_LiteOS', 'net', 'wpa_supplicant'),
    'hilink': os.path.join('components', 'hilink'),
    'OHOS': os.path.join('components', 'OHOS'),
}

"""
proj_lib_cfg dictionary, contains the sub-folders which includes the source code.
Format as
    module name : {library name:["folder1", "folder2"]}
"""
proj_lib_cfg = {
    #flashboot
    'boot':{ #module name
        'boot': [ #library name
            'startup', #source code folders
            os.path.join('drivers', 'lsadc'),
            os.path.join('drivers', 'flash'),
            os.path.join('drivers', 'efuse'),
            os.path.join('drivers', 'gpio'),
            os.path.join('drivers', 'io'),
            os.path.join('common', 'nvm'),
            os.path.join('common', 'partition_table'),
            os.path.join('..', '..', 'third_party', 'u-boot-v2019.07', 'u-boot-v2019.07', 'lib'),
            os.path.join('..', 'commonboot', 'efuse'),
            os.path.join('..', 'commonboot', 'flash'),
            'lzmaram',
            'upg',
            'secure'
        ]
    },
    #loaderboot
    'loaderboot':{ #module name
        'loaderboot': [ #library name
            'startup', #source code folders
            'common',
            os.path.join('common', 'nvm'),
            os.path.join('common', 'partition_table'),
            os.path.join('drivers', 'lsadc'),
            os.path.join('drivers', 'efuse'),
            os.path.join('drivers', 'flash'),
            os.path.join('..', '..', 'third_party', 'u-boot-v2019.07', 'u-boot-v2019.07', 'lib'),
            os.path.join('..', 'commonboot', 'efuse'),
            os.path.join('..', 'commonboot', 'flash'),
            'secure'
        ]
    },
    #drivers
    'drv':{
        'flash' : [
            'flash'
        ],
        'uart' : [
            'uart'
        ],
        'adc' : [
            'adc'
        ],
        'tsensor' : [
            'tsensor'
        ],
    },
    #system module
    'sys':{
        'cfg' : [
            'cfg'
        ],
        'cpup' : [
            'cpup'
        ],
        'parttab' : [
            'partition_table'
        ],
        'upg' : [
            'upg'
        ],
    },
    #os modules
    'os':{
        'res_cfg': [
            os.path.join('kernel' , 'redirect')
        ]
    },
    #third parties and components
    'mbedtls':{'mbedtls':['library']},
    'mqtt':{'mqtt':['src']},
    'coap':{'coap':['src']},
    'histreaming':{'histreaming':['src']},
    'at':{'at':['src']},
    'iperf':{'iperf':['src']},
    'cjson': {
        'cjson':['cjson'],
        'cjson_utils':['cjson_utils'],
    },
    'lwip':{
        'lwip':[
            os.path.join('src', 'arch'),
            os.path.join('src', 'api'),
            os.path.join('src', 'core'),
            os.path.join('src', 'core', 'ipv4'),
            os.path.join('src', 'core', 'ipv6'),
            os.path.join('src', 'netif'),
            os.path.join('src', 'core', 'nat64'),
        ],
    },
    'wpa':{'wpa':['wpa_supplicant', 'src', 'hostapd', 'ltos_src']},
    'hilink':{'hilink':['.']},
    'OHOS':{
        'OHOS':[
            os.path.join('kv', 'src'),
            os.path.join('token', 'src'),
        ],
    },
}

"""
Base Settings.
defines:
    common:
        Modify common would take effect in each module building process.
        Insert global defines if needed.
    others, like coap, at:
        Insert module defines below, only affects the module compiling.

liteos_inc_path:
    Separate os header path to avoid the path too deep.
    common: global setting.

common_inc_path:
    include path contains the head files, MUST start with '#'.
    common: global setting.

"""
proj_environment = {
    'ar_flags':{
        'common':['cr']
    },

    'cc_flags':{
        'common':[]
    },

    'link_flags':{
        'common':['-nostartfiles', '-nostdlib', '-static', '--gc-sections']
    },

    'link_scripts_flag':{
        'common':[os.path.join('-Iplatform', 'os', 'Huawei_LiteOS', 'kernel', 'include'),'-Iconfig']
    },

    'as_flags':{
        'common':[]
    },

    'defines':{
        'common':[
            ('PRODUCT_USR_SOFT_VER_STR', r'\"%s\"'%product_soft_ver_str),
            'CYGPKG_POSIX_SIGNALS',
            '__ECOS__',
            '__RTOS_',
            'PRODUCT_CFG_HAVE_FEATURE_SYS_ERR_INFO',
            '__LITEOS__',
            'LIB_CONFIGURABLE',
            'LOSCFG_SHELL',
            'LOSCFG_CACHE_STATICS', # This option is used to control whether Cache hit ratio statistics are supported.
            'CUSTOM_AT_COMMAND',
            'LOS_COMPILE_LDM',
            'LOS_CONFIG_IPERF3',
            ('SECUREC_ENABLE_SCANF_FILE', 0),
        ],
        'at':['_PRE_WLAN_FEATURE_MFG_TEST'],
        'coap':['WITH_LWIP', 'MEMP_USE_CUSTOM_POOLS=1'],
        'histreaming':[],
        'cjson':[],
        'lwip':['WITH_LWIP', 'LWIP_NOASSERT', '__LITEOS__', 'LIB_CONFIGURABLE', ('LWIP_ENABLE_DIAG_CMD', '0'), 'LWIP_NETIF_DEFAULT_LINK_DOWN=1', 'LWIP_HI3861_THREAD_SLEEP=1',],

        'wpa':['INCLUDE_UNUSED', 'WLAN_HEADERS', 'WIRELESS_EXT=0', 'BT_SETUP=0', 'REGCODE_REMAPPING=0', 'SOFTAP_MODE',
            'SOFTMAC_FILE_USED', 'WPA_SUPPLICANT', 'HOSTAPD', 'CONFIG_NO_CONFIG_WRITE', 'CONFIG_WPA', 'CONFIG_SHA256',
            'OS_NO_C_LIB_DEFINES',
            'OPENSSL_DISABLE_OLD_DES_SUPPORT', 'THIRTY_TWO_BIT', 'OPENSSL_NO_SOCK', 'CONFIG_IEEE80211N', 'NEED_AP_MLME',
            'CONFIG_INTERNAL_LIBTOMMATH',
            'LTM_FAST', 'CONFIG_NO_RADIUS', 'CONFIG_NO_ACCOUNTING', 'CONFIG_NO_VLAN', 'CONFIG_NO_CONFIG_BLOBS',
            'CONFIG_CTRL_IFACE',
            'LOS_INLINE_FUNC_CROP', 'CONFIG_SAE_NO_FFC', 'CONFIG_SAE_NO_PW_ID', 'CONFIG_SAE_ONE_ECC_CURVE', 'CONFIG_SAE_CROP', 'IEEE8021X_EAPOL', 'CONFIG_TI_COMPILER', 'CONFIG_CRYPTO_INTERNAL', 'CONFIG_LITEOS_WPA',
            'LOS_WPA_EVENT_CALLBAK',
            'LOS_CONFIG_NO_VLAN', 'LOS_CONFIG_MESH_TRIM', 'LOS_CONFIG_HOSTAPD_QOS', 'HISI_WPA_MINI',
            'LOS_CONFIG_HOSTAPD_SECURITY',
            'LOS_CONFIG_HISI_DRIVER_NOT_SUPPORT', 'LOS_CONFIG_HOSTAPD_EAP_CIPHERS', 'HISI_WPA_KEY_MGMT_CROP',
            'LOS_CONFIG_HOSTAPD_TKIP_MIC',
            'LOS_CONFIG_HOSTAPD_RRM', 'HISI_EAP_TRIM', 'CONFIG_NO_HOSTAPD_LOGGER', 'LOS_CONFIG_80211_IES_CROP',
            'CONFIG_NO_RANDOM_POOL', 'CONFIG_NO_STDOUT_DEBUG', 'CONFIG_IEEE80211W', 'HISI_CODE_CROP',
            'CONFIG_NO_WPA_MSG', 'CONFIG_PRINT_NOUSE', 'HISI_SCAN_SIZE_CROP', 'CONFIG_SAE_NO_FFC',
            'CONFIG_DRIVER_HISILICON', 'LOS_WPA_PATCH', 'LOS_CONFIG_HOSTAPD_PMKSA', 'LOS_HOSTAPD_HT_CONFIG_CROP', 'LOS_HOSTAPD_CONFIG_CROP', 'LOS_CONFIG_ACL_CROP',
            'WITH_LWIP', 'TEST3516CV300',
            'CONFIG_STRERROR',
            ('LWIP_ENABLE_DIAG_CMD', '0'),
            '__LITEOS__',
        ],
    },

    'opts':{
        'common':['-mabi=ilp32', '-march=rv32imc', '-falign-functions=2', '-msave-restore',
            '-fno-optimize-strlen', '-freorder-blocks-algorithm=simple', '-fno-schedule-insns',
            '-fno-inline-small-functions',
            '-fno-inline-functions-called-once', '-Wa,-enable-c-lbu-sb', '-mtune=size', '-fno-strict-aliasing',
            '-msmall-data-limit=0', '-nostdinc', '-fno-aggressive-loop-optimizations', '-fno-builtin', '-std=c99',
            '-Os','-femit-lli', '-ffunction-sections', '-fdata-sections', '-fno-exceptions', '-fno-short-enums',
            '-fno-common', '-Wall', '-Wundef', '-fldm-stm-optimize', '-DLOS_COMPILE_LDM',
            '-fstack-protector-strong', '-freg-struct-return', '-fvisibility=hidden', '-pipe'
        ],

        'drv':['-Werror', '-fsigned-char'],

        'sys':['-Werror', '-fsigned-char'],
        'mqtt':['-funsigned-char'],
        'coap':['-fsigned-char'],
        'histreaming':[],
        'lwip':['-fsigned-char'],
        'wpa':['-funsigned-char'],
    },

    'liteos_inc_path':{
        'common':[
            os.path.join('kernel', 'base', 'include'),
            os.path.join('targets', 'hi3861v100', 'include'),
            os.path.join('targets', 'hi3861v100', 'plat', 'riscv'),
        ],
        'drv':[],

        'sys':[
            os.path.join('kernel', 'extended', 'include'),
        ],

        'os':[os.path.join('kernel', 'extended', 'include'),],

        'mqtt':[],

        'mbedtls':[],

        'coap':[],

        'histreaming':[],

        'at':[
            os.path.join('targets', 'hi3861v100', 'commons'),
        ],

        'wpa':[
            os.path.join('net', 'wpa_supplicant'),
            os.path.join('net', 'wpa_supplicant', 'src'),
            os.path.join('net', 'wpa_supplicant', 'src', 'utils'),
            os.path.join('net', 'wpa_supplicant', 'src', 'drivers'),
            os.path.join('net', 'wpa_supplicant', 'wpa_supplicant'),
            os.path.join('net', 'wpa_supplicant', 'src', 'wps'),
            os.path.join('net', 'wpa_supplicant', 'src', 'p2p'),
            os.path.join('net', 'wpa_supplicant', 'src', 'ap'),
            os.path.join('net', 'wpa_supplicant', 'src', 'eap_peer'),
            os.path.join('net', 'wpa_supplicant', 'src', 'eap_server'),
            os.path.join('net', 'wpa_supplicant', 'src', 'eap_common'),
            os.path.join('net', 'wpa_supplicant', 'src', 'common'),
            os.path.join('net', 'wpa_supplicant', 'hostapd'),
            os.path.join('net', 'wpa_supplicant', 'ltos_src'),
            os.path.join('net', 'wpa_supplicant', 'src', 'crypto'),
            os.path.join('net', 'wpa_supplicant', 'src', 'eapol_supp'),
            os.path.join('net', 'wpa_supplicant', 'src', 'l2_packet'),
            os.path.join('net', 'wpa_supplicant', 'src', 'rsn_supp'),
            os.path.join('net', 'wpa_supplicant', 'src', 'tls'),
        ],
    },

    'common_inc_path':{
        'common':[
            os.path.join('#', 'include', 'base'),
            os.path.join('#', 'platform', 'include'),
            os.path.join('#', 'config'),
            os.path.join('#', 'config', 'nv'),
        ],
        'drv':[os.path.join('#', 'platform', 'drivers', 'dma'),
               os.path.join('#', 'platform', 'drivers', 'flash'),
        ],

        'sys':[
            os.path.join('#', 'platform', 'system', 'include'),
            os.path.join('#', 'platform', 'system', 'upg'),
            os.path.join('#', 'platform', 'drivers', 'flash'),
        ],
        'os':[
        ],

        'mqtt':[os.path.join('#', 'third_party', 'paho.mqtt.c', 'src'),
            os.path.join('#', 'third_party', 'mbedtls', 'include'),
            os.path.join('#', 'platform', 'os', 'Huawei_LiteOS', 'kernel', 'include'),
        ],

        'mbedtls':[
            os.path.join('#', 'third_party', 'mbedtls', 'include'),
            os.path.join('#', 'third_party', 'mbedtls', 'include', 'mbedtls'),
            os.path.join('#', 'platform', 'drivers', 'cipher'),
            os.path.join('#', 'components', 'lwip_sack', 'include'),
        ],

        'at':[
            os.path.join('#', 'platform', 'drivers', 'uart'),
            os.path.join('#', 'platform', 'system', 'cpup'),
            os.path.join('#', 'platform', 'at'),
            os.path.join('#', 'components', 'at', 'src'),
            os.path.join('#', 'components', 'wifi', 'include'),
            os.path.join('#', 'components', 'lwip_sack', 'include', 'lwip'),
            os.path.join('#', 'components', 'iperf2', 'include'),
            os.path.join('#', 'config', 'diag'),
        ],

        'coap':[
            os.path.join('#', 'third_party', 'libcoap'),
            os.path.join('#', 'third_party', 'libcoap', 'include', 'coap2'),
        ],
        'histreaming':[
            os.path.join('#', 'components', 'histreaming', 'src')
        ],
        'cjson':[os.path.join('#', 'third_party', 'cjson'),],

        'lwip': [
            os.path.join('#', 'components', 'mcast6', 'include'),
            os.path.join('#', 'components', 'ripple', 'include'),
            os.path.join('#', 'components', 'ripple', 'exports'),
            os.path.join('#', 'components', 'ripple', 'src', 'platform', 'lwip'),
            os.path.join('#', 'third_party', 'libcoap', 'include', 'coap2'),
            os.path.join('#', 'third_party', 'libcoap'),
        ],
        'wpa':[
            os.path.join('#', 'third_party', 'mbedtls', 'include'),
            os.path.join('#', 'platform', 'drivers', 'cipher'),
            os.path.join('#', 'components', 'wifi', 'include'),
            os.path.join('#', 'components', 'lwip_sack', 'include'),
        ],
        'hilink':[
            os.path.join('#', 'components', 'hilink', 'include'),
        ],
        'OHOS':[
            os.path.join('#', 'components', 'OHOS', 'kv', 'src'),
            os.path.join('#', 'components', 'OHOS', 'token', 'src'),
            os.path.join('#', 'include', 'OHOS'),
        ],
    },
} # End of proj_environment

# env for makefile
common_str = ''
module_str = {}
fixed_config = {'module':deepcopy(compile_module), 'lib_cfg':deepcopy(proj_lib_cfg), 'env_cfg':deepcopy(proj_environment)}

def set_str_2make(type, macro, macro_val, macro_cfg, env_type = None, env_mod_type = None, depends = None):
    type_list = ['module', 'lib_cfg', 'env_cfg']
    global common_str
    global module_str
    if type not in type_list:
        print('[ERROR] type err')
        sys.exit(1)
    elif type == 'module':
        strs = 'ifeq ($(%s), %s)\n'%(macro, macro_val)
        strs = '%s\tCOMPILE_MODULE += %s\n'%(strs, macro_cfg)
        #strs = '%s\tLIBS += $(addprefix -l, $(%s_libs))\n'%(strs, macro_cfg)
        strs = '%sendif\n'%strs
        common_str = '%s%s'%(common_str, strs)
    elif type == 'lib_cfg':
        strs = 'ifeq ($(%s), %s)\n'%(macro, macro_val)
        for mod in macro_cfg:
            libs = macro_cfg[mod]
            str_2_mod = 'ifeq ($(%s), %s)\n'%(macro, macro_val)
            for lib in libs:
                src = macro_cfg[mod][lib]
                str_2_mod = '%s\t%s_srcs += %s\n'%(str_2_mod, lib, ' '.join(src))
            str_2_mod = '%sendif\n'%(str_2_mod)
            if mod not in module_str:
                module_str[mod] = str_2_mod
            else:
                org_str = module_str[mod]
                module_str[mod] = '%s%s'%(org_str, str_2_mod)
            strs = '%s\t%s_libs += %s\n'%(strs, mod, lib)
        strs = '%sendif\n'%strs
        common_str = '%s%s'%(common_str, strs)
    elif type == 'env_cfg':
        if env_mod_type == 'common':
            strs = condition_str(macro, macro_val, macro_cfg, env_type, depends)
            common_str = '%s%s'%(common_str, strs)
        elif env_mod_type in module_str:
            strs = condition_str(macro, macro_val, macro_cfg, env_type, depends, True)
            org_str = module_str[env_mod_type]
            module_str[env_mod_type] = '%s%s'%(org_str, strs)
        else:
            module_str[env_mod_type] = condition_str(macro, macro_val, macro_cfg, env_type, depends, True)

def set_config(type, macro, macro_val, macro_cfg, env_type = None, env_mod_type = None, depends = None):
    type_list = ['module', 'lib_cfg', 'env_cfg']
    if type not in type_list:
        print('[ERROR] type err')
        sys.exit(1)
    elif type == 'module':
        if scons_get_cfg_val(macro) == macro_val:
            compile_module.append(macro_cfg)
    elif type == 'lib_cfg':
        if scons_get_cfg_val(macro) == macro_val:
            for mod in macro_cfg:
                libs = macro_cfg[mod]
                for lib in libs:
                    src = macro_cfg[mod][lib]
                    if lib not in proj_lib_cfg[mod]:
                        proj_lib_cfg[mod][lib] = src
                    else:
                        proj_lib_cfg[mod][lib].extend(src)
    elif type == 'env_cfg':
        cfg = select_added_cfg(macro, macro_val, macro_cfg, depends)
        if cfg is not None:
            proj_environment[env_type][env_mod_type].extend(cfg)

    set_str_2make(type, macro, macro_val, macro_cfg, env_type, env_mod_type, depends)

# Configrations
set_config('module', 'CONFIG_MQTT', 'y', 'mqtt')
set_config('module', 'CONFIG_CJSON', 'y', 'cjson')
set_config('module', 'CONFIG_LIBCOAP', 'y', 'coap')
set_config('module', 'CONFIG_IPERF', 'y', 'iperf')

set_config('lib_cfg', 'CONFIG_I2C_SUPPORT', 'y', {'drv': {'i2c': ['i2c']}})
set_config('lib_cfg', 'CONFIG_SPI_SUPPORT', 'y', {'drv': {'spi': ['spi']}})
set_config('lib_cfg', 'CONFIG_PWM_SUPPORT', 'y', {'drv': {'pwm': ['pwm']}})

set_config('env_cfg', 'CONFIG_I2C_SUPPORT', 'y', ['CONFIG_I2C_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_DMA_SUPPORT', 'y', ['CONFIG_DMA_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_SPI_SUPPORT', 'y', ['CONFIG_SPI_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_PWM_SUPPORT', 'y', ['CONFIG_PWM_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_PWM_HOLD_AFTER_REBOOT', 'y', ['CONFIG_PWM_HOLD_AFTER_REBOOT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_I2S_SUPPORT', 'y', ['CONFIG_I2S_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_COMPRESSION_OTA_SUPPORT', 'y', ['CONFIG_COMPRESSION_OTA_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_DUAL_PARTITION_OTA_SUPPORT', 'y', ['CONFIG_DUAL_PARTITION_OTA_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_AT_SUPPORT', 'y', ['CONFIG_AT_COMMAND'], 'defines', 'common')
set_config('env_cfg', 'LOSCFG_DAQ', 'y', ['FEATURE_DAQ'], 'defines', 'common')
set_config('env_cfg', 'LOSCFG_BACKTRACE', 'y', ['LOS_BACKTRACE'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_UART0_SUPPORT', 'y', ['CONFIG_UART0_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_UART1_SUPPORT', 'y', ['CONFIG_UART1_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_UART2_SUPPORT', 'y', ['CONFIG_UART2_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_MQTT', 'y', ['MBEDTLS', 'HIGH_PERFORMANCE', 'LOSCFG_CONFIG_MQTT', ('LOSCFG_NET_MQTT', 'y')], 'defines', 'common')

set_config('env_cfg', 'CONFIG_LINK_NULL', 'y', ['CONFIG_LINK_NULL'], 'defines', 'common')
set_config('module', 'CONFIG_HISTREAMING_SUPPORT', 'y', 'histreaming')
set_config('env_cfg', 'CONFIG_HISTREAMING_SUPPORT', 'y', ['CONFIG_HISTREAMING_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_COLORFUL_LIGHT', 'y', ['CONFIG_COLORFUL_LIGHT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_TRAFFIC_LIGHT', 'y', ['CONFIG_TRAFFIC_LIGHT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_ENVIRONMENTAL_MONITORING', 'y', ['CONFIG_ENVIRONMENTAL_MONITORING'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_ENVIRONMENTAL_MONITORING', 'y', ['CONFIG_ENVIRONMENTAL_MONITORING'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_NFC', 'y', ['CONFIG_NFC'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_ALL_DEMO', 'y', ['CONFIG_ALL_DEMO'], 'defines', 'common')

set_config('env_cfg', 'CONFIG_TARGET_CHIP_HI3861', 'y', [('PRODUCT_CFG_CHIP_VER_STR', r'\"Hi3861V100\"'), 'CHIP_VER_Hi3861', ('CONFIG_CHIP_PRODUCT_NAME', r'\"Hi3861\"')], 'defines', 'common')
set_config('env_cfg', 'CONFIG_TARGET_CHIP_HI3861L', 'y', [('PRODUCT_CFG_CHIP_VER_STR', r'\"Hi3861LV100\"'), 'CHIP_VER_Hi3861L', ('CONFIG_CHIP_PRODUCT_NAME', r'\"Hi3861L\"')], 'defines', 'common')
set_config('env_cfg', 'CONFIG_CHIP_PKT_48K', 'y', ['CONFIG_CHIP_PKT_48K'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_CHIP_PKT_32K', 'y', ['CONFIG_CHIP_PKT_32K'], 'defines', 'common')

set_config('env_cfg', 'CONFIG_QUICK_SEND_MODE', 'y', ['CONFIG_QUICK_SEND_MODE'], 'defines', 'common')

macro = 'LOSCFG_COMPILER_HI3861_ASIC'
macro_cfg = {
            'y': ['HI_BOARD_ASIC'],
            'others': ['HI_BOARD_FPGA']}
macro_val = list(macro_cfg.keys())
set_config('env_cfg', macro, macro_val, macro_cfg, 'defines', 'common')

macro = 'LOSCFG_COMPILER_HI3861_FLASH'
macro_cfg = {
            'y': ['HI_ON_FLASH'],
            'others': ['HI_ON_RAM']}
macro_val = list(macro_cfg.keys())
set_config('env_cfg', macro, macro_val, macro_cfg, 'defines', 'common')

macro = 'CONFIG_MESH_SUPPORT'
macro_cfg = {
            'y': ['CONFIG_MESH_SUPPORT', 'LOS_CONFIG_MESH', 'LOS_CONFIG_MESH_GTK', 'CONFIG_MESH', 'CONFIG_SAE', 'CONFIG_ECC', 'LOS_CONFIG_HOSTAPD_MGMT', 'LOSCFG_APP_MESH', 'LWIP_DEBUG_OPEN', 'LWIP_SMALL_SIZE_MESH=1'],
            'others': ['LWIP_SMALL_SIZE_MESH=0']}
macro_val = list(macro_cfg.keys())
set_config('env_cfg', macro, macro_val, macro_cfg, 'defines', 'common')

macro = 'CONFIG_WPA3_SUPPORT'
macro_cfg = {
            'y': ['CONFIG_SAE', 'CONFIG_ECC', 'CONFIG_WPA3', 'CONFIG_SME', 'LTOS_CONFIG_NO_INTERNAL_SME'],
            'others': ['LOS_CONFIG_NO_PMKSA']}
macro_val = list(macro_cfg.keys())
set_config('env_cfg', macro, macro_val, macro_cfg, 'defines', 'common')

set_config('env_cfg', 'CONFIG_LWIP_LOWPOWER', 'y', ['CONFIG_LWIP_LOWPOWER'], 'defines', 'lwip')
set_config('env_cfg', 'LOSCFG_NET_MQTT', 'y', ['LOSCFG_CONFIG_MQTT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_LWIP_SMALL_SIZE', 'y', ['CONFIG_LWIP_SMALL_SIZE'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_LWIP_SMALL_SIZE_MESH', 'y', ['CONFIG_LWIP_SMALL_SIZE_MESH'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_NETIF_HOSTNAME', 'y', ['CONFIG_NETIF_HOSTNAME'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_DHCP_VENDOR_CLASS_IDENTIFIER', 'y', ['CONFIG_DHCP_VENDOR_CLASS_IDENTIFIER'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_DHCPS_GW', 'y', ['CONFIG_DHCPS_GW'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_UART_DMA_SUPPORT', 'y', ['CONFIG_UART_DMA_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_SDIO_SUPPORT', 'y', ['CONFIG_SDIO_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_SPI_DMA_SUPPORT', 'y', ['CONFIG_SPI_DMA_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_TEE_HUKS_SUPPORT', 'y', ['CONFIG_TEE_HUKS_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_TEE_HUKS_DEMO_SUPPORT', 'y', ['CONFIG_TEE_HUKS_DEMO_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_FLASH_ENCRYPT_SUPPORT', 'y', ['CONFIG_FLASH_ENCRYPT_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_WPS_SUPPORT', 'y', ['CONFIG_WPS_SUPPORT', 'CONFIG_WPS', 'EAP_WSC'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_FILE_SYSTEM_SUPPORT', 'y', ['CONFIG_FILE_SYSTEM_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'CONFIG_DIAG_SUPPORT', 'y', ['CONFIG_DIAG_SUPPORT'], 'defines', 'common')
set_config('env_cfg', 'LOSCFG_KASAN', 'y', ['LOSCFG_DEBUG_KASAN'], 'defines', 'common', depends=('LOSCFG_KASAN_EXAMPLES_DEMO', 'y'))
set_config('env_cfg', 'LOSCFG_KASAN', 'y', ['LOSCFG_DEBUG_KASAN'], 'defines', 'common', depends=('LOSCFG_KASAN_LITEOS_NET_COAP', 'y'))


set_config('env_cfg', 'CONFIG_CJSON', 'y', ['CJSON_NESTING_LIMIT=$(CONFIG_CJSON_NESTING_LIMIT)',], 'defines', 'cjson')
set_config('env_cfg', 'CONFIG_LIBCOAP', 'y', ['LOSCFG_NET_LIBCOAP'], 'defines', 'coap')
set_config('env_cfg', 'LOSCFG_TEST_NET', 'y', ['LWIP_TESTBED'], 'defines', 'lwip')
set_config('env_cfg', 'LOSCFG_NET_LIBCOAP', 'y', [('MEMP_USE_CUSTOM_POOLS', 1), ('LWIP_LIBCOAP', 1)], 'defines', 'lwip')
set_config('env_cfg', 'CONFIG_IPERF', 'y', ['CONFIG_IPERF_SUPPORT'], 'defines', 'at')



set_config('env_cfg', 'LOSCFG_BACKTRACE', 'y', ['-fno-omit-frame-pointer'], 'as_flags', 'common')


set_config('env_cfg', 'HB_LITEOS_COMPILE_TESTCASE', 'y', ['-DHI1131TEST'], 'link_scripts_flag', 'common')
set_config('env_cfg', 'LOSCFG_KASAN', 'y', ['-DLOSCFG_DEBUG_KASAN'], 'link_scripts_flag', 'common')
set_config('env_cfg', 'CONFIG_FLASH_ENCRYPT_SUPPORT', 'y', ['-DCONFIG_FLASH_ENCRYPT_SUPPORT'], 'link_scripts_flag', 'common')
set_config('env_cfg', 'CONFIG_TEE_HUKS_SUPPORT', 'y', ['-DCONFIG_TEE_HUKS_SUPPORT'], 'link_scripts_flag', 'common')
set_config('env_cfg', 'CONFIG_TARGET_SIG_RSA_V15', 'y', ['-DCONFIG_TARGET_SIG_RSA_V15'], 'link_scripts_flag', 'common')
set_config('env_cfg', 'CONFIG_TARGET_SIG_RSA_PSS', 'y', ['-DCONFIG_TARGET_SIG_RSA_PSS'], 'link_scripts_flag', 'common')
set_config('env_cfg', 'CONFIG_TARGET_SIG_ECC', 'y', ['-DCONFIG_TARGET_SIG_ECC'], 'link_scripts_flag', 'common')
set_config('env_cfg', 'CONFIG_CHIP_PKT_48K', 'y', ['-DCONFIG_CHIP_PKT_48K'], 'link_scripts_flag', 'common')
set_config('env_cfg', 'CONFIG_CHIP_PKT_32K', 'y', ['-DCONFIG_CHIP_PKT_32K'], 'link_scripts_flag', 'common')

macro = 'LOSCFG_COMPILER_HI3861_ASIC'
macro_cfg = {
            'y': ['-DHI_BOARD_ASIC'],
            'others': ['-DHI_BOARD_FPGA']}
macro_val = list(macro_cfg.keys())
set_config('env_cfg', macro, macro_val, macro_cfg, 'link_scripts_flag', 'common')

macro = 'LOSCFG_COMPILER_HI3861_FLASH'
macro_cfg = {
            'y': ['-DHI_ON_FLASH'],
            'others': ['-DHI_ON_RAM']}
macro_val = list(macro_cfg.keys())
set_config('env_cfg', macro, macro_val, macro_cfg, 'link_scripts_flag', 'common')


set_config('env_cfg', 'LOSCFG_KERNEL_LITEKERNEL', 'y', [os.path.join('kernel', 'include')], 'liteos_inc_path', 'common')
set_config('env_cfg', 'LOSCFG_KERNEL_RUNSTOP', 'y', [os.path.join('kernel', 'extended', 'runstop'),], 'liteos_inc_path', 'common')
set_config('env_cfg', 'LOSCFG_COMPAT_POSIX', 'y', [os.path.join('components', 'posix', 'include')], 'liteos_inc_path', 'common')
set_config('env_cfg', 'LOSCFG_COMPAT_LINUX', 'y', [os.path.join('components', 'linux', 'include')], 'liteos_inc_path', 'common')
set_config('env_cfg', 'LOSCFG_SHELL', 'y', [os.path.join('shell', 'include')], 'liteos_inc_path', 'common')
set_config('env_cfg', 'LOSCFG_NET_TELNET', 'y', [os.path.join('net', 'telnet', 'include')], 'liteos_inc_path', 'common')
set_config('env_cfg', 'LOSCFG_LIB_LIBC', 'y',
            [os.path.join('components', 'lib', 'libc', 'hw', 'include'),
            os.path.join('components', 'lib', 'libc', 'musl', 'include'),
            os.path.join('components', 'lib', 'libc', 'musl', 'arch', 'generic'),
            os.path.join('components', 'lib', 'libc', 'musl', 'arch', 'riscv32'),
            os.path.join('components', 'lib', 'libc', 'nuttx', 'include'),
            os.path.join('components', 'lib', 'libsec', 'include'),
            os.path.join('targets', 'hi3861v100', 'config'),
            os.path.join('targets', 'hi3861v100', 'user'),
            os.path.join('targets', 'hi3861v100', 'plat'),
            os.path.join('targets', 'hi3861v100', 'extend', 'include'),
            os.path.join('arch'),
            ], 'liteos_inc_path', 'common')

set_config('env_cfg', 'CONFIG_CJSON', 'y', [os.path.join('#', 'third_party', 'cjson')], 'common_inc_path', 'common')
set_config('env_cfg', 'LOSCFG_NET_LWIP_SACK', 'y', [os.path.join('#', 'components', 'lwip_sack', 'include'),], 'common_inc_path', 'common')
set_config('env_cfg', 'CONFIG_MQTT', 'y', [os.path.join('#', 'third_party', 'paho.mqtt.c', 'src'),], 'common_inc_path', 'common')
set_config('env_cfg', 'CONFIG_LIBCOAP', 'y', [os.path.join('#', 'third_party', 'libcoap', 'include', 'coap2')], 'common_inc_path', 'common')

set_config('env_cfg', 'LOSCFG_BACKTRACE', 'y', ['-fno-omit-frame-pointer'], 'opts', 'common')
set_config('env_cfg', 'LOSCFG_KASAN', 'y', ['-fsanitize=kernel-address', '-fasan-shadow-offset=1835008', '--param asan-stack=1', '-fsanitize=bounds-strict'], 'opts', 'common', depends=('LOSCFG_KASAN_EXAMPLES_DEMO', 'y'))
set_config('env_cfg', 'LOSCFG_KASAN', 'y', ['-fsanitize=kernel-address', '-fasan-shadow-offset=1835008', '--param asan-stack=1', '-fsanitize=bounds-strict'], 'opts', 'common', depends=('LOSCFG_KASAN_LITEOS_NET_COAP', 'y'))

# -------------------------------hilink support-------------------------------
if scons_usr_bool_option('CONFIG_HILINK') == 'y':
    os_lib_path.append(os.path.join('components', 'hilink', 'lib'))
set_config('module', 'CONFIG_HILINK', 'y', 'hilink')
set_config('env_cfg', 'CONFIG_HILINK', 'y', ['CONFIG_HILINK'], 'defines', 'common')

# -------------------------------histreaming support-------------------------------
if scons_usr_bool_option('CONFIG_HISTREAMING_SUPPORT') == 'y':
    os_lib_path.append(os.path.join('components', 'histreaming', 'lib'))
set_config('module', 'CONFIG_HISTREAMING_SUPPORT', 'y', 'histreaming')
set_config('env_cfg', 'CONFIG_HISTREAMING_SUPPORT', 'y', ['CONFIG_HISTREAMING_SUPPORT'], 'defines', 'common')

# -------------------------------OHOS support-------------------------------
if scons_get_cfg_val('CONFIG_OHOS') == 'y':
    os_lib_path.append(os.path.join('components', 'OHOS', 'ndk', 'libs'))
set_config('module', 'CONFIG_OHOS', 'y', 'OHOS')
set_config('env_cfg', 'CONFIG_OHOS', 'y', ['CONFIG_OHOS', ('HOS_PRODUCT_TYPE', r'\"\"'), ('HOS_SERIAL', r'\"\"'), ('HOS_MANUFACTURE', r'\"\"'),
    ('HOS_BRAND', r'\"\"'), ('HOS_MARKET_NAME', r'\"\"'), ('HOS_PRODUCT_SERIES', r'\"\"'), ('HOS_PRODUCT_MODEL', r'\"\"'), ('HOS_SOFTWARE_MODEL', r'\"\"'),
    ('HOS_HARDWARE_MODEL', r'\"\"'), ('HOS_HARDWARE_PROFILE', r'\"\"'), ('HOS_BOOTLOADER_VERSION', r'\"\"'),
    ('HOS_SECURITY_PATCH_TAG', r'\"2020-12-01\"'), ('HOS_ABI_LIST', r'\"\"'), ('HOS_INCREMENTAL_VERSION', r'\"HarmonyOS_2.0.1.36\"'),
    ('HOS_BUILD_TYPE', r'\"release\"'), ('HOS_BUILD_USER', r'\"jenkins\"'), ('HOS_BUILD_TIME', r'\"2020-11-30_19:17:39\"'),
    ('HOS_BUILD_HOST', r'\"linux\"'), ('HOS_BUILD_ROOTHASH', r'\"\"')
    ], 'defines', 'common')

def get_fixed_config(type = 'all'):
    if type == 'all':
        return fixed_config
    elif type in ['module', 'lib_cfg', 'env_cfg']:
        return fixed_config[type]
    else:
        print('[ERROR] type err!')

def get_make_str(module = 'common'):
    if module == 'common':
        return common_str
    elif module in module_str:
        return module_str[module]
    else:
        print('[INFO] %s not exists module config for makefile!'%module)

def get_str_dict():
    return (common_str, module_str)

def get_proj_env():
    return proj_environment

def get_proj_env_value(env_name, module):
    if module in proj_environment[env_name].keys():
        return proj_environment[env_name].get(module)
    else:
        return None

def set_proj_env_value_by_macro(env_name, module, env_value, macro='', macro_value=''):
    pass

def print_log_env(env, cwd_path):
    if log_output_flag is False:
        return
    print('----------------------------enter real:', os.path.realpath(str(cwd_path)))
    print('CPPPATH:', env['CPPPATH'])
    print('DEFINES:', env['CPPDEFINES'])
    print('CC:', env['CC'])

def insert_module(module):
    compile_module.append(module)

def insert_module_dir(name, path):
    module_dir[name] = path

def insert_lib_cfg(name, cfg):
    proj_lib_cfg[name] = cfg

def insert_env_defs(name, defs):
    proj_environment['defines'][name] = defs

def insert_os_include(name, includes):
    proj_environment['liteos_inc_path'][name] = includes

def insert_common_include(name, includes):
    proj_environment['common_inc_path'][name] = includes

def insert_ar_flags(name, flags):
    proj_environment['ar_flags']['common'].extend(flags)

def insert_as_flags(name, flags):
    proj_environment['as_flags']['common'].extend(flags)

def insert_ld_flags(name, flags):
    proj_environment['link_flags']['common'].extend(flags)

def insert_cc_flags(name, flags):
    proj_environment['opts'][name] = flags

def set_factory_mode():
    proj_environment['defines']['common'].append('CONFIG_FACTORY_TEST_MODE')
