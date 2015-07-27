/**
 *  bitvector.c
 *  
 *  Hiroshi Tokaku <tkk@hongo.wide.ad.jp>
 **/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "common.h"
#include "bit_utils.h"
#include "bitvector.h"
#include "prefetch.h"

#define bv_malloc malloc
#define bv_free free

struct bit_vector*
bv_create(elem_t bit_size)
{
    struct bit_vector *bv;
    elem_t size = ROUNDUP128((ROUNDUP8(bit_size) >> 3));
    //elem_t size = ROUNDUP256((ROUNDUP8(bit_size) >> 3));
    size_t msize = sizeof(struct bit_vector) +
                   sizeof(uint8_t) * size;
    bv = (struct bit_vector *) bv_malloc(msize);
    if (bv == NULL) return NULL;

    bv->allocated = size;
    bv->size = bit_size;
    memset(bv->arr, 0, sizeof(uint8_t) * size);

    return bv;
}

void
bv_destroy(struct bit_vector* bv)
{
    bv_free(bv);
}

void
bv_prefetch(struct bit_vector* bv)
{
    uint8_t* arr = bv->arr;
    rte_prefetch2(arr);
    //elem_t allocated = bv->allocated;
    //for (int i = 0; i < allocated; i += 8) 
    //    rte_prefetch2(arr+i);
}

void
bv_set(struct bit_vector* bv, elem_t index, bool val)
{
    assert(index <= bv->size);
    int bit_index = index & 7U;
    int byte_index = index >> 3;
    int n = bv->arr[byte_index] & ~(1 << (bit_index));
    bv->arr[byte_index] = n | (val << (bit_index));
}

bool
bv_value(struct bit_vector* bv, elem_t index)
{
    assert(index <= bv->size);
    return (bv->arr[(index >> 3)] >> (index & 7U));
}

struct bit_vector*
bv_not(struct bit_vector* bv1)
{
    struct bit_vector* bv2 = bv_create(bv1->size);
    if (bv2 == NULL) return NULL;

    int count = bv1->size;
    uint8_t *arr1 = bv1->arr;
    uint8_t *arr2 = bv2->arr;
    for (int i = 0; i < count; i++) {
        arr2[i] = ~arr1[i];
    }
    return bv2;
}

int
bv_ffs(struct bit_vector* bv)
{
    elem_t count = bv->allocated >> 3;
    uint8_t *arr = bv->arr;
    for (int i = 0; i < count; i++) {
        uint64_t cur = *(uint64_t*)(arr + (i << 3));
        int lsb = ffsll(cur);
        //LOG (INFO, "i=%d ffsll(%llu) = %d\n", i, cur, lsb);
        if (unlikely(lsb)) return lsb-1 + (i << 6);
    }
    return -1;
}

struct bit_vector*
bv_and(struct bit_vector* bv1, struct bit_vector* bv2)
{
    elem_t bit_size = min(bv1->size, bv2->size);
    struct bit_vector* bv3 = bv_create(bit_size);
    if (bv3 == NULL) return NULL;

    uint8_t *arr1 = bv1->arr;
    uint8_t *arr2 = bv2->arr;
    uint8_t *arr3 = bv3->arr;
    int count = bv3->allocated;
    for (int i = 0; i < count; i++) {
        arr3[i] = arr1[i] & arr2[i];
    }
    return bv3;
}

struct bit_vector*
bv_or(struct bit_vector* bv1, struct bit_vector* bv2)
{
    elem_t bit_size = min(bv1->size, bv2->size);
    struct bit_vector* bv3 = bv_create(bit_size);
    if (bv3 == NULL) return NULL;

    uint8_t *arr1 = bv1->arr;
    uint8_t *arr2 = bv2->arr;
    uint8_t *arr3 = bv3->arr;
    int count = bv3->allocated;
    for (int i = 0; i < count; i++) {
        arr3[i] = arr1[i] | arr2[i];
    }
    return bv3;
}

struct bit_vector*
bv_xor(struct bit_vector* bv1, struct bit_vector* bv2)
{
    elem_t bit_size = min(bv1->size, bv2->size);
    struct bit_vector* bv3 = bv_create(bit_size);
    if (bv3 == NULL) return NULL;

    uint8_t *arr1 = bv1->arr;
    uint8_t *arr2 = bv2->arr;
    uint8_t *arr3 = bv3->arr;
    int count = bv3->allocated;
    for (int i = 0; i < count; i++) {
        arr3[i] = arr1[i] ^ arr2[i];
    }
    return bv3;
}

void
_bv_and(struct bit_vector* bv1, struct bit_vector* bv2)
{
    int count = min(bv1->allocated, bv2->allocated);
 
    uint8_t *arr1 = bv1->arr;
    uint8_t *arr2 = bv2->arr;
 
    for (int i = 0; i < count; i++) {
        arr1[i] &= arr2[i];
    }
}

void
_bv_or(struct bit_vector* bv1, struct bit_vector* bv2)
{
    int count = min(bv1->allocated, bv2->allocated);
 
    uint8_t *arr1 = bv1->arr;
    uint8_t *arr2 = bv2->arr;
 
    for (int i = 0; i < count; i++) {
        arr1[i] |= arr2[i];
    }
}

void
_bv_xor(struct bit_vector* bv1, struct bit_vector* bv2)
{
    int count = min(bv1->allocated, bv2->allocated);
 
    uint8_t *arr1 = bv1->arr;
    uint8_t *arr2 = bv2->arr;
 
    for (int i = 0; i < count; i++) {
        arr1[i] ^= arr2[i];
    }
}

void
__bv_and(struct bit_vector* dst, struct bit_vector* bv1, struct bit_vector* bv2)
{
    uint8_t *arr1 = bv1->arr;
    uint8_t *arr2 = bv2->arr;
    uint8_t *arr3 = dst->arr;
    int count = dst->allocated;
    for (int i = 0; i < count; i++) {
        arr3[i] = arr1[i] & arr2[i];
    }
}

void
__bv_or(struct bit_vector* dst, struct bit_vector* bv1, struct bit_vector* bv2)
{
    uint8_t *arr1 = bv1->arr;
    uint8_t *arr2 = bv2->arr;
    uint8_t *arr3 = dst->arr;
    int count = dst->allocated;
    for (int i = 0; i < count; i++) {
        arr3[i] = arr1[i] | arr2[i];
    }
}

void
__bv_xor(struct bit_vector* dst, struct bit_vector* bv1, struct bit_vector* bv2)
{
    uint8_t *arr1 = bv1->arr;
    uint8_t *arr2 = bv2->arr;
    uint8_t *arr3 = dst->arr;
    int count = dst->allocated;
    for (int i = 0; i < count; i++) {
        arr3[i] = arr1[i] ^ arr2[i];
    }
}

void
bv_print(struct bit_vector* bv)
{
    LOG(INFO, "bit_vector: %p size : %lu\n"
        "arr       : %p\n", bv, bv->size, bv->arr);
    int size = (bv->size >> 3) - 1 + ((bv->size & 7U) ? 1 : 0);
    for (int i = size; i >= 0; --i) {
        uint8_t val = bv->arr[i];
        for (int j = 7; j >= 0; --j) 
            printf("%u", val >> j & 1);
        if (!(i & 3)) 
            printf("\n");
        else
            printf(" ");
    }
    printf("\n");
}

