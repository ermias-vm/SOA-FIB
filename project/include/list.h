/**
 * @file list.h
 * @brief Doubly-linked list interface definitions for ZeOS.
 *
 * This header defines list structures and manipulation macros
 * for generic doubly-linked list operations in the kernel.
 *
 * Simple doubly linked list implementation.
 *
 * SAMPLE USE:
 *   // Declare an uninitialized list named 'list'
 *   struct list_head list;
 *
 *   // Initialize the list 'list'
 *   INIT_LIST_HEAD( &list );
 *
 *   // Define structures to be inserted in the list
 *   struct element {
 *     int a;
 *     char b;
 *     struct list_head anchor; //This is the anchor in the list
 *     ...
 *   };
 *
 *   // Declare a new element to be inserted in the list
 *   struct element el;
 *
 *   // Add the new element to the list, using the element's anchor.
 *   list_add( &(el.anchor), &list );
 *
 *   // Get the first element of the list
 *   struct list_head * e = list_first( &list );
 *
 *   // Remove the selected element from the list
 *   list_del( e );
 *
 *   // Get the container of this list element
 *   struct element * realelement = list_entry( e, struct element, anchor );
 *   realelement->a = 0x666;
 *
 *   // Traverse the list
 *   list_for_each( e, &list ) {
 *      // do whatever with 'e'
 *      struct element * realelement = list_entry( e, struct element, anchor );
 *      ...
 *   }
 */

#ifndef _LINUX_LIST_H
#define _LINUX_LIST_H

/** List head structure for doubly-linked list */
struct list_head {
    struct list_head *next, *prev;
};

/**
 * @brief Initialize an empty list.
 *
 * Initializes a list head so that it points to itself,
 * creating an empty circular list.
 *
 * @param head List head to initialize.
 */
void INIT_LIST_HEAD(struct list_head *head);

/**
 * @brief Add a new entry after the specified head.
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 *
 * @param new New entry to be added.
 * @param head List head to add it after.
 */
void list_add(struct list_head *new, struct list_head *head);

/**
 * @brief Add a new entry before the specified head.
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 *
 * @param new New entry to be added.
 * @param head List head to add it before.
 */
void list_add_tail(struct list_head *new, struct list_head *head);

/**
 * @brief Delete entry from list.
 *
 * Deletes an entry from the list by making the prev/next entries
 * point to each other.
 *
 * Note: list_empty() on entry does not return true after this,
 * the entry is in an undefined state.
 *
 * @param entry The element to delete from the list.
 */
void list_del(struct list_head *entry);

/**
 * @brief Test whether list is the last entry in list head.
 *
 * Tests whether the given list entry is the last entry in the list.
 *
 * @param list The entry to test.
 * @param head The head of the list.
 * @return 1 if list is the last entry, 0 otherwise.
 */
int list_is_last(const struct list_head *list, const struct list_head *head);

/**
 * @brief Test whether a list is empty.
 *
 * Tests whether the given list head represents an empty list.
 *
 * @param head The list to test.
 * @return 1 if the list is empty, 0 otherwise.
 */
int list_empty(const struct list_head *head);

/**
 * @brief Get the struct for this entry.
 *
 * Returns the containing structure of a list_head element.
 *
 * @param ptr The &struct list_head pointer.
 * @param type The type of the struct this is embedded in.
 * @param member The name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member)                                                              \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/**
 * @brief Iterate over a list.
 *
 * Iterates over all entries in a list.
 *
 * @param pos The &struct list_head to use as a loop cursor.
 * @param head The head for your list.
 */
#define list_for_each(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * @brief Iterate over a list safe against removal of list entry.
 *
 * Iterates over all entries in a list, allowing safe removal of
 * the current entry during iteration.
 *
 * @param pos The &struct list_head to use as a loop counter.
 * @param n Another &struct list_head to use as temporary storage.
 * @param head The head for your list.
 */
#define list_for_each_safe(pos, n, head)                                                           \
    for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

/**
 * @brief Returns the first list item.
 *
 * Returns a pointer to the first entry in the list.
 *
 * @param head The head for your list.
 */
#define list_first(head) (head)->next

#endif /* _LINUX_LIST_H */
