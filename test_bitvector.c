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
    for (int i = bv->size - 1; i >= 0; --i) {
        uint8_t val = bv->arr[i];
        for (int j = 7; j >= 0; --j) 
            printf("%u", val >> j & 1);
        if ((i & 3) == 3) 
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
    
    {
        struct bit_vector* bv = bv_create(15);
        assert(bv != NULL);
        bv_print(bv);
        for (int i = 0; i < 15; ++i) {
            printf("set %dth bit true\n", i);
            bv_set(bv, i, true);
            bv_print(bv);
        }
        for (int i = 0; i < 15; ++i) {
            printf("set %dth bit false\n", i);
            bv_set(bv, i, false);
            bv_print(bv);
        }
        bv_print(bv);
    }
}
