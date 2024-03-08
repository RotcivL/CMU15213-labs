/* 
 * Simple, 32-bit and 64-bit clean allocator based on implicit free
 * lists, first-fit placement, and boundary tag coalescing, as described
 * in the CS:APP3e text. Blocks must be aligned to doubleword (8 byte) 
 * boundaries. Minimum block size is 16 bytes. 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mm.h"
#include "memlib.h"

/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

// #define DEBUG // uncomment this line to enable debugging

#ifdef DEBUG
/* When debugging is enabled, these form aliases to useful functions */
#define dbg_printf(...) printf(__VA_ARGS__)
#define dbg_checkheap(verbose) mm_checkheap(verbose)
#else
/* When debugging is disnabled, no code gets generated for these */
#define dbg_printf(...)
#define dbg_checkheap(verbose)
#endif

/* Basic constants and macros */
/* double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define WSIZE       4       /* Word and header/footer size (bytes) */ 
#define DSIZE       8       /* Double word size (bytes) */
#define CHUNKSIZE  (1<<12)  /* Extend heap by this amount (bytes) */  
#define MINSIZE     16      /* Minimum size of a block */

#define MAX(x, y) ((x) > (y)? (x) : (y))  

/* Pack a size and allocated bit into a word */
#define PACK(size, prev_alloc, alloc)  ((size) | ((prev_alloc) << 1) |(alloc)) 

/* Read and write a word at address p */
#define GET(p)       (*(unsigned *)(p))            
#define PUT(p, val)  (*(unsigned *)(p) = (val))    

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)                 (GET(p) & ~0x7)                   
#define GET_ALLOC(p)                (GET(p) & 0x1)
#define GET_PREV_ALLOC(p)           ((GET(p) & 0x2) >> 1)   
#define SET_PREV_ALLOC(p, alloc)    (PUT(p, PACK(GET_SIZE(p), alloc, GET_ALLOC(p))))         

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)                      
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE) 

/* Given non-free block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE))) 
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) 

/* Given free block ptr bp, get pointers of next blocks */
#define NEXT(bp)            ((void *)(*(size_t*)(bp)))
#define SET_NEXT(bp, next)  ((*(size_t*)(bp)) = (size_t)(next))
#define FREELIST_COUNT      8
/* Global variables */
static char *heap_listp = 0;    /* Pointer to first block */  
static char *epilogue;          /* pointer to epilogue block */
static char *freeLists;         /* Pointer to start of freelist segment in heap*/


/* Function prototypes for internal helper routines */
static int in_heap(void *bp);
static int aligned(void *bp);
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void *list_insert(void *bp);
static void list_remove(void *bp);
static void *find_list(size_t size);


/* 
 * mm_init - Initialize the memory manager 
 */
int mm_init(void) {
    /* Create the initial empty heap */
    if ((freeLists = mem_sbrk(FREELIST_COUNT*DSIZE+2*WSIZE)) == (void *)-1) 
        return -1;

    /* initialise free lists to point to null*/
    memset(freeLists, 0, FREELIST_COUNT*DSIZE);

    /* set heap pointer to end of free list */
    heap_listp = freeLists+FREELIST_COUNT*DSIZE;

    PUT(heap_listp, PACK(0, 1, 1));             /* Prolgue Footer*/
    PUT(heap_listp + WSIZE, PACK(0, 1, 1));     /* Epilogue header */ 
    heap_listp += 2*WSIZE;     
    epilogue = heap_listp;        

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE) == NULL) 
        return -1;

    dbg_printf("complete init\n");

    dbg_checkheap(__LINE__);
    return 0;
}

/* 
 * malloc - Allocate a block with at least size bytes of payload 
 */
void *malloc(size_t size) {
    dbg_checkheap(__LINE__);

    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;      

    if (heap_listp == 0){
        mm_init();
    }
    /* Ignore spurious requests */
    if (size == 0) {
        dbg_checkheap(__LINE__);
        return NULL;
    }


    /* Adjust block size to include overhead and alignment reqs. */
    if (size + WSIZE <= MINSIZE)
        asize = MINSIZE;                                      
    else
        asize = ALIGN(size+WSIZE);

    /* Search the free list for a fit */
    bp = find_fit(asize);

    /* No fit found. Get more memory and place the block */
    if (bp == NULL) { 
        extendsize = MAX(asize,CHUNKSIZE);
        dbg_printf("malloc bp null\n");

        if ((bp = extend_heap(extendsize)) == NULL)  
            return NULL;  
    }

    /* Remove block from free list*/
    list_remove(bp);

    /* Place malloc block into free block */
    place(bp, asize);     

    dbg_printf("Malloc size %zd on address %p.\n", asize, bp);
    dbg_checkheap(__LINE__);                            
    return bp;
} 

/* 
 * free - Free a block 
 */
