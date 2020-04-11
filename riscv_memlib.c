/*
 * Malloc Lab
 *
 * Name: Yifan Yu
 * ID: 1600012998@pku.edu.cn
 *
 * 此版本整体上采用分离空闲链表(segregated free lists)+最佳适配(best fit)的思路进行动态内存分配。
 * 对于相邻的空闲块采用立即合并的处理方式。
 * 在具体实现上进行了一些优化，使得分配块只需要header不需要footer，最小块大小减少到8，从而减少overhead:
 *
 * 分离空闲链表具体由4个small size block的list和10个large size block的list组成:
 * small size block lists：第0-3号链表分别储存所有大小为8（最小块大小）、16、24、32字节的空闲块;
 * large size block lists：第4-12号中第i号链表存储所有大小属于(2^(i+1), 2^(i+2)]字节的空闲块;
 *                         第13号链表存储所有大小大于last_list_sizeover=2^14字节的空闲块;
 * 所有small size block lists插入时直接插入到链表头;
 * 所有large size block lists内部按空闲块大小从小到大排序，这样搜索时first fit就是best fit.
 * 指向12个链表头的指针存储在堆的最开始部分，在序言块(prologue block)之前.
 *
 * 通过利用header中倒数第2位标记地址上前一个块是否被分配(prev_alloc bit)，可以使得分配块不需要footer
 * 再通过header与footer中倒数第3位标记大小是否为8字节（最小块），可以使得最小块大小降到8字节
 * 具体分配块与空闲块的结构如下（每一行为4字节）：
 *
 *                                      | last 3 bits
 *
 * 大于8字节的分配块：
 *                       header  <size> | <0><prev_alloc><alloc=1>
 *                       bp ->   <有效载荷>
 *                               <...>
 *
 * 大小为8字节的分配块：
 *                       header  <size> | <1><prev_alloc><alloc=1>
 *                       bp ->   <有效载荷>
 *
 * 大于8字节的空闲块：
 *                       header  <size> | <0><prev_alloc><alloc=0>
 *                       bp ->   <list_prev>
 *                               <list_next>
 *                               <...>
 *                       footer  <size> | <0><0><alloc=0>
 *
 * 大小为8字节的空闲块：
 *                          <list_prev> | <1><prev_alloc><alloc=0>
 *                          <list_next> | <1><0><alloc=0>
 *
 * 若地址相连的前一个块被分配，当前块的prev_alloc位为1，否则为0;
 * bp标示出了指向当前块的指针(void *)指向的位置;
 * 空闲块中的1、2字节分别存储了指向链表中前后块的指针，若为表头则list_prev=NULL(0);
 *
 * 除此以外，在naive的realloc基础上修改了再分配的逻辑，详见realloc函数的注释。
 */
#include <string.h>
#include "riscv_memlib.h"

#define ALIGNMENT   8
#define ALIGN(size) ((size + (ALIGNMENT-1)) & ~0x7)

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Basic constants and macros */
#define WSIZE       4       /* Word and header/footer size (bytes) */
#define DSIZE       8       /* Double word size (bytes) */
// #define CHUNKSIZE  (1 << 9)
#define EXTENDSIZE  80      /* Magical number: Extend heap by this amount (words) */

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (val))

/* Read the size and (previous blocks's) allocated fields from address p */
#define GET_SIZE(p)         ((GET(p) & 0x4) ? DSIZE : (GET(p) & ~0x7))
#define GET_ALLOC(p)        (GET(p) & 0x1)
#define GET_PREV_ALLOC(p)   (GET(p) & 0x2)

/* Set the prev_alloc bit of address p */
#define SET_PREV_ALLOC(p)   (GET(p) |= 0x2)
#define SET_PREV_UNALLOC(p) (GET(p) &= ~0x2)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Read the address of the next/previous block with same size in corresponding list */
#define LIST_PREV(bp)  ((GET(HDRP(bp)) & 0x4) ? GET(HDRP(bp)) : (GET(bp)))
#define LIST_NEXT(bp)  ((GET(HDRP(bp)) & 0x4) ? GET(bp) : GET((char *)(bp) + WSIZE))

