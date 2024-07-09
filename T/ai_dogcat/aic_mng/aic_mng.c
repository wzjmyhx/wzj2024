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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

//#include "hi_ext_util.h"
#include "mpp_help.h"
#include "plug_mng.h"
#include "aic_mng.h"

#define PLUG_DIR_DEF            "./plugs" // Ĭ�ϲ��Ŀ¼
#define PLUG_UUID_MAX           256 // ���uuid������ֽڳ���
#define PREPLUG_ID_MAX          9 // Ԥ�ȼ��صĲ����ŵ����ֵ����0��ʼ

#define VENC_SIZET_DEF          PIC_720P // �����Ĭ��size
#define PIRIOD_LOG_INTVAL       (5 * 1000) // �ڼ�ͳ����Ϣ������

#define VDEC_WIDTH_MAX          2688 // ����֧�ֵ����width
#define VDEC_HEIGHT_MAX         2160 // ����֧�ֵ����height
#define VDEC_FPS_DEF            25 // ����֧�ֵ�Ĭ��fps

#define SCENE_PARAM_FILE_DEF    "./scene_auto/param/sensor_imx335" // snssor�����ļ�
#define INIT_PIC_FILE_DEF       "./res/black_640x360.jpg" // ����UVC�ĳ�ʼͼƬ

#define STATUS_INFO_MAX         (128 * 1024) // status info buf��size, ��ʱ���ö���buf
#define STATUS_EVT_NAME         "status" // status�¼�����

#define STRM_USER_MAX           8 // stream user�������Ŀ

/*
    AICʹ�õ�MPP��ԴID��Ĭ��ֵ��������ͨ�������ļ�����
*/
#define AIC_VDEC_CHN            0 // Ĭ��ʹ�õ�VDEC chn
#define AIC_VENC_CHN            0 // Ĭ��ʹ�õ�VENC chn
#define AIC_VPSS_GRP            0 // Ĭ��ʹ�õ�VPSS group
#define AIC_VPSS_ZIN_CHN        0 // Ĭ��ʹ�õ�VPSS �Ŵ�ͨ��
#define AIC_VPSS_ZOUT_CHN       1 // Ĭ��ʹ�õ�VPSS ��Сͨ��

#define SIZE_TO_MPP_WIDTH_1920    1920
#define SIZE_TO_MPP_HEIGHT_1080   1080
#define SIZE_TO_MPP_WIDTH_1280    1280
#define SIZE_TO_MPP_HEIGHT_720    720
#define SIZE_TO_MPP_WIDTH_640     640
#define SIZE_TO_MPP_HEIGHT_360    360
#define SIZE_TO_MPP_WIDTH_3840    3840
#define SIZE_TO_MPP_HEIGHT_2160   2160
#define SIZE_TO_MPP_WIDTH_2592    2592
#define SIZE_TO_MPP_HEIGHT_1536   1536
#define SIZE_TO_MPP_HEIGHT_1944   1944

/**
    ����AIC�¼���user����Ϣ.
*/
typedef struct AicEvtUser {
    struct list_head lnode;

    uintptr_t cltId;
    char evtName[TINY_BUF_SIZE];

    AicEvtProc proc;
    void *user;
}   AicEvtUser;

/**
    ����AI�����user����Ϣ.
*/
typedef struct AiSvcUser {
    struct list_head lnode;

    uintptr_t cltId;
    char plugUuid[PLUG_UUID_MAX];

    AiResProc proc;
    void *user;
}   AiSvcUser;

/**
    AI����job.
    ��ʾһ֡��������
*/
typedef struct AiCalJob {
    bool busy; // �Ƿ����ڱ�ʹ��
    int sigFd; // ���֪ͨ�ź�

    int res; // ִ�н��
    int grpId; // VPSS group ID
    int chnId; // VPSS chn ID
    VIDEO_FRAME_INFO_S frm; // [in|out]֡���������֡����������ͼ�ε���֡

    char *resJson; // �������󷵻ص�resJson
    char plugUuid[PLUG_UUID_MAX]; // ִ�д�job��plugUuid����svc����ֵ
}   AiCalJob;

/**
    AI�������.
*/
typedef struct AiCalSvc {
    EvtMon *evtMon; // ������EvtMon��EvtMon������һ����̨�߳�
    int sigFd; // ָʾ��job���ӵ��ź�
}   AiCalSvc;

/**
    �ڼ�ͳ����Ϣ.
*/
typedef struct PiriodStatis {
    int64_t begTime; // �ڼ���ʼʱ��(ms)

    int vpssOutNum; // VPSS�����֡����Ҳ����VPSS�����֡��
    int vencOutNum; // VENC�����֡����Ҳ����VENC�����֡��
    int aiCalNum; // AI�����֡��

    int64_t aiCalCost; // AI��������ʱ
    int64_t fmtFrmCost; // AI����ǰ֡����(resize��)��ʱ
    int64_t cvtVoFrmCost; // VENC���֡ת��Ϊvo��ʽ�ĺ�ʱ
}   PiriodStatis;

/**
    aic stream user node.
*/
typedef struct AicStrmUser {
    const IAicStrmUser *itf;
    void *user;
}   AicStrmUser;

/**
    AI Camera Manager.
*/
typedef struct AicMng {
    bool inited; // �����Ƿ�inited
    bool initing; // �Ƿ����ڳ�ʼ��

    EvtMon *evtMon; // MainEvtMon()�Ŀ��ֵ
    // UvcDevT *uvcDev; // ʹ�õ�UvcDev

    // mpp�ɼ���fd
    int vpssFd; // ָʾVPSS�������fd
    int vencFd; // ָʾVENC�������fd

    // ��ǰMppSessʹ�õ���ԴID
    VPSS_GRP vpssGrp;
    VPSS_CHN vpssChn0;
    VPSS_CHN vpssChn1;
    VDEC_CHN vdecChn;
    VENC_CHN vencChn;

    // MppSess config
    ViCfg viCfg;
    VdecCfg vdecCfg;
    VpssCfg vpssCfg;
    VencCfg vencCfg;

    // MppSess
    MppSess *viSess; // VI(sensor)+VPSS | VDEC+VPSS
    MppSess *vencSess; // VENC

    // MppSess���
    OsdSet *osdSet; // OSD set����plug��resFrm�����OSD
    uint32_t inFrmIdc; // ����frame��ID����������������mpp frame��timeRef

    // vi���
    AicViType viType; // vi type��ͬʱֻ��֧��һ��type
    bool viByHost; // VI�Ƿ�host������
    int64_t rxPicNum; // һ��HOST-VI���յ���ͼƬ��Ŀ
    int viOpenRef; // sensor���򿪵ļ�����������ʾ��ǰ�ж��ٸ�user���ڴ�sensor

    // vo���
    uint32_t voTypes; // vo types����ͬʱ֧�ֶ��type
    int voOpenRef; // vo���򿪵ļ�����������ʾ��ǰ�ж��ٸ�user���ڴ�vo

    AiCalSvc aiCalSvc; // AI������񣬻����һ����̨�߳�
    AiCalJob aiCalJob; // AI�������񣬰�������һ֡ͼƬ�������Ϣ
    char *prevResJson; // ǰ��AI���㷵�ص�resJson

    pthread_mutex_t plugMutex; // AiCalSvc��PlugLoadJob��ͬʱ���ʲ������Ҫ����
    AiPlugLib workPlug; // ��ǰ���صĲ��

    struct list_head aiSvcUsers; // ��ǰ���AI�����user list

    struct list_head plugInfos; // �����Ϣ�б���Ԫ��ΪAicPlugInfo
    struct list_head svcPlugs; // �������й�������Ĳ���б���Ԫ��ΪAicPlugInfo

    struct list_head evtUsers; // �����¼���user list

    int statusChgFd; // ָʾ״̬�����fd

    int blkCallNum; // �˶���������ִ�еĺ�ʱ������������������ȷ�������ڼ�����
    PiriodStatis piriodStatis; // �ڼ�ͳ����Ϣ

    // ���ڹ��UVC����ĳ�ʼͼƬ�����������
    bool inTxInitPic; // ���ڷ��ͳ�ʼͼƬ
    int initPicSent; // ��γ�ʼͼƬ���Ͳ������ѷ��͵�ͼƬ������������log
    uint8_t *initPicBuf; // �洢��ʼͼƬ��buf
    int initPicLen; // ��ʼͼƬ���ֽڳ���

    AicStrmUser strmUserTab[STRM_USER_MAX]; // �洢stream user��table
    size_t strmUserNum; // node number in strmUserTab
}   AicMng;

/**
    Ϊ������ģ���ʹ�ã��Լ�uvc histrm�ӿڵ����ƣ�AicMngȫ��ֻ��һ��ʵ��.
*/
static AicMng g_aic = { 0 };

/*************************************************************************************************
    util
*************************************************************************************************/
#define SP_AIC_UTIL

