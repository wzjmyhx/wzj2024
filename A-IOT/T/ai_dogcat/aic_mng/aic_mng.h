/*
 * Copyright (c) 2022 HiSilicon (Shanghai) Technologies CO., LIMITED.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AIC_MNG_H
#define AIC_MNG_H

#include <stdint.h>
#include <stdbool.h>

#include "mpp_sess.h"
#include "plug_mng.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
    video input type.
    ͬһʱ�̣�ֻ֧��һ����Դ��
*/
typedef enum AicViType {
    AIC_VIT_NONE = 0,
    AIC_VIT_SENSOR, // from local sensor, the default
    AIC_VIT_HOST, // from host (PC/TV)
    AIC_VIT_FILE, // from file, ��δ֧��
    AIC_VIT_CLOUD, // from cloud����δ֧��
    AIC_VIT_BUTT
}AicViType;

/**
    video output type.
    �ǻ����ϵ���ɹ��档
*/
#define AIC_VOT_UVC     HI_BIT0 // output via UVC
#define AIC_VOT_IP      HI_BIT1 // output via IP
#define AIC_VOT_RTSPD   HI_BIT4 // output via RTSPD

/**
    AI������������������.
*/
typedef void (*AiResProc)(void* user,
    uint32_t cltId, const char* plugUuid, const char* resJson, int resLen);

/**
    �¼�������������.
*/
typedef void (*AicEvtProc)(void* user,
    uint32_t cltId, const char* evtName, const char* evtJson, int evtLen);

/**
    aic stream user�ӿ�.
*/
typedef struct IAicStrmUser {
    bool (*OnStrmOn)(void* user, int vencChn, PAYLOAD_TYPE_E codecType, int width, int height);
    void (*OnStrmOff)(void* user);
    void (*OnVencFrm)(void* user, VencFrm* frm); // VENC out frame
    void (*OnVideoFrm)(void* user, VIDEO_FRAME_INFO_S* frm); // VI/VPSS out frame
}IAicStrmUser;

/**
    ǰ������.
*/
struct uvc_device_ext; // aic user�������ͷ�ļ���ǰ���������Ա����ͷ�ļ�include <uvc_dev.h>.

/**
    AicMng init/exit.
*/
void AicInit(void);
void AicExit(void);

/**
    �Ƿ���strm����״̬����histm_aicʹ��.
    �����滻ԭ�е�HiStmCheckUvcStatus()��
*/
bool AicInStrm(void);

/**
    submit 1 jpg to venc.
*/
int AicSubmitJpg(uint8_t* data, int len, bool endOfStrm);

/**
    get vi type.
*/
AicViType AicGetViType(void);

/**
    get vo types.
*/
uint32_t AicGetVoTypes(void);

/**
    start/stop VI.
*/
int AicStartVi(AicViType viType, bool byHost,
    int outWidth, int outHeight, PIXEL_FORMAT_E pixFormat);
int AicStopVi(AicViType viType, bool byHost);

/**
    start/stop VO.

    @param voType ����ΪAIC_VOT_UVC��
    @param codecType Ŀǰ��֧��MJPEG��
*/
int AicStartVo(uint32_t voType, int width, int height, PAYLOAD_TYPE_E codecType);
int AicStopVo(uint32_t voType);

/**
    stop ���е�vi, vo.
    NOTE: ��stop AIC_VOT_UVC������UVC����.
*/
int AicStopVios(bool byHost);

/**
    ���ͳ�ʼframe.
    Ŀǰ����UVC userʹ�ã����ƿ�UVC����ͨ��������֡��turn strm on������.
*/
int AicTxInitPic(void);

/**
    ����ai���.

    @param uuid Ϊnull string��ʾunload��ǰ����������ʾ����@param uuidָ���Ĳ����
    @param reload Ϊtrueʱ���������õ��²����ǰ�Ѽ��أ�Ҳ���¼���֮��
    @param plug ���ؼ��صĲ������Ϣ������Ǽ��ز����Ļ���
*/
int AicSetAiPlug(const char* uuid, AiPlugLib* plug, bool reload);

/**
    �б�ai�����Ϣ.

    @param resJson[out]: ����json��ʽ�Ĳ���б���
    @param rsvSize[in]: �˺����ڴ���resJson����ʱ������ͷ�������Ŀռ�(�ֽڳ���).
    @return �ɹ��򷵻�plug�����������򷵻ظ�ֵ.
*/
int AicListAiPlugs(MemBlk** resJson, int rsvSize);

/**
    �б�working plugs.

    @param plugInfos[out]: ���working plugs��Ϣ�������ڵ�ΪAiPlugInfo��
    @return �ɹ����򷵻������working plugs����Ŀ������Ϊ0�����򷵻ظ�������ֵ��ʾ�����롣
*/
int AicListWorkPlugs(struct list_head* plugInfos);

/**
    ����AI�����¼�.

    ���ĺ󣬵�AI����������ʱ�������proc()���������.

    @param cltId[in]: client ID.
    @param plugUuid[in]: ��ʾ�ض�AI����Ĳ����ID.
    @param proc[in]: ����������Ĵ�������.
    @param user[in]: ��Ϊuser��������aiResProc()���������͵�ָ��.
    @return �ɹ�����0�����򷵻ظ�������ֵ��ʾ������.
*/
int AicSubsAiSvc(uintptr_t cltId, const char* plugUuid, AiResProc proc, void* user);

/**
    ȥ����AI�����¼�.

    ȥ���Ĵ�ǰͨ��AicSubsAiSvc()���ĵ�AI����.

    @param cltId[in]: client ID.
    @param plugUuid[in]: ��ʾ�ض�AI����Ĳ����ID.
    @return �ɹ�����0�����򷵻ظ�������ֵ��ʾ������.
*/
int AicUnsubsAiSvc(uintptr_t cltId, const char* plugUuid);

/**
    ����AIC�¼�(����AI�����¼�).

    ���ĺ󣬵�AIC�����¼�ʱ�������proc()���û��ύ�¼�.

    @param cltId[in]: client ID.
    @param evtName[in]: ��ʾ�����ĵ��¼����ƣ���ΪNULL��ʾ���������¼�.
    @param proc[in]: �¼���������.
    @param user[in]: ��Ϊuser��������aicEvtProc()���������͵�ָ��.
    @return �ɹ�����0�����򷵻ظ�������ֵ��ʾ������.
*/
int AicSubsEvt(uintptr_t cltId, const char* evtName, AicEvtProc proc, void* user);

/**
    ȥ����AIC�¼�(����AI�����¼�).

    ȥ���Ĵ�ǰͨ��AicSubsEvt()���ĵ��¼�.

    @param cltId[in]: client ID.
    @param evtName[in]: ��ʾȥ���ĵ��¼����ƣ���ΪNULL��ʾȥ���������¼�.
    @return �ɹ�����0�����򷵻ظ�������ֵ��ʾ������.
*/
int AicUnsubsEvt(uintptr_t cltId, const char* evtName);

/**
    ��õ�ǰ״̬.
    @param resJson[out]: ����JSON��ʽ��״̬��
*/
int AicGetStatus(MemBlk* resJson);

/**
    ע��stream user.
*/
bool AicAddStrmUser(const IAicStrmUser* itf, void* user);

/**
    ע��stream user.
*/
bool AicDelStrmUser(const IAicStrmUser* itf, void* user);

#ifdef __cplusplus
}
#endif

#endif // AIC_MNG_H
