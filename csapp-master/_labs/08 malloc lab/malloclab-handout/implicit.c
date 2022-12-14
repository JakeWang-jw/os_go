/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "JW",
    /* First member's full name */
    "wangjingwei",
    /* First member's email address */
    "wangjingwei@tp-link.com.cn",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

#define SIZE_T_SIZE sizeof(size_t)

/* Base constants and macros */
#define WSIZE		4		/* Word and header/footer size (bytes) */
#define DSIZE		8		/* Double word size (bytes) */
#define CHUNKSIZE	(1<<12)	/* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word*/
#define PACK(size, alloc)	((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)		(*(unsigned int *)(p))
#define PUT(p, val)	(*(unsigned int *)(p) = val)

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)		(GET(p) & ~0x7)
#define GET_ALLOC(p)	(GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)	((char* )(bp) - WSIZE)
#define FTRP(bp)	((char* )(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)	((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)	((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Heap prologue block pointer*/
static void *heap_listp;

static void *coalesce(void *ptr)
{
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(ptr)));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
	size_t size = GET_SIZE(HDRP(ptr));

	if (prev_alloc && next_alloc) {				/* Case 1 */
		return ptr;
	} else if (prev_alloc && !next_alloc) {		/* Case 2 */
		size += GET_SIZE(HDRP(NEXT_BLKP(ptr)));
		PUT(HDRP(ptr), PACK(size, 0));
		PUT(FTRP(ptr), PACK(size, 0));
	} else if (!prev_alloc && next_alloc) {		/* Case 3 */
		size += GET_SIZE(FTRP(PREV_BLKP(ptr)));
		PUT(FTRP(ptr), PACK(size, 0));
		PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
		ptr = PREV_BLKP(ptr);
	} else if (!prev_alloc && !next_alloc) {	/* Case 4 */
		size += GET_SIZE(HDRP(PREV_BLKP(ptr))) 
			+ GET_SIZE(FTRP(NEXT_BLKP(ptr)));
		PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
		PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 0));
		ptr = PREV_BLKP(ptr);
	}

	return ptr;
}

static void *extend_heap(size_t words)
{
	char *bp;
	size_t size;
	
	/* Allocate an even number of words to maintain alignment */
	size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
	if ((long)(bp = mem_sbrk(size)) == -1)
		return NULL;
	
	/* Initialize free block header/footer and the epilogue header */
	PUT(HDRP(bp), PACK(size, 0));			/* Free block header */
	PUT(FTRP(bp), PACK(size, 0));			/* Free block footer */
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));	/* New epilogue header */
	
	/* Coalesce if the previous block was free */
	return coalesce(bp);
}

static void *find_fit(size_t asize)
{
    void *bp = NEXT_BLKP(heap_listp);

    while (GET_SIZE(HDRP(bp)) != 0)
    {
        if (GET_ALLOC(HDRP(bp)) == 0 &&
            GET_SIZE(HDRP(bp)) >= asize)
        {
            break;
        }

        bp = NEXT_BLKP(bp);
    }

    if (GET_SIZE(HDRP(bp)) == 0)
    {
        goto end;
    }

    return bp;
end:
    return NULL;
}

static void place(void *bp, size_t asize)
{
    size_t remain_size = GET_SIZE(HDRP(bp)) - asize;

    if (asize <= GET_SIZE(HDRP(bp)) - WSIZE * 2)
    {
        PUT(FTRP(bp), PACK(remain_size, 0));
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        PUT(HDRP(NEXT_BLKP(bp)), PACK(remain_size, 0));
    }
    else
    {
        PUT(HDRP(bp), PACK(GET_SIZE(HDRP(bp)), 1));
        PUT(FTRP(bp), PACK(GET_SIZE(HDRP(bp)), 1));
    }
}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
    	return -1;

    PUT(heap_listp, 0);								/* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));	/* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));	/* Prologue footer */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));		/* Epilogue header */
    heap_listp += (2*WSIZE);

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) {
    	return -1;
   	}

    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;		/* Adjusted block size */
    size_t extendsize;	/* Amount to extend heap if no fit */
   	char *bp;

   	/* Ignore spurious requests */
   	if (size == 0)
    {
   		return NULL;
    }

   	/* Adjust block size to include overhead and alignment reqs. */
   	if (size <= DSIZE)
   		asize = 2 * DSIZE;
   	else
   		asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);

   	/* Search the free list for a fit */
   	if ((bp = find_fit(asize)) != NULL) {
   		place(bp, asize);
   		return bp;
   	}

   	/* No fit found. Get more memory and place the block */
   	extendsize = MAX(asize, CHUNKSIZE);
   	if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
   		return NULL;
   	place(bp, asize);

   	return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t asize = 0;
    size_t old_size = GET_SIZE(HDRP(ptr));
    void *new_ptr = NULL;

    if (ptr == NULL)
    {
        ptr = mm_malloc(size);
        goto end;
    }
    else if (size == 0)
    {
        mm_free(ptr);
    }

   	if (size <= DSIZE)
    {
   		asize = 2 * DSIZE;
    }
   	else
    {
   		asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
    }

    if (asize > old_size - 2 * WSIZE
        && asize <= old_size)
    {
        PUT(FTRP(ptr), PACK(asize, 1));
        PUT(HDRP(ptr), PACK(asize, 1));
        goto end;
    }
    else if (asize > old_size)
    {
        new_ptr = mm_malloc(asize);
        memcpy(new_ptr, ptr, old_size);
        mm_free(ptr);
    }
    else
    {
        place(ptr, asize);
        goto end;
    }

    return new_ptr;
end:
    return ptr;
}