/**
    ��UVC��size typeת��ΪMPP��.
*/
static PIC_SIZE_E SizeToMpp(int width, int height)
{
    if (width == SIZE_TO_MPP_WIDTH_1920 && height == SIZE_TO_MPP_HEIGHT_1080) {
        return PIC_1080P;
    } else if (width == SIZE_TO_MPP_WIDTH_1280 && height == SIZE_TO_MPP_HEIGHT_720) {
        return PIC_720P;
    } else if (width == SIZE_TO_MPP_WIDTH_640 && height == SIZE_TO_MPP_HEIGHT_360) {
        return PIC_360P;
    } else if (width == SIZE_TO_MPP_WIDTH_3840 && height == SIZE_TO_MPP_HEIGHT_2160) {
        return PIC_3840x2160;
    } else if (width == SIZE_TO_MPP_WIDTH_2592 && height == SIZE_TO_MPP_HEIGHT_1536) {
        return PIC_2592x1536;
    } else if (width == SIZE_TO_MPP_WIDTH_2592 && height == SIZE_TO_MPP_HEIGHT_1944) {
        return PIC_2592x1944;
    } else {
        LOGE("cannot map {%dx%d} to sizeType, regard 1080p\n", width, height);
        return PIC_1080P;
    }
}

/**
    ����AicEvtUser.
*/
static AicEvtUser* AicEvtUserNew(uintptr_t cltId, const char* evtName, AicEvtProc proc, void* user)
{
    AicEvtUser *self = (AicEvtUser*)malloc(sizeof(*self));
    HI_ASSERT(self);
    self->cltId = cltId;
    HiStrxfrm(self->evtName, evtName ? evtName : "", sizeof(self->evtName));
    self->proc = proc;
    self->user = user;
    return self;
}

/**
    ����AiSvcUser.
*/
static AiSvcUser* AiSvcUserNew(uintptr_t cltId, const char* plugUuid, AiResProc proc, void* user)
{
    AiSvcUser *self = (AiSvcUser*)malloc(sizeof(*self));
    HI_ASSERT(self);
    self->cltId = cltId;
    HiStrxfrm(self->plugUuid, plugUuid ? plugUuid : "", sizeof(self->plugUuid));
    self->proc = proc;
    self->user = user;
    return self;
}

/*************************************************************************************************
    MPP session ���ݴ���
*************************************************************************************************/
#define SP_AIC_INNER

/**
    Ϊframe����id.
*/
static inline uint32_t AicGenFrmId(void)
{
    // �滻ԭ����timeRef������venc�������ڣ�vi������timeRefҲ�ᵥ�����������MPP�������
    // �ȼ�2���Ա�������0����mppʵ�������timeRef�������2��ʼ
    while ((g_aic.inFrmIdc += 2) == 0) {}
    return g_aic.inFrmIdc;
}

/**
    ��user�ַ�StrmOn.
*/
static inline void AicDispStrmOn(PAYLOAD_TYPE_E codecType, int outWidth, int outHeight)
{
    if (g_aic.strmUserNum > 0) {
        for (size_t i = 0; i < HI_ARRAY_SIZE(g_aic.strmUserTab); i++) {
            AicStrmUser *node = &g_aic.strmUserTab[i];
            if (node->itf && node->itf->OnStrmOn) {
                node->itf->OnStrmOn(node->user, g_aic.vencChn, codecType, outWidth, outHeight);
            }
        }
    }
}

/**
    ��user�ַ�StrmOff.
*/
static inline void AicDispStrmOff(void)
{
    if (g_aic.strmUserNum > 0) {
        for (size_t i = 0; i < HI_ARRAY_SIZE(g_aic.strmUserTab); i++) {
            AicStrmUser *node = &g_aic.strmUserTab[i];
            if (node->itf && node->itf->OnStrmOff) {
                node->itf->OnStrmOff(node->user);
            }
        }
    }
}

/**
    ��user�ַ�VencFrm.
*/
static inline void AicDispVencFrm(VencFrm* frm)
{
    if (g_aic.strmUserNum > 0) {
        for (size_t i = 0; i < HI_ARRAY_SIZE(g_aic.strmUserTab); i++) {
            AicStrmUser *node = &g_aic.strmUserTab[i];
            if (node->itf && node->itf->OnVencFrm) {
                node->itf->OnVencFrm(node->user, frm);
            }
        }
    }
}

/**
    ��user�ַ�VideoFrm.
*/
static inline void AicDispVideoFrm(VIDEO_FRAME_INFO_S* frm)
{
    if (g_aic.strmUserNum > 0) {
        for (size_t i = 0; i < HI_ARRAY_SIZE(g_aic.strmUserTab); i++) {
            AicStrmUser *node = &g_aic.strmUserTab[i];
            if (node->itf && node->itf->OnVideoFrm) {
                node->itf->OnVideoFrm(node->user, frm);
            }
        }
    }
}

/**
    ����uuid��Ӧ��AI Plug.
*/
static AiPlugInfo* AicFindPlug(const char* uuid)
{
    AiPlugInfo *info = NULL;
    struct list_head *node = NULL;

    HiGlbLock();
    list_for_each(node, &g_aic.plugInfos) {
        info = list_entry(node, AiPlugInfo, lnode);
        if (strcmp(uuid, info->uuid) == 0) {
            HiGlbUnlock();
            return info;
        }
    }
    HiGlbUnlock();
    return NULL;
}

/**
    load plug only.
*/
static int AicLoadPlug(const char* uuid, const AiPlugInfo *info)
{
    HI_ASSERT(!g_aic.workPlug.itf);
    AiPlugLib newPlug;
    int ret;

    // load���
    ret = PmLoadPlugLib(&newPlug, info->path);
    if (ret < 0) {
        LOGE("load plug FAIL, ret=%d\n", ret);
        return ret;
    }

    // loadģ��
    HI_ASSERT(g_aic.osdSet);
    HI_ASSERT(newPlug.itf); // ����newPlug.itf�пմ���
    HI_ASSERT(newPlug.itf->Load);
    ret = newPlug.itf->Load(&newPlug.model, g_aic.osdSet);
    if (ret < 0) {
        LOGE("load plug model FAIL, ret=%d\n", ret);
        PmUnloadPlugLib(&newPlug);
        return ret;
    }
    HI_ASSERT(newPlug.model);

    // ��������workPlug
    HiGlbLock();
    g_aic.workPlug = newPlug;
    HiGlbUnlock();
    return 0;
}

/**
    unload��ǰ������AI plug.
    ��Ҫ�ڼ���plugMutex����ô˺�����
*/
static int AicUnloadPlug(void)
{
    AiPlugLib curPlug; // ���Լ�¼��ǰworking plug
    int ret;

    if (!g_aic.workPlug.itf) {
        return 0;
    }

    // �����Ƴ�workPlug
    HiGlbLock();
    curPlug = g_aic.workPlug;
    if (memset_s(&g_aic.workPlug, sizeof(g_aic.workPlug), 0, sizeof(g_aic.workPlug)) != EOK) {
        HI_ASSERT(0);
    }
    HiGlbUnlock();

    LOGI("AIC: unload plug '%s' ...\n", curPlug.uuid);
    if (curPlug.model && (ret = curPlug.itf->Unload(curPlug.model)) < 0) {
        LOGE("unload plug model FAIL, ret=%d\n", ret);
    }
    ret = PmUnloadPlugLib(&curPlug);
    LOGI("AIC: unload plug done\n");
    return ret;
}

/**
    �������õ�plugs.
    Ŀǰ��ʱֻ֧��һ��plug�ļ��أ����ɹ����ص�һ��plug�󣬺��Ժ�����plug
*/
static int AicPreloadPlugs(void)
{
    char key[SMALL_BUF_SIZE];
    int plugNum = 0;
    int ret;

    // ö��conf����ҪΪ0~REPLUG_ID_MAX���������ȡ��plugUuid�����ز��
    for (int i = 0; i <= PREPLUG_ID_MAX; i++) {
        // ��conf��plug[i]��Ӧ��uuid
        if (snprintf_s(key, sizeof(key), sizeof(key) - 1, "work_plugs:plug%d", i) < 0) {
            HI_ASSERT(0);
        }
        const char* plugUuid = GetCfgStr(key, NULL);
        if (!plugUuid || !*plugUuid) {
            continue;
        }

        // ���ز���� ��ʧ�ܣ���򵥺���
        ret = AicSetAiPlug(plugUuid, NULL, false);
        if (ret) {
            LOGE("load plug '%s' FAIL, ret=%d\n", plugUuid, ret);
            continue;
        }

        // Ŀǰ��ʱֻ֧��һ��plug,���سɹ����˳�
        plugNum++;
        break;
    }
    return plugNum;
}

/**
    ����status�¼��������͸�������.
*/
int AicGenStatusEvt(void)
{
    MemBlk *evt = MemBlkNew2(HUGE_BUF_SIZE, TINY_BUF_SIZE);
    HI_ASSERT(evt);

    int ret = AicGetStatus(evt);
    HI_EXP_GOTO(ret < 0, END, "AicGetStatus FAIL, ret=%d\n", ret);

    struct list_head *node = NULL;
    list_for_each(node, &g_aic.evtUsers) {
        AicEvtUser *user = list_entry(node, AicEvtUser, lnode);
        if (!user->evtName[0] || strcmp(user->evtName, STATUS_EVT_NAME) == 0) {
            HI_ASSERT(user->proc);
            user->proc(user->user, user->cltId, STATUS_EVT_NAME, (char*)MemBlkData(evt), MemBlkLen(evt));
        }
    }
    ret = 0;

    END:
        MemBlkDelete(evt);
        return ret;
}

