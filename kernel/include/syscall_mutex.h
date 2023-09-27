/** @file   syscall.h
 *
 *  @brief  prototypes for system calls to support mutexes for lab 4
 *  @note   Not for public release, do not share
 *
 *  @date   last modified 25 October 2022
 *  @author CMU 14-642
**/

#ifndef _SYSCALL_MUTEX_H_
#define _SYSCALL_MUTEX_H_

#include <unistd.h>

#define UNLOCKED ((uint32_t)-1)

/**
 * @brief      The struct for a mutex.
 */
typedef struct {
  volatile uint32_t locked_by;
  volatile uint32_t prio_ceil;
  // You may fill in additional fields in this struct if you require.
} kmutex_t;

/**
 * @brief      Used to create a mutex object. The mutex resides in kernel
 *             space. The user receives a handle to it. With memory
 *             protection, the user cannot modify it. However,
 *             tt can still be passed around and used with
 *             lock, unlock.
 *
 * @param      max_prio  The maximum priority of a thread which could use
 *                       this mutex.
 *
 * @return     A pointer to the mutex. NULL if max_mutexes would be exceeded.
 */
kmutex_t* sys_mutex_init(uint32_t max_prio);

/**
 * @brief      Lock a mutex
 *
 *             This function will not return until the current thread has
 *             obtained the mutex.
 *
 * @param[in]  mutex  The mutex to act on.
 */
void sys_mutex_lock(kmutex_t* mutex);

/**
 * @brief      Unlock a mutex
 *
 * @param[in]  mutex  The mutex to act on.
 */
void sys_mutex_unlock(kmutex_t* mutex);

#endif /* _SYSCALL_MUTEX_H_ */
