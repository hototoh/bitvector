/**
 * Hiroshi Tokaku <tkk@hongo.wide.ad.jp>
 **/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include "bitvector.h"

void
macro_test()
{
    assert(BV_ROUNDUP8(0) == 0);
    assert(BV_ROUNDUP8(1) == 8);
    assert(BV_ROUNDUP8(2) == 8);
    assert(BV_ROUNDUP8(8) == 8);
    assert(BV_ROUNDUP8(9) == 16);
    assert(BV_ROUNDUP32(0) == 0);
    assert(BV_ROUNDUP32(1) == 32);
    assert(BV_ROUNDUP32(16) == 32);
    assert(BV_ROUNDUP32(31) == 32);
    assert(BV_ROUNDUP32(32) == 32);
    assert(BV_ROUNDUP32(33) == 64);
    assert(BV_ROUNDUP128(0) == 0);
    assert(BV_ROUNDUP128(1) == 128);
    assert(BV_ROUNDUP128(2) == 128);
    assert(BV_ROUNDUP128(127) == 128);
    assert(BV_ROUNDUP128(128) == 128);
    assert(BV_ROUNDUP128(129) == 256);
}

void
bv_print(struct bit_vector* bv)
{
    printf("bit_vector: %p size : %lu\n"
           "arr       : %p\n", bv, bv->size, bv->arr);
    int size = bv->size / 8 - 1 + (bv->size % 8 ? 1 : 0);
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

int
main()
{
    macro_test();

    for (int i = 0; i <= 1024 * 32; i++) {
        int size = i;
        struct bit_vector* bv = bv_create(size);
        assert(bv != NULL);
        for (int j = 0; j < size; ++j) {
            bv_set(bv, j, true);
        }
        bv_print(bv);
        for (int j = 0; j < size; ++j) {
            bv_set(bv, j, false);
        }
        bv_destroy(bv);
    }

}
