#pragma once

#include <stdbool.h>

typedef struct ListNode
{
    void *data;
    struct ListNode *next;
    struct ListNode *prev;
} ListNode;

typedef struct
{
    ListNode *head;
    ListNode *tail;
    unsigned int tag;
    unsigned int count;
} List;

typedef void (*List_FreeItemFunc)(void *);

List *List_Create(unsigned int tag);
void List_Destroy(List *list, bool free_items, List_FreeItemFunc free_item_func);
void List_PushBack(List *list, void *data);
void *List_GetAt(List *list, int index);
void *List_Remove(List *list, void *data);
void *List_RemoveAt(List *list, int index);
