#include <stdlib.h>
#include <stdbool.h>

#include "list.h"
#include "memory_manager.h"

static ListNode *CreateNode(unsigned int tag, void *data);
static void *RemoveNodeFromList(List *list, ListNode *node);

List *List_Create(unsigned int tag)
{
    List *list = MemoryManager_Alloc(tag, sizeof(List));

    list->head = NULL;
    list->tail = NULL;
    list->tag = tag;
    list->count = 0;

    return list;
}

void List_Destroy(List *list, bool free_items, List_FreeItemFunc free_item_func)
{
    ListNode *current_node = list->head;

    while (current_node != NULL)
    {
        ListNode *next_node = current_node->next;

        if (free_items)
        {
            if (free_item_func)
                free_item_func(current_node->data);
            else
                MemoryManager_Dealloc(list->tag, current_node->data);
        }

        MemoryManager_Dealloc(list->tag, current_node);

        current_node = next_node;
    }

    MemoryManager_Dealloc(list->tag, list);
}

void List_PushBack(List *list, void *data)
{
    ListNode *node = CreateNode(list->tag, data);

    if (list->count == 0)
    {
        node->next = NULL;
        node->prev = NULL;

        list->head = node;
        list->tail = node;
    }
    else
    {
        node->next = NULL;
        node->prev = list->tail;

        list->tail->next = node;
        list->tail = node;
    }

    list->count++;
}

void *List_GetAt(List *list, int index)
{
    ListNode *current_node = list->head;

    for (int i = 0; current_node != NULL && i < index; i++)
        current_node = current_node->next;

    return current_node ? current_node->data : NULL;
}

void *List_Remove(List *list, void *data)
{
    ListNode *current_node = list->head;

    for (int i = 0; current_node != NULL && current_node->data != data; i++)
        current_node = current_node->next;

    if (current_node != NULL)
    {
        return RemoveNodeFromList(list, current_node);
    }

    return NULL;
}

void *List_RemoveAt(List *list, int index)
{
    ListNode *current_node = list->head;

    for (int i = 0; current_node != NULL && i < index; i++)
        current_node = current_node->next;

    if (current_node != NULL)
    {
        return RemoveNodeFromList(list, current_node);
    }

    return NULL;
}

bool List_Includes(List *list, void *data)
{
    ListNode *current_node = list->head;

    for (int i = 0; current_node != NULL && current_node->data != data; i++)
        current_node = current_node->next;

    return current_node != NULL;
}

static ListNode *CreateNode(unsigned int tag, void *data)
{
    ListNode *node = MemoryManager_Alloc(tag, sizeof(ListNode));

    node->data = data;

    return node;
}

static void *RemoveNodeFromList(List *list, ListNode *node)
{
    if (node == list->head)
    {
        ListNode *new_head = node->next;

        if (new_head != NULL)
            new_head->prev = NULL;
        else
            list->tail = NULL;

        list->head = new_head;

        void *data = node->data;

        MemoryManager_Dealloc(list->tag, node);
        list->count--;

        return data;
    }

    if (node == list->tail)
    {
        ListNode *new_tail = node->prev;

        new_tail->next = NULL;
        list->tail = new_tail;

        void *data = node->data;

        MemoryManager_Dealloc(list->tag, node);
        list->count--;

        return data;
    }

    node->prev->next = node->next;
    node->next->prev = node->prev;

    void *data = node->data;

    MemoryManager_Dealloc(list->tag, node);
    list->count--;

    return data;
}
