/**
 * @file hi_list_sdk.h
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
  * @defgroup iot_list List
  * @ingroup system
  */
#ifndef __HI_LIST_SDK_H__
#define __HI_LIST_SDK_H__

#include <hi_types_base.h>

/****************************************************************************/
HI_START_HEADER

/*
 * Note: This file renames the linked list interface and uses the LTOS interface to implement it again.
 * CNcomment:��ע�����ļ�������ӿ�����������ʹ��LTOS�ӿ�����ʵ�֡�
 */
typedef struct hi_list {
    struct hi_list *prev;
    struct hi_list *next;
} hi_list;

/*
 * Function description:
 * Initialize the head node. Note that this node is used only for management, not the data node entered by the user.
 * CNcomment:������������ʼ��ͷ�ڵ㣬ע��˽ڵ�����ڹ��������û���������ݽڵ�
 */
__attribute__((always_inline)) static inline hi_void hi_list_init(hi_list *list)
{
    list->next = list;
    list->prev = list;
}

/*
 * Function description: Inserts a node as the first node in the list.
 * CNcomment:������������node����Ϊlist�ĵ�һ���ڵ�
 */
__attribute__((always_inline)) static inline hi_void hi_list_head_insert(hi_list *node, hi_list *list)
{
    node->next = list->next;
    node->prev = list;
    list->next->prev = node;
    list->next = node;
}

/*
 * Function description: Inserts a node as the last node in the list.
 * CNcomment:������������node����Ϊlist�����һ���ڵ�
 */
__attribute__((always_inline)) static inline hi_void hi_list_tail_insert(hi_list *node, hi_list *list)
{
    hi_list_head_insert(node, list->prev);
}

/*
 * Function description: Deletes a node from the linked list.
 * CNcomment:������������������ɾ��ĳ���ڵ�
 */
__attribute__((always_inline)) static inline hi_void hi_list_delete(hi_list *node)
{
    if (node->next == HI_NULL || node->prev == HI_NULL) {
        return;
    }

    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->next = (hi_list *)HI_NULL;
    node->prev = (hi_list *)HI_NULL;
}

/*
 * Function description: Deletes the first node in the linked list without releasing the related memory.
 * CNcomment:����������ɾ������ĵ�һ���ڵ㣬���ͷ�����ڴ�
 */
__attribute__((always_inline)) static inline hi_list *hi_list_delete_head(hi_list *list)
{
    hi_list *del_node;

    del_node = list->next;
    if (del_node == list || del_node == HI_NULL) {
        return HI_NULL;
    }

    hi_list_delete(del_node);
    return del_node;
}

/*
 * Function description: Deletes the tail node of the linked list without releasing the related memory.
 * CNcomment:����������ɾ������β���ڵ㣬���ͷ�����ڴ�
 */
__attribute__((always_inline)) static inline hi_list *hi_list_delete_tail(hi_list *list)
{
    hi_list *del_node;

    del_node = list->prev;
    if (del_node == list || del_node == HI_NULL) {
        return HI_NULL;
    }

    hi_list_delete(del_node);
    return del_node;
}

/*
 * Function description: Determines whether the linked list is empty.
 * CNcomment:�����������ж������Ƿ�Ϊ��
 */
__attribute__((always_inline)) static inline hi_bool hi_is_list_empty(hi_list *list)
{
    if (list->next == HI_NULL || list->prev == HI_NULL) {
        return HI_TRUE;
    }
    return (hi_bool)(list->next == list);
}

/*
 * Function description: Deinitializes the linked list. The management node clears the list.
 * Other member nodes are still connected in a bidirectional linked list.
 * CNcomment:����������ȥ��ʼ����������ڵ���գ�������Ա�ڵ���β������Ȼ��һ��˫������
 */
__attribute__((always_inline)) static inline hi_void hi_list_del_init(hi_list *list)
{
    list->next->prev = list->prev;
    list->prev->next = list->next;

    list->next = list;
    list->prev = list;
}

/*
 * Function description: Adds linked list 2 to the end of linked list 1.
 * CNcomment:����������������2��������1��β��
 */
__attribute__((always_inline)) static inline hi_void hi_list_join_tail(hi_list *list1, hi_list *list2)
{
    list1->prev->next = list2->next;
    list2->next->prev = list1->prev;
    list2->prev->next = list1;
    list1->prev = list2->prev;
}

/*
 * Function description: Adds linked list 2 to the header of linked list 1.
 * CNcomment:����������������2��������1��ͷ��
 */
__attribute__((always_inline)) static inline hi_void hi_list_join_head(hi_list *list1, hi_list *list2)
{
    /* list2 is empty. */
    if (list2->next == list2) {
        return;
    }

    list2->prev->next = list1->next;
    list1->next->prev = list2->prev;
    list1->next = list2->next;
    list2->next->prev = list1;
}

/*
 * Function description: Extracts the first element from the last_node element in linked list 2 and
 * adds the first element to the header of empty linked list 1.
 * CNcomment:����������������2�дӵ�һ��Ԫ�ص�last_nodeԪ��ժ���� ���������1��ͷ��
 */
__attribute__((always_inline)) static inline hi_void hi_list_remove_head(hi_list *list1, hi_list *list2,
                                                                         hi_list *last_node)
{
    list1->next = list2->next;
    list1->prev = last_node;

    list2->next = last_node->next;
    ((hi_list *)(last_node->next))->prev = list2;

    last_node->next = list1;
    if (last_node->prev == list2) {
        last_node->prev = list1;
    }
}

#define hi_list_init_macro(_list_name) hi_list _list_name = { (hi_list*)&(_list_name), (hi_list*)&(_list_name) }

/* 
 * Obtains the pointer of the first node.
 * CNcomment:��ȡ��һ���ڵ�ָ��
 */
#define hi_list_first(object) ((object)->next)

#define hi_list_last(object) ((object)->prev)

#define hi_list_entry(item, type, member) \
    ((type*)((char*)(item) - hi_offset_of_member(type, member)))

#define hi_list_for_each_entry(item, list, type, member)   \
    for ((item) = hi_list_entry((list)->next, type, member); \
         &(item)->member != (list);                          \
         (item) = hi_list_entry((item)->member.next, type, member))

#define hi_list_for_each_entry_safe(list, item, pnext, type, member) \
    for ((item) = hi_list_entry((list)->next, type, member),           \
         (pnext) = hi_list_entry((item)->member.next, type, member);     \
         &(item)->member != (list);                                    \
         (item) = (pnext), (pnext) = hi_list_entry((item)->member.next, type, member))

#define hi_list_for_each_entry_continue_safe(pitem, list, item, pnext, type, member) \
    for ((item) = hi_list_entry((pitem)->next, type, member),                          \
         (pnext) = hi_list_entry((item)->member.next, type, member);                     \
         &((item)->member) != (list);                                                \
         (item) = (pnext), (pnext) = hi_list_entry((pnext)->member.next, type, member))

/* Simple Implementation of Bidirectional Linked List Operation */
#define hi_list_head(list) \
    hi_list list = { &(list), &(list) }

#define hi_list_for_each(item, list) \
    for ((item) = (list)->next; (item) != (list); (item) = (item)->next)

#define hi_list_for_each_safe(item, pnext, list)                  \
    for ((item) = (list)->next, (pnext) = (item)->next; (item) != (list); \
         (item) = (pnext), (pnext) = (item)->next)

HI_END_HEADER
#endif  /* __HI_STDLIB_H__ */
