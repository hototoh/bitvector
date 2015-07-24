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

#include "common.h"
#include "bit_utils.h"
#include "bitvector.h"

void
macro_test()
{
    assert(ROUNDUP8(0) == 0);
    assert(ROUNDUP8(1) == 8);
    assert(ROUNDUP8(2) == 8);
    assert(ROUNDUP8(8) == 8);
    assert(ROUNDUP8(9) == 16);
    assert(ROUNDUP32(0) == 0);
    assert(ROUNDUP32(1) == 32);
    assert(ROUNDUP32(16) == 32);
    assert(ROUNDUP32(31) == 32);
    assert(ROUNDUP32(32) == 32);
    assert(ROUNDUP32(33) == 64);
    assert(ROUNDUP128(0) == 0);
    assert(ROUNDUP128(1) == 128);
    assert(ROUNDUP128(2) == 128);
    assert(ROUNDUP128(127) == 128);
    assert(ROUNDUP128(128) == 128);
    assert(ROUNDUP128(129) == 256);
}

int
main()
{
    macro_test();

    //for (int i = 0; i <= 1024 * 32; i++) {
    for (int i = 0; i <= 1024; i++) {
        int size = i;
        struct bit_vector* bv = bv_create(size);
        assert(bv != NULL);
        for (int j = 0; j < size; ++j) {
            bv_set(bv, j, true);
        }
        bv_print(bv);
        for (int j = 0; j < size; ++j) {
            int res = bv_ffs(bv);
            //LOG(INFO, "i=%d: j=%d =?= bv_ffs(bv)=%d  ?\n", i, j, res);
            assert(j == res);
            bv_set(bv, j, false);
        }
        assert(-1 == bv_ffs(bv));
        bv_destroy(bv);
    }
}