/**
    ���ļ����س�ʼͼƬ.
*/
static int AicLoadInitPic(void)
{
    const char* initPicFile = GetCfgStr("aic_mng:init_pic_file", INIT_PIC_FILE_DEF);
    LOGI("AIC: load init pic '%s' ...\n", initPicFile);

    HI_ASSERT(!g_aic.initPicBuf);
    long ret = FileLoadToBuf(initPicFile, &g_aic.initPicBuf, true);
    HI_EXP_RET(ret < 0, ret, "AIC: load init pic '%s' FAIL, err=%ld\n", initPicFile, ret);
    HI_ASSERT(ret > 0); // �������0
    g_aic.initPicLen = ret;

    LOGI("AIC: load init pic done, size=%ld\n", ret);
    return ret;
}

/**
    �ͷų�ʼͼƬ.
*/
static void AicFreeInitPic(void)
{
    if (g_aic.initPicBuf) {
        free(g_aic.initPicBuf);
        g_aic.initPicBuf = NULL;
        g_aic.initPicLen = 0;
    }
}

/**
    ���ͳ�ʼͼƬ.
*/
int AicTxInitPic(void)
{
    VDEC_STREAM_S vdecStrm;
    int ret;

    // Ŀǰ��֧����vitypeΪBY_HOSTʱ���ͳ�ʼͼƬ�����Ը�����Ҫ����
    HI_ASSERT(g_aic.viOpenRef > 0 && g_aic.viType == AIC_VIT_HOST);

    // ֻloadһ��file
    if (!g_aic.initPicBuf && AicLoadInitPic() < 0) {
        return -1;
    }

    LOGI("AIC: tx init pic %d ...\n", g_aic.initPicSent + 1);
    HI_ASSERT(g_aic.initPicBuf && g_aic.initPicLen > 0);

    // �ύ��VDEC
    if (memset_s(&vdecStrm, sizeof(vdecStrm), 0, sizeof(vdecStrm)) != EOK) {
        HI_ASSERT(0);
    }

    vdecStrm.u64PTS = 0;
    vdecStrm.pu8Addr = g_aic.initPicBuf;
    vdecStrm.u32Len = g_aic.initPicLen;
    vdecStrm.bEndOfFrame = HI_TRUE;
    vdecStrm.bEndOfStream = HI_FALSE;

    ret = HI_MPI_VDEC_SendStream(g_aic.vdecChn, &vdecStrm, 0);
    if (ret) {
        LOGE("HI_MPI_VDEC_SendStream FAIL, ret=%#x\n", ret);
        return -1;
    }

    g_aic.inTxInitPic = true;
    g_aic.initPicSent++;
    return 0;
}

/**
    ����AiCalJob.
    ���job������AI��������̴߳�����
*/
static void AicAddAiJob(const VIDEO_FRAME_INFO_S* frm)
{
    AiCalSvc *svc = &g_aic.aiCalSvc;
    AiCalJob *job = &g_aic.aiCalJob;

    // Ҫ���ж�AiCalSvc�Ƿ�ready�����Ƿ���job���ڴ���
    if (!svc->evtMon || job->busy) {
        LOGD("discard ai-job for %s\n", (!svc->evtMon) ? "svc-offline" : "in-job");
        int ret = HI_MPI_VPSS_ReleaseChnFrame(g_aic.vpssGrp, g_aic.vpssChn0, frm);
        HI_EXP_LOGE(ret, "HI_MPI_VPSS_ReleaseChnFrame FAIL, err=%#x\n", ret);
        return;
    }

    // ���job
    job->busy = true;
    job->res = -1;
    job->grpId = g_aic.vpssGrp;
    job->chnId = g_aic.vpssChn0;
    job->frm = *frm;
    HI_ASSERT(!job->resJson);
    job->resJson = NULL;
    job->plugUuid[0] = 0;

    // ����AI��������̴߳���
    HI_ASSERT(svc->sigFd >= 0);
    EventFdInc(svc->sigFd);
}

/**
    ִ��AI��������.

    �˺�����AiCalSvc�߳�ִ�С�
    ���ڲ�����Զ�̬����/ж�أ��˺�����ִ��ʱ��Ҫ����plugMutex��

    @param frm [in|out]������������frame����������˽��ͼ�ε�frame
*/
static int AicExecAiJob(AiCalJob* job)
{
    VIDEO_FRAME_INFO_S* frm = &job->frm;
    VIDEO_FRAME_INFO_S srcFrm; // ����plug��Ҫ������frame
    int ret;

    MutexLock(&g_aic.plugMutex);

    // ���ж��Ƿ�����˲����δ���ز���������ģ���ʱ��ʾ����AI���㣬͸��frame
    if (!g_aic.workPlug.itf || !g_aic.workPlug.model) { // no plug loaded
        ret = 0;
        goto END;
    } else {
        HiStrxfrm(job->plugUuid, g_aic.workPlug.uuid, sizeof(job->plugUuid));
    }

    // resize frameΪ�����Ҫ�ĸ�ʽ/���أ�frm���ᱻ�޸�
    HI_ASSERT(g_aic.workPlug.width > 0 && g_aic.workPlug.height > 0);
    ret = MppFrmResize(frm, &srcFrm, g_aic.workPlug.width, g_aic.workPlug.height);
    HI_CHK_GOTO(ret, END, "AIC: for resize FAIL, ret=%d\n", ret);

    // ���ò�����㣬����Ὣ���֡ͨ��resFrm(Ҳ��frm)����
    VIDEO_FRAME_INFO_S *resFrm = frm; // ��ǿ��ͨ��@param frm���resFrm
    ret = g_aic.workPlug.itf->Cal(g_aic.workPlug.model, &srcFrm, resFrm, &job->resJson);
    MppFrmDestroy(&srcFrm);
    HI_EXP_LOGE(ret < 0, "plug cal FAIL, ret=%d\n", ret);

    END:
        MutexUnlock(&g_aic.plugMutex);
        job->res = ret;
        return ret;
}

/**
    AiCalSvc��������AI������������.

    �˺�����AiCalSvc�����߳�ִ�С�
    ���߳�ͨ��job->sigFd֪ͨAiCalSvc�����̡߳�
*/
static void AicOnJobAdd(void* user, int fd, uint32_t evts)
{
    AiCalSvc *svc = &g_aic.aiCalSvc;
    AiCalJob *job = &g_aic.aiCalJob;

    HI_ASSERT(fd == svc->sigFd);
    pthread_t ret = pthread_self();
    pthread_t res = EmThrdId(svc->evtMon);
    HI_ASSERT(ret == res);

    EvtChkRet(evts, FDE_IN, fd);
    LOGV("detect job-add-fd IN, fd=%d\n", fd);
    EventFdClear(svc->sigFd);

    // NOTE: ����Ҳ���ܼ���
    HI_ASSERT(job->busy);
    AicExecAiJob(job);
    HI_ASSERT(job->sigFd >= 0);
    EventFdInc(job->sigFd);
}

/**
    ��������AI�����������.
    �����frame�ύ��VENC���룬�����json���͸�host��
*/
static void AicOnJobEnd(void* user, int fd, uint32_t evts)
{
    AiCalJob *job = &g_aic.aiCalJob;
    int ret;

    HI_ASSERT(fd == job->sigFd);
    EvtChkRet(evts, FDE_IN, evts);
    LOGV("detect job-end-fd IN, fd=%d\n", fd);
    EventFdClear(job->sigFd);

    // ͨ��busy��־������job��add���˺�������Ҳ���ܼ���
    HI_ASSERT(job->busy);

    // ��resFrm�ύ��VENC����
    if (g_aic.vencSess) {
        ret = HI_MPI_VENC_SendFrame(g_aic.vencChn, &job->frm, 0);
        HI_EXP_LOGE(ret, "HI_MPI_VENC_SendFrame failed! ERR=%#x, discard the frm\n", ret);
    } else {
        LOGW("venc discard, for VENC not ready\n");
    }

    // �ύresJson�����ĵ�user, ����resJson��prevResJson��ͬʱ���ύ��user
    if (job->resJson && *job->resJson &&
        (!g_aic.prevResJson || strcmp(g_aic.prevResJson, job->resJson) != 0)) {
        int resLen = strlen(job->resJson);
        struct list_head *node = NULL;
        list_for_each(node, &g_aic.aiSvcUsers) {
            AiSvcUser *user = list_entry(node, AiSvcUser, lnode);
            if (!user->plugUuid[0] || strcmp(user->plugUuid, job->plugUuid) == 0) {
                HI_ASSERT(user->proc);
                user->proc(user->user, user->cltId, job->plugUuid, job->resJson, resLen);
            }
        }
    }

    // ����prevResJson
    // NOTE: ������ܻᱻж�أ����䷵�ص�resJson�ڲ��ж�غ���Ȼ���ã�ֻ�ǲ����__FILE__��
    // NOTE: string�������Ϊ��Ч��resJson��й©��dump��__FILE__ʱ���쳣��
    if (g_aic.prevResJson) {
        free(g_aic.prevResJson);
    }
    g_aic.prevResJson = job->resJson;
    job->resJson = NULL;

    // ��frame�ύ��ע���user
    AicDispVideoFrm(&job->frm);

    ret = HI_MPI_VPSS_ReleaseChnFrame(job->grpId, job->chnId, &job->frm);
    HI_EXP_LOGE(ret, "HI_MPI_VPSS_ReleaseChnFrame FAIL, ret=%#x\n", ret);
    job->busy = false; // ִ���괦�������reset busy��־
}

