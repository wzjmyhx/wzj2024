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

#ifndef EXT_PLUG_H
#define EXT_PLUG_H

#include "aic_mng.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
    ����ӿ�.
*/
int ExtPlugInit(const IAicStrmUser** userItf, void** userObj);
int ExtPlugExit(void);

typedef int (*ExtPlugInitFunc)(const IAicStrmUser** userItf, void** userObj);
typedef int (*ExtPlugExitFunc)(void);

/**
    VO�����̬�������Ϣ.
*/
typedef struct ExtPlugLib {
    struct list_head lnode;

    void* hnd; // lib handle
    ExtPlugInitFunc init;
    ExtPlugExitFunc exit;

    const IAicStrmUser* userItf;
    void *userObj;
}ExtPlugLib;

/**
    ����ָ����VO�����̬��.

    ����plug�����ض�̬�⣬��ͨ��`plug`���ش�����plug��
    ���ص�plugӦͨ��`ExtPlugUnload`ж�غ��ͷš�
*/
int ExtPlugLoad(ExtPlugLib** self, const char* path);

/**
    ж��ָ����VO�����̬��.
    ж�ض�̬�⣬���ͷ�`plug`��
*/
void ExtPlugUnload(ExtPlugLib* plug);

#ifdef __cplusplus
}
#endif

#endif // EXT_PLUG_H
