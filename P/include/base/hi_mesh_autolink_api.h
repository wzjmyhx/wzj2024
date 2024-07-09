/**
 * @file hi_mesh_autolink_api.h
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
 * @defgroup hi_mesh_autolink WiFi Mesh Autolink Settings
 * @ingroup hi_wifi
 */

#ifndef __HI_MESH_AUTOLINK_API_H__
#define __HI_MESH_AUTOLINK_API_H__

#include "hi_wifi_mesh_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @ingroup hi_mesh_autolink
 *
 * Auth type of mesh autolink.CNcomment: Mesh�Զ���������֤����.CNend
 */
typedef enum {
    HI_MESH_OPEN,                       /**< Auth Type:Open.*//**< CNcomment:��֤����:����.CNend */
    HI_MESH_AUTH,                       /**< Auth Type:Auth.*//**< CNcomment:��֤����:����.CNend */

    HI_MESH_AUTH_TYPE_BUTT              /**< Boundary of hi_mesh_autolink_auth_type.*/
                                        /**< CNcomment:hi_mesh_autolink_auth_typeö�ٶ���.CNend */
}hi_mesh_autolink_auth_type;


/**
 * @ingroup hi_mesh_autolink
 *
 * Node type that usr config of mesh autolink.CNcomment: Mesh�Զ��������û�����ڵ�����.CNend
 */
typedef enum {
    HI_MESH_USRCONFIG_MBR,                  /**< User Config:MBR Role.*//**< CNcomment: �û�ָ��MBR�ڵ��ɫ.CNend */
    HI_MESH_USRCONFIG_MR,                   /**< User Config:MR Role.*//**< CNcomment: �û�ָ��MR�ڵ��ɫ.CNend */
    HI_MESH_USRCONFIG_MSTA,                 /**< User Config:MSTA Role.*//**< CNcomment: �û�ָ��MSTA�ڵ��ɫ.CNend */
    HI_MESH_USRCONFIG_AUTO,                 /**< User Config:Auto Role(Result:MBR/MR).*/
                                            /**< CNcomment: �Զ���ѡ�ڵ��ɫ(MBR/MR��ѡ��).CNend */
/**< Role of node which cannot join in Mesh.(Not User Config). CNcomment: δ���������ķ��ؽ��(���û�����).CNend */
    HI_MESH_AUTOLINK_ROUTER_MSTA,
/**< Boundary of hi_mesh_autolink_usrcfg_node_type.*//**< CNcomment:hi_mesh_autolink_usrcfg_node_typeö�ٶ���.CNend */
    HI_MESH_USRCONFIG_BUTT
}hi_mesh_autolink_usrcfg_node_type;


/**
 * @ingroup hi_mesh_autolink
 *
 * Node type information.CNcomment: Mesh�Զ������Ľڵ���Ϣ��.CNend
 */
typedef struct {
    char ifname_first[WIFI_IFNAME_MAX_SIZE + 1];     /**< First Interface name.*//**< CNcomment: ��һ���ӿ�����.CNend */
/**< First Interface name length. *//**< CNcomment: ��һ���ӿ����Ƶĳ���.CNend */
    int len_first;
    char ifname_second[WIFI_IFNAME_MAX_SIZE + 1];    /**< Second Interface name.*//**< CNcomment: �ڶ����ӿ�����.CNend */
/**< Second Interface name length. *//**< CNcomment: �ڶ����ӿ����Ƶĳ���.CNend */
    int len_second;
}hi_mesh_autolink_role_cb_info;

/**
 * @ingroup hi_mesh_autolink
 *
 * Struct of mesh autolink config.CNcomment:mesh�Զ��������ò���CNend
 *
 */
typedef struct {
    char ssid[HI_WIFI_MAX_SSID_LEN + 1];                   /**< SSID.*//**< CNcomment: SSID.CNend */
/**< Auth type of Autolink.*//**< CNcomment: Autolink��֤����.CNend */
    hi_mesh_autolink_auth_type auth;
    char key[HI_WIFI_MESH_KEY_LEN_MAX + 1];                /**< Password.*//**< CNcomment: ����.CNend */
    hi_mesh_autolink_usrcfg_node_type usr_cfg_role;        /**< Node type that usr config of mesh autolink.*/
                                                           /**< CNcomment: �û����õĽڵ��ɫ.CNend */
}hi_mesh_autolink_config;

/**
 * @ingroup hi_mesh_autolink
 *
 * Struct of autolink role callback.CNcomment:Mesh�Զ�������ɫ�ص��ṹ��.CNend
 *
 */
typedef struct {
/**< Node type that usr config of mesh autolink.*//**< CNcomment: �û����õĽڵ��ɫ.CNend */
    hi_mesh_autolink_usrcfg_node_type role;
    hi_mesh_autolink_role_cb_info info;       /**< Event Information.*//**< CNcomment: �¼���Ϣ.CNend */
} hi_mesh_autolink_role_cb;

/**
 * @ingroup hi_mesh_autolink
 *
 * callback function definition of mesh autolink result.CNcommment:mesh�Զ���������ص��ӿڶ���.CNend
 */
typedef void (* hi_mesh_autolink_cb)(const hi_mesh_autolink_role_cb *role);

