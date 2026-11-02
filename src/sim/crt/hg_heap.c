/* C runtime replacement, memory: see hg_heap.h. */
#include <stdint.h>
#include <stdlib.h>
#include "hg_heap.h"

#define HG_HEAP_MAGIC 0x4d454748u   /* "HGEM" */
#define HG_HEAP_KEY 0x5a17c3e9u

/* 16 bytes, so the block keeps the alignment malloc gave the head */
typedef struct HgHeapHead {
    uint32_t magic;
    uint32_t check;     /* the head's own address ^ HG_HEAP_KEY: a stray "HGEM" in front of a DLL block never matches */
    uint32_t size;
    uint32_t unused;
} HgHeapHead;

static void *hg_heap_open(HgHeapHead *h, size_t size)
{
    if (h == NULL)
        return NULL;
    h->magic = HG_HEAP_MAGIC;
    h->check = (uint32_t)(uintptr_t)h ^ HG_HEAP_KEY;
    h->size = (uint32_t)size;
    h->unused = 0;
    return h + 1;
}

int hg_heap_owns(const void *block)
{
    const HgHeapHead *h = (const HgHeapHead *)block - 1;

    return block != NULL && h->magic == HG_HEAP_MAGIC && h->check == ((uint32_t)(uintptr_t)h ^ HG_HEAP_KEY);
}

void *hg_malloc(size_t size)
{
    if (size > 0xffffffe0u - sizeof(HgHeapHead))
        return NULL;
    /* malloc(0) gives a block, as the original's (it rounds 0 up to one paragraph) */
    return hg_heap_open((HgHeapHead *)malloc(sizeof(HgHeapHead) + (size != 0 ? size : 1)), size);
}

void *hg_calloc(size_t count, size_t size)
{
    size_t n = count * size;

    if (size != 0 && n / size != count)
        return NULL;
    if (n > 0xffffffe0u - sizeof(HgHeapHead))
        return NULL;
    return hg_heap_open((HgHeapHead *)calloc(1, sizeof(HgHeapHead) + (n != 0 ? n : 1)), n);
}

void *hg_realloc(void *block, size_t size)
{
    HgHeapHead *h;

    if (block == NULL)
        return hg_malloc(size);
    if (size == 0) {
        hg_free(block);
        return NULL;
    }
    if (size > 0xffffffe0u - sizeof(HgHeapHead))
        return NULL;
    h = (HgHeapHead *)realloc((HgHeapHead *)block - 1, sizeof(HgHeapHead) + size);
    if (h == NULL)
        return NULL;   /* the old block stays, as the original */
    return hg_heap_open(h, size);
}

void hg_free(void *block)
{
    HgHeapHead *h;

    if (block == NULL)
        return;
    h = (HgHeapHead *)block - 1;
    h->magic = 0;
    free(h);
}