/**
    ��������VPSS���������(��frame�������).
*/
static void AicOnVpssOut(void* user, int fd, uint32_t evts)
{
    HI_ASSERT(fd == g_aic.vpssFd);
    EvtChkRet(evts, FDE_IN, fd);
    LOGD("detect vpss-fd IN, fd=%d\n", fd);

    VIDEO_FRAME_INFO_S frm;
    int ret;

    // ��VPSS��ȡframe, ��ȡ����frameû��map�����ַ������������Ҫע��
    ret = HI_MPI_VPSS_GetChnFrame(g_aic.vpssGrp, g_aic.vpssChn0, &frm, 0);
    HI_EXP_RET_NONE(ret, "HI_MPI_VPSS_GetChnFrame FAIL, err=%#x, grp=%d, chn=%d\n",
        ret, g_aic.vpssGrp, g_aic.vpssChn0);

    // VOδ����ʱ��discard frame
    if (g_aic.voOpenRef <= 0) {
        LOGD("discard vpss-out frm, for no VO working\n");
        ret = HI_MPI_VPSS_ReleaseChnFrame(g_aic.vpssGrp, g_aic.vpssChn0, &frm);
        HI_EXP_LOGE(ret, "HI_MPI_VPSS_ReleaseChnFrame FAIL, err=%#x\n", ret);
        return;
    }

    // �滻ԭ����timeRef������venc�������ڣ�vi������timeRefҲ�ᵥ�����������MPP �������
    // �ȼ�2���Ա�������0����mppʵ�������timeRef�������2��ʼ
    while ((g_aic.inFrmIdc += 2) == 0) {}
    frm.stVFrame.u32TimeRef = g_aic.inFrmIdc;

    // �����Ż�Ϊ���������Ƿ�����˲����δ����ʱ��ֱ��ͨ��venc���
    AicAddAiJob(&frm);
}

/**
    ������VENC���������(��frame�������).
*/
static void AicOnVencOut(void* user, int fd, uint32_t evts)
{
    HI_ASSERT(fd == g_aic.vencFd);
    EvtChkRet(evts, FDE_IN, fd);
    LOGV("detect venc-fd IN, fd=%d\n", fd);

    VencFrm *vstrm = NULL;
    int ret;

    // ��VENC get frame
    ret = VencGetFrmx(&vstrm, g_aic.vencChn);
    HI_EXP_GOTO(ret, END, "VencGetFrm FAIL, ret=%#x\n", ret);

    // �ȸ�λ�˱�־��user�ڴ�����frameʱ���ܻ����AicInitTxReq�������øñ�־.
    g_aic.inTxInitPic = false;

    // ��frame�ύ��ע���user
    AicDispVencFrm(vstrm);

    END:
        if (vstrm) {
            // �ƶ���VencStrmDefRef����
            VencFrmDelRef(vstrm);
        }
}

/**
    ������status�ı�.
*/
static void AicOnStatusChg(void* user, int fd, uint32_t evts)
{
    HI_ASSERT(fd == g_aic.statusChgFd);
    EvtChkRet(evts, FDE_IN, fd);
    LOGV("detect status-chg IN, fd=%d\n", fd);

    EventFdClear(fd);
    AicGenStatusEvt();
}

/**
    ����AI�������.
*/
static void AicStartAiSvc(void)
{
    // create AlCalJob
    AiCalJob *job = &g_aic.aiCalJob;
    HI_ASSERT(!job->busy);
    HI_ASSERT(job->sigFd < 0);
    job->sigFd = EventFdCreate();
    HI_ASSERT(job->sigFd >= 0);
    if (EmAddFd(g_aic.evtMon, job->sigFd, FDE_IN, AicOnJobEnd, &g_aic) < 0) {
        HI_ASSERT(0);
    }
    HI_ASSERT(!job->resJson);

    // create AlCalSvc
    AiCalSvc *svc = &g_aic.aiCalSvc;
    HI_ASSERT(!svc->evtMon);
    HI_ASSERT(svc->sigFd < 0);
    svc->sigFd = EventFdCreate();
    HI_ASSERT(svc->sigFd >= 0);

    // start AlCalSvc
    if (EmCreate(&svc->evtMon) < 0) {
        HI_ASSERT(0);
    }
    if (EmAddFd(svc->evtMon, svc->sigFd, FDE_IN, AicOnJobAdd, &g_aic) < 0) {
        HI_ASSERT(0);
    }
    EmStart(svc->evtMon); // ��ᴴ����̨�̣߳�����ʼ����
}

/**
    ��ֹAI�������.
*/
static void AicStopAiSvc(void)
{
    // stop and destroy AlCalSvc ...
    AiCalSvc *svc = &g_aic.aiCalSvc;
    EmStop(svc->evtMon); // ���������ֹ���ȴ���̨�߳̽���

    // NOTE: ��Ҫ��stop����DelFd
    HI_ASSERT(svc->sigFd >= 0);
    if (EmDelFd(svc->evtMon, svc->sigFd) < 0) {
        HI_ASSERT(0);
    }
    if (close(svc->sigFd) < 0) {
        HI_ASSERT(0);
    }
    svc->sigFd = -1;

    EmDestroy(svc->evtMon);
    svc->evtMon = NULL;

    // destroy AlCalJob ...
    AiCalJob *job = &g_aic.aiCalJob;

    // destroyδִ�е�AiCalJob
    if (job->busy) {
        int ret = HI_MPI_VPSS_ReleaseChnFrame(job->grpId, job->chnId, &job->frm);
        if (ret != HI_SUCCESS) {
            LOGE("HI_MPI_VPSS_ReleaseChnFrame FAIL, ret=%#x\n", ret);
        }
        job->busy = false;
    }

    HI_ASSERT(job->sigFd >= 0);
    if (EmDelFd(g_aic.evtMon, job->sigFd) < 0) {
        HI_ASSERT(0);
    }
    if (close(job->sigFd) < 0) {
        HI_ASSERT(0);
    }
    job->sigFd = -1;

    job->res = -1;
    job->grpId = -1;
    job->chnId = -1;
    if (memset_s(&job->frm, sizeof(job->frm), 0, sizeof(job->frm)) != EOK) {
        HI_ASSERT(0);
    }

    // �п��ܵ�ǰ���Ѿ���ɵ�AI���㣬�����̻߳�δ���ü���������Ҫ�ͷ���Ӧ��Դ
    if (job->resJson) {
        LOGW("free job.resJson for it not proc by main thrd\n");
        free(job->resJson);
        job->resJson = NULL;
    }
}

/**
    �����������.
    ������Ҫ���й�������Ĳ���������������
    ���ز��������������
    plug.startSvcҲ����������ṩ�˹�������Ĳ�����Լ�������Դ��
*/
static int AicStartPlugSvcs(void)
{
    struct list_head *node = NULL;
    AiPlugLib *plug = NULL;
    int svcNum = 0;
    int ret;

    list_for_each(node, &g_aic.plugInfos) {
        AiPlugInfo *info = list_entry(node, AiPlugInfo, lnode);
        if (!info->mngSvc) {
            continue;
        }

        LOGI("AIC: start mng svc of plug '%s' ...\n", info->uuid);

        // ���ز��
        plug = (AiPlugLib*)malloc(sizeof(*plug));
        HI_ASSERT(plug);
        ret = PmLoadPlugLib(plug, info->path);
        if (ret < 0) {
            LOGE("load ai plug '%s' FAIL, ret=%d\n", info->uuid, ret);
            free(plug);
            plug = NULL; // ensure by meeting
            continue;
        }

        // ��������
        HI_ASSERT(plug->itf && plug->itf->StartSvc);
        ret = plug->itf->StartSvc();
        if (ret != 0) {
            LOGE("start plug svc of '%s' FAIL, ignore, ret=%d\n", info->uuid, ret);
            PmUnloadPlugLib(plug);
            free(plug);
            plug = NULL; // ensure by meeting
            continue;
        }

        // ���ӵ�svcPlus�У�Ŀǰ�ǲ���Ҫ��������
        list_add_tail(&plug->lnode, &g_aic.svcPlugs);
        svcNum++;
    }
    return svcNum;
}

/**
    ��ֹ�������Ĳ������.
*/
static int AicStopPlugSvcs(void)
{
    struct list_head *node = NULL;
    struct list_head *next = NULL;
    int svcNum = 0;
    int ret;

    // ��svcInfos�м�¼�����в���ķ�����ֹ����ж�ز��
    list_for_each_safe(node, next, &g_aic.svcPlugs) {
        AiPlugLib *plug = list_entry(node, AiPlugLib, lnode);
        LOGI("AIC: stop mng svc of plug '%s' ...\n", plug->uuid);

        // ��ֹ����
        HI_ASSERT(plug->itf && plug->itf->StopSvc);
        ret = plug->itf->StopSvc();
        if (ret != 0) {
            LOGE("stop plug svc of '%s' FAIL, ignore, ret=%d\n", plug->uuid, ret);
        }

        // ��svcPlus��ɾ����Ŀǰ�ǲ���Ҫ��������
        list_del(&plug->lnode);

        // ж�ز��
        PmUnloadPlugLib(plug);
        free(plug);
        svcNum++;
    }
    return svcNum;
}

/**
    ��ȡVPSS��out chn fd��������֮.
*/
static void AicMonVpss(void)
{
    HI_ASSERT(g_aic.vpssFd < 0);
    g_aic.vpssFd = HI_MPI_VPSS_GetChnFd(g_aic.vpssGrp, g_aic.vpssChn0);
    if (g_aic.vpssFd < 0) {
        LOGE("HI_MPI_VPSS_GetChnFd, ret=%x\n", g_aic.vpssFd);
    } else {
        if (EmAddFd(g_aic.evtMon, g_aic.vpssFd, FDE_IN | FDE_ET, AicOnVpssOut, NULL) < 0) {
            HI_ASSERT(0);
        }
    }
}

/**
    ������VPSS��out chn�ļ���.
*/
static void AicUnmonVpss(void)
{
    if (g_aic.vpssFd < 0) {
        return;
    }

    if (EmDelFd(g_aic.evtMon, g_aic.vpssFd) < 0) {
        HI_ASSERT(0);
    }
    int ret = HI_MPI_VPSS_CloseFd();
    if (ret != 0) {
        LOGE("close VPSS fd FAIL, err=%#x\n", ret);
    }
    g_aic.vpssFd = -1;
}

/**
    ��ȡVENC��out chn fd��������֮.
*/
static void AicMonVenc(void)
{
    HI_ASSERT(g_aic.vencFd < 0);
    g_aic.vencFd = HI_MPI_VENC_GetFd(g_aic.vencChn);
    if (g_aic.vencFd < 0) {
        LOGE("HI_MPI_VENC_GetFd FAIL, ret=%x\n", g_aic.vencFd);
    } else {
        if (EmAddFd(g_aic.evtMon, g_aic.vencFd, FDE_IN | FDE_ET, AicOnVencOut, NULL) < 0) {
            HI_ASSERT(0);
        }
    }
}

/**
    ������VENC��out chn�ļ���.
*/
static void AicUnmonVenc(void)
{
    if (g_aic.vencFd < 0) {
        return;
    }

    if (EmDelFd(g_aic.evtMon, g_aic.vencFd) < 0) {
        HI_ASSERT(0);
    }
    int ret = HI_MPI_VENC_CloseFd(g_aic.vencChn);
    if (ret) {
        LOGE("close VENC fd FAIL, err=%#x\n", ret);
    }
    g_aic.vencFd = -1;
}

/**
    create VI(SENSOR)+VPSS session.

    NOTE: MPP�е�VIָ����SENSOR��video input����HiOpenais�е�vi��һ�����졣
    HiOpenais��vi������������SENSOR���롣
*/
static HI_S32 AicCreateSnsVpss(int outWidth, int outHeight, PIXEL_FORMAT_E pixFormat)
{
    LOGI("start sensor->vpss, size={%dx%d}\n", outWidth, outHeight);
    VPSS_GRP vpssGrp = AIC_VPSS_GRP;
    VPSS_CHN vpssChn = AIC_VPSS_ZOUT_CHN; // ��Сͨ��
    int ret;

    // VI config
    ViCfgInit(&g_aic.viCfg);
    ViCfgSetDev(&g_aic.viCfg, 0, -1);
    ViCfgSetPipe(&g_aic.viCfg, 0, -1, -1, -1, -1);
    ViCfgSetChn(&g_aic.viCfg, 0, pixFormat, -1, -1, COMPRESS_MODE_SEG);

    // VPSS config
    SIZE_S snsMaxSize = CurSnsMaxSize(ViCfgSnsType(&g_aic.viCfg));
    LOGI("AIC: snsMaxSize=%ux%u\n", snsMaxSize.u32Width, snsMaxSize.u32Height);
    VpssCfgInit(&g_aic.vpssCfg);
    VpssCfgSetGrp(&g_aic.vpssCfg, vpssGrp, NULL, snsMaxSize.u32Width, snsMaxSize.u32Height);
    g_aic.vpssCfg.grpAttr.enPixelFormat = pixFormat;
    VpssCfgAddChn(&g_aic.vpssCfg, vpssChn, NULL, outWidth, outHeight);

    HI_ASSERT(!g_aic.viSess);
    ret = ViVpssCreate(&g_aic.viSess, &g_aic.viCfg, &g_aic.vpssCfg);
    if (ret != 0) {
        LOGE("ViVpss Sess create FAIL, ret=%#x\n", ret);
        return ret;
    }

    const char* sceneParamFile = GetCfgStr("aic_mng:scene_param_file", SCENE_PARAM_FILE_DEF);
    ret = SceneInit(sceneParamFile);
    if (ret != 0) {
        LOGE("SceneInit FAIL, ret=%#x\n", ret);
        MppSessDestroy(g_aic.viSess);
        g_aic.viSess = NULL;
        return ret;
    }

    g_aic.vpssGrp = vpssGrp;
    g_aic.vpssChn0 = vpssChn;
    return 0;
}

/**
    create VDEC+VPSS session.
*/
HI_S32 AicCreateVdecVpss(int outWidth, int outHeight, PIXEL_FORMAT_E pixFormat)
{
    VPSS_GRP vpssGrp = AIC_VPSS_GRP;
    VPSS_CHN vpssChn = AIC_VPSS_ZIN_CHN; // �Ŵ�ͨ��
    int ret;

    // VPSS config
    VpssCfgInit(&g_aic.vpssCfg);
    VpssCfgSetGrp(&g_aic.vpssCfg, vpssGrp, NULL, VDEC_WIDTH_MAX, VDEC_HEIGHT_MAX);
    g_aic.vpssCfg.grpAttr.enPixelFormat = pixFormat;
    g_aic.vpssCfg.grpAttr.bNrEn = HI_FALSE;
    VpssCfgAddChn(&g_aic.vpssCfg, vpssChn, NULL, outWidth, outHeight);

    // ����MppSess
    HI_ASSERT(!g_aic.viSess);
    ret = VdecVpssCreate(&g_aic.viSess, &g_aic.vdecCfg, &g_aic.vpssCfg, true);
    HI_CHK_RET(ret, "VdecVpss Sess create FAIL, ret=%#x\n", ret);

    g_aic.vpssGrp = vpssGrp;
    g_aic.vpssChn0 = vpssChn;
    return 0;
}

/**
    create VENC session.
*/
static int AicCreateVenc(PAYLOAD_TYPE_E codecType, int width, int height)
{
    VENC_GOP_MODE_E enGopMode = VENC_GOPMODE_NORMALP;
    SAMPLE_RC_E enRcMode = SAMPLE_RC_CBR;
    PIC_SIZE_E sizeType;
    int ret;

    sizeType = SizeToMpp(width, height);
    if (codecType == PT_H264 || codecType == PT_H265) {
        enRcMode = SAMPLE_RC_AVBR;
    }

    VencCfgInit(&g_aic.vencCfg, g_aic.vencChn, sizeType, codecType, enGopMode, enRcMode);
    HI_ASSERT(!g_aic.vencSess);
    ret = VencCreate(&g_aic.vencSess, &g_aic.vencCfg);
    HI_CHK_RET(ret, "VencCreate FAIL, ret=%#x\n", ret);
    return 0;
}

/**
    start vi.
*/
int AicStartVi(AicViType viType, bool byHost,
    int outWidth, int outHeight, PIXEL_FORMAT_E pixFormat)
{
    HI_ASSERT(viType > AIC_VIT_NONE && viType < AIC_VIT_BUTT);
    int ret;

    pixFormat = (int)pixFormat < 0 ? PIXEL_FORMAT_YVU_SEMIPLANAR_420 : pixFormat;

    if (g_aic.viOpenRef > 0) {
        LOGW("AIC: start VI{%d} ignore, for viOpenRef=%d\n", viType, g_aic.viOpenRef);
        g_aic.viOpenRef++;
        return 0;
    }

    // create vi+vpss sess
    if (viType == AIC_VIT_SENSOR) {
        ret = AicCreateSnsVpss(outWidth, outHeight, pixFormat);
    } else if (viType == AIC_VIT_HOST) {
        ret = AicCreateVdecVpss(outWidth, outHeight, pixFormat);
    } else {
        HI_ASSERT(0);
        ret = -1;
    }
    if (ret != 0) {
        g_aic.viOpenRef = 0;
        return ret;
    }

    g_aic.viOpenRef = 1;
    g_aic.viType = viType;
    g_aic.viByHost = byHost;
    g_aic.rxPicNum = 0; // ʵ�ʽ���VIT_HOST������

    AicMonVpss();
    AicStartAiSvc(); // ai service��VI����

    // ��vi������ʼ��ͳ����Ϣ
    if (memset_s(&g_aic.piriodStatis, sizeof(g_aic.piriodStatis), 0, sizeof(g_aic.piriodStatis)) != EOK) {
        HI_ASSERT(0);
    }
    g_aic.piriodStatis.begTime = HiClockMs();

    EventFdInc(g_aic.statusChgFd); // ֪ͨstatus���
    return 0;
}

/**
    stop vi.
*/
int AicStopVi(AicViType viType, bool byHost)
{
    HI_ASSERT(viType > AIC_VIT_NONE && viType < AIC_VIT_BUTT);

    if (g_aic.viOpenRef <= 0) {
        LOGW("AIC: stop VO{%x} ignore, for no vi opened\n", viType);
        return 0;
    }
    if (--g_aic.viOpenRef > 0) {
        LOGW("AIC: stop VO{%x} ignore, for viOpenRef=%d\n", viType, g_aic.voOpenRef);
        return 0;
    }

    // ��ֹVI
    LOGI("AIC: stop VI{%d} ...\n", g_aic.viType);
    HI_ASSERT(g_aic.viType > 0);

    // Ҫ����ֹai���������destroy MppSess����Ϊ��Ҫ��destroy��MppSess�л�õ�buf
    AicStopAiSvc();

    AicUnmonVpss();
    if (g_aic.viType == AIC_VIT_SENSOR) {
        SceneExit();
    }
    MppSessDestroy(g_aic.viSess);
    g_aic.viSess = NULL;

    g_aic.vpssGrp = -1;
    g_aic.vpssChn0 = -1;
    g_aic.vpssChn1 = -1;

    g_aic.viType = AIC_VIT_NONE;
    g_aic.viByHost = false;

    EventFdInc(g_aic.statusChgFd); // ֪ͨstatus���
    LOGI("AIC: stop VI{%d} done\n", g_aic.viType);
    return 0;
}

/**
    start vo.
*/
int AicStartVo(uint32_t voType, int outWidth, int outHeight, PAYLOAD_TYPE_E codecType)
{
    HI_ASSERT(voType == AIC_VOT_UVC || voType == AIC_VOT_IP || voType == AIC_VOT_RTSPD);

    // ֧�ֶ��VO����
    if (g_aic.voOpenRef > 0) {
        LOGW("AIC: start VO{%x} ignore, for VO{%x} exist\n", voType, g_aic.voTypes);
        g_aic.voTypes |= voType;
        g_aic.voOpenRef++;
        EventFdInc(g_aic.statusChgFd); // ֪ͨstatus���
        return 0;
    }

    // ����VENC
    LOGI("AIC: start VENC ...\n");
    HI_ASSERT(!g_aic.vencSess);

    int ret = AicCreateVenc(codecType, outWidth, outHeight);
    HI_CHK_RET(ret, "create VencSess FAIL, ret=%#x\n", ret);
    g_aic.voTypes |= voType;
    g_aic.inFrmIdc = 0; // �ɲ���
    g_aic.voOpenRef = 1;

    AicMonVenc(); // ��ȡVENC��out chn fd��������֮
    EventFdInc(g_aic.statusChgFd); // ֪ͨstatus���

    // ֪ͨע���user
    AicDispStrmOn(codecType, outWidth, outHeight);

    LOGI("AIC: start VENC done\n");
    return 0;
}

/**
    stop vo.
*/
int AicStopVo(uint32_t voType)
{
    HI_ASSERT(voType == AIC_VOT_UVC || voType == AIC_VOT_IP || voType == AIC_VOT_RTSPD);

    if (g_aic.voOpenRef <= 0) {
        LOGW("AIC: stop VO{%x} ignore, for voOpenRef=%d\n", voType, g_aic.voOpenRef);
        return 0;
    }

    g_aic.voTypes &= ~voType;
    if (--g_aic.voOpenRef > 0) {
        LOGW("AIC: stop VO{%x} ignore, for voOpenRef=%d\n", voType, g_aic.voOpenRef);
        EventFdInc(g_aic.statusChgFd); // ֪ͨstatus���
        return 0;
    }

    // ��ֹVO
    LOGI("AIC: stop VO{%x} ...\n", g_aic.voTypes);
    HI_ASSERT(g_aic.vencSess);
    HI_ASSERT(g_aic.vencFd >= 0);

    AicUnmonVenc();
    MppSessDestroy(g_aic.vencSess);
    g_aic.vencSess = NULL;
    g_aic.inFrmIdc = 0;

    EventFdInc(g_aic.statusChgFd); // ֪ͨstatus���

    // ���ͷţ�������UVC��������
    // ��Ҫ��λ�˱�־���������û�л��Ḵλ��, ��ǰ���ڷ���init picҲ�޷�
    g_aic.inTxInitPic = false;
    g_aic.initPicSent = 0;

    // ֪ͨע���user
    AicDispStrmOff();

    LOGI("AIC: stop VO{%x} done\n", voType);
    return 0;
}

/**
    stop ���е�vi, vo.
    NOTE: ��stop AIC_VOT_UVC������UVC����.
*/
int AicStopVios(bool byHost)
{
    int ret = 0;

    if (g_aic.viType && AicStopVi(g_aic.viType, byHost) < 0) {
        ret = -1;
    }
    if ((g_aic.voTypes & AIC_VOT_IP) && AicStopVo(AIC_VOT_IP) < 0) {
        ret = -1;
    }
    return ret;
}

/**
    AicMng constructor.
*/
static void AicCreate(AicMng* self)
{
    self->inited = true;

    self->evtMon = MainEvtMon();
    self->vpssFd = -1;
    self->vencFd = -1;
    self->vpssGrp = -1;
    self->vpssChn0 = -1;
    self->vpssChn1 = -1;
    self->vdecChn = AIC_VDEC_CHN; // vdecChn�ǹ̶���
    self->vencChn = AIC_VENC_CHN; // vencChn�ǹ̶���
    self->osdSet = NULL;
    self->viSess = NULL;
    self->vencSess = NULL;
    self->viType = AIC_VIT_NONE;
    self->voTypes = 0;
    self->viByHost = false;
    self->viOpenRef = 0;
    self->voOpenRef = 0;

    VdecCfgInit(&self->vdecCfg, g_aic.vdecChn, PT_MJPEG, VIDEO_MODE_FRAME, VB_SOURCE_MODULE, NULL);
    self->vdecCfg.fps = VDEC_FPS_DEF;

    self->inFrmIdc = 0;
    self->aiCalSvc.evtMon = NULL;
    self->aiCalSvc.sigFd = -1;

    self->aiCalJob.busy = false;
    self->aiCalJob.sigFd = -1;
    self->aiCalJob.res = -1;
    self->aiCalJob.grpId = -1;
    self->aiCalJob.chnId = -1;
    self->aiCalJob.resJson = NULL;
    self->prevResJson = NULL;
    RecurMutexInit(&self->plugMutex);

    INIT_LIST_HEAD(&self->plugInfos);
    INIT_LIST_HEAD(&self->svcPlugs);
    INIT_LIST_HEAD(&self->aiSvcUsers);
    INIT_LIST_HEAD(&self->evtUsers);

    self->statusChgFd = EventFdCreate();
    HI_ASSERT(self->statusChgFd >= 0);
    if (EmAddFd(self->evtMon, self->statusChgFd, FDE_IN, AicOnStatusChg, NULL) < 0) {
        HI_ASSERT(0);
    }

    self->blkCallNum = 0;

    self->inTxInitPic = false;
    self->initPicSent = 0;
    self->initPicBuf = NULL;
    self->initPicLen = 0;
}

/**
    AicMng destructor.
*/
static void AicDestroy(AicMng* self)
{
    struct list_head *node = NULL;
    struct list_head *next = NULL;

    self->inited = false;

    list_for_each_safe(node, next, &self->plugInfos) {
        AiPlugInfoDelete(list_entry(node, AiPlugInfo, lnode));
    }
    int ret = list_empty(&self->svcPlugs);
    HI_ASSERT(ret);

    if (EmDelFd(self->evtMon, self->statusChgFd) < 0) {
        HI_ASSERT(0);
    }
    self->statusChgFd = -1;

    HI_ASSERT(!self->workPlug.itf);
    HI_ASSERT(!self->aiCalSvc.evtMon);
    HI_ASSERT(self->aiCalJob.sigFd < 0);
    HI_ASSERT(!self->aiCalJob.resJson);

    if (self->prevResJson) {
        free(self->prevResJson);
        self->prevResJson = NULL;
    }

    HI_EXP_LOGE(!list_empty(&self->evtUsers), "%d evtUsers leak\n", list_size(&self->evtUsers));
    while (!list_empty(&self->evtUsers)) {
        AicEvtUser *user = list_entry(self->evtUsers.next, AicEvtUser, lnode);
        free(user);
    }

    HI_EXP_LOGE(!list_empty(&self->aiSvcUsers), "%d aiSvcUsers leak\n", list_size(&self->aiSvcUsers));
    while (!list_empty(&self->aiSvcUsers)) {
        AiSvcUser *user = list_entry(self->aiSvcUsers.next, AiSvcUser, lnode);
        free(user);
    }

    HI_ASSERT(!self->viSess);
    HI_ASSERT(!self->vencSess);

    AicFreeInitPic();
    MutexDestroy(&self->plugMutex);

    if (memset_s(self, sizeof(*self), 0, sizeof(*self)) != EOK) {
        HI_ASSERT(0);
    }
}

/*************************************************************************************************
    ��Ҫֱ�Ӷ���ӿ�
*************************************************************************************************/
#define SP_AIC_API

/**
    ��ʼ��.
*/
void AicInit(void)
{
    HI_ASSERT(!g_aic.inited);

    AicCreate(&g_aic);
    g_aic.initing = true;

    // ������venc������OsdSet����AiCalJobʹ��
    HI_ASSERT(!g_aic.osdSet);
    g_aic.osdSet = OsdsCreate(HI_OSD_BINDMOD_VENC, 0, g_aic.vencChn);
    HI_ASSERT(g_aic.osdSet);

    // ��ʼ��ʱɨ��һ�β��
    const char* plugDir = GetCfgStr("aic_mng:plug_dir", PLUG_DIR_DEF);
    PmScanPlug(plugDir, &g_aic.plugInfos);
    LOGI("AIC: detect %d plugs\n", list_size(&g_aic.plugInfos));

    // ������Ҫ������������Ĳ���������������
    if (GetCfgBool("aic_mng:start_plug_svc", false)) {
        AicStartPlugSvcs();
    }

    // �������õ�plugs����˺���������app��ʼ��ʱִ�У����ﲻ������ʱ�߳�ִ�м���
    AicPreloadPlugs();
    g_aic.initing = false;
}

/**
    ȥ��ʼ��.
*/
void AicExit(void)
{
    bool ret = IsMainThrd();
    HI_ASSERT(ret);
    HI_ASSERT(g_aic.inited);
    HI_ASSERT(!g_aic.blkCallNum); // ���ܻ��к�ʱ������ִ�У���APPȷ��

    // �ֶ���ֹvi/vo����ֹviʱ����ֹAiCalSvc
    if (g_aic.viType) {
        AicStopVi(g_aic.viType, g_aic.viByHost);
    }
    if (g_aic.voTypes) {
        AicStopVo(g_aic.voTypes);
    }

    // ж�ص�ǰ��������ں�̨�߳̾�����ֹ�����ﲻ����unload���Ҳû������
    MutexLock(&g_aic.plugMutex);
    AicUnloadPlug();
    MutexUnlock(&g_aic.plugMutex);

    // ��ֹ����Ĺ������񣬲�ж������
    AicStopPlugSvcs();

    // destroy OsdSet
    HI_ASSERT(g_aic.osdSet);
    OsdsClear(g_aic.osdSet);
    OsdsDestroy(g_aic.osdSet);

    AicDestroy(&g_aic);
}

/**
    �Ƿ���UVC strmģʽ.
*/
bool AicInStrm(void)
{
    bool ret = IsMainThrd();
    HI_ASSERT(ret);
    HI_ASSERT(g_aic.inited);
    return (g_aic.inited && (g_aic.voTypes & AIC_VOT_UVC));
}

/**
    submit 1 jpg.
*/
int AicSubmitJpg(uint8_t* data, int len, bool endOfStrm)
{
    bool res = IsMainThrd();
    HI_ASSERT(res);
    HI_ASSERT(g_aic.inited);
    HI_ASSERT(data);
    VDEC_STREAM_S vdecStrm;
    HI_S32 ret;

    g_aic.rxPicNum++;

    if (g_aic.viType != AIC_VIT_HOST || g_aic.inTxInitPic) {
        if (g_aic.rxPicNum > VDEC_FPS_DEF) { // �����������log����discard pic�����ϴ�ʱ��log���൱��ÿ��logһ��
            LOGW("dicard host jpeg, for vdec not ready\n");
        }
        ret = -1;
        goto END;
    }

    if (memset_s(&vdecStrm, sizeof(vdecStrm), 0, sizeof(vdecStrm)) != EOK) {
        HI_ASSERT(0);
    }
    vdecStrm.u64PTS = 0;
    vdecStrm.pu8Addr = data;
    vdecStrm.u32Len = len;
    vdecStrm.bEndOfFrame = HI_TRUE;
    vdecStrm.bEndOfStream = endOfStrm ? HI_TRUE : HI_FALSE;

    ret = HI_MPI_VDEC_SendStream(g_aic.vdecChn, &vdecStrm, 0);
    if (ret != HI_SUCCESS) {
        LOGE("HI_MPI_VDEC_SendStream FAIL, ret=%#x\n", ret);
    }

    END:
        return ret;
}

/**
    get vi type.
*/
AicViType AicGetViType(void)
{
    bool ret = IsMainThrd();
    HI_ASSERT(ret);
    HI_ASSERT(g_aic.inited);
    return g_aic.viType;
}

/**
    get vo type.
*/
uint32_t AicGetVoTypes(void)
{
    bool ret = IsMainThrd();
    HI_ASSERT(ret);
    HI_ASSERT(g_aic.inited);
    return g_aic.voTypes;
}

/**
    ����AI���.
*/
int AicSetAiPlug(const char* uuid, AiPlugLib* plugLib, bool reload)
{
    uuid = uuid ? uuid : "";
    AiPlugInfo *info = NULL;
    int ret;

    __sync_add_and_fetch(&g_aic.blkCallNum, 1);
    MutexLock(&g_aic.plugMutex);

    const char* curUuid = g_aic.workPlug.uuid ? g_aic.workPlug.uuid : "null";
    const char* curDesc = g_aic.workPlug.desc ? g_aic.workPlug.desc : "null";

    if (!*uuid) { // uuidΪnull��ʾunload����
        LOGI("AIC: set plug {'%s', '%s'} => null ...\n", curUuid, curDesc);
    } else { // load / unload+load����
        // ����plug, �ҵ���˵����������load����
        info = AicFindPlug(uuid);
        HI_EXP_CMD_GOTO(!info, (ret = -1), END, "set plug FAIL, for plug '%s' not found\n", uuid);
        LOGI("AIC: set plug {'%s', '%s'} => {'%s', '%s'} ...\n", curUuid, curDesc, info->uuid, info->desc);
    }

    // ���Ҫload�Ĳ���Ѿ���load����@param reloadΪfalse������Ը�����
    bool loaded = g_aic.workPlug.itf && g_aic.workPlug.uuid && strcmp(uuid, g_aic.workPlug.uuid) == 0;
    if (loaded && !reload) {
        LOGW("load plug '%s' ignore, for it loaded\n", uuid);
        ret = 0;
        goto END;
    }

    // �������Ϊunload������ǰû�в����load������Ը�����
    if (!*uuid && !g_aic.workPlug.itf) {
        LOGW("unload plug ignore, for no working plug\n");
        ret = 0;
        goto END;
    }

    // ��unload��ǰ���
    if (g_aic.workPlug.itf) {
        AicUnloadPlug();
    }

    // û��ָ��uuid��˵��ֻ��unload���
    ret = *uuid ? AicLoadPlug(uuid, info) : 0;

    END:
        if (plugLib) {
            if (ret >= 0) {
                *plugLib = g_aic.workPlug;
            } else if (memset_s(plugLib, sizeof(*plugLib), 0, sizeof(*plugLib)) != EOK) {
                HI_ASSERT(0);
            }
            INIT_LIST_HEAD(&plugLib->lnode);
        }

        // ָʾstatus������˺����ɺ�̨�߳�ִ�У�����ֱ�ӵ���AicGenStatusEvt
        if (!g_aic.initing) {
            EventFdInc(g_aic.statusChgFd);
        }

        LOGI("AIC: set plug '%s' %s\n", uuid, (ret < 0 ? "FAIL" : "done"));
        MutexUnlock(&g_aic.plugMutex);
        __sync_sub_and_fetch(&g_aic.blkCallNum, 1);
        return ret;
}

/**
    �б�ai�����Ϣ.
*/
int AicListAiPlugs(MemBlk** resJson, int rsvSize)
{
    bool ret = IsMainThrd();
    HI_ASSERT(ret);
    HI_ASSERT(g_aic.inited);
    HI_ASSERT(resJson);
    *resJson = NULL;
    AiPlugInfo *info = NULL;
    struct list_head *node = NULL;
    size_t size = 0;
    int n;

    if (list_empty(&g_aic.plugInfos)) {
        // ��Ȼ��������Ĵ����������ʱret=0����ʾû�в��
        LOGW("list ai plugs with 0 plug\n");
    }

    // �������json����ռ�
    size += rsvSize > 0 ? rsvSize : 0;
    size += TINY_BUF_SIZE; // header, ʵ��24�͹���
    n = 0;
    list_for_each(node, &g_aic.plugInfos) {
        info = list_entry(node, AiPlugInfo, lnode);
        HI_ASSERT(info->prof);
        size += strlen(info->prof);
        size += TINY_BUF_SIZE; // ���Ϊ��jsonʱ�Ķ��⿪����ʵ��16�͹���
        n++;
    }

    HI_ASSERT(size > 0);
    *resJson = MemBlkNew2(size, rsvSize);
    HI_ASSERT(*resJson);
    char *buf = (char*)(*resJson)->data;

    // ���Ϊ��json
    int len = (*resJson)->len;
    len += snprintf_s(buf + len, size - len, size - len - 1, "[\n");
    n = 0;
    list_for_each(node, &g_aic.plugInfos) {
        info = list_entry(node, AiPlugInfo, lnode);
        HI_ASSERT(info->prof);

        if (n > 0) {
            len += snprintf_s(buf + len, size - len, size - len - 1, ",\n");
            HI_ASSERT(len < size);
        }
        n++;
        len += snprintf_s(buf + len, size - len, size - len - 1, "%s", info->prof);
        HI_ASSERT(len < size);
    }
    len += snprintf_s(buf + len, size - len, size - len - 1, "\n]");

    HI_ASSERT(len < size);
    (*resJson)->len = len;
    return n; // number of plugs
}

/**
    �б�working plugs.
*/
int AicListWorkPlugs(struct list_head* plugInfos)
{
    bool ret = IsMainThrd();
    HI_ASSERT(ret);
    HI_ASSERT(g_aic.inited);
    HI_ASSERT(plugInfos);
    int num = 0;

    // ��С����������workPlugs�ķ��ʣ���������plugMutex������˺������ʱ������userʹ���Ѷ�
    HiGlbLock();

    if (g_aic.workPlug.itf) {
        HI_ASSERT(g_aic.workPlug.uuid && *g_aic.workPlug.uuid);
        AiPlugInfo *info = AicFindPlug(g_aic.workPlug.uuid);
        HI_ASSERT(info); // working plug����Ϣ��Ȼ������

        // cloneһ�ݣ������ӵ�plugInfos��
        info = AiPlugInfoClone(info);
        list_add_tail(&info->lnode, plugInfos);
        num++;
    }

    HiGlbUnlock();
    return num;
}

/**
    ����AI�������¼�.
*/
int AicSubsAiSvc(uintptr_t cltId, const char* plugUuid, AiResProc proc, void* user)
{
    bool res = IsMainThrd();
    HI_ASSERT(res);
    HI_ASSERT(g_aic.inited);
    int ret = -1;

    // δָ��plugUuid��ʾ��������plugs�ķ���
    if (!plugUuid || !*plugUuid) {
        AiSvcUser *svcUser = AiSvcUserNew(cltId, "", proc, user); // plugUuid��Ϊnull,ensure by xiao
        list_add_tail(&svcUser->lnode, &g_aic.aiSvcUsers);
        return 0;
    }

    // ��С����������workPlugs�ķ��ʣ���������plugMutex������˺������ʱ������userʹ���Ѷ�
    HiGlbLock();

    // �ж�plugUuid��Ӧ�Ĳ���Ƿ���working״̬
    if (!g_aic.workPlug.itf) {
        LOGE("subs ai svc FAIL, for no plug in working\n");
        goto END;
    }
    HI_ASSERT(g_aic.workPlug.uuid && *g_aic.workPlug.uuid);
    if (strcmp(g_aic.workPlug.uuid, plugUuid) != 0) {
        LOGE("subs ai svc FAIL, for plug '%s' not in working\n", plugUuid);
        goto END;
    }

    // ����AiSvcUser�����ӵ�aiSvcUsers��
    // ��Ȼ������ִ����Ϊ��ȷ��ע���ڼ�workPlug��������
    // �Ա��ڽ���֧��workPlug���ʱ��ע��״̬��֮����ͬ��
    AiSvcUser *svcUser = AiSvcUserNew(cltId, plugUuid, proc, user);
    list_add_tail(&svcUser->lnode, &g_aic.aiSvcUsers);
    ret = 0;

    END:
        HiGlbUnlock();
        return ret;
}

/**
    ȥ����AI�������¼�.
    ��Ϊֻ�����̲߳Ż�ִ�ж�aiSvcUsers���޸ģ������ﲻ����workPlug������������.
*/
int AicUnsubsAiSvc(uintptr_t cltId, const char* plugUuid)
{
    bool ret = IsMainThrd();
    HI_ASSERT(ret);
    struct list_head *node = NULL;
    struct list_head *next = NULL;

    // ƥ����˳�ѭ�������ݴ�user���ظ�����
    list_for_each_safe(node, next, &g_aic.aiSvcUsers) {
        AiSvcUser *svcUser = list_entry(node, AiSvcUser, lnode);
        if (svcUser->cltId == cltId &&
            (!plugUuid || !*plugUuid || strcmp(svcUser->plugUuid, plugUuid) == 0)) {
            list_del(node);
            free(svcUser);
        }
    }
    return 0;
}

/**
    ����AIC�¼�(����AI�����¼�).
*/
int AicSubsEvt(uintptr_t cltId, const char* evtName, AicEvtProc proc, void* user)
{
    bool ret = IsMainThrd();
    HI_ASSERT(ret);
    HI_ASSERT(g_aic.inited);

    AicEvtUser *evtUser = AicEvtUserNew(cltId, evtName, proc, user); // ensure by xiao
    list_add_tail(&evtUser->lnode, &g_aic.evtUsers);

    return 0;
}

/**
    ȥ����AIC�¼�(����AI�����¼�).
*/
int AicUnsubsEvt(uintptr_t cltId, const char* evtName)
{
    bool ret = IsMainThrd();
    HI_ASSERT(ret);
    struct list_head *node = NULL;
    struct list_head *next = NULL;

    // ƥ����˳�ѭ�������ݴ�user���ظ�����
    list_for_each_safe(node, next, &g_aic.evtUsers) {
        AicEvtUser *evtUser = list_entry(node, AicEvtUser, lnode);
        if (evtUser->cltId == cltId &&
            (!evtName || !*evtName || strcmp(evtUser->evtName, evtName) == 0)) {
            list_del(node);
            free(evtUser);
        }
    }
    return 0;
}

/**
    ��õ�ǰ״̬.
*/
int AicGetStatus(MemBlk* statusJson)
{
    bool res = IsMainThrd();
    HI_ASSERT(res);
    HI_ASSERT(statusJson);
    struct list_head plugInfos = LIST_HEAD_INIT(plugInfos); // working plug info list
    int ret;

    MemBlk *blk = statusJson;
    char *buf = (char*)blk->data;
    int len = blk->len;

    // ����begin, VI, VO
    len += snprintf_s(buf + len, blk->size - len, blk->size - len - 1, "{ \"vi\": %d", g_aic.viType);
    len += snprintf_s(buf + len, blk->size - len, blk->size - len - 1, ", \"vo\": %u", g_aic.voTypes);

    // ���workPlugs��Ϣ��������ΪJSON����
    ret = AicListWorkPlugs(&plugInfos);
    HI_EXP_RET(ret < 0, ret, "AicListWorkPlugs() FAIL, ret=%d\n", ret);
    len += snprintf_s(buf + len, blk->size - len, blk->size - len - 1, ", \"workPlugs\": [");
    bool firstField = true;
    while (!list_empty(&plugInfos)) {
        AiPlugInfo *plug = list_entry(plugInfos.next, AiPlugInfo, lnode);
        len += snprintf_s(buf + len, blk->size - len, blk->size - len - 1, "%s{\"uuid\": \"%s\"}",
            (firstField ? " " : ", "), plug->uuid);
        firstField = false;
        list_del(plugInfos.next);
        AiPlugInfoDelete(plug);
    }
    len += snprintf_s(buf + len, blk->size - len, blk->size - len - 1, " ]}");

    blk->len = len;
    return 0;
}

/**
    ע��stream user.
*/
bool AicAddStrmUser(const IAicStrmUser* itf, void* user)
{
    bool ret = IsMainThrd();
    HI_ASSERT(ret);
    HI_ASSERT(itf);

    for (size_t i = 0; i < HI_ARRAY_SIZE(g_aic.strmUserTab); i++) {
        AicStrmUser *node = &g_aic.strmUserTab[i];
        if (!node->itf) {
            node->itf = itf;
            node->user = user;
            g_aic.strmUserNum++;
            return true;
        }
    }

    LOGE("add StrmUser FAIL, for tab full, size=%uz\n", g_aic.strmUserNum);
    return false;
}

/**
    ע��stream user.
*/
bool AicDelStrmUser(const IAicStrmUser* itf, void* user)
{
    bool ret = IsMainThrd();
    HI_ASSERT(ret);
    HI_ASSERT(itf);

    for (size_t i = 0; i < HI_ARRAY_SIZE(g_aic.strmUserTab); i++) {
        AicStrmUser *node = &g_aic.strmUserTab[i];
        if (node->itf == itf && node->user == user) {
            node->itf = NULL;
            node->user = NULL;
            g_aic.strmUserNum--;
            return true;
        }
    }

    LOGE("del StrmUser FAIL, for user not found\n");
    return false;
}

