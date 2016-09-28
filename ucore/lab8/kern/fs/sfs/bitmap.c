#include <defs.h>
#include <string.h>
#include <bitmap.h>
#include <kmalloc.h>
#include <error.h>
#include <assert.h>

#define WORD_TYPE           uint32_t
// CHAR_BIT = 8
#define WORD_BITS           (sizeof(WORD_TYPE) * CHAR_BIT)

struct bitmap {
    uint32_t nbits;
    uint32_t nwords;
    WORD_TYPE *map;
};

// bitmap_create - allocate a new bitmap object.
struct bitmap *
bitmap_create(uint32_t nbits) {
    static_assert(WORD_BITS != 0);
    assert(nbits != 0 && nbits + WORD_BITS > nbits);

    struct bitmap *bitmap;
    if ((bitmap = kmalloc(sizeof(struct bitmap))) == NULL) {
        return NULL;
    }

    uint32_t nwords = ROUNDUP_DIV(nbits, WORD_BITS); // 一共需要多少个word才能装满nbits
    WORD_TYPE *map;
    if ((map = kmalloc(sizeof(WORD_TYPE) * nwords)) == NULL) {
        kfree(bitmap);
        return NULL;
    }

    bitmap->nbits = nbits, bitmap->nwords = nwords; // nbits代表一共有效的bit数，nword表示存储这些bit用了多少个word
    bitmap->map = memset(map, 0xFF, sizeof(WORD_TYPE) * nwords); // 每一位都标记为1

    /* mark any leftover bits at the end in use(0) */
    if (nbits != nwords * WORD_BITS) { // 如果存在多余的bit，要将它们标记为in use
        uint32_t ix = nwords - 1, overbits = nbits - ix * WORD_BITS;

        assert(nbits / WORD_BITS == ix);
        assert(overbits > 0 && overbits < WORD_BITS);

        for (; overbits < WORD_BITS; overbits ++) {
            bitmap->map[ix] ^= (1 << overbits); // 正在使用中的位标记为1
        }
    }
    return bitmap;
}

// bitmap_alloc - locate a cleared bit, set it, and return its index.
// 这玩意其实就是找到一个空闲块的下标
int
bitmap_alloc(struct bitmap *bitmap, uint32_t *index_store) {
    WORD_TYPE *map = bitmap->map; // 首先得到map
    uint32_t ix, offset, nwords = bitmap->nwords;
    for (ix = 0; ix < nwords; ix ++) {
        if (map[ix] != 0) { 
			// WORD_BITS = 32 * 8
            for (offset = 0; offset < WORD_BITS; offset ++) {
                WORD_TYPE mask = (1 << offset); // 一位一位来尝试
                if (map[ix] & mask) { // 如果这一位是1,表示这一块未曾使用
                    map[ix] ^= mask; // 将其标记为已经在使用， ^代表按位异或
                    *index_store = ix * WORD_BITS + offset; // 得到下标
                    return 0;
                }
            }
            assert(0);
        }
    }
    return -E_NO_MEM;
}

// bitmap_translate - according index, get the related word and mask
static void
bitmap_translate(struct bitmap *bitmap, uint32_t index, WORD_TYPE **word, WORD_TYPE *mask) {
    assert(index < bitmap->nbits);
	uint32_t ix = index / WORD_BITS; // 占用了多少个word
	uint32_t offset = index % WORD_BITS; // 偏移量
    *word = bitmap->map + ix; // 这是得到index所知识的块的word以及对应的mask
    *mask = (1 << offset);
}

// bitmap_test - according index, get the related value (0 OR 1) in the bitmap

bool
bitmap_test(struct bitmap *bitmap, uint32_t index) {
    WORD_TYPE *word, mask;
    bitmap_translate(bitmap, index, &word, &mask);
    return (*word & mask); // 好吧，果然强大
}

// bitmap_free - according index, set related bit to 1
void
bitmap_free(struct bitmap *bitmap, uint32_t index) {
    WORD_TYPE *word, mask;
    bitmap_translate(bitmap, index, &word, &mask);
    assert(!(*word & mask));
    *word |= mask; // 按位或
}

// bitmap_destroy - free memory contains bitmap
void
bitmap_destroy(struct bitmap *bitmap) {
    kfree(bitmap->map);
    kfree(bitmap);
}

// bitmap_getdata - return bitmap->map, return the length of bits to len_store
void *
bitmap_getdata(struct bitmap *bitmap, size_t *len_store) {
    if (len_store != NULL) {
        *len_store = sizeof(WORD_TYPE) * bitmap->nwords;
    }
    return bitmap->map;
}

