/**
* @file hi_wifi_api.h
*
* Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved. \n
* Description: header file for wifi quick send api.CNcomment:������WiFi ��������api�ӿ�ͷ�ļ�.CNend\n
* Author: Hisilicon \n
* Create: 2019-01-03
*/

/**
 * @defgroup hi_wifi_basic WiFi Basic Settings
 * @ingroup hi_wifi
 */

#ifndef __HI_WIFI_QUICK_SEND_API_H__
#define __HI_WIFI_QUICK_SEND_API_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/**
 * @ingroup hi_wifi_quick_send_basic
 *
 * Sequence number field offset in 802.11 frame.CNcomment:���к��ֶ���802.11֡�е�ƫ��.CNend
 */
#define QUICK_SEND_SEQ_NUM_OFFSET 22

/**
 * @ingroup hi_wifi_quick_send_basic
 *
 * Source mac address field offset in 802.11 frame.CNcomment:Դmac��ַ�ֶ���802.11֡�е�ƫ��.CNend
 */
#define QUICK_SEND_SRC_MAC_OFFSET 10

/**
 * @ingroup hi_wifi_quick_send_basic
 *
 * destination mac address field offset in 802.11 frame.CNcomment:Ŀ��mac��ַ�ֶ���802.11֡�е�ƫ��.CNend
 */
#define QUICK_SEND_DST_MAC_OFFSET 16

/**
 * @ingroup hi_wifi_quick_send_basic
 *
 * Length of mac address field.CNcomment:mac��ַ�ֶγ���.CNend
 */
#define QUICK_SEND_MAC_LEN 6

/**
 * @ingroup hi_wifi_quick_send_basic
 *
 * Offset of bssid field.CNcomment:bssid�ֶ�ƫ��.CNend
 */
#define QUICK_SEND_BSSID_OFFSET 4

/**
 * @ingroup hi_wifi_quick_send_basic
 *
 * Length of bssid field.CNcomment:bssid�ֶγ���.CNend
 */
#define QUICK_SEND_BSSID_LEN 6

/**
 * @ingroup hi_wifi_quick_send_basic
 *
 * Send frame success.CNcomment:���ͳɹ�.CNend
 */
#define QUICK_SEND_SUCCESS   1

/**
 * @ingroup hi_wifi_quick_send_basic
 *
 * Invalid send result.CNcomment:��Ч�ķ��ͽ��.CNend
 */
#define QUICK_SEND_RESULT_INVALID   255

/**
 * @ingroup hi_wifi_quick_send_basic
 *
 * Max ssid length.CNcomment:ssid��󳤶�.CNend
 */
#define QUICK_SEND_SSID_MAX_LEN  (32 + 1)

/**
 * @ingroup hi_wifi_quick_send_basic
 *
 * Mac address length.CNcomment:mac��ַ����.CNend
 */
#define QUICK_SEND_MAC_ADDR_LEN  6

/**
 * @ingroup hi_wifi_quick_send_basic
 *
 * Mac address length.CNcomment:mac��ַ����.CNend
 */
typedef enum {
    WIFI_PHY_MODE_11N = 0,
    WIFI_PHY_MODE_11G = 1,
    WIFI_PHY_MODE_11B = 2,
    WIFI_PHY_MODE_BUTT
}hi_wifi_phy_mode;

/**
 * @ingroup hi_wifi_basic
 *
 * Struct of frame filter config in monitor mode.CNcomment:���Ľ��չ�������.CNend
 */
typedef struct {
    char mdata_en : 1;  /**< get multi-cast data frame flag. CNcomment: ʹ�ܽ����鲥(�㲥)���ݰ�.CNend */
    char udata_en : 1;  /**< get single-cast data frame flag. CNcomment: ʹ�ܽ��յ������ݰ�.CNend */
    char mmngt_en : 1;  /**< get multi-cast mgmt frame flag. CNcomment: ʹ�ܽ����鲥(�㲥)�����.CNend */
    char umngt_en : 1;  /**< get single-cast mgmt frame flag. CNcomment: ʹ�ܽ��յ��������.CNend */
    char resvd    : 4;  /**< reserved bits. CNcomment: �����ֶ�.CNend */
} hi_wifi_rx_filter;

