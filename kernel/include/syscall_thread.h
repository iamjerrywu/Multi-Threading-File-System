/** @file   syscall_thread.h
 *
 *  @brief  system calls to support thread library for lab 4
 *  @note   Not for public release, do not share
 *
 *  @date   last modified 25 October 2022
 *  @author CMU 14-642
**/

#ifndef _SYSCALL_THREAD_H_
#define _SYSCALL_THREAD_H_

#include <unistd.h>
#include <mpu.h>
#include <stdbool.h>

#define ROUNDROBIN

#define USER_THREAD_STACK_SIZE 32768
#define KERNAL_THREAD_STACK_SIZE 32768
#define RVR_MAX_VALUE 0XFFFFFF
#define MAIN_THREAD_ID 15
#define IDLE_THREAD_ID 14


typedef struct psp_stack_frame psp_t;
struct psp_stack_frame {
    uint32_t r0;   /** @brief reg r0 value */
    uint32_t r1;   /** @brief reg r1 value */
    uint32_t r2;   /** @brief reg r2 value */
    uint32_t r3;   /** @brief reg r3 value */
    uint32_t r12;  /** @brief reg r12 value */
    uint32_t lr;   /** @brief reg lr value */
    uint32_t pc;   /** @brief reg pc value */
    uint32_t xPSR; /** @brief reg xPSR value */
};

typedef struct msp_stack_frame msp_t;
struct msp_stack_frame {
    uint32_t psp;  /** @brief reg psp value */
    uint32_t r4;   /** @brief reg r4 value */
    uint32_t r5;   /** @brief reg r5 value */
    uint32_t r6;   /** @brief reg r6 value */
    uint32_t r7;   /** @brief reg r7 value */
    uint32_t r8;   /** @brief reg r8 value */
    uint32_t r9;   /** @brief reg r9 value */
    uint32_t r10;  /** @brief reg r10 value */
    uint32_t r11;  /** @brief reg r11 value */
    uint32_t lr;   /** @brief reg lr value */
};

typedef enum tcb_status tcb_e;
enum tcb_status {
    WAITING = 0,   /** @brief WAITING */
    RUNNABLE = 1,  /** @brief RUNNABLE */
    RUNNING = 2,   /** @brief RUNNNING */
    INACTIVE = 3,  /** @brief INACTIVE */
};

typedef struct tcb tcb_t;
struct tcb {
    tcb_e status;                /** @brief status */
    uint32_t svc_status;         /** @brief svc_status */
    uint32_t pc;                 /** @brief pc */
    uint32_t lr;                 /** @brief lr */
    psp_t* psp;                  /** @brief psp */
    msp_t* msp;                  /** @brief msp */ 
    uint32_t C;                  /** @brief C */
    uint32_t T;                  /** @brief T */
    uint32_t priority;           /** @brief priority */
    uint32_t d_priority;         /** @brief d_priority */
    uint32_t remained_T;         /** @brief remained_T */
    uint32_t remained_C;         /** @brief remained_C */
    uint32_t total_C;            /** @brief total_C */
    uint32_t* user_base;         /** @brief user_base */
    uint32_t* kernel_base;       /** @brief kernal_base */
    uint32_t  mutexes_arr[20];   /** @brief mutexas_arr */
    uint32_t  last;              /** @brief last */
    uint32_t first;              /** @brief first */
    uint32_t locked_by_mutex_id; /** @brief locked_by_mutex_id */
};


void reset_tcb(tcb_t*tcb);

/** @brief initialize thread switching
 *  @note  must be called before creating threads or scheduling
 *
 *  @param max_threads      max number of threads to be created
 *  @param stack_size       stack size in words of all created stacks
 *  @param idle_fn          function pointer for idle thread, NULL for default
 *  @param mem_protect      mpu mode, either PER_THREAD or KERNEL_ONLY
 *  @param max_mutexes      max number of mutexes supported
 *
 *  @return     0 for success, -1 for failure
 */
int sys_thread_init(uint32_t max_threads, uint32_t stack_sizes, void* idle_fn, mpu_mode mem_protect, uint32_t max_mutexes);

/** @brief create a new thread as specified, if UB allows
 *
 *  @param fn       thread function pointer
 *  @param prio     thread priority, with 0 being highest
 *  @param C        execution time (scheduler ticks)
 *  @param T        task period (scheduler ticks)
 *  @param vargp    thread function argument
 *
 *  @return     0 for success, -1 for failure
 */
int sys_thread_create(void* fn, uint32_t prio, uint32_t C, uint32_t T, void* vargp);

/** @brief tell the kernel to start running threads using Systick
 *  @note  returns only after all threads complete or are killed
 *
 *  @param frequency  frequncy of context switches in Hz
 *
 *  @return     0 for success, -1 for failure
 */
int sys_scheduler_start(uint32_t frequency);

/** @brief get the current time in ticks */
uint32_t sys_get_time();

/** @brief get the dynamic priority of the running thread */
uint32_t sys_get_priority();

/** @brief get the total elapsed time for the running thread */
uint32_t sys_thread_time();

/** @brief deschedule thread and wait until next turn */
void sys_wait_until_next_period();

/** @brief kill the running thread
 *
 *  @note  locks if main or idle thread is running or thread holds a mutex
 *
 *  @return does not return
 */
void sys_thread_kill();

void init_tcb(void);

int32_t find_available_thread(void);
void udpate_thread_time(void);
tcb_t *schedule_next_thread(void);
tcb_t* round_robin_scheduler(void);
tcb_t* RMS_scheduler(void);
void default_idle_thread(void);
void init_mpu(tcb_t* cur, tcb_t *next);
void sys_init_file_system(void);
int sys_create_path(char* path, int is_file);
int sys_write_file(char* path, char* data);
uint32_t sys_read_file(char* path);
int sys_copy_path(char* path1, char* path2);
int sys_delete_path(char* path);

#endif /* _SYSCALL_THREAD_H_ */
