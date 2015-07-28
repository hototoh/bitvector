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

#include "array_lookup.h"
#include "radixtrie.h"
#include "common.h"

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



void
performance_test(struct multi_bv_arrays* mbarr,
                 testset* testsets, int test_num)
{
    uint64_t dummy = 0;
    double start = NOW();
    for (int i = 0; i < test_num; i++) {        
        struct bit_vector* bv = mbvs_lookup(mbarr, (uint8_t*) &testsets[i]);
        dummy += bv_ffs(bv);
        bv_destroy(bv);
    }
    double end = NOW();
    printf("dummy_print: %llu\n", dummy);
    DISPLAY(test_num, start, end);
    return ;
}

bool
gen_testset(testset** testsets, int test_num)
{
    testset *tsets = (testset *) malloc(sizeof(testset) * test_num);
    if (tsets == NULL) return false;

    for (int i = 0; i < test_num; ++i) {
        uint32_t res = 0;
        for (int j = 0; j < 4; ++j) {
            int rand_num = rand() & 255UL;
            res <<= 8;
            res += rand_num;
        }
        tsets[i] = (testset) res;
    }
    *testsets = tsets;
        
    return true;
}

void
print_usage()
{
    printf("Usage: ./benchmark"
           " <bitvector size> <testset num> [<batch_num> = 1]\n");
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

    int batch_num;
    if (argc >= 4) {
        batch_num = atoi(argv[3]);
    }

    int test_num = atoi(argv[2]);
    testset* testsets;
    if(!gen_testset(&testsets, test_num)) {
        LOG(ERR, "Failed to generate testset.");
        goto err0;
    }

    LOG(INFO, "parse confg rules\n");
    struct prefix_match_rules *config_rules = parse_rules(argv[1]);
    if (config_rules == NULL) {
        LOG(ERR, "Failed to parse rules.\n");
        goto err1;
    }
    LOG(INFO, "[SUCCESS] parsed confg rules\n\n");
    
    int count = config_rules->count;
    LOG(INFO, "create multi_bv_arrays(4, %d)\n", count);
    struct multi_bv_arrays *mbarr = mbvs_create(4, count);
    if (mbarr == NULL) {
        LOG(ERR, "Failed to create multi_bv_arrays.\n");
        goto err2;
    }

    for (int i = 0; i < count; i++) {
        struct prefix_match_rule* rule = &config_rules->arr[i];
        uint32_t rule_id = rule->id;
        uint32_t ip_address = rule->ip_address;
        uint32_t prefix_len = rule->prefix_len;
        uint8_t *ip_addrs = (uint8_t*)&ip_address;
        mbvs_add_rules(mbarr, ip_addrs, prefix_len, rule_id);
    }
    LOG(INFO, "[SUCCESS] setup finished.\n\n");
    
    LOG(INFO, "start performance test\n");
    performance_test(mbarr, testsets, test_num);
    LOG(INFO, "[SUCCESS] performance test\n\n");

    free(testsets);
    mbvs_destroy(mbarr);
    destroy_prefix_match_rules(config_rules);
    return 0;
err2:
    destroy_prefix_match_rules(config_rules);
err1:
    free(testsets);
err0:
    return 1;
}
