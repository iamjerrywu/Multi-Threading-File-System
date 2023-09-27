/** @file   syscall.c
 *
 *  @brief  base syscall implementations for Lab 4 tasks
 *  @note   Not for public release, do not share
 *
 *  @date   last modified 25 October 2022
 *  @author CMU 14-642
**/

#include <arm.h>
#include <syscall.h>
#include <unistd.h>
#include <printk.h>
#include <timer.h>
#include <gpio.h>

extern uint32_t __heap_base;
const uint16_t stdin = 0;
const uint16_t stdout = 1;


uint32_t used_heap_size = 0;
/**
 * system sbrk call.
 */
void* sys_sbrk(int incr) {
    static char* cur_heap_ptr;
    if (used_heap_size + incr < HEAP_SIZE) {
        cur_heap_ptr = (char*)(&__heap_base + used_heap_size);
        used_heap_size+=incr;
        return (void*)cur_heap_ptr;
    }
    return (void*)-1;
}

/**
 * System read call.
 */
int sys_read(int file, char* ptr, int len) {
    if (file != stdin) {
        return -1;
    }
    uint16_t actual = rtt_has_data(0);
    uint16_t read_bytes;
    if (actual < len) {
        read_bytes = rtt_read(0, ptr, actual);
    } else {
        read_bytes = rtt_read(0, ptr, len);
    }
    return read_bytes;
}

/**
 * System write call.
 */
int sys_write(int file, char* ptr, int len) {
    // (void)file;
    if (file != stdout) {
        return -1;
    }
    uint16_t write_bytes = rtt_write(0, ptr, len);
    return write_bytes;
}


/**
 * System exit call.
 */
void sys_exit(int status) {
    printk("System Terminated!\n");
    if (status != 0) {
        gpio_init(1, 15, 1, 0, 0);
        gpio_set(1, 15);
    }
    while (1) {
        disable_interrupts();
        wait_for_interrupt();
    }
}


/* syscalls for custom user projects */

/**
 * System delay call.
 */
void sys_delay_ms(uint32_t ms) {
    (void)ms;
    systick_delay(ms);
    // printk_int(ms);
}

/**
 * System lux call.
 */
uint16_t sys_lux_read() {
    return 0;
}

/**
 * System pix call.
 */
void sys_pix_set(uint8_t red, uint8_t green, uint8_t blue) {
    (void) red;
    (void) green;
    (void) blue;
}