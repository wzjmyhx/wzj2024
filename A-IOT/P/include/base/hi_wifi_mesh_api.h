/**
 * @file hi_wifi_mesh_api.h
 *
 * Copyright (c) 2020 HiSilicon (Shanghai) Technologies CO., LIMITED.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @defgroup hi_wifi_mesh WiFi Mesh Settings
 * @ingroup hi_wifi
 */

#ifndef __HI_WIFI_MESH_API_H__
#define __HI_WIFI_MESH_API_H__

#include "hi_wifi_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @ingroup hi_wifi_mesh
 *
 * max auth type length.CNcomment:�û��������֤��ʽ��󳤶�CNend
 */
#define WPA_MAX_AUTH_TYPE_INPUT_LEN     32

/**
 * @ingroup hi_wifi_mesh
 *
 * max mesh key length.CNcomment:�û������mesh��Կ��󳤶�CNend
 */
#define HI_WIFI_MESH_KEY_LEN_MAX 63

/**
 * @ingroup hi_wifi_basic
 * Struct of scan result.CNcomment:ɨ�����ṹ��CNend
 */
typedef struct {
    char ssid[HI_WIFI_MAX_SSID_LEN + 1];    /**< SSID ֻ֧��ASCII�ַ� */
    unsigned char bssid[HI_WIFI_MAC_LEN];   /**< BSSID */
    unsigned int channel;                   /**< �ŵ��� */
    hi_wifi_auth_mode auth;                 /**< ��֤���� */
    int rssi;                               /**< �ź�ǿ�� */
    unsigned char resv : 4;                 /**< Reserved */
    unsigned char hisi_mesh_flag : 1;       /**< HI MESH��־ */
    unsigned char is_mbr : 1;               /**< �Ƿ���MBR��־ */
    unsigned char accept_for_sta : 1;       /**< �Ƿ�����STA���� */
    unsigned char accept_for_peer : 1;      /**< �Ƿ�����Mesh AP���� */
    unsigned char bcn_prio;                 /**< BCN���ȼ� */
    unsigned char peering_num;              /**< �Զ����ӵ���Ŀ */
    unsigned short mesh_rank;               /**< mesh rankֵ */
    unsigned char switch_status;            /**< ���mesh ap�Ƿ����ŵ��л�״̬ */
} hi_wifi_mesh_scan_result_info;

/**
 * @ingroup hi_wifi_mesh
 *
 * Struct of connected mesh.CNcomment:�����ӵ�peer�ṹ�塣CNend
 *
 */
typedef struct {
    unsigned char mac[HI_WIFI_MAC_LEN];       /**< �Զ�mac��ַ */
    unsigned char mesh_bcn_priority;          /**< BCN���ȼ� */
    unsigned char mesh_is_mbr : 1;            /**< �Ƿ���MBR */
    unsigned char mesh_block : 1;             /**< block�Ƿ���λ */
    unsigned char mesh_role : 1;              /**< mesh�Ľ�ɫ */
} hi_wifi_mesh_peer_info;

/**
 * @ingroup hi_wifi_mesh
 *
 * Struct of mesh's config.CNcomment:mesh���ò���CNend
 *
 */
typedef struct {
    char ssid[HI_WIFI_MAX_SSID_LEN + 1];     /**< SSID ֻ֧��ASCII�ַ� */
    char key[HI_WIFI_AP_KEY_LEN + 1];        /**< ���� */
    hi_wifi_auth_mode auth;                  /**< ��֤���ͣ�ֻ֧��HI_WIFI_SECURITY_OPEN��HI_WIFI_SECURITY_SAE */
    unsigned char channel;                   /**< �ŵ��� */
} hi_wifi_mesh_config;

/**
 * @ingroup hi_wifi_mesh
 *
 * Status type.CNcomment:mesh�ڵ�״̬����CNend
 *
 */
typedef enum {
    MAC_HISI_MESH_UNSPEC = 0, /* δȷ��mesh�ڵ��ɫ */
    MAC_HISI_MESH_STA,        /* Mesh-STA�ڵ��ɫ */
    MAC_HISI_MESH_MG,         /* Mesh-MG�ڵ��ɫ */
    MAC_HISI_MESH_MBR,        /* Mesh-MBR�ڵ��ɫ */

    MAC_HISI_MESH_NODE_BUTT,
} mac_hisi_mesh_node_type_enum;
typedef unsigned char mac_hisi_mesh_node_type_enum_uint8;

/**
 * @ingroup hi_wifi_mesh
 *
 * Struct of node's config.CNcomment:node���ò���CNend
 *
 */
typedef struct _mac_cfg_mesh_nodeinfo_stru {
    mac_hisi_mesh_node_type_enum_uint8 node_type;           /* ���ڵ��ɫ */
    unsigned char mesh_accept_sta;                          /* �Ƿ����sta���� */
    unsigned char user_num;                                 /* �����û��� */
    unsigned char privacy;                                  /* �Ƿ���� */
    unsigned char chan;                                     /* �ŵ��� */
    unsigned char priority;                                 /* bcn���ȼ� */
    unsigned char rsv[2];                                   /* 2 byte���� */
}mac_cfg_mesh_nodeinfo_stru;

