/** @file   lib642.c
 *
 *  @brief  cuustom functions for user programs and threading
 *  @note   Not for public release, do not share
 *
 *  @date   last modified 14 October 2022
 *  @author CMU 14-642
 **/

#include <lib642.h>

void spin_wait(uint32_t ms) {
    uint32_t targetTime = thread_time() + ms;

    while(thread_time() < targetTime) {
        wait_for_interrupt();
    }
    return;
}

void spin_until(uint32_t time) {
    while(get_time() < time) {
        wait_for_interrupt();
    }
    return;
}

void print_num_status(int thread_num) {
    printf("t=%lu\tThread %d\n", (uint32_t)get_time(), thread_num);
}

void print_num_status_cnt(int thread_num, int cnt) {
    printf("t=%lu\tThread %d\tCnt: %d\n", (uint32_t)get_time(), thread_num, cnt);
}

void print_status(char* thread_name) {
    printf("t=%lu\tThread %s\n", (uint32_t)get_time(), thread_name);
}

void print_status_cnt(char* thread_name, int cnt) {
    printf("t=%lu\tThread %s\tCnt: %d\n", (uint32_t)get_time(), thread_name, cnt);
}

void print_status_prio(char* thread_name) {
    printf("t=%lu\tThread %s\tPrio: %lu\n", (uint32_t)get_time(), thread_name, (uint32_t)get_priority());
}

void print_status_prio_cnt(char* thread_name, int cnt) {
    printf("t=%lu\tThread %s\tPrio: %lu\tCnt: %d\n", (uint32_t)get_time(), thread_name, (uint32_t)get_priority(), cnt);
}

uint32_t print_fibs(int limit, int interval, uint32_t mod) {
    if(interval == 0) interval = 1;
    int i = 1;
    uint32_t a = 0, b = 1, c;

    while(i < limit) {
    i++;
        c = (a + b) % mod;

        if(i % interval == 0) {
            printf("Fib[ %d ] = %lu (mod %lu)\n", i, c, mod);
        }
        a = b;
        b = c;
    }

    return b;
}