/**
* @ingroup  hi_mesh_autolink
* @brief  Mesh start autolink network.CNcomment:mesh�����Զ�������CNend
*
* @par Description:
*          Mesh start autolink network.CNcomment:mesh�����Զ�������CNend
*
* @attention  1. SSID only supports ASCII characters.
*                CNcomment:1. SSID ֻ֧��ASCII�ַ�.CNend
* @param  config [IN]  Type  #hi_mesh_autolink_config * mesh's autolink configuration.CNcomment:mesh�Զ��������á�CNend
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency:
*            @li hi_wifi_api.h: WiFi API
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_mesh_start_autolink(hi_mesh_autolink_config *config);

/**
* @ingroup  hi_mesh_autolink
* @brief  Close mesh autolink.CNcomment:ֹͣmesh�Զ�����ģ�顣CNend
*
* @par Description:
*           Close mesh autolink.CNcomment:ֹͣmesh�Զ�����ģ�顣CNend
*
* @attention  NULL
* @param  NULL
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency:
*            @li hi_wifi_mesh_api.h: WiFi-MESH API
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_mesh_exit_autolink(void);

/**
* @ingroup  hi_mesh_autolink
* @brief  Set the rssi threshold of the router when MBR node connect.
*           CNcomment:����MBR�ڵ����·������RSSI��ֵ��CNend
*
* @par Description:
*           Set the router rssi threshold.CNcomment:����·������RSSI��ֵ��CNend
*
* @attention  1. The valid range of RSSI threshold is -127 ~ 127.
*                CNcomment:1. RSSI��ֵ����Ч��Χ��-127         ~ 127.CNend
* @param  router_rssi [IN]  Type  #int mesh's rssi threshold of mbr connecting to router.
*           CNcomment:MBR������·������RSSI��ֵCNend
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency: NULL
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_mesh_autolink_set_router_rssi_threshold(int router_rssi);

/**
* @ingroup  hi_mesh_autolink
* @brief  Get the rssi threshold of the router when MBR node connect.
*           CNcomment:��ȡMBR�ڵ����·������RSSI��ֵ��CNend
*
* @par Description:
*           Get the router rssi threshold.CNcomment:��ȡ·������RSSI��ֵ��CNend
*
* @attention 1. The memories of <router_rssi> memories are requested by the caller.
*               CNcomment:1. <router_rssi>�ɵ����������ڴ�CNend
* @param  router_rssi [OUT]  Type  #int * mesh's rssi threshold of mbr connecting to router.
*           CNcomment:MBR������·������RSSI��ֵCNend
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency: NULL
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_mesh_autolink_get_router_rssi_threshold(int *router_rssi);

/**
* @ingroup  hi_mesh_autolink
* @brief  Set the bandwidth value of the mesh network. CNcomment:����Mesh����Ĵ���CNend
*
* @par Description:
*           Set the bandwidth value of the mesh network. CNcomment:����Mesh����Ĵ���CNend
*
* @attention 1. Should be called before calling hi_mesh_start_autolink api to establish mesh network.
*            CNcomment:1. ��Ҫ�ڵ���hi_mesh_start_autolink�ӿ��齨����ǰ����CNend
* @param  bw [IN]  Type  #hi_wifi_bw The bandwidth value of mesh network.
*           CNcomment:Mesh����Ĵ���ֵCNend
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency: NULL
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_mesh_autolink_set_bw(hi_wifi_bw bw);

/**
* @ingroup  hi_mesh_autolink
* @brief  Get the bandwidth value of the mesh network. CNcomment:��ȡMesh����Ĵ���CNend
*
* @par Description:
*           Get the bandwidth value of the mesh network. CNcomment:��ȡMesh����Ĵ���CNend
*
* @attention 1. The memories of <bw> memories are requested by the caller.
*               CNcomment:1. <bw>�ɵ����������ڴ�CNend
* @param  bw [OUT]  Type  #hi_wifi_bw * The bandwidth value of mesh network.
*           CNcomment:Mesh����Ĵ���ֵCNend
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency: NULL
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_mesh_autolink_get_bw(hi_wifi_bw *bw);

/**
* @ingroup  hi_mesh_autolink
* @brief  register mesh autolink user callback interface.CNcomment:ע��Mesh�Զ������ص������ӿ�.CNend
*
* @par Description:
*           register mesh autolink user callback interface.CNcomment:ע��Mesh�Զ������ص������ӿ�.CNend
*
* @attention  NULL
* @param  role_cb  [OUT]    Type #hi_mesh_autolink_cb, role callback .CNcomment:�ص�����.CNend
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency: NULL
* @see  NULL
* @since Hi3861_V100R001C00
*/
int hi_mesh_autolink_register_event_callback(hi_mesh_autolink_cb role_cb);

/**
* @ingroup  hi_mesh_autolink
* @brief  set device oui interface.CNcomment:���ò�Ʒ�ĳ���oui����.CNend
*
* @par Description:
*           set device ou interface.CNcomment:���ò�Ʒ�ĳ���oui����.CNend
*
* @attention  NULL
* @param  oui      [IN]    Type #hi_u8 *, oui code .CNcomment:��Ʒ���ұ��룬Ϊ3���ֽ��޷���������Χ0-0XFF.CNend
* @param  oui_len  [IN]    Type #hi_u8, oui code lenth .CNcomment:���ұ��볤��,����Ϊ3���ֽ�.CNend
*
* @retval #HISI_OK        Execute successfully.
* @retval #HISI_FAIL      Execute failed.
* @par Dependency: NULL
* @see  NULL
* @since Hi3861_V100R001C00
*/
hi_s8 hi_mesh_set_oui(const hi_u8 *oui, hi_u8 oui_len);


#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif /* end of hi_mesh_autolink_api.h */
