/**
 * Hiroshi Tokaku <tkk@hongo.wide.ad.jp>
 **/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>

#include "bitvector.h"

#define bv_malloc malloc
#define bv_free free
#define max(x, y) x < y ? y : x 
#define min(x, y) x < y ? x : y 

struct bit_vector*
bv_create(elem_t _size)
{
    struct bit_vector *bv;
    elem_t size = BV_ROUNDUP128(_size / 8);
    size_t msize = sizeof(struct bit_vector) +
                   sizeof(uint8_t) * size;
    bv = (struct bit_vector *) bv_malloc(msize);
    if (bv != NULL) {
        bv->size = size;
    }
    return bv;
}

void
bv_destroye(struct bit_vector* bv)
{
    bv_free(bv);
}
 
void
bv_set(struct bit_vector* bv, elem_t index, bool val)
{
    int bit_index = index & 7U;
    int byte_index = index >> 3;
    int n = bv->arr[byte_index] | (1 << (bit_index - 1));    
    bv->arr[byte_index] = n & (val << (bit_index - 1));
}

bool
bv_value(struct bit_vector* bv, elem_t index)
{
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

struct bit_vector*
bv_and(struct bit_vector* bv1, struct bit_vector* bv2)
{
    int count = min(bv1->size, bv2->size);
    struct bit_vector* bv3 = bv_create(count);
    if (bv3 == NULL) return NULL;

    uint8_t *arr1 = bv1->arr;
    uint8_t *arr2 = bv2->arr;
    uint8_t *arr3 = bv3->arr;
    for (int i = 0; i < count; i++) {
        arr3[i] = arr1[i] & arr2[i];
    }
    return bv3;
}

struct bit_vector*
bv_or(struct bit_vector* bv1, struct bit_vector* bv2)
{
    int count = min(bv1->size, bv2->size);
    struct bit_vector* bv3 = bv_create(count);
    if (bv3 == NULL) return NULL;

    uint8_t *arr1 = bv1->arr;
    uint8_t *arr2 = bv2->arr;
    uint8_t *arr3 = bv3->arr;
    for (int i = 0; i < count; i++) {
        arr3[i] = arr1[i] | arr2[i];
    }
    return bv3;
}

struct bit_vector*
bv_xor(struct bit_vector* bv1, struct bit_vector* bv2)
{
    int count = min(bv1->size, bv2->size);
    struct bit_vector* bv3 = bv_create(count);
    if (bv3 == NULL) return NULL;

    uint8_t *arr1 = bv1->arr;
    uint8_t *arr2 = bv2->arr;
    uint8_t *arr3 = bv3->arr;
    for (int i = 0; i < count; i++) {
        arr3[i] = arr1[i] ^ arr2[i];
    }
    return bv3;
}