/**
* @ingroup  hi_wifi_mesh
* @brief  Mesh disconnect peer by mac address.CNcomment:meshָ���Ͽ����ӵ����硣CNend
*
* @par Description:
*          Mesh disconnect peer by mac address.CNcomment:softapָ���Ͽ����ӵ����硣CNend
*
* @attention  NULL
* @param  addr             [IN]     Type  #const unsigned char *, peer mac address.CNcomment:�Զ�MAC��ַ��CNend
* @param  addr_len         [IN]     Type  #unsigned char, peer mac address length.CNcomment:�Զ�MAC��ַ���ȡ�CNend
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency:
*            @li hi_wifi_mesh_api.h: WiFi API
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_wifi_mesh_disconnect(const unsigned char *addr, unsigned char addr_len);

/**
* @ingroup  hi_wifi_mesh
* @brief  Start mesh interface.CNcomment:����mesh��CNend
*
* @par Description:
*           Add mesh interface.CNcomment:����mesh��CNend
*
* @attention  1. The memories of <ifname> and <len> should be requested by the caller��
*                the input value of len must be the same as the length of ifname��the recommended length is 17Bytes��.\n
*                CNcomment:1. <ifname>��<len>�ɵ����������ڴ棬�û�д��len��ֵ������ifname����һ�£����鳤��Ϊ17Bytes��.CNend \n
*             2. SSID only supports ASCII characters.
*                CNcomment:2. SSID ֻ֧��ASCII�ַ�.CNend \n
*             3. This is a blocking function.CNcomment:3���˺���Ϊ��������.CNend
* @param config    [IN]     Type  #hi_wifi_mesh_config * mesh's configuration.CNcomment:mesh���á�CNend \n
* @param ifname    [IN/OUT] Type  #char * mesh interface name.CNcomment:������mesh�ӿ����ơ�CNend \n
* @param len       [IN/OUT] Type  #int * mesh interface name length.CNcomment:������mesh�ӿ����Ƶĳ��ȡ�CNend \n
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency:
*            @li hi_wifi_mesh_api.h: WiFi API
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_wifi_mesh_start(hi_wifi_mesh_config *config, char *ifname, int *len);

/**
* @ingroup  hi_wifi_mesh
* @brief  Connect to mesh device by mac address.CNcomment:ͨ���Զ�mac��ַ����mesh��CNend
*
* @par Description:
*           Connect to mesh device by mac address.CNcomment:ͨ���Զ�mac��ַ����mesh��CNend
*
* @attention  NULL
* @param  mac             [IN]    Type  #const unsigned char * peer mac address.CNcomment:�Զ�mesh�ڵ��mac��ַ��CNend
* @param  len             [IN]    Type  #const int   the len of mac address.CNcomment:mac��ַ�ĳ��ȡ�CNend
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency:
*            @li hi_wifi_mesh_api.h: WiFi-MESH API
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_wifi_mesh_connect(const unsigned char *mac, const int len);

/**
* @ingroup  hi_wifi_mesh
* @brief  Set mesh support/not support mesh peer connections.CNcomment:����mesh֧��/��֧��mesh peer���ӡ�CNend
*
* @par Description:
*           Set mesh support/not support mesh peer connections.CNcomment:����mesh֧��/��֧��mesh peer���ӡ�CNend
*
* @attention  1. Default support peer connect.CNcomment:1. Ĭ��֧��mesh peer���ӡ�CNend \n
*             2. The enable_peer_connect value can only be 1 or 0. CNcomment:2. enable_peer_connectֵֻ��Ϊ1��0��CNend
* @param  enable_accept_peer    [IN]    Type  #unsigned char flag to support mesh connection.
*                                             CNcomment:�Ƿ�֧��mesh���ӵı�־��CNend
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency:
*            @li hi_wifi_mesh_api.h: WiFi-MESH API
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_wifi_mesh_set_accept_peer(unsigned char enable_peer_connect);

/**
* @ingroup  hi_wifi_mesh
* @brief  Set mesh support/not support mesh sta connections.CNcomment:����mesh֧��/��֧��mesh sta���ӡ�CNend
*
* @par Description:
*           Set mesh support/not support mesh sta connections.CNcomment:����mesh֧��/��֧��mesh sta���ӡ�CNend
*
* @attention 1. Default not support sta connect. CNcomment:1. Ĭ�ϲ�֧��mesh sta���ӡ�CNend \n
*            2. The enable_sta_connect value can only be 1 or 0. CNcomment:2. enable_sta_connectֵֻ��Ϊ1��0��CNend \n
             3. This value can only be modified after the node joins the Mesh network.
                CNcomment: 3. ֻ�нڵ����Mesh�����ſ����޸ĸ�ֵ�� CNend
* @param  enable_accept_sta    [IN]    Type  #unsigned char flag to support mesh sta connection.
*                                            CNcomment:�Ƿ�֧��sta���ӵı�־��CNend
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency:
*            @li hi_wifi_mesh_api.h: WiFi-MESH API
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_wifi_mesh_set_accept_sta(unsigned char enable_sta_connect);

/**
* @ingroup  hi_wifi_mesh
* @brief  Set sta supports mesh capability.CNcomment:����sta֧��mesh������CNend
*
* @par Description:
*           Set sta supports mesh capability.CNcomment:sta֧��mesh������CNend
*
* @attention 1. Default is not mesh sta. CNcomment:1. Ĭ�ϲ���mesh sta��CNend \n
*            2. The enable value can only be 1 or 0.. CNcomment:2. enableֵֻ��Ϊ1��0��CNend
* @param  enable          [IN]    Type  #unsigned char flag of sta's ability to support mesh.
*                                       CNcomment:sta֧��mesh�����ı�־��CNend
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency:
*            @li hi_wifi_mesh_api.h: WiFi-MESH API
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_wifi_set_mesh_sta(unsigned char enable);

/**
* @ingroup  hi_wifi_mesh
* @brief  Start mesh sta scan. CNcomment:mesh sta ɨ�衣CNend
*
* @par Description:
*           Start mesh sta scan. CNcomment:mesh sta ɨ�衣CNend
*
* @attention  NULL
* @param void.
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency:
*            @li hi_wifi_mesh_api.h: WiFi-MESH API
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_wifi_mesh_sta_scan(void);

/**
* @ingroup  hi_wifi_mesh
* @brief  Start mesh sta advance scan.CNcomment:mesh sta �߼�ɨ�衣CNend
*
* @par Description:
*           Start mesh sta advance scan.
*
* @attention  1. Advance scan can scan with ssid only,channel only,bssid only,prefix_ssid only��
*             and the combination parameters scanning does not support. \n
*             CNcomment:1 .�߼�ɨ��ֱ𵥶�֧�� ssidɨ�裬�ŵ�ɨ�裬bssidɨ�裬ssidǰ׺ɨ��, ��֧����ϲ���ɨ�跽ʽ��CNend \n
*             2. Scanning mode, subject to the type set by scan_type.
*             CNcomment:2 .ɨ�跽ʽ����scan_type���������Ϊ׼��CNend
* @param  sp          [IN]    Type #hi_wifi_scan_params * parameters of scan.CNcomment:ɨ�������������CNend
*
* @retval #HISI_OK    Execute successfully.
* @retval #HISI_FAIL  Execute failed.
* @par Dependency:
*            @li hi_wifi_mesh_api.h: WiFi-MESH API
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_wifi_mesh_sta_advance_scan(hi_wifi_scan_params *sp);

/**
* @ingroup  hi_wifi_mesh
* @brief  Start mesh peer scan. CNcomment:mesh peer ɨ�衣CNend
*
* @par Description:
*           Start mesh peer scan. CNcomment:mesh peer ɨ�衣CNend
*
* @attention  NULL
* @param void
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency:
*            @li hi_wifi_mesh_api.h: WiFi-MESH API
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_wifi_mesh_scan(void);

/**
* @ingroup  hi_wifi_mesh
* @brief  Start mesh peer advance scan.CNcomment:mesh peer �߼�ɨ�衣CNend
*
* @par Description:
*           Start mesh peer advance scan.CNcomment:mesh peer �߼�ɨ�衣CNend
*
* @attention  1. Advance scan can scan with ssid only,channel only,bssid only,prefix_ssid only��
*             and the combination parameters scanning does not support. \n
*             CNcomment:1 .�߼�ɨ��ֱ𵥶�֧�� ssidɨ�裬�ŵ�ɨ�裬bssidɨ�裬ssidǰ׺ɨ��, ��֧����ϲ���ɨ�跽ʽ��CNend \n
*             2. Scanning mode, subject to the type set by scan_type.
*             CNcomment:2 .ɨ�跽ʽ����scan_type���������Ϊ׼��CNend
* @param  sp          [IN]    Type  #hi_wifi_scan_params * mesh's scan parameters.CNcomment:mesh peer֧�ֵ�ɨ�跽ʽ��CNend
*
* @retval #HISI_OK    Execute successfully.
* @retval #HISI_FAIL  Execute failed.
* @par Dependency:
*            @li hi_wifi_mesh_api.h: WiFi-MESH API
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_wifi_mesh_advance_scan(hi_wifi_scan_params *sp);

/**
* @ingroup  hi_wifi_mesh
* @brief  Get the results of mesh peer scan.CNcomment:��ȡ mesh peer ɨ������Ľ����CNend
*
* @par Description:
*           Get the results of mesh peer scan..CNcomment:��ȡ mesh peer ɨ������Ľ����CNend
*
* @attention  1.ap_list: malloc by user.CNcomment:1.ɨ�������������û���̬����CNend \n
*             2.ap_list max size: (hi_wifi_mesh_scan_result_info ap_list) * 32. \n
*             CNcomment:2.ap_list ���Ϊ��hi_wifi_mesh_scan_result_info ap_list��* 32��CNend \n
*             3.ap_num:Parameters can be passed in to specify the number of scanned results.The maximum is 32. \n
*             CNcomment:3.���Դ��������ָ����ȡɨ�赽�Ľ�����������Ϊ32��CNend \n
*             4.If the callback function of the reporting user is used,
*             ap_num refers to bss_num in event_wifi_scan_done. \n
*             CNcomment:4.���ʹ���ϱ��û��Ļص�������ap_num�ο�event_wifi_scan_done�е�bss_num��CNend \n
*             5.ap_num should be same with number of hi_wifi_mesh_scan_result_info structures applied,
*             Otherwise, it will cause memory overflow. \n
*             CNcomment:5.ap_num�������hi_wifi_mesh_scan_result_info�ṹ������һ�£������������ڴ������CNend \n
*             6. SSID only supports ASCII characters.
*             CNcomment:6. SSID ֻ֧��ASCII�ַ�.CNend \n
*             7. The rssi in the scan results needs to be divided by 100 to get the actual rssi. \n
*             CNcomment:7. ɨ�����е�rssi��Ҫ����100���ܻ��ʵ�ʵ�rssi.CNend
* @param  ap_list         [IN/OUT]    Type #hi_wifi_mesh_scan_result_info * ap_list.CNcomment:ɨ�赽�Ľ����CNend
* @param  ap_num          [IN/OUT]    Type #unsigned int * number of scan result.CNcomment:ɨ�赽��������Ŀ��CNend
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency:
*            @li hi_wifi_mesh_api.h: WiFi-MESH API
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_wifi_mesh_scan_results(hi_wifi_mesh_scan_result_info *ap_list, unsigned int *ap_num);

/**
* @ingroup  hi_wifi_mesh
* @brief  Get the results of mesh sta scan.CNcomment:��ȡ mesh sta ɨ������Ľ����CNend
*
* @par Description:
*           Get the results of mesh sta scan..CNcomment:��ȡ mesh sta ɨ������Ľ����CNend
*
* @attention  1.ap_list: malloc by user.CNcomment:1.ɨ�������������û���̬����CNend \n
*             2.max size: (hi_wifi_mesh_scan_result_info ap_list) * 32. \n
*             CNcomment:2.�㹻�Ľṹ���С�����Ϊ��hi_wifi_mesh_scan_result_info ap_list��* 32��CNend \n
*             3.ap_num:Parameters can be passed in to specify the number of scanned results.The maximum is 32.
*             CNcomment:3.���Դ��������ָ����ȡɨ�赽�Ľ�����������Ϊ32��CNend \n
*             4.If the callback function of the reporting user is used,
*             ap_num refers to bss_num in event_wifi_scan_done. \n
*             CNcomment:4.���ʹ���ϱ��û��Ļص�������ap_num�ο�event_wifi_scan_done�е�bss_num��CNend \n
*             5.ap_num should be same with number of hi_wifi_mesh_scan_result_info structures applied,Otherwise,
*             it will cause memory overflow. \n
*             CNcomment:5.ap_num�������hi_wifi_mesh_scan_result_info�ṹ������һ�£������������ڴ������CNend \n
*             6. SSID only supports ASCII characters.
*             CNcomment:6. SSID ֻ֧��ASCII�ַ�.CNend \n
*             7. The rssi in the scan results needs to be divided by 100 to get the actual rssi. \n
*             CNcomment:7. ɨ�����е�rssi��Ҫ����100���ܻ��ʵ�ʵ�rssi.CNend
* @param  ap_list         [IN/OUT]    Type #hi_wifi_mesh_scan_result_info * ap_list.CNcomment:ɨ�赽�Ľ����CNend
* @param  ap_num          [IN/OUT]    Type #unsigned int * number of scan result.CNcomment:ɨ�赽��������Ŀ��CNend
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency:
*            @li hi_wifi_mesh_api.h: WiFi-MESH API
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_wifi_mesh_sta_scan_results(hi_wifi_mesh_scan_result_info *ap_list, unsigned int *ap_num);

/**
* @ingroup  hi_wifi_mesh
* @brief  Close mesh interface.CNcomment:ֹͣmesh�ӿڡ�CNend
*
* @par Description:
*           Close mesh interface.CNcomment:ֹͣmesh�ӿڡ�CNend
*
* @attention  1. This is a blocking function.CNcomment:1���˺���Ϊ��������.CNend
* @param  NULL
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency:
*            @li hi_wifi_mesh_api.h: WiFi-MESH API
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_wifi_mesh_stop(void);

/**
* @ingroup  hi_wifi_mesh
* @brief  Get all user's information of mesh.CNcomment:mesh��ȡ�����ӵ�peer����Ϣ��CNend
*
* @par Description:
*           Get all user's information of mesh.CNcomment:mesh��ȡ�����ӵ�peer����Ϣ��CNend
*
* @attention  1.peer_list: malloc by user.CNcomment:1.ɨ�������������û���̬����CNend \n
*             2.peer_list max size: (hi_wifi_mesh_peer_info peer_list) * 6. \n
*             CNcomment:2.peer_list �㹻�Ľṹ���С�����Ϊhi_wifi_mesh_peer_info* 6��CNend \n
*             3.peer_num:Parameters can be passed in to specify the number of connected peers.The maximum is 6. \n
*             CNcomment:3.���Դ��������ָ����ȡ�ѽ����peer���������Ϊ6��CNend \n
*             4.peer_num should be the same with number of hi_wifi_mesh_peer_info structures applied, Otherwise,
*             it will cause memory overflow. \n
*             CNcomment:4.peer_num�������hi_wifi_mesh_peer_info�ṹ������һ�£������������ڴ������CNend
* @param  peer_list        [IN/OUT]    Type  #hi_wifi_mesh_peer_info *, peer information.CNcomment:���ӵ�peer��Ϣ��CNend
* @param  peer_num         [IN/OUT]    Type  #unsigned int *, peer number.CNcomment:peer�ĸ�����CNend
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency:
*            @li hi_wifi_mesh_api.h: WiFi API
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_wifi_mesh_get_connected_peer(hi_wifi_mesh_peer_info *peer_list, unsigned int *peer_num);

/**
* @ingroup  hi_wifi_mesh
* @brief  Set switch channel enable or disable.CNcomment:����MESH AP�ŵ�ʹ�ܻ�����Ч.CNend
*
* @par Description:
*         Set switch channel and disable switch channel.CNcomment:����MESH AP�ŵ�ʹ�ܻ�����Ч.CNend
*
* @attention  default switch channel is enable��set HI_FALSE is disable and HI_TRUE is enable
* @param  ifname           [IN]     Type  #const char *, interface name.CNcomment:�ӿ���.CNend
* @param  ifname_len       [IN]     Type  #unsigned char, interface name length.CNcomment:�ӿ�������.CNend
* @param  enable           [IN]     Type  #unsigned char, enable or disable.CNcomment:ʹ�ܻ�����Ч.CNend
*
* @retval #HISI_OK  Excute successfully
* @retval #Other    Error code
* @par Dependency:
*            @li hi_wifi_mesh_api.h: WiFi API
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_wifi_protocol_chn_switch_enable(const char *ifname, unsigned char ifname_len, hi_u8 enable);

/**
* @ingroup  hi_wifi_mesh
* @brief  Set switch channel.CNcomment:����MESH AP�ŵ��л�.CNend
*
* @par Description:
*         Set switch channel.CNcomment:����MESH AP�ŵ��л�.CNend
*
* @attention  NULL
* @param  ifname        [IN]   Type  #const char *, interface name.CNcomment:�ӿ���.CNend
* @param  ifname_len    [IN]   Type  #unsigned char, interface name length.CNcomment:�ӿ�������.CNend
* @param  channel       [IN]   Type  #unsigned char, switch channel.CNcomment:�ŵ���.CNend
* @param  switch_count  [IN]   Type  #unsigned char, switch channel offset beacons.CNcomment:�л��ŵ�ƫ��beacon�ĸ���.CNend
*
* @retval #HISI_OK  Excute successfully
* @retval #Other           Error code
* @par Dependency:
*            @li hi_wifi_mesh_api.h: WiFi API
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_wifi_protocol_chn_switch(const char *ifname, unsigned char ifname_len,
    unsigned char channel, unsigned char switch_count);

/**
* @ingroup  hi_wifi_mesh
* @brief  Set switch channel.CNcomment:��ѯMESH �ڵ���Ϣ.CNend
*
* @par Description:
*         Set switch channel.CNcomment:��ѯMESH �ڵ���Ϣ.CNend
*
* @attention  NULL
* @param  mesh_node_info     [OUT]     Type  #mac_cfg_mesh_nodeinfo_stru *, node info.CNcomment:�ڵ���Ϣ.CNend
*
* @retval #HISI_OK  Excute successfully
* @retval #Other           Error code
* @par Dependency:
*            @li hi_wifi_mesh_api.h: WiFi API
* @see  NULL
* @since Hi3861_V100R001C00
*/
unsigned int hi_wifi_get_mesh_node_info(mac_cfg_mesh_nodeinfo_stru *mesh_node_info);

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif /* end of hi_wifi_mesh_api.h */