/* Minimum block size (bytes) */
#define MinBlockSize       DSIZE

/* Block with different size store in seperate lists */
#define small_list_num     4
#define large_list_num     10

/* Block bigger than threshold will fall into large size block lists */
#define threshold          32

/* last list's minimum block size */
#define last_list_sizeover (1 << 14)

/* Global variables */
static char *heap_listp = 0;              /* Pointers to first block */
static unsigned int *segListHeaders = 0;  /* Pointers to segregated list headers */
static unsigned long heap_base_addr;      /* heap base address */

/*** END OF Global variables ***/

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void *realloc_coalesce(void *bp, size_t rsize);
static void remove_block(void *bp);
static void insert_block(void *bp);
static unsigned int get_listid(size_t bsize);

/* Heap checker helper routines */
static void checkblock(void *bp, int lineno);
static int checklist(void *bp, int lineno, unsigned int listid);

/* Some simple inline helper functions */

/* Get the pointer to actual address from a 32-bit address value v*/
static inline void *get_pointer(unsigned int v)
{
    v = (v & ~0x7);
    return v == 0U ? NULL : (void *)(v + heap_base_addr);
}

/* Get the 32-bit address value from an actual pointer */
static inline unsigned int get_waddr(void *bp)
{
    return bp == NULL ? 0U : (unsigned int)((unsigned long)(bp) - heap_base_addr);
}

/* Set the list_prev field */
static inline void set_prev(void *bp, unsigned int waddr)
{
    PUT(bp, waddr);
}

/* Set the list_next field */
static inline void set_next(void *bp, unsigned int waddr)
{
    PUT((char *)(bp) + WSIZE, waddr);
}
/*** End of simple helper functions ***/

/*
 * mm_init - Initialize the memory manager
 */
int mm_init(void)
{
    /* Create Segregated list headers */
    if ((segListHeaders = (unsigned int *)mem_sbrk(ALIGN(
        4*(small_list_num + large_list_num)))) == (void *) -1)
        return -1;
    memset(segListHeaders, 0, 4*(small_list_num + large_list_num));

    /* Set heap base address */
    heap_base_addr = (unsigned long) segListHeaders;

    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *) -1)
        return -1;
    PUT(heap_listp, 0);                          /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (3*WSIZE), (PACK(0, 1) | 0x2));     /* Epilogue header */
    heap_listp += (2*WSIZE);

    /* Extend the empty heap with a free block of EXTENDSIZE words */
    if (extend_heap(EXTENDSIZE) == NULL)
        return -1;

    return 0;
}

/*
 * malloc - Allocate a block with at least size bytes of payload
 */
void *malloc(size_t size)
{
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;

    if (heap_listp == 0){
        mm_init();
    }

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= WSIZE)
        asize = MinBlockSize;
    else
        asize = DSIZE * ((size + (WSIZE) + (DSIZE-1)) / DSIZE);

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize/WSIZE, EXTENDSIZE);
    if ((bp = extend_heap(extendsize)) == NULL)
        return NULL;
    place(bp, asize);

    return bp;
}

/*
 * free - Free a block
 */
void free(void *bp)
{
    if (!bp || (unsigned long)(bp) % ALIGNMENT != 0)
        return;
    if (bp < mem_heap_lo() || bp > mem_heap_hi())
        return;

    size_t size = GET_SIZE(HDRP(bp));
    if (heap_listp == 0){
        mm_init();
    }

    unsigned int prev_alloc = GET_PREV_ALLOC(HDRP(bp));
    if (size > MinBlockSize) {
        PUT(HDRP(bp), (PACK(size, 0) | prev_alloc));
        PUT(FTRP(bp), PACK(size, 0));

        /* The prev_alloc field of the following block needs to be set to 0 */
        SET_PREV_UNALLOC(HDRP(NEXT_BLKP(bp)));
    }
    else {
        PUT(HDRP(bp), (0x4 | prev_alloc));
        PUT(bp, 0x4);

        SET_PREV_UNALLOC((char *)(bp) + WSIZE);
    }
    void *freedblock = coalesce(bp);

    /* Insert the coalesced free block into lists */
    insert_block(freedblock);
}