/**
* @ingroup  hi_wifi_basic
* @brief  Config tx parameter.CNcomment:���÷��Ͳ���.CNend
*
* @par Description:
*           Config tx parameter.CNcomment:���÷��Ͳ���.CNend
*
* @attention  1.CNcomment:��֡ǰ��Ҫ����һ��,��ʼ���ŵ�/���ʵȲ���.CNend\n
* @param  ch����        [IN]     Type #char *, channel. CNcomment:�ŵ��ַ���,"0"~"14",��"6".CNend
* @param  rate_code     [IN]     Type #char *, rate code. CNcomment:�������ַ���.
*                                phy_modeΪ0ʱ,ֵ��Χ:"0"~"7",�ֱ��Ӧmcs0~mcs7;
*                                phy_modeΪ1ʱ,ֵ��Χ:"48","24","12","6","54","36","18","9"Mbps)
*                                phy_modeΪ2ʱ,ֵ��Χ:"1","2","5.5","11"Mbps).CNend
* @param  phy_mode      [IN]     Type #hi_wifi_phy_mode, phy mode. CNcomment:Э��ģʽ,��11b/g/n.CNend
*
* @retval #HISI_OK  Excute successfully
* @retval #Other           Error code
*
* @par Dependency:
*            @li hi_wifi_api.h: WiFi API
* @see  NULL
* @since Hi3861_V100R001C00
*/
unsigned int hi_wifi_cfg_tx_para(char *ch, char *rate_code, hi_wifi_phy_mode phy_mode);

/**
* @ingroup  hi_wifi_quick_send_basic
* @brief  Get frame send result.CNcomment:��ȡ���ͽ��.CNend
*
* @par Description:
*           Get frame send result.CNcomment:��ȡ���ͽ��.CNend
*
* @attention  NULL
* @param      NULL
*
* @retval  #QUICK_SEND_SUCCESS   Send success.
* @retval #Other  Error code
*
* @par Dependency:
*            @li hi_wifi_quick_send_api.h: WiFi API
* @see  NULL
* @since Hi3861_V100R001C00
*/
unsigned char hi_wifi_get_quick_send_result(void);

/**
* @ingroup  hi_wifi_quick_send_basic
* @brief  Reset frame send result.CNcomment:���÷��ͱ��Ľ��Ϊ��Ч.CNend
*
* @par Description:
*           Reset frame send result.CNcomment:���÷��ͱ��Ľ��Ϊ��Ч.CNend
*
* @attention  1.CNcomment:Called before send.CNend\n
* @param      NULL
*
* @retval NULL
*
* @par Dependency:
*            @li hi_wifi_quick_send_api.h: WiFi API
* @see  NULL
* @since Hi3861_V100R001C00
*/
void hi_wifi_reset_quick_send_result(void);

/**
* @ingroup  hi_wifi_quick_send_basic
* @brief  Send a custom 802.11 frame.CNcomment:�����û�����802.11����.CNend
*
* @par Description:
*           Send a custom 802.11 frame.CNcomment:�����û�����802.11����.CNend
*
* @attention  1.CNcomment:���֧�ַ���1400�ֽڵı���.CNend\n
*             2.CNcomment:�����밴��802.11Э���ʽ��װ.CNend\n
*             3.CNcomment:����ֵ����ʾ�����Ƿ�ɹ����뷢�Ͷ���,����ʾ�տڷ���״̬.CNend\n
*             3.CNcomment:���ͽ����hi_wifi_get_quick_send_result()�ӿڲ�ѯ.CNend\n
* @param  payload       [IN]     Type #unsigned char *, payload. CNcomment:֡����.CNend
* @param  len           [IN]     Type #unsigned short, frame length. CNcomment:֡����.CNend
*
* @retval #HISI_OK  Excute successfully
* @retval #Other           Error code
*
* @par Dependency:
*            @li hi_wifi_quick_send_api.h: WiFi API
* @see  NULL
* @since Hi3861_V100R001C00
*/
unsigned int hi_wifi_tx_proc_fast(unsigned char *payload, unsigned short len);

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif /* end of hi_wifi_quick_send_api.h */

