/** @file   syscall.h
 *
 *  @brief  prototypes for base system calls for lab 4
 *  @note   Not for public release, do not share
 *
 *  @date   last modified 14 October 2022
 *  @author CMU 14-642
**/

#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#define HEAP_SIZE 8000

void* sys_sbrk(int incr);

int sys_write(int file, char* ptr, int len);

int sys_read(int file, char* ptr, int len);

void sys_exit(int status);

void sys_delay_ms(uint32_t ms);

uint16_t sys_lux_read();

void sys_pix_set(uint8_t red, uint8_t green, uint8_t blue);

#endif /* _SYSCALL_H_ */