/*
 * calloc - Allocate the block and set it to zero.
 */
void *calloc(size_t nmemb, size_t size)
{
    size_t bytes = nmemb * size;
    void *newptr;

    newptr = malloc(bytes);
    if (newptr)
        memset(newptr, 0, bytes);

    return newptr;
}

/*
 * realloc - reallocate a allocated block
 *
 * if the required size is larger than the original block size:
 *      if the previous/next block is free and coalesced block is large enough:
 *          return the coalesced block.
 *      else:
 *          do the naive realloc procedure.
 * else:
 *      if the required size is same as original size:
 *          simply return the block
 *      else:
 *          split the original block if possible (remaining size > minimum block size)
 * (经测试当size小于原始size时，不split的效果反而好，因此这部分代码被注释掉了)
 *
 */
void *realloc(void *ptr, size_t size)
{
    size_t oldsize, asize;
    void *newptr;

    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0) {
        free(ptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if(ptr == NULL) {
        return malloc(size);
    }

    /* Get the adjusted size to allocate */
    if (size <= WSIZE)
        asize = MinBlockSize;
    else
        asize = DSIZE * ((size + (WSIZE) + (DSIZE-1)) / DSIZE);

    oldsize = GET_SIZE(HDRP(ptr));
    if (oldsize == asize) {     // simply return
        return ptr;
    }

    else if (oldsize < asize) {
        /* Try to coalesce adjacent free block */
        newptr = realloc_coalesce(ptr, asize);
        if (!newptr) {      /* Cannot coalesce or not large enough */
            newptr = malloc(size);

            /* If realloc() fails the original block is left untouched  */
            if (!newptr)
                return NULL;

            /* Copy the old data. */
            memcpy(newptr, ptr, oldsize);

            /* Free the old block. */
            free(ptr);
        }
        else if (newptr != ptr)
            memmove(newptr, ptr, oldsize);

        return newptr;
    }

    /* oldsize > size */
    size_t splitsize = oldsize - asize;
    if (splitsize > MinBlockSize) {
        if (asize > MinBlockSize)
            PUT(HDRP(ptr), (PACK(asize, 1) | GET_PREV_ALLOC(HDRP(ptr))));
        else
            PUT(HDRP(ptr), 0x5 | GET_PREV_ALLOC(HDRP(ptr)));

        if (splitsize > MinBlockSize)
            PUT(HDRP(NEXT_BLKP(ptr)), PACK(splitsize, 1) | 0x2);
        else
            PUT(HDRP(NEXT_BLKP(ptr)), 0x7);

        free(NEXT_BLKP(ptr));
    }

    return ptr;
}

/*
 * The remaining routines are internal helper routines
 */

/*
 * extend_heap - Extend heap with free block and return its block pointer
 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    unsigned int prev_alloc = GET_PREV_ALLOC(HDRP(bp));
    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), (PACK(size, 0) | prev_alloc));         /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

    /* Coalesce if the previous block was free */
    void *newblock = coalesce(bp);
    insert_block(newblock);
    return newblock;
}

/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
static void *coalesce(void *bp)
{
    void *prev_bp = PREV_BLKP(bp);
    void *next_bp = NEXT_BLKP(bp);
    size_t prev_alloc = GET_PREV_ALLOC(HDRP(bp));
    size_t next_alloc = GET_ALLOC(HDRP(next_bp));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {            /* Case 1 */
        return bp;
    }

    else if (prev_alloc && !next_alloc) {      /* Case 2 */
        remove_block(next_bp);
        size += GET_SIZE(HDRP(next_bp));
        PUT(HDRP(bp), (PACK(size, 0) | GET_PREV_ALLOC(HDRP(bp))));
        PUT(FTRP(bp), PACK(size, 0));
    }

    else if (!prev_alloc && next_alloc) {      /* Case 3 */
        remove_block(prev_bp);
        size += GET_SIZE(FTRP(prev_bp));
        bp = prev_bp;
        PUT(HDRP(bp), (PACK(size, 0) | GET_PREV_ALLOC(HDRP(bp))));
        PUT(FTRP(bp), PACK(size, 0));
    }

    else {                                     /* Case 4 */
        remove_block(prev_bp);
        remove_block(next_bp);
        size += GET_SIZE(HDRP(next_bp)) +
            GET_SIZE(FTRP(prev_bp));
        bp = prev_bp;
        PUT(HDRP(bp), (PACK(size, 0) | GET_PREV_ALLOC(HDRP(bp))));
        PUT(FTRP(bp), PACK(size, 0));
    }

    return bp;
}

/*
 * realloc_coalesce - Helper function to realloc,
 *                    Check if the reallocating block has adjacent free block
 *                    that can be coalesced to cover the required size.
 */
static void *realloc_coalesce(void *bp, size_t rsize)
{
    void *prev_bp = PREV_BLKP(bp);
    void *next_bp = NEXT_BLKP(bp);
    size_t prev_alloc = GET_PREV_ALLOC(HDRP(bp));
    size_t next_alloc = GET_ALLOC(HDRP(next_bp));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {            /* Case 1 */
        return NULL;
    }

    else if (prev_alloc && !next_alloc) {      /* Case 2 */
        size += GET_SIZE(HDRP(next_bp));
        if (size < rsize)
            return NULL;
        remove_block(next_bp);
        PUT(HDRP(bp), (PACK(size, 1) | GET_PREV_ALLOC(HDRP(bp))));
        SET_PREV_ALLOC(HDRP(NEXT_BLKP(bp)));
    }

    else if (!prev_alloc && next_alloc) {      /* Case 3 */
        size += GET_SIZE(FTRP(prev_bp));
        if (size < rsize)
            return NULL;
        remove_block(prev_bp);
        bp = prev_bp;
        PUT(HDRP(bp), (PACK(size, 1) | GET_PREV_ALLOC(HDRP(bp))));
        SET_PREV_ALLOC(HDRP(NEXT_BLKP(bp)));
    }

    else {                                     /* Case 4 */
        size += GET_SIZE(HDRP(next_bp)) +
            GET_SIZE(FTRP(prev_bp));
        if (size < rsize)
            return NULL;
        remove_block(prev_bp);
        remove_block(next_bp);
        bp = prev_bp;
        PUT(HDRP(bp), (PACK(size, 1) | GET_PREV_ALLOC(HDRP(bp))));
        SET_PREV_ALLOC(HDRP(NEXT_BLKP(bp)));
    }

    return bp;
}


static unsigned int ceil_log2(unsigned int v)
{
    unsigned int _count = 0, _tmp = 1;
    while (_tmp < v) {
        _tmp <<= 1;
        _count++;
    }
    return _count;
}

/*
 * get_listid - return the correct list's id for a block with size bsize
 */
static unsigned int get_listid(size_t bsize)
{
    if (bsize <= threshold)
        return (bsize / ALIGNMENT) - 1;
    else if (bsize >= last_list_sizeover)
        return (small_list_num+large_list_num)-1;
    else return ceil_log2(bsize)-2;
}

/*
 * remove_block - remove the free block from its list
 */
static void remove_block(void *bp)
{
    unsigned int prev = LIST_PREV(bp);
    unsigned int next = LIST_NEXT(bp);
    if (GET_SIZE(HDRP(bp)) == MinBlockSize) {
        void *prev_bp = get_pointer(prev);
        void *next_bp = get_pointer(next);
        if (prev_bp)
            PUT(prev_bp, next);
        else
            segListHeaders[0] = next;

        if (next_bp)
            PUT(HDRP(next_bp), prev | GET_PREV_ALLOC(HDRP(next_bp)));
        return;
    }

    if (prev) {
        void *prev_bp = get_pointer(prev);
        set_next(prev_bp, next);
    }
    else {
        unsigned int listid = get_listid(GET_SIZE(HDRP(bp)));
        segListHeaders[listid] = next;
    }

    if (next) {
        void *next_bp = get_pointer(next);
        set_prev(next_bp, prev);
    }
}

/*
 * insert_block - insert a free block into the corresponding list
 */
