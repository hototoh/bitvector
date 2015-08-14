/**
 *  benchmark.c
 *
 *  Hiroshi Tokaku <tkk@hongo.wide.ad.jp>
 **/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#include "common.h"
#include "bit_utils.h"
#include "bitvector.h"
#ifndef ERR
#define ERR
#endif
#ifndef WARNING
#define WARNING
#endif
#ifndef INFO
#define INFO
#endif
#ifndef DEBUG
#define DEBUG
#endif
#define LOG(x, fmt, ...) \
  printf(#x " %s (%u):" fmt, __func__, __LINE__, ## __VA_ARGS__)

#define NOW get_time
#define DISPLAY(n, s, e) display_performance(n, s, e)

static double
get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double) tv.tv_sec + (double) tv.tv_usec * 1e-6;
}

static void
display_performance(int op_num, double start, double end)
{
    double dur = end - start;
    double perf = op_num / dur;
    if (perf >= 1e6) {
        printf("%lf Mops\n", perf * 1e-6);
    } else if (perf >= 1e3) {
        printf("%lf Kops\n", perf * 1e-3);
    } else {
        printf("%lf ops\n", perf);
    }
}           

typedef uint32_t testset;

struct prefix_match_rule {
    uint32_t id;
    uint32_t ip_address;    
    uint32_t prefix_len;
};

struct prefix_match_rules {
    int num;
    int count;
    struct prefix_match_rule arr[0];
};

struct prefix_match_rules *
create_prefix_match_rules(int num) {
    struct prefix_match_rules* rules;
    size_t msize = sizeof(struct prefix_match_rules) +
                   sizeof(struct prefix_match_rule) * num;
    rules = (struct prefix_match_rules*) malloc(msize);
    if (rules == NULL) return NULL;

    memset(rules, 0, msize);
    rules->num = num;
    return rules;
}

void
destroy_prefix_match_rules(struct prefix_match_rules *rules) {
    free(rules);
}

bool
resize(struct prefix_match_rules **rules)
{
    int num = (*rules)->num;
    size_t msize = sizeof(struct prefix_match_rules) +
                   sizeof(struct prefix_match_rule) * num * 2;
    
    struct prefix_match_rules *tmp;
    tmp = (struct prefix_match_rules*) realloc(*rules, msize);
    if (tmp == NULL) {
        free(*rules);
        return false;
    }

    *rules = tmp;
    (*rules)->num *= 2;
    return true;
}

bool
append(struct prefix_match_rules **rules, uint32_t id,
       uint32_t ip_address, uint32_t prefix_len)
{
    int count = (*rules)->count;
    if ((*rules)->count == (*rules)->num) {
        if (!resize(rules)) return false;
    }

    struct prefix_match_rule* rule = &(*rules)->arr[count];
    rule->id = id;
    rule->ip_address = ip_address;
    rule->prefix_len = prefix_len;
    (*rules)->count += 1;

    return true;
}

struct prefix_match_rules*
parse_rules(char* filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        LOG(ERR, "Fail to open rule file\n");
        goto err0;
    }
    
    struct prefix_match_rules *rules = create_prefix_match_rules(1);
    if (rules == NULL) {
        LOG(ERR,"failed to create test rules.\n");
        goto err1;
    }

    size_t n = 0;
    char *buffer = NULL;
    while(getline(&buffer, &n, fp) > 0) {
        uint32_t rule_id, prefix_len;
        uint32_t ip_addrs[4];
        sscanf(buffer, "%d %u.%u.%u.%u/%d", &rule_id,
               ip_addrs+3, ip_addrs+2, ip_addrs+1, ip_addrs,
               &prefix_len);
        
        uint32_t ip_address = 0;
        for (int i = 0; i < 4; i++) {
            ip_address <<=8;
            ip_address += ip_addrs[3-i];
        }
        /*
        printf("%d %u.%u.%u.%u/%d\n", rule_id,
               ip_addrs[3], ip_addrs[2], ip_addrs[1], ip_addrs[0],
               prefix_len);
        */

        if (!append(&rules, rule_id, ip_address, prefix_len)) {
            LOG(ERR, "Failed to append item\n");
            goto err2;
        }
    }

    fclose(fp);
    return rules;
err2:
    destroy_prefix_match_rules(rules);  
err1:
    fclose(fp);
err0:
    return NULL;
}

/*
void
bv_and_performance(struct bit_vector** bvs0, struct bit_vector** bvs1,
                   testset* testsets, int test_num)
{
    uint64_t dummy = 0;
    printf("bvs0[0]->size: %lu\n", bvs0[0]->size);
    struct bit_vector* dst = bv_create(bvs0[0]->size);
    if (dst == NULL) {
        LOG(ERR, "Failed to allocate dst bit vector\n");
        return ;
    }

    double start = NOW();
    for (int i = 0; i < test_num; i++) {        
        struct bit_vector* bv0 = bvs0[i];
        struct bit_vector* bv1 = bvs1[i];
        bv_and_with_dst_128(dst, bv0, bv1);
        dummy += dst->arr[testsets[i]];
    }
    double end = NOW();

    bv_destroy(dst);
    printf("dummy_print: %lu\n", dummy);
    DISPLAY(test_num, start, end);
    return ;
}
*/

