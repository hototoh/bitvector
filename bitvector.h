/**
 *  bitvector.h
 * 
 *  Hiroshi Tokaku <tkk@hongo.wide.ad.jp>
 **/
#ifndef BITVECTOR_H
#define BITVECTOR_H

#include <stdint.h>
#include <stdbool.h>

typedef size_t elem_t;

struct bit_vector {
    // allocated array size
    elem_t allocated;
    // available bit length
    elem_t size; 
    
    // bit vector body
    uint8_t arr[0] __attribute__((aligned(16)));
};

struct bit_vector*
bv_create(elem_t bit_size);

void
bv_destroy(struct bit_vector* bv);

void
bv_print(struct bit_vector* bv);
 
void
bv_set(struct bit_vector* bv, elem_t index, bool val);

bool
bv_value(struct bit_vector* bv,elem_t index);

struct bit_vector*
bv_not(struct bit_vector* bv1);

// if all bit is 0, return -1
int
bv_ffs(struct bit_vector* bv);

struct bit_vector*
bv_and(struct bit_vector* bv1, struct bit_vector* bv2);

struct bit_vector*
bv_or(struct bit_vector* bv1, struct bit_vector* bv2);

struct bit_vector*
bv_xor(struct bit_vector* bv1, struct bit_vector* bv2);

void
_bv_and(struct bit_vector* bv1, struct bit_vector* bv2);

void
_bv_or(struct bit_vector* bv1, struct bit_vector* bv2);

void
_bv_xor(struct bit_vector* bv1, struct bit_vector* bv2);

#endif