static void insert_block(void *bp)
{
    size_t bsize = GET_SIZE(HDRP(bp));
    unsigned int listid = 0;
    unsigned int waddr = get_waddr(bp);
    void *next_bp;

    if (bsize == MinBlockSize) {
        next_bp =  get_pointer(segListHeaders[listid]);
        PUT(bp, (segListHeaders[listid] | 0x4 | GET_PREV_ALLOC(HDRP(bp))));
        PUT(HDRP(bp), (0x4 | GET_PREV_ALLOC(HDRP(bp))));
        if (next_bp)
            PUT(HDRP(next_bp), (waddr | 0x4 | GET_PREV_ALLOC(HDRP(next_bp))));
        segListHeaders[listid] = (waddr | 0x4 | GET_PREV_ALLOC(HDRP(bp)));
        return;
    }

    listid = get_listid(bsize);
    next_bp = get_pointer(segListHeaders[listid]);

    if (bsize <= threshold) {   // insert to small size list
        set_next(bp, segListHeaders[listid]);
        set_prev(bp, 0U);
        if (next_bp)
            set_prev(next_bp, waddr);
        segListHeaders[listid] = waddr;
    }
    else {
        if (!next_bp) {   // empty list
            set_prev(bp, 0U);
            set_next(bp, 0U);
            segListHeaders[listid] = waddr;
        }
        else {
            void *p = next_bp;

            /* find the correct position */
            while (GET_SIZE(HDRP(p)) < bsize && LIST_NEXT(p))
                p = get_pointer(LIST_NEXT(p));

            if (!LIST_NEXT(p)) {      // insert at the end
                set_prev(bp, get_waddr(p));
                set_next(bp, 0U);
                set_next(p, waddr);
            }
            else {
                if (p == next_bp) {   // insert at the front
                    set_next(bp, segListHeaders[listid]);
                    set_prev(bp, 0U);
                    set_prev(next_bp, waddr);
                    segListHeaders[listid] = waddr;
                }
                else {                // insert in the middle
                    next_bp = get_pointer(LIST_NEXT(p));
                    set_prev(bp, get_waddr(p));
                    set_next(bp, get_waddr(next_bp));
                    set_prev(next_bp, waddr);
                    set_next(p, waddr);
                }
            }
        }
    }
}

/*
 * place - Place block of asize bytes at start of free block bp
 *         and split if remainder would be at least minimum block size
 */
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    unsigned int prev_alloc = GET_PREV_ALLOC(HDRP(bp));
    remove_block(bp);

    if ((csize - asize) >= MinBlockSize) {
        if (asize == MinBlockSize)
            PUT(HDRP(bp), (0x5 | prev_alloc));
        else {
            PUT(HDRP(bp), (PACK(asize, 1) | prev_alloc));
            PUT(FTRP(bp), PACK(asize, 1));
        }

        bp = NEXT_BLKP(bp);
        if (csize-asize == MinBlockSize) {
            PUT(HDRP(bp), (0x4 | 0x2));
            PUT(bp, 0x4);
        }
        else {
            PUT(HDRP(bp), (PACK(csize-asize, 0) | 0x2));
            PUT(FTRP(bp), PACK(csize-asize, 0));
        }
        insert_block(bp);
    }
    else {
        if (csize == MinBlockSize)
            PUT(HDRP(bp), (0x5 | prev_alloc));
        else {
            PUT(HDRP(bp), (PACK(csize, 1) | prev_alloc));
            PUT(FTRP(bp), PACK(csize, 1));
        }
        SET_PREV_ALLOC(HDRP(NEXT_BLKP(bp)));
    }
}

/*
 * find_fit - Find a fit for a block with asize bytes
 */
static void *find_fit(size_t asize)
{
    void *bp;
    unsigned int listid;

    /* Best-fit search */
    for (listid = get_listid(asize); listid < small_list_num+large_list_num; listid++) {
        bp = get_pointer(segListHeaders[listid]);
        if (!bp) continue;
        while (GET_SIZE(HDRP(bp)) < asize && LIST_NEXT(bp))
            bp = get_pointer(LIST_NEXT(bp));
        if (GET_SIZE(HDRP(bp)) >= asize)
            return bp;
    }

    return NULL;
}