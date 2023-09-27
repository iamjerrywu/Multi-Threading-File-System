/** @file   lib642.h
 *
 *  @brief  custom functions for user programs and threading
 *  @note   Not for public release, do not share
 *
 *  @date   last modified 14 October 2022
 *  @author CMU 14-642
**/

#ifndef _LIB642_H_
#define _LIB642_H_

#include <stdio.h>
#include <stdint.h>

#define UNUSED __attribute__((unused))
#define intrinsic __attribute__((always_inline)) static inline

#define RET_GOOD 0x900d
#define RET_FAIL 0xfa17
#define RET_0642 0x0642

typedef enum { PER_THREAD = 1, KERNEL_ONLY = 0 } mpu_mode;

/**
 * @brief      Initialize the thread library
 *
 *             A user program must call this initializer before attempting to
 *             create any threads or start the scheduler.
 *
 * @param      max_threads        max number of threads created
 * @param      stack_size         Declares the size in words of all the stacks
 *                                for subsequent calls to thread create.
 * @param      idle_func          Pointer to a thread function to run when no
 *                                other threads are runnable, if arg is NULL,
 *                                then kernel will supply default idle thread.
 * @param      memory_protection  If KERNEL_ONLY, then kernel will be
 *                                protected if PER_THREAD, perthread mem
 *                                protection in addition to kernel protection.
 * @param      max_mutexes        max number of mutexes created
 *
 * @return     0 on success or -1 on failure
 */
int thread_init(uint32_t max_threads,
                uint32_t stack_size,
                void (*idle_func)(void),
                mpu_mode memory_protection,
                uint32_t max_mutexes);

/**
 * @brief      Create a new thread running the given function
 *
 * @param      fn     Pointer to the function to run in the new thread.
 * @param      prio   Priority of this thread. Lower number are higher
 *                    priority.
 * @param      C      Real time execution time (ms).
 * @param      T      Real time task period (ms).
 * @param      vargp  Pointer to an argument.
 *
 * @return     0 on success or -1 on failure. Runs ub test for new task set
 *             will fail if over ub test.
 */
int thread_create(void (*fn)(void* vargp),
                  uint32_t prio,
                  uint32_t C,
                  uint32_t T,
                  void* vargp);

/**
 * @brief      Allow the kernel to start running the added task set.
 *
 *             This function should enable IRQs and thus enable your
 *             scheduler. The kernel will test that a task set with this new
 *             task is scheduleable before running and may return an error if
 *             this is not the case.
 *
 * @param[in]  freq  The frequency
 *
 * @return     0 on success or -1 on failure
 */
int scheduler_start(uint32_t frequency);

/**
 * @brief      Get the current time.
 *
 * @return     The time in ticks.
 */
uint32_t get_time();

/**
 * @brief      Get the effective priority of the current running thread
 *
 * @return     The thread's effective priority
 */
uint32_t get_priority();

/**
 * @brief      Gets the total elapsed time for the thread (since its first
 *             ever period.)
 *
 * @return     The time in ticks.
 */
uint32_t thread_time();

/**
 * @brief      Waits efficiently by descheduling thread.
 */
void wait_until_next_period();

/**
 * @brief      Type definition for mutex, opaque to user
 */
typedef struct {
  char unused;
} mutex_t;

/**
 * @brief      Initialize a mutex
 *
 *             A user program calls this function to obtain a mutex.
 *
 * @param      max_prio  The maximum priority of a thread which could use
 *                       this mutex.
 *
 * @return     A mutex handle, uniquely referring to this mutex. NULL if
 *             max_mutexes would be exceeded.
 */
mutex_t* mutex_init(uint32_t max_prio);

/**
 * @brief      Lock a mutex
 *
 *             This function will not return until the current thread has
 *             obtained the mutex.
 *
 * @param      mutex  The mutex to act on.
 */
void mutex_lock(mutex_t* mutex);

/**
 * @brief      Unlock a mutex
 *
 * @param      mutex  The mutex to act on.
 */
void mutex_unlock(mutex_t* mutex );

/**
 * @brief      Runs expr, and if it has a non-zero return value, abort program.
 *             Prints information before aborting.
 */
#define ABORT_ON_ERROR( expr , ... ) { \
  int status = (expr); if (status) { \
    printf("%s: Line %d\n", __FILE__, __LINE__); \
    printf(#expr "\n\tfailed with status %d", status); \
    printf("\n" __VA_ARGS__); \
    exit(1); \
  } \
}


/**
 * @brief      Calls wfi, which is a hint instruction. It will either put the
 *             processor to sleep or spin.
 */
intrinsic void wait_for_interrupt() {
  __asm volatile("wfi");
}

/**
 * @brief       Pretends to do work until the given time is past.
 *              For grading.
 *
 * @param time  time at which to stop working
 */
void spin_until(uint32_t time);

/**
 * @brief       Pretends that the thread is doing work for ms milliseconds.
 *              For grading.
 *
 * @param ms    milliseconds to work for
 */
void spin_wait(uint32_t ms);


/**
 * @brief Prints basic status information of a thread
 *
 * @param thread_num    thread number
 * @param thread_name   name of the thread
 * @param cnt           a counter variable
 */
//@{
void print_num_status(int thread_num);
void print_num_status_cnt(int thread_num, int cnt);
void print_status(char* thread_name);
void print_status_prio(char* thread_name);
void print_status_cnt(char* thread_name, int cnt);
void print_status_prio_cnt(char* thread_name, int cnt);
//@}

/**
 * @brief           Prints out fibonacci numbers mod mod
 *
 * @param limit     stop at fib[limit]
 * @param interval  print only fib[interval * k], k integer
 * @param mod       modulo (to get the last digits only)
 *
 * @return          fib[limit] mod mod
 */
uint32_t print_fibs(int limit, int interval, uint32_t mod);

#undef intrinsic

#endif /* _LIB642_H_ */
