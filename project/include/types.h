/**
 * @file types.h
 * @brief Basic type definitions and data structures for ZeOS.
 *
 * This header defines fundamental data types, structures, and
 * constants used throughout the ZeOS kernel and system components.
 */

#ifndef __TYPES_H__
#define __TYPES_H__

/** System types definition **/
/*****************************/

/* 8-bit unsigned integer */
typedef unsigned char Byte;

/* 16-bit unsigned integer */
typedef unsigned short int Word;

/* 32-bit unsigned integer */
typedef unsigned long DWord;

#define highWord(address) (Word)(((address) >> 16) & 0xFFFF)
#define lowWord(address) (Word)((address)&0xFFFF)
#define midByte(address) (Byte)(((address) >> 16) & 0xFF)
#define highByte(address) (Byte)(((address) >> (16 + 8)) & 0xFF)
#define high4Bits(limit) (Byte)(((limit) >> 16) & 0x0F)

/* Segment Descriptor structure for GDT/LDT entries */
typedef struct /* Segment Descriptor */
{
    Word limit;    /* Segment limit (low 16 bits) */
    Word lowBase;  /* Base address (low 16 bits) */
    Byte midBase;  /* Base address (middle 8 bits) */
    Byte flags1;   /* Access rights and type flags */
    Byte flags2;   /* Granularity and limit (high 4 bits) */
    Byte highBase; /* Base address (high 8 bits) */
} Descriptor;      /* R1: pg. 3-11, 4-3 */

/* Interrupt/Trap Gate structure for IDT entries */
typedef struct /* Gate */
{
    Word lowOffset;       /* Handler offset (low 16 bits) */
    Word segmentSelector; /* Code segment selector */
    Word flags;           /* Gate type and access flags */
    Word highOffset;      /* Handler offset (high 16 bits) */
} Gate;                   /* R1: pg. 5-11 */

/* Task State Segment structure for hardware task switching */
typedef struct               /* TASK STATE SEGMENT      */
{                            /*                         */
    Word PreviousTaskLink;   /* 0          R1: pg. 6-5  */
    Word none1;              /*                         */
    DWord esp0;              /* 4  \                    */
    Word ss0;                /* 8  |                    */
    Word none2;              /*    |                    */
    DWord esp1;              /* 12 |  Stack pointers    */
    Word ss1;                /* 16 |-    for each       */
    Word none3;              /*    |  privilege level   */
    DWord esp2;              /* 20 |                    */
    Word ss2;                /*    |                    */
    Word none4;              /* 24/                     */
    DWord cr3;               /* 28\                     */
    DWord eip;               /* 32 |                    */
    DWord eFlags;            /* 36 |                    */
    DWord eax;               /* 40 |                    */
    DWord ecx;               /* 44 |                    */
    DWord edx;               /* 48 |                    */
    DWord ebx;               /* 52 |                    */
    DWord esp;               /* 56 |                    */
    DWord ebp;               /* 60 |                    */
    DWord esi;               /* 64 |                    */
    DWord edi;               /* 68 |- Saved registers   */
    Word es;                 /* 72 |                    */
    Word none5;              /*    |                    */
    Word cs;                 /* 76 |                    */
    Word none6;              /*    |                    */
    Word ss;                 /* 80 |                    */
    Word none7;              /*    |                    */
    Word ds;                 /* 84 |                    */
    Word none8;              /*    |                    */
    Word fs;                 /* 88 |                    */
    Word none9;              /*    |                    */
    Word gs;                 /* 92 |                    */
    Word none10;             /*   /                     */
    Word LDTSegmentSelector; /* 96                      */
    Word none11;             /*     The offset in the   */
    Word debugTrap;          /* 100 TSS Segment to the  */
    Word IOMapBaseAddress;   /* 102 io permision bitmap */
} TSS;                       /* size = 104 B + i/o permission bitmap and     */
                             /* interrupt redirection bitmap (R1 pg 6.6)     */

/** Registers: **/
/****************/

/* Structure for loading GDTR/IDTR registers */
typedef struct {
    Word limit __attribute__((packed)); /* Table limit (size - 1) */
    DWord base __attribute__((packed)); /* Table base address */
} Register;                             /* GDTR, IDTR */

/** Segment Selector **/
/**********************/

/* 16-bit segment selector for accessing GDT/LDT entries */
typedef Word Selector;
/*                                                     */
/*  /--------------------------------------\           */
/*  |   Index                       |TI|RPL|           */
/*  \--------------------------------------/           */
/*  15                             3  2 1 0            */
/*                                                     */
/*  Index = Index in the GDT/LDT descriptor table      */
/*  TI    = 0 -> GDT ; 1 -> LDT                        */
/*  RPL   = Requestor Privilege Level     R1: pg. 3-9  */
/*******************************************************/

/**************************************************************************/
/*** THE EFLAGS ****************************************** R1: pg. 2-8 ****/
/**                                                                      **/
/**   -----------------------------------------------------------------  **/
/**  |                     |I|V|V|A|V|R| |N| I |O|D|I|T|S|Z| |A| |P| |C| **/
/**  | Reserved (set to 0) | |I|I| | | |0| | O | | | | | | |0| |0| |1| | **/
/**  |                     |D|P|F|C|M|F| |T|PL |F|F|F|F|F|F| |F| |F| |F| **/
/**   -----------------------------------------------------------------  **/
/**  31                  22 21        16  14  12    9 8 7 6 5 4 3 2 1 0  **/
/**                                                                      **/
/** ID :  Identification Flag          IF:   Interrupt Enable Flag       **/
/** VIP:  Virtual Interrupt Pending    TF:   Trap Flag                   **/
/** VIF:  Virtual Interrupt Flag       SF:   Sign Flag                   **/
/** AC:   Alignment Check              ZF:   Zero Flag                   **/
/** VM:   Virtual 8086 Mode            AF:   Auxiliary Carry Flag        **/
/** RF:   Resume Flag                  PF:   Parity Flag                 **/
/** NT:   Nested task Flag             CF:   Carry Flag                  **/
/** IOPL: I/O Privilege Level                                            **/
/**                                                                      **/
/**************************************************************************/
/* Initial EFLAGS value with interrupts enabled */
#define INITIAL_EFLAGS 0x00000200

/* NULL pointer definition */
#define NULL 0

/* Page table entry union for x86 paging */
typedef union {
    unsigned int entry; /* Raw 32-bit page table entry */
    struct {
        unsigned int present : 1;     /* Page present in memory */
        unsigned int rw : 1;          /* Read/write permission */
        unsigned int user : 1;        /* User/supervisor access */
        unsigned int write_t : 1;     /* Write-through caching */
        unsigned int cache_d : 1;     /* Cache disabled */
        unsigned int accessed : 1;    /* Page accessed flag */
        unsigned int dirty : 1;       /* Page dirty flag */
        unsigned int ps_pat : 1;      /* Page size/PAT */
        unsigned int global : 1;      /* Global page */
        unsigned int avail : 3;       /* Available for OS use */
        unsigned int pbase_addr : 20; /* Physical base address */
    } bits;
} page_table_entry;

#endif /* __TYPES_H__ */