void
bv_and_performance(struct bit_vector** bvs0, testset* testsets, int test_num)
{
    uint64_t dummy = 0;
    struct bit_vector* dst = bv_create(bvs0[0]->size);
    if (dst == NULL) {
        LOG(ERR, "Failed to allocate dst bit vector\n");
        return ;
    }

    LOG(INFO, "start warm_up performance \n");
    for (int i = 0; i < test_num-1; ++i) {
        struct bit_vector* bv0 = bvs0[i];
        struct bit_vector* bv1 = bvs0[i+1];
        bv_and_with_dst_128(dst, bv0, bv1);
    }
    LOG(INFO, "[SUCCESS] performance test\n\n");

    int count = 16000;
    double start = NOW();
    for (int j = 0; j < count; ++j) {
        for (int i = 0; i < test_num; i++) {        
            int rand_index = testsets[i];
            struct bit_vector* bvs[8];
            
            bvs[0] = bvs0[i];
            bvs[1] = bvs0[(i+j+1) & (test_num-1)];
            bvs[2] = bvs0[(i+(j+1)*2) & (test_num-1)];
            bvs[3] = bvs0[(i+(j+1)*3) & (test_num-1)];
            
            bvs[4] = bvs0[(i+(j+1)*4) & (test_num-1)];
            bvs[5] = bvs0[(i+(j+1)*5) & (test_num-1)];
            bvs[6] = bvs0[(i+(j+1)*6) & (test_num-1)];
            bvs[7] = bvs0[(i+(j+1)*7) & (test_num-1)];
            bv_multiple_and_256(dst, bvs, 8);
            //*/
            //bv_multiple_and_256(dst, bvs, 4);
            //bv_multiple_and_128(dst, bvs, 4);
            /*
            struct bit_vector* bv0 = bvs0[i];
            struct bit_vector* bv1 = bvs0[(rand_index+j) & (test_num-1)];
            bv_and_with_dst_256(dst, bv0, bv1);
            */
            dummy += dst->arr[rand_index];
        }
    }
    double end = NOW();

    bv_destroy(dst);
    printf("dummy_print: %lu\n", dummy);
    DISPLAY(test_num*count, start, end);
    return ;
}

bool
gen_bitvectors(struct bit_vector*** bvs, int bv_size, int bv_num)
{
    struct bit_vector ** tmp = (struct bit_vector **) malloc(sizeof(struct bit_vector*) * bv_num);
    if (tmp == NULL) return false;

    for (int i = 0; i < bv_num; ++i) {
        tmp[i] = bv_create(bv_size);
        if (tmp[i] == NULL) goto out;

        int byte = (tmp[i]->size) >> 3;
        for (int j = 0; j < byte; ++j) {
            int rand_num = rand() & 255UL;
            tmp[i]->arr[j] = rand_num;
        }
    }
    *bvs = tmp; 
    return true;
out:
    for (int i = 0; i < bv_num; ++i) {
        if (tmp[i]) bv_destroy(tmp[i]);
    }
    free(tmp);
    return false;
}

void
print_usage()
{
    printf("Usage: ./benchmark"
           " <bitvector size> <num>\n");
}

int 
main(int argc, char **argv) 
{
    srand((unsigned)time(NULL));

    if (argc < 3) {
        printf("Please specified rule file\n");
        print_usage();
        exit(1);
    }
    int bv_size = atoi(argv[1]);
    int bv_num = atoi(argv[2]);

    LOG(INFO, "generate testset\n");
    testset* testsets = (testset *) malloc(sizeof(testset) * bv_num);
    if (testsets == NULL) {
        printf("Failed to genereate testsets\n");
        return 1;
    }
    
    int byte_size = ROUNDUP8(bv_size) >> 3;
    for (int i = 0; i < bv_num; ++i) {
        testsets[i] = rand() % byte_size;
    }

    //struct bit_vector** bvs = NULL, **bvss = NULL;
    //if(!gen_bitvectors(&bvs, bv_size, bv_num)
    //   || 
    //   !gen_bitvectors(&bvss, bv_size, bv_num)) {
    //    LOG(ERR, "Failed to generate testset.");
    //    goto err0;
    //}
    struct bit_vector** bvs = NULL;
    if(!gen_bitvectors(&bvs, bv_size, bv_num)) {
        LOG(ERR, "Failed to generate testset.");
        goto err0;
    }
    LOG(INFO, "[SUCCESS] generate testset\n\n");
        
    LOG(INFO, "start performance test\n");
    bv_and_performance(bvs, testsets, bv_num);
    LOG(INFO, "[SUCCESS] performance test\n\n");

    /*
    if (bvss) {
        for (int i = 0; i < bv_num; ++i) {
            if (bvss[i]) bv_destroy(bvss[i]);
        }
        free(bvss);
    }
    */
    for (int i = 0; i < bv_num; ++i) {
        if (bvs[i]) bv_destroy(bvs[i]);
    }
    free(bvs);
    return 0;

err0:
    /*
    if (bvss) {
        for (int i = 0; i < bv_num; ++i) {
            if (bvss[i]) bv_destroy(bvss[i]);
        }
        free(bvss);
    }
    */
    for (int i = 0; i < bv_num; ++i) {
        if (bvs[i]) bv_destroy(bvs[i]);
    }
    free(bvs);

    return 1;
}
