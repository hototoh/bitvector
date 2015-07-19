#ifndef BITVECTOR_H
#define BITVECTOR_H

#include <stdint.h>
#include <stdbool.h>

#define BV_ROUNDUP8(x) (((x) + 7UL) & (~7UL))
#define BV_ROUNDUP16(x) (((x) + 15UL) & (~15UL))
#define BV_ROUNDUP32(x) (((x) + 31UL) & (~31UL))
#define BV_ROUNDUP64(x) (((x) + 63UL) & (~63UL))
#define BV_ROUNDUP128(x) (((x) + 127UL) & (~127UL))
#define BV_ROUNDUP4K(x) (((x) + 4095UL) & (~4095UL))
#define BV_ROUNDUP1M(x) (((x) + 1048575UL) & (~1048575UL))
#define BV_ROUNDUP2M(x) (((x) + 2097151UL) & (~2097151UL))

typedef size_t elem_t;

struct bit_vector {
    // allocated bit vector byte size
    elem_t size; 
    
    // bit vector body
    uint8_t arr[0] __attribute__((aligned(16)));
} ;

struct bit_vector*
bv_create(elem_t size);

void
bv_destroye(struct bit_vector* bv);
 
void
bv_set(struct bit_vector* bv, elem_t index, bool val);

bool
bv_value(struct bit_vector* bv,elem_t index);

struct bit_vector*
bv_not(struct bit_vector* bv1);

struct bit_vector*
bv_and(struct bit_vector* bv1, struct bit_vector* bv2);

struct bit_vector*
bv_or(struct bit_vector* bv1, struct bit_vector* bv2);

struct bit_vector*
bv_xor(struct bit_vector* bv1, struct bit_vector* bv2);

struct bit_vector*
bv_plus(struct bit_vector* bv1, struct bit_vector* bv2);

struct bit_vector*
bv_minus(struct bit_vector* bv1, struct bit_vector* bv2);




#endif
