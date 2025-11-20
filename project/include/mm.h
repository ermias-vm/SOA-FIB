/**
 * @file mm.h
 * @brief Memory management interface and definitions for ZeOS.
 *
 * This header defines memory management functions, data structures,
 * and constants for paging, virtual memory, and frame allocation.
 */

#ifndef __MM_H__
#define __MM_H__

#include <mm_address.h>
#include <sched.h>
#include <types.h>

#define FREE_FRAME 0
#define USED_FRAME 1

/* Bytemap to mark the free physical pages */
extern Byte phys_mem[TOTAL_PAGES];

/* Page directories for each task */
extern page_table_entry dir_pages[NR_TASKS][TOTAL_PAGES];
/* User page tables for each task */
extern page_table_entry pagusr_table[NR_TASKS][TOTAL_PAGES];

/**
 * @brief Initialize the physical memory frame management
 *
 * Sets up the physical memory bitmap to track which memory frames
 * are free or used. Marks system frames as used and user frames as free.
 *
 * @return 0 on success
 */
int init_frames(void);

/**
 * @brief Allocate a free physical memory frame
 *
 * Searches for and allocates the first available physical memory frame.
 * Marks the frame as used in the physical memory bitmap.
 *
 * @return Frame number if successful, -1 if no free frames available
 */
int alloc_frame(void);

/**
 * @brief Free a physical memory frame
 *
 * Marks a physical memory frame as free in the bitmap, making it
 * available for future allocations.
 *
 * @param frame Frame number to free
 */
void free_frame(unsigned int frame);

/**
 * @brief Set up user page tables for a task
 *
 * Configures the page directory and user page tables for a task,
 * mapping user virtual addresses to physical memory frames.
 *
 * @param task Pointer to the task structure to set up pages for
 */
void set_user_pages(struct task_struct *task);

/* Global Descriptor Table pointer for memory segmentation */
extern Descriptor *gdt;

/* Task State Segment structure used for hardware task switching */
extern TSS tss;

/**
 * @brief Initialize directory pages for all tasks
 *
 * Sets up the page directory entries for all tasks in the system,
 * configuring the mapping to user page tables with proper permissions.
 */
void init_dir_pages();

/**
 * @brief Initialize page tables with kernel pages
 *
 * Sets up the page tables for all tasks with kernel page mappings.
 * Initializes kernel pages as present and accessible.
 */
void init_table_pages();

/**
 * @brief Enable paging by setting the PE flag in CR0
 *
 * Modifies the CR0 register to enable paging in the processor,
 * activating virtual memory management.
 */
void set_pe_flag();

/**
 * @brief Free user pages allocated to a task
 *
 * Releases all user memory pages (data pages) allocated to a task
 * back to the free frame pool and clears the page table entries.
 *
 * @param task Pointer to the task structure whose pages should be freed
 */
void free_user_pages(struct task_struct *task);

/**
 * @brief Initialize memory management subsystem
 *
 * Sets up the complete memory management system including frame allocation,
 * page directories, and virtual memory mapping.
 */
void init_mm();

/**
 * @brief Set the CR3 register (page directory base)
 *
 * Assembly routine that loads the CR3 register with a new page directory
 * address, effectively switching the memory context.
 *
 * @param dir Pointer to the page directory to load
 */
void set_cr3(page_table_entry *dir);

/**
 * @brief Initialize the Global Descriptor Table
 *
 * Sets up the GDT with kernel and user code/data segments and loads
 * the GDTR register.
 */
void setGdt();

/**
 * @brief Initialize the Task State Segment
 *
 * Sets up the TSS for hardware task switching and privilege level
 * transitions, then loads the TR register.
 */
void setTSS();

/**
 * @brief Set a page table entry mapping
 *
 * Creates a mapping in a page table from a virtual page to a physical frame.
 * Sets the appropriate page table entry with the frame address and access flags.
 *
 * @param PT Pointer to the page table
 * @param page Virtual page number to map
 * @param frame Physical frame number to map to
 */
void set_ss_pag(page_table_entry *PT, unsigned page, unsigned frame);

/**
 * @brief Delete a page table entry mapping
 *
 * Removes a mapping from a page table by clearing the page table entry
 * for the specified virtual page.
 *
 * @param PT Pointer to the page table
 * @param page Virtual page number to unmap
 */
void del_ss_pag(page_table_entry *PT, unsigned page);

/**
 * @brief Get physical frame from virtual page
 *
 * Retrieves the physical frame number that a virtual page is mapped to
 * in the specified page table.
 *
 * @param PT Pointer to the page table
 * @param page Virtual page number to look up
 * @return Physical frame number, or 0 if page is not mapped
 */
unsigned int get_frame(page_table_entry *PT, unsigned int page);

#endif /* __MM_H__ */