void free(void *bp) {
    if (bp == 0) 
        return;
    
    if (heap_listp == 0) {
        mm_init();
        return;
    }
    size_t size = GET_SIZE(HDRP(bp));

    int prev_alloc = GET_PREV_ALLOC(HDRP(bp));
    PUT(HDRP(bp), PACK(size, prev_alloc, 0));
    PUT(FTRP(bp), PACK(size, prev_alloc, 0));

    bp = coalesce(bp);
    SET_PREV_ALLOC(HDRP(NEXT_BLKP(bp)), 0);
    dbg_printf("hdr %d on address %p.\n", GET(HDRP(NEXT_BLKP(bp))), NEXT_BLKP(bp));
    dbg_printf("free size %zd on address %p.\n", size, bp);
    dbg_checkheap(__LINE__);


}

/*
 * realloc - Naive implementation of realloc
 */
void *realloc(void *ptr, size_t size) {
    size_t oldsize;
    void *newptr;

    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0) {
        mm_free(ptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if(ptr == NULL) {
        return mm_malloc(size);
    }

    newptr = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if(!newptr) {
        return 0;
    }

    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(ptr));
    if(size < oldsize) oldsize = size;
    memcpy(newptr, ptr, oldsize);

    /* Free the old block. */
    mm_free(ptr);

    return newptr;
}

/* 
 * mm_checkheap - Check the heap for correctness. Helpful hint: You
 *                can call this function using mm_checkheap(__LINE__);
 *                to identify the line number of the call site.
 */
void mm_checkheap(int lineno) { 
    
    // prologue at start of heap, epilogue at end of heap
    void *prologue = heap_listp-WSIZE;
    if (!in_heap(HDRP(prologue))) {
        printf("Prologue not in heap bounds %p\n", HDRP(prologue));
        printf("Error in line %d\n", lineno);
    }

    if (mem_heap_hi()+1 != epilogue) {
        printf("Epilogue not last block in heap");
        printf("Error in line %d\n", lineno);
    }

    // block aligned
    void *bp;
    int free_count = 0;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!in_heap(bp)) {
            printf("Block not in heap\n");
            printf("Error in line %d\n", lineno);
        }
        if (!aligned(bp)) {
            printf("Block not aligned\n");
            printf("Error in line %d\n", lineno);
        }
        if (GET_SIZE(HDRP(bp)) < MINSIZE) {
            printf("Block too small, %d instead of %d\n", GET_SIZE(HDRP(bp)), MINSIZE);
            printf("Error in line %d\n", lineno);
        }
        if (!GET_ALLOC(HDRP(bp))) {
            free_count++;
            if (!GET_ALLOC(HDRP(NEXT_BLKP(bp)))) {
                printf("two contiguous free blocks not yet coalesced\n");
                printf("Error in line %d\n", lineno);
            }
            if (GET(HDRP(bp)) != GET(FTRP(bp))) {
                printf("Mismatch in header and footer contents\n");
                printf("Error in line %d\n", lineno);
            }
        } 
        if (GET_ALLOC(HDRP(bp)) != GET_PREV_ALLOC(HDRP(NEXT_BLKP(bp)))) {
            printf("Mismatch in alloc of current block %p and prev alloc of next block\n", bp);
            printf("alloc: %d, prev_aloc: %d\n", GET_ALLOC(HDRP(bp)), GET_PREV_ALLOC(HDRP(NEXT_BLKP(bp))));
            printf("Error in line %d\n", lineno);
        }
    }

    void *freeListEnd = freeLists+(FREELIST_COUNT*DSIZE);

    for (void *list = freeLists; list < freeListEnd; list+=DSIZE) {
        void *curr = list;
        while (NEXT(curr)) {
            free_count--;
            void *bp = NEXT(curr);
            if (GET_ALLOC(HDRP(bp))) {
                printf("Allocated block in free list\n");
                printf("Error in line %d\n", lineno);
            }
            curr = NEXT(curr);
        }
    }


    if (free_count) {
        printf("Free list size and number of free blocks mismatch, %d diffrence\n", free_count);
        printf("Error in line %d\n", lineno);
    }
    
    for (void *list = freeLists; list < freeListEnd; list+=DSIZE) {
        void *slow = list, *fast = list;
        while (NEXT(fast) && NEXT(NEXT(fast))) {
            slow = NEXT(slow);
            fast = NEXT(NEXT(fast));
            if (slow == fast) {
                printf("Cycle in free list\n");
                printf("Error in line %d\n", lineno);
                break;
            }
        }
    }

}

/* 
 * Checkheap helper routines
 */

/*
 * in_heap - checks if blockpoint is within the allocated heap
 */
static int in_heap(void *bp) {
    return bp >= mem_heap_lo() && bp <= mem_heap_hi();
}

/*
 * Aligned - check if block is aligned
 */
static int aligned(void *bp) {
    return !((size_t)bp & 0x7);
}
/* 
 * The remaining routines are internal helper routines 
 */

/* 
 * extend_heap - Extend heap with free block and return its block pointer
 */
static void *extend_heap(size_t size) {
    char *bp;

    if ((long)(bp = mem_sbrk(size)) == -1)  
        return NULL;                                        

    int prev_alloc = GET_PREV_ALLOC(HDRP(epilogue));
    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, prev_alloc, 0));         /* Free block header */   
    PUT(FTRP(bp), PACK(size, prev_alloc, 0));         /* Free block footer */   
    epilogue += size;
    PUT(HDRP(epilogue), PACK(0, 0, 1)); /* New epilogue header */ 

    /* Coalesce if the previous block was free */
    return coalesce(bp);                                          
}

/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
static void *coalesce(void *bp) {
    int prev_alloc = GET_PREV_ALLOC(HDRP(bp));
    int next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {            /* Case 1 */
        list_insert(bp);
        return bp;
    }

    else if (prev_alloc && !next_alloc) {      /* Case 2 */
        list_remove(NEXT_BLKP(bp));

        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 1, 0));
        PUT(FTRP(bp), PACK(size, 1, 0));
    }

    else if (!prev_alloc && next_alloc) {      /* Case 3 */
        list_remove(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        int prev_prev_alloc = GET_PREV_ALLOC(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, prev_prev_alloc, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, prev_prev_alloc, 0)); //
        bp = PREV_BLKP(bp);
    }

    else {                                     /* Case 4 */
        list_remove(NEXT_BLKP(bp));
        list_remove(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + 
            GET_SIZE(FTRP(NEXT_BLKP(bp)));
        int prev_prev_alloc = GET_PREV_ALLOC(HDRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, prev_prev_alloc, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, prev_prev_alloc, 0));
        bp = PREV_BLKP(bp);
    }
    list_insert(bp);

    return bp;
}

/* 
 * place - Place block of asize bytes at start of free block bp 
 *         and split if remainder would be at least minimum block size
 */
static void place(void *bp, size_t asize) {
    size_t csize = GET_SIZE(HDRP(bp));   
    int prev_alloc = GET_PREV_ALLOC(HDRP(bp));

    if ((csize - asize) >= MINSIZE) { 
        PUT(HDRP(bp), PACK(asize, prev_alloc, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-asize, 1, 0));
        PUT(FTRP(bp), PACK(csize-asize, 1, 0));
        list_insert(bp);
    }
    else { 
        PUT(HDRP(bp), PACK(csize, prev_alloc, 1));
        SET_PREV_ALLOC(HDRP(NEXT_BLKP(bp)), 1);
    }

}

/* 
 * find_fit - Find a fit for a block with asize bytes 
 */
static void *find_fit(size_t asize) {
    /* get starting list */
    void *list = find_list(asize);
    
    /* First-fit search */
    void *freeListEnd = freeLists + FREELIST_COUNT*DSIZE;
    dbg_printf("Find list size %ld on address %p endlist %p.\n", asize, list, freeListEnd);

    for (;list < freeListEnd; list +=DSIZE) {
        void *curr = list;
        while (NEXT(curr)) {
            void *bp = NEXT(curr);
            if (GET_SIZE(HDRP(bp)) >= asize) {
                return bp;
            }
            curr = NEXT(curr);
        }
    }

    return NULL; /* No fit */

}

/* 
 *  List Routines
 */

/* 
 * list_insert - insert a free block into list
 */
static void *list_insert(void *bp) {
    dbg_printf("List insert size %d on address %p.\n", GET_SIZE(HDRP(bp)),bp);
    void *list = find_list(GET_SIZE(HDRP(bp)));

    SET_NEXT(bp, NEXT(list));
    SET_NEXT(list, bp);

    return bp;
}

/*
 * list_remove - remove a block from free list
 */
static void list_remove(void *bp) {
    dbg_printf("List remove size %d on address %p.\n", GET_SIZE(HDRP(bp)),bp);
    void *list = find_list(GET_SIZE(HDRP(bp)));

    void *curr = list;
    while (NEXT(curr) && NEXT(curr) != bp) {
        curr = NEXT(curr);
    }
    if (!curr) {
        return;
    }
    SET_NEXT(curr, NEXT(bp));
}

/* 
 * find_list - find the smallest list that can contain a free block that fits the size required
 * 8 Lists in heap - lists start from 2^4 to 2^12 - 1, each list holds [2^x, 2^(x+1) - 1]
 */
static void *find_list(size_t size) {
    if (size >= (1 << 11)) 
        return freeLists+(FREELIST_COUNT-1)*DSIZE;

    void *list = freeLists;
    size >>= 4; 
    while (size > 1) {
        list += DSIZE;
        size >>= 1;
    }
    return list;
}