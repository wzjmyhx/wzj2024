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

#ifndef AI_PLUG_H
#define AI_PLUG_H

#include <stdint.h>
#include <stdbool.h>
#include "hi_comm_video.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
    ���Ӧ��ѭ��Լ��.
*/
#define AI_PLUG_MAGIC       0x3D2E837B // ���Ӧ���ص�magic
#define AI_PLUG_EXTNAME     "plug" // ����ļ�����׺

/*
    ����#include "mmp_img.h"��ֱ������Ҫʹ�õ�OsdSet����.
*/
typedef struct OsdSet OsdSet;

/**
    AIC����ӿ�.

    ���Ӧʵ�ָýӿڣ���ͨ��aic_plug_itf���Ⱪ¶��

    hiopenais�ڻ��IAicPlug�󣬻����load()����ģ�ͣ�������cal()ִ��һ֡ͼƬ�ļ��㡣
    ��������Ҫʹ�ò��ʱ��hiopenais�����unlock()ж��ģ�͡�

    �����ģ��ʱ�������ɲ���������京�塣������Դ����������͵Ķ���ͨ��
    load()���ظ�hiopenais��hiopenais��͸��ʹ���������ò���Ľӿڽӿڡ�
*/
typedef struct AiPlug {
    /**
        ��ȡ���profile.

        ���Ӧά�ַ��ص�profile����Ч�ԣ�hiopenais���Ḵ������

        @return ���ز����profile.
    */
    const HI_PCHAR (*Prof)(void);

    /**
        ����ģ��.

        ��Ҫʹ�ò���ļ������ʱ��hiopenais����ô˺�����

        Ŀǰ�Ķ��壬�ڲ�������������ڣ�hiopenais������unload��load��ģ��ǰ���ٴε���
        �˺�������load��

        @param model[out]: ������������ģ�Ͷ���
        @param osds[in]: hiopenais������OsdSet�������ά��������������resFrm�е������֡�
        @return �ɹ��򷵻�0�����򷵻ظ�������ֵ��ʾ�����롣
    */
    int (*Load)(uintptr_t* model, OsdSet* osds);

    /**
        ж��ģ��.

        hiopenais���ô˺�����ж��ģ�͡��˺�hiopenais���ܻ����µ���load()����ģ�͡�

        @param model[in]: load()���ص�@param model��
        @return �ɹ��򷵻�0�����򷵻ظ�������ֵ��ʾ�����롣
    */
    int (*Unload)(uintptr_t model);

    /**
        ����һ��ͼƬ.

        hiopenais����һ֡ͼƬ@param srcFrm���������AI���㡣������Խ������������֡�
        ͼ��(�����)���ӵ�@param resFrm�У������ɽṹ��JSON�����Ա�ʾ��������

        hiopenais�Ѿ���@param srcFrm�ķֱ��ʰ����profile�е�Ҫ�����Ե����������ظ�ʽ��
        ��YUV/RGB���Լ���ͨ��/��ͨ����hiopenaisδ��ת������Ҫ�������������hiopenais����
        ����һ���Ĵ������Լ����������ĸ�����

        ��Ҫע�����@srcFrm��@dstFrm�ķֱ��ʿ����ǲ�ͬ�ģ������resFrm�е�����Ϣ��
        ������resJsonʱ��Ҫ������ת������������ù��ߺ���rect_box_tran()��ת����

        resJson�ɲ��������䣬��hiopenais�����ͷš�

        @param model[in]: load()���ص�@param model��
        @param srcFrm[in]: �������ͼƬ֡��
        @param resFrm[in|out]: ���֡������������ϵ���ͼ�Ρ����ֵ���Ϣ��
        @param resJson[out]: �ṹ��������ݣ�JSON��ʽ����ṹ�ɲ���Զ��塣
        @return �ɹ��򷵻�0�����򷵻ظ�������ֵ��ʾ�����롣
    */
    int (*Cal)(uintptr_t model, VIDEO_FRAME_INFO_S* srcFrm,
        VIDEO_FRAME_INFO_S* resFrm, char** resJson);

    /**
        ����/��ֹ�����������.

        ��������Զ����������hiopenais��������/��ֹʱ�Զ�����/��ֹ�÷���

        �����������ĺ����ɲ�����壬hiopenais�������͡����͵İ���ʱʶ�����㷨�ĵ׿������
        ��������Ҫ��Camera��֧�ֵ׿���������Զ�������������׿⡣

        �������ʹ��lite_httpd�ṩ��http������host��ͨ��http������������ͨ�š�

        startSvc()��������
        stopSvc()��ֹ����

        @return �ɹ��򷵻�0�����򷵻ظ�������ֵ��ʾ�����롣
    */
    int (*StartSvc)(void);
    int (*StopSvc)(void);
}   AiPlug;

/**
    ��ȡ����ӿ�.

    @param magic[out]: ���ͨ��������AIC_PLUG_MAGIC��
    @return ���ز��ʵ�ֵ�IAicPlug�ӿڡ�
*/
const AiPlug* AiPlugItf(uint32_t* magic);
typedef const AiPlug* (*AIPlugItfFunc)(uint32_t* magic);

#ifdef __cplusplus
}
#endif

#endif // AI_PLUG_H
