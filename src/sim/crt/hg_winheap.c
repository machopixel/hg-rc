/* The game's Windows-style heaps: see hg_winheap.h. */
#include <stdlib.h>
#include <string.h>
#include "hg_winheap.h"

typedef struct HgWinHeap HgWinHeap;

#define HG_BLOCK_MAGIC 0x50484748u  /* "HGHP" */
#define HG_BLOCK_KEY 0x3c96a5e1u

typedef struct HgBlock {
    uint32_t magic;
    uint32_t check;         /* the head's own address ^ HG_BLOCK_KEY; 0 once freed */
    struct HgBlock *prev, *next;
    HgWinHeap *heap;
    size_t size;
} HgBlock;

/* the head rounded up to 16 bytes, so the block keeps malloc's alignment */
#define HG_HEAD_SIZE ((sizeof(HgBlock) + 15u) & ~(size_t)15u)
#define HG_HEAD(block) ((HgBlock *)((char *)(block) - HG_HEAD_SIZE))
#define HG_DATA(head) ((void *)((char *)(head) + HG_HEAD_SIZE))

struct HgWinHeap {
    HgWinHeap *next;        /* the list of live heaps */
    HgBlock *blocks;
    size_t maximum;         /* 0: growable */
    size_t used;
};

static HgWinHeap *g_heaps;

static HgWinHeap *find_heap(uint32_t heap)
{
    HgWinHeap *h;

    for (h = g_heaps; h != NULL; h = h->next)
        if ((uint32_t)(uintptr_t)h == heap)
            return h;
    return NULL;
}

/* the block's head when the block belongs to h (its head says so: no walk through the heap's list) */
static HgBlock *find_block(HgWinHeap *h, const void *block)
{
    HgBlock *b;

    if (block == NULL)
        return NULL;
    b = HG_HEAD(block);
    if (b->magic != HG_BLOCK_MAGIC || b->check != ((uint32_t)(uintptr_t)b ^ HG_BLOCK_KEY) || b->heap != h)
        return NULL;
    return b;
}

static void link_block(HgWinHeap *h, HgBlock *b)
{
    b->magic = HG_BLOCK_MAGIC;
    b->check = (uint32_t)(uintptr_t)b ^ HG_BLOCK_KEY;
    b->heap = h;
    b->prev = NULL;
    b->next = h->blocks;
    if (h->blocks != NULL)
        h->blocks->prev = b;
    h->blocks = b;
}

static void unlink_block(HgWinHeap *h, HgBlock *b)
{
    b->check = 0;
    if (b->prev != NULL)
        b->prev->next = b->next;
    else
        h->blocks = b->next;
    if (b->next != NULL)
        b->next->prev = b->prev;
}

static int fits(const HgWinHeap *h, size_t old, size_t now)
{
    return h->maximum == 0 || now <= old || h->used - old + now <= h->maximum;
}

uint32_t Hg_HeapCreate(uint32_t options, uint32_t initial, uint32_t maximum)
{
    HgWinHeap *h = (HgWinHeap *)calloc(1, sizeof(HgWinHeap));

    (void)options;
    (void)initial;
    if (h == NULL)
        return 0;
    h->maximum = maximum;
    h->next = g_heaps;
    g_heaps = h;
    return (uint32_t)(uintptr_t)h;
}

int Hg_HeapDestroy(uint32_t heap)
{
    HgWinHeap *h = find_heap(heap), **link;
    HgBlock *b, *next;

    if (h == NULL)
        return 0;
    for (b = h->blocks; b != NULL; b = next) {
        next = b->next;
        free(b);
    }
    for (link = &g_heaps; *link != h; link = &(*link)->next)
        ;
    *link = h->next;
    free(h);
    return 1;
}

void *Hg_HeapAlloc(uint32_t heap, uint32_t flags, uint32_t bytes)
{
    HgWinHeap *h = find_heap(heap);
    HgBlock *b;

    if (h == NULL || bytes > 0x7ffdefffu || !fits(h, 0, bytes))
        return NULL;
    /* 0 bytes gives a block, as Windows' */
    b = (HgBlock *)((flags & HG_HEAP_ZERO_MEMORY) != 0 ? calloc(1, HG_HEAD_SIZE + (bytes != 0 ? bytes : 1))
                                                         : malloc(HG_HEAD_SIZE + (bytes != 0 ? bytes : 1)));
    if (b == NULL)
        return NULL;
    b->size = bytes;
    h->used += bytes;
    link_block(h, b);
    return HG_DATA(b);
}

void *Hg_HeapReAlloc(uint32_t heap, uint32_t flags, void *block, uint32_t bytes)
{
    HgWinHeap *h = find_heap(heap);
    HgBlock *b, *moved;
    size_t old;

    if (h == NULL || bytes > 0x7ffdefffu)
        return NULL;
    b = find_block(h, block);
    if (b == NULL)
        return NULL;
    old = b->size;
    if (!fits(h, old, bytes))
        return NULL;
    if (bytes > old && (flags & HG_HEAP_REALLOC_IN_PLACE_ONLY) != 0)
        return NULL;
    if (bytes > old) {
        unlink_block(h, b);
        moved = (HgBlock *)realloc(b, HG_HEAD_SIZE + bytes);
        if (moved == NULL) {
            link_block(h, b);
            return NULL;
        }
        b = moved;
        link_block(h, b);
        if ((flags & HG_HEAP_ZERO_MEMORY) != 0)
            memset((char *)HG_DATA(b) + old, 0, bytes - old);
    }
    /* a smaller size keeps the block where it is (its memory stays allocated) */
    b->size = bytes;
    h->used = h->used - old + bytes;
    return HG_DATA(b);
}

int Hg_HeapFree(uint32_t heap, uint32_t flags, void *block)
{
    HgWinHeap *h = find_heap(heap);
    HgBlock *b;

    (void)flags;
    if (h == NULL)
        return 0;
    if (block == NULL)
        return 1;
    b = find_block(h, block);
    if (b == NULL)
        return 0;
    unlink_block(h, b);
    h->used -= b->size;
    free(b);
    return 1;
}

uint32_t Hg_HeapSize(uint32_t heap, uint32_t flags, const void *block)
{
    HgWinHeap *h = find_heap(heap);
    HgBlock *b = h != NULL ? find_block(h, block) : NULL;

    (void)flags;
    return b != NULL ? (uint32_t)b->size : 0xffffffffu;
}

int Hg_HeapValidate(uint32_t heap, uint32_t flags, const void *block)
{
    HgWinHeap *h = find_heap(heap);

    (void)flags;
    if (h == NULL)
        return 0;
    return block == NULL || find_block(h, block) != NULL;
}

int Hg_HeapOwns(uint32_t heap)
{
    return find_heap(heap) != NULL;
}
