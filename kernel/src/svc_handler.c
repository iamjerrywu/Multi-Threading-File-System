/** @file   svc_handler.c
 *
 *  @brief  implementation of basic and custom SVC calls
 *  @note   Not for public release, do not share
 *
 *  @date   last modified 25 October 2022
 *  @author CMU 14-642
**/

#include <arm.h>
#include <printk.h>
#include <svc_num.h>
#include <unistd.h>
#include <syscall.h>
#include <syscall_thread.h>
#include <rtt.h>
#include <mpu.h>
#include <syscall_mutex.h>

typedef struct { 
  uint32_t r0; 
  uint32_t r1; 
  uint32_t r2; 
  uint32_t r3; 
  uint32_t r12; 
  uint32_t lr; 
  uint32_t pc; 
  uint32_t xPSR; 
  uint32_t fifth_par;
} stack_frame_t;

/**
 * Svc handler.
 */
void svc_c_handler(void* psp) {
  // printk("here!\n");
  stack_frame_t* s = (stack_frame_t*)psp; 
  char* svc_num_ptr = (char*)((s->pc) - 2); 
  int svc_num = (int)*svc_num_ptr; /* from the svc inst */ 
  switch(svc_num) {
    case SVC_FSTAT:
      s->r0 = -1;
      break;
    case SVC_ISATTY:
      s->r0 = 1;
      break;
    case SVC_LSEEK:
      s->r0 = -1;
      break;
    case SVC_SBRK:
      s->r0 = (uint32_t)sys_sbrk((int)s->r0);
      break;
    case SVC_WRITE:
      s->r0 = sys_write((int)s->r0, (char*)s->r1, (int)s->r2);
      break;
    case SVC_READ:
      s->r0 = sys_read((int)s->r0, (char*)s->r1, (int)s->r2);
      break;
    case SVC_THR_INIT:
      s->r0 = sys_thread_init((uint32_t) s->r0, (uint32_t)s->r1, (void*)s->r2, (mpu_mode)s->r3, (uint32_t)s->fifth_par);
      break;
    case SVC_THR_CREATE:
      s->r0 = sys_thread_create((void*) s->r0, (uint32_t)s->r1, (uint32_t)s->r2, (uint32_t)s->r3, (void*)s->fifth_par);
      break;
    case SVC_EXIT:
      sys_exit(1);
      break;
    case SVC_DELAY_MS:
      sys_delay_ms((uint32_t) s->r0);
      break;
    case SVC_WAIT:
      sys_wait_until_next_period();
      break;
    case SVC_TIME:
      s->r0 = sys_get_time();
      break;
    case SVC_SCHD_START:
      s->r0 = sys_scheduler_start((uint32_t)s->r0);
      break;
    case SVC_LUX:
      s->r0 = sys_lux_read();
      break;
    case SVC_PIX:
      sys_pix_set((uint8_t) s->r0, (uint8_t) s->r1, (uint8_t) s->r2);
      break;
    case SVC_PRIORITY :
      s->r0 = sys_get_priority();      
      break;
    case SVC_THR_TIME:
      s->r0 = sys_thread_time();      
      break;
    case SVC_THR_KILL:
      sys_thread_kill();      
      break;
    case SVC_MUT_INIT:
      s->r0 = (uint32_t)sys_mutex_init((uint32_t)s->r0);      
      break;
    case SVC_MUT_LOK:
      sys_mutex_lock((kmutex_t*)s->r0);      
      break;
    case SVC_MUT_ULK:
      sys_mutex_unlock((kmutex_t*)s->r0);      
      break;
    case SVC_INIT_FILE_SYSTEM:
      sys_init_file_system();      
      break;
    case SVC_CREATE_PATH:
      s->r0 = sys_create_path((char*)s->r0, (int)s->r1);      
      break;
    case SVC_WRITE_FILE:
      s->r0 = sys_write_file((char*)s->r0, (char*)s->r1);      
      break;
    case SVC_READ_FILE:
      s->r0 = (uint32_t)sys_read_file((char*)s->r0);      
      break;
    case SVC_COPY_PATH:
      s->r0 = sys_copy_path((char*)s->r0, (char*)s->r1);      
      break;
    case SVC_DELETE_PATH:
      s->r0 = sys_delete_path((char*)s->r0);      
      break;
    default:
      printk("Not implemented, svc num %d\n", svc_num);
      sys_exit(1);
      break;

  }
}