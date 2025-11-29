/**
 * @file list.c
 * @brief Doubly-linked list implementation for ZeOS.
 *
 * This file provides a generic doubly-linked list implementation
 * used for process queues and kernel data structure management.
 */

#include <list.h>

/**
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct list_head *new, struct list_head *prev,
                              struct list_head *next) {
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

/**
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head *prev, struct list_head *next) {
    next->prev = prev;
    prev->next = next;
}

void INIT_LIST_HEAD(struct list_head *list) {
    list->next = list;
    list->prev = list;
}

void list_add(struct list_head *new, struct list_head *head) {
    __list_add(new, head, head->next);
}

void list_add_tail(struct list_head *new, struct list_head *head) {
    __list_add(new, head->prev, head);
}

void list_del(struct list_head *entry) {
    __list_del(entry->prev, entry->next);
    entry->next = (void *)0;
    entry->prev = (void *)0;
}

int list_is_last(const struct list_head *list, const struct list_head *head) {
    return list->next == head;
}

int list_empty(const struct list_head *head) {
    return head->next == head;
}
