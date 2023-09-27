/** @file   svc_stubs.S
 *
 *  @brief  Stub functions for 14642 syscalls
 *  @note   Not for public release, do not share
 *
 *  @date   last modified 14 October 2022
 *  @author CMU 14-642
**/

.cpu cortex-m4
.syntax unified
.section .svc_stub
.thumb

#include "../../kernel/include/svc_num.h"

.global _sbrk
_sbrk:
  svc 0x0
  BX lr

.global _write
_write:
  svc 0x1
  BX lr

.global _fstat
_fstat:
  svc 0x3
  BX lr

.global _isatty
_isatty:
  svc 0x4
  BX lr

.global _lseek
_lseek:
  svc 0x5
  BX lr

.global _read
_read:
  svc 0x6
  BX lr

.global _getpid
_getpid:
  mov r0, #1
  BX lr

.global _exit
_exit:
  svc 0x7
  BX lr

.global thread_init
thread_init:
  svc 0x9
  BX lr  

.global thread_create
thread_create:
  svc 0xA
  BX lr

.type   thread_kill, %function
.global thread_kill
thread_kill:
  /*bkpt*/
  svc 0xB
  BX lr

.global scheduler_start
scheduler_start:
  /* bkpt */
  svc 0x12
  BX  lr  

.global mutex_init
mutex_init:
  svc 0xD
  BX lr  

.global mutex_lock
mutex_lock:
  svc 0xE
  BX lr  

.global mutex_unlock
mutex_unlock:
  svc 0xF
  BX lr  

.global wait_until_next_period
wait_until_next_period:
  svc 0x10
  BX lr  

.global get_time
get_time:
  svc 0x11
  BX lr  

.global get_priority
get_priority:
  svc 0x13
  BX lr  

.global thread_time
thread_time:
  svc 0x14
  BX lr  

.global delay_ms
delay_ms:
  svc 0x16
  BX lr
  
.global lux_read
lux_read:
  svc 0x17
  BX lr

.global pix_set
pix_set:
  svc 0x18
  BX lr

.global create_path
create_path:
  svc 25
  BX lr

.global write_file
write_file:
  svc 26
  BX lr

.global read_file
read_file:
  svc 27
  BX lr

.global copy_path
copy_path:
  svc 28
  BX lr

.global delete_path
delete_path:
  svc 29
  BX lr

.global init_file_system
init_file_system:
  svc 30
  BX lr


/* the following stubs do not need to be implemented */

.global _start
_start:
  bkpt

.global _close
_close:
  svc 0x2
  BX lr

  
.global _kill
_kill:
  svc 0x8
  BX lr  
  
.global _gettimeofday
_gettimeofday:
  bkpt

.global _times
_times:
  bkpt


