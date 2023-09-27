/** @file   syscall_thread.c
 *
 *  @brief  thread syscall implementation for lab 4
 *  @note   Not for public release, do not share
 *
 *  @date   last modified 14 October 2022
 *  @author CMU 14-642
**/

#include <arm.h>
#include <mpu.h>
#include <printk.h>
#include <syscall.h>
#include <syscall_mutex.h>
#include <syscall_thread.h>
#include <unistd.h>
#include <timer.h>
#include <stdbool.h>
#include <file_system.h>

#define USER_THREAD_STACK_SIZE 32768
#define KERNAL_THREAD_STACK_SIZE 32768

extern uint32_t __psp_stack_base;
extern void thread_kill();

/** @brief      Initial XPSR value, all 0s except thumb bit. */
#define XPSR_INIT 0x1000000

/** @brief Return code to return to user mode with user stack.*/
#define LR_RETURN_TO_USER_PSP 0xFFFFFFFD

/** @brief thread stack region limits */
extern char __thread_u_stacks_limit, __thread_k_stacks_limit;
extern char __thread_u_stacks_base, __thread_k_stacks_base;


/** @brief precomputed values for UB test */
float ub_table[] = {
    0.000, 1.000, .8284, .7798, .7568,
     .7435, .7348, .7286, .7241, .7205,
     .7177, .7155, .7136, .7119, .7106,
     .7094, .7083, .7075, .7066, .7059,
     .7052, .7047, .7042, .7037, .7033,
     .7028, .7025, .7021, .7018, .7015,
     .7012, .7009
};


tcb_t* cur_thread = NULL;

int32_t allowed_threads_num = 0;
// since idle thread priority = 15, and also idle threads tcb won't be updated
int32_t created_threads_num = -1;
uint32_t thread_stack_size = 0; 
tcb_t tcbs[16];
float ub_value = 0;
// static runnable_t runnable_pool;
// static waiting_t waiting_pool;
uint32_t tick = 0;
uint32_t round_robin_idx = -1;
bool switch_main = false;
mpu_mode memory_protec;

kmutex_t mutexes[32];
uint32_t allowed_mutexes_num = 0;
uint32_t created_mutexes_num = 0;
uint32_t waiting_mutex_id = 32;
bool change_prio = false;

uint32_t mutex_prios[20];
uint32_t max_mutex_priority = 16;
kmutex_t *max_mutex_ptr = NULL;

void systick_c_handler() {
    // disable_interrupts();
    
    tick++;

    udpate_thread_time();
    // breakpoint();
    pend_pendsv();  
    // enble_interrupts();
    
}

void *pendsv_c_handler(void *msp){
    // disable_interrupts();
    clear_pendsv();
    // tcb_t* next_thread = round_robin_scheduler();
    // breakpoint();
    if (switch_main) {
        // 
        // breakpoint();
        // printk("switch to main!\n");
        switch_main = false;
        if (memory_protec == PER_THREAD) {
            mm_region_disable(6);
            mm_region_disable(7);
            // enable next
        
            mm_region_enable(6, (void*)tcbs[MAIN_THREAD_ID].user_base, mm_log2ceil_size(thread_stack_size), 0, 1);
            mm_region_enable(7, (void*)tcbs[MAIN_THREAD_ID].kernel_base, mm_log2ceil_size(thread_stack_size), 0, 1);
        }
        return tcbs[MAIN_THREAD_ID].msp; 
    }
    
    // if (next_thread == cur_thread) return msp;
    // printk("pend sv!\n");
    // breakpoint();
    cur_thread->svc_status = (tcb_e)get_svc_status();
    cur_thread->msp = (msp_t*)msp;
    if (cur_thread->status == RUNNING)
        cur_thread->status = RUNNABLE;
    // msp_t* mp = (msp_t*)msp;
    // psp_t* p = (psp_t*)(mp->psp);
    // printk_int(p->lr);
    
    // tcb_t* next_thread = round_robin_scheduler();
    // breakpoint();
    tcb_t* next_thread = RMS_scheduler();
    if (cur_thread != next_thread) {
        // breakpoint();
    }
    // breakpoint();
    if (memory_protec == PER_THREAD) {
        init_mpu(cur_thread, next_thread);
    }
    cur_thread = next_thread;

    cur_thread->status = RUNNING;
    set_svc_status(cur_thread->svc_status);
    
    // breakpoint();
    // enable_interrupts();
    // breakpoint();
    return (void*)cur_thread->msp;
}

int sys_thread_init(uint32_t max_threads, uint32_t stack_size, void* idle_fn, mpu_mode memory_protection, uint32_t max_mutexes) {
    
    // bytes 
    thread_stack_size = 1 << (mm_log2ceil_size(stack_size) + 2);
    if (thread_stack_size < 1024) {
        thread_stack_size = 1024;
    }
    if (((thread_stack_size * max_threads) > USER_THREAD_STACK_SIZE) && ((thread_stack_size * max_threads) > KERNAL_THREAD_STACK_SIZE)) {
        return -1;
    }

    allowed_threads_num = max_threads;
    allowed_mutexes_num = max_mutexes;
    // init_pool();
    
    init_tcb();
    // enable_mpu();
    
    memory_protec = memory_protection;
    
    // idle thread 
    if (idle_fn == NULL) idle_fn = &default_idle_thread;
    sys_thread_create(idle_fn, IDLE_THREAD_ID, 32767, 32767, 0);
    tcbs[MAIN_THREAD_ID].priority = MAIN_THREAD_ID;

    if (memory_protec == KERNEL_ONLY) {
        mm_region_enable(6, &__thread_u_stacks_limit, mm_log2ceil_size(16 * thread_stack_size), 0, 1);
        mm_region_enable(7, &__thread_k_stacks_limit, mm_log2ceil_size(16 * thread_stack_size), 0, 1);
    } else {
        tcbs[MAIN_THREAD_ID].user_base = (uint32_t*)(&__thread_u_stacks_limit + thread_stack_size * MAIN_THREAD_ID);
        tcbs[MAIN_THREAD_ID].kernel_base = (uint32_t*)(&__thread_k_stacks_limit + thread_stack_size * MAIN_THREAD_ID);
        mm_region_enable(6, (void*)tcbs[MAIN_THREAD_ID].user_base, mm_log2ceil_size(thread_stack_size), 0, 1);
        mm_region_enable(7, (void*)tcbs[MAIN_THREAD_ID].kernel_base, mm_log2ceil_size(thread_stack_size), 0, 1);
    }
    cur_thread = &tcbs[MAIN_THREAD_ID];
    return 0;
}

int sys_thread_create(void* fn, uint32_t prio, uint32_t C, uint32_t T, void* vargp) {
    // breakpoint();
    // check1: check if execeed max threads
    if (tcbs[prio].status != INACTIVE) {
        // breakpoint();
        printk("error, status not inactive\n");
        return -1;
    }
    if (created_threads_num >= allowed_threads_num) { 
        printk("over allowed threads num\n");
        return -1;
    }
    // check2: check if violate ub test
    // breakpoint();
    // if created_thread_num is smaller than zero, it means we create idle thread here.
    if (created_threads_num >= 0) {
        // start to check from first thread, we don't need to check this rule when thread number is zero.
        // Note: created_thread_num will start from zero here.
        if (ub_value + ((float)C/(float)T) > ub_table[created_threads_num + 1])  {
            // printk("ub value violation!\n");
            return -1;
        }
        ub_value += (float)C/(float)T;

    }

    // reset_tcb(&tcbs[prio]);

    tcb_t* tcb =  &tcbs[prio];
    // add_runnable_thread(tcb);
    
    tcb->C = C;
    tcb->T = T;
    tcb->remained_C = C;
    tcb->remained_T = T;
    tcb->locked_by_mutex_id = 32;
    tcb->svc_status = 0;
    tcb->priority = prio;
    tcb->d_priority = prio;
    tcb->status = RUNNABLE;
    tcb->first = 0;
    tcb->last = 0;
    for (uint32_t i = 0; i < 20; i++) {
        tcb->mutexes_arr[i] = 32;
    }
    tcb->kernel_base = (uint32_t*)(&__thread_k_stacks_limit + thread_stack_size * prio);
    msp_t* msp = (msp_t*) (&__thread_k_stacks_limit + thread_stack_size * (prio + 1)) ;
    msp = msp - 1;  
    tcb->msp = msp;
    
    tcb->user_base = (uint32_t*)(&__thread_u_stacks_limit + thread_stack_size * prio);
    psp_t* psp = (psp_t*) (&__thread_u_stacks_limit + thread_stack_size * (prio + 1));
    psp = psp - 1;
    tcb->psp = psp;
    
    tcb->msp->psp = (uint32_t)psp;
    psp->r0 = (uint32_t) vargp;
    psp->lr = (uint32_t) &thread_kill;
    psp->pc = (uint32_t)fn;
    psp->xPSR = XPSR_INIT;

    msp->psp = (uint32_t)psp;
    msp->lr = LR_RETURN_TO_USER_PSP;
    
    created_threads_num++;
    // breakpoint();
    return 0;
}

int sys_scheduler_start(uint32_t frequency) {
    (void)frequency;
    uint32_t rvr_value = 64000000/frequency;
    if (rvr_value > RVR_MAX_VALUE) {
        rvr_value = RVR_MAX_VALUE;
    }
    REG_WRITE((uint32_t *)(SYST_CSR), 0);
    // rvr max value = 0XFFFFFF
    REG_WRITE((uint32_t *)(SYST_RVR), 64000000/frequency);
    REG_WRITE((uint32_t *)(SYST_CSR), 7);


    pend_pendsv();

    return 0;
}

uint32_t sys_get_priority() {
    return cur_thread->d_priority;
}

uint32_t sys_get_time() {
    return tick;
}

uint32_t sys_thread_time() {
    return cur_thread->total_C;
}

void sys_thread_kill() {
    if (cur_thread->priority == IDLE_THREAD_ID) {
        return;
    };
    if (cur_thread->priority == MAIN_THREAD_ID) {
        sys_exit(0);
        return;
    }
    cur_thread->status = INACTIVE;
    created_threads_num--;
    ub_value-=(float)cur_thread->C/(float)cur_thread->T;

    int32_t inactive_cnt = 0; 
    // check with allowed threads number!
    for (int32_t i = 0; i < allowed_threads_num; i++) {
        if (tcbs[i].status == INACTIVE) {
            inactive_cnt++;
        }
    }
    if (inactive_cnt >= allowed_threads_num) {
        switch_main = true;
    }
    pend_pendsv();
}

void sys_wait_until_next_period() {
    // disable_interrupts();
    // breakpoint();
    cur_thread->status = WAITING;
    cur_thread->remained_C = 0;
    pend_pendsv();
    // senable_interrupts();
}

kmutex_t* sys_mutex_init(uint32_t max_prio) {
    if (created_mutexes_num == allowed_mutexes_num) {
        breakpoint();
        return NULL;
    }
    kmutex_t* mutex = &mutexes[max_prio];
    mutex->prio_ceil = max_prio;
    mutex->locked_by = 16;
    created_mutexes_num++;

    return mutex;
} 

void sys_mutex_lock(kmutex_t* mutex) {
    // current thread's priority can not over the mutex's priority ceil
    if (cur_thread->priority >= mutex->prio_ceil) {
        // the mutex has already locked by current thread in the past
        if (mutex->locked_by == cur_thread->priority) {
            printk("[Debug] Cannot locked by thread which already take this mutex!\n");
            return;
        }
        // rule 2
        // the thread's priority is lower than the largest mutex priority
        // should be blocked
        // while (max_mutex_priority != 16 && cur_thread->priority > max_mutex_priority) {
        while (max_mutex_ptr != NULL && cur_thread->priority > max_mutex_ptr->prio_ceil && max_mutex_ptr->locked_by != cur_thread->priority) {
        
            // cur_thread->locked_by_mutex_id = max_mutex_priority;
            cur_thread->locked_by_mutex_id = max_mutex_ptr->prio_ceil;
            pend_pendsv();
        }

        // rule 1
        // the mutex has been locked by other thread.
        // we should switch to the thread which has this mutex.
        while (mutex->locked_by != 16) {
            // printk("waiting for lock\n");
            //breakpoint();
            waiting_mutex_id = mutex->prio_ceil;
            pend_pendsv();
        }

        
        
        // lock this mutex.
        mutex->locked_by = cur_thread->priority;
        
        // find max mutex priority
        // max_mutex_priority == 16 means we don't have mutex now.
        // if (max_mutex_priority == 16 || max_mutex_priority < mutex->prio_ceil) {
        if (max_mutex_ptr == NULL || max_mutex_ptr->prio_ceil < mutex->prio_ceil) {
        
            // max_mutex_priority = mutex->prio_ceil;
            max_mutex_ptr = mutex;
               
        }
        // update mutex array
        for (uint32_t i = 0;i < 20; i++) {
            // mutex_prios[i] == 16 means this is a default value, not have mutex.
            if (mutex_prios[i] == 16) {
                mutex_prios[i] = mutex->prio_ceil;
                break;
            }
        }
        
        for (uint32_t i = 0;i < 20; i++) {
            if (cur_thread->mutexes_arr[i] == 32) {
                cur_thread->mutexes_arr[i] = mutex->prio_ceil;
                break;
            }
        }
        
        pend_pendsv();
        return;
    } 
    sys_thread_kill();
    printk("[Debug] Current thread priority lower than mutex priority ceil, have to be killed!\n");
}



void sys_mutex_unlock(kmutex_t* mutex) {
    // if this thread has this mutex
    if (mutex->locked_by == cur_thread->priority) {
        // reset this mutex(unlock the mutex)
        mutex->locked_by = 16;
        // reset waiting_mutex_id(has already unlock the mutex that was need by other thread)
        waiting_mutex_id = 32;
        //update mutex array and find second largest mutex priority
        //compare is the largest prio number
        int compare = -1;
        for (uint32_t i = 0;i < 20; i++) {
            if (mutex_prios[i] == mutex->prio_ceil) {
                mutex_prios[i] = 16;
            } else if (mutex_prios[i] != 16 && (int)mutex_prios[i] > compare && mutex_prios[i] != mutex->prio_ceil) {
                compare = mutex_prios[i];
            }
        }
        //there is not mutex now
        if (compare == -1) {
            max_mutex_ptr = NULL;
        } else {
            max_mutex_ptr = &mutexes[compare];
        }

        for (uint32_t i = 0;i < 20; i++) {
            if (cur_thread->mutexes_arr[i] == mutex->prio_ceil) {
                cur_thread->mutexes_arr[i] = 32;
                cur_thread->d_priority = cur_thread->priority;
                change_prio = false;
                break;
            }
        }

        // we have already unlock the mutex, we should clean the possible block of thread
        for (int32_t i = 0; i < allowed_threads_num; i++) {
            if (tcbs[i].locked_by_mutex_id == mutex->prio_ceil) {
                tcbs[i].locked_by_mutex_id = 32;
            }
        }
    
        pend_pendsv();
        return;
    }
    printk("[Debug] Current thread cannot unlock mutex!\n");
}


void init_tcb(void) {
    for (int32_t i = 0; i < 15; i++) {
        tcbs[i].status = INACTIVE;
    }
}

void init_mpu(tcb_t* cur, tcb_t *next) {
    if (cur->priority != MAIN_THREAD_ID) {
    
        // disable cur
        mm_region_disable(6);
        mm_region_disable(7);
    }
        // enable next
    if (next->priority != MAIN_THREAD_ID) {
        mm_region_enable(6, (void*)next->user_base, mm_log2ceil_size(thread_stack_size), 0, 1);
        mm_region_enable(7, (void*)next->kernel_base, mm_log2ceil_size(thread_stack_size), 0, 1);
    }
}

void reset_tcb(tcb_t*tcb) {
    tcb->status = INACTIVE;
    tcb->svc_status = 0;
    tcb->pc = 0;
    tcb->lr = 0;
    tcb->psp = 0;
    tcb->msp = 0;
    tcb->C = 0;
    tcb->T = 0;
    tcb->priority = 0;
    tcb->remained_T = 0;
    tcb->remained_C = 0;
    tcb->total_C = 0;
}

void udpate_thread_time(void) {
    cur_thread->remained_C-=1;
    cur_thread->total_C+=1;
    for (int32_t i = 0; i < allowed_threads_num; i++) {
        // printk_int(created_threads_num);
        if (tcbs[i].status != INACTIVE) {
            tcbs[i].remained_T-=1;
            // printk_int(tcbs[i].remained_T);
            // breakpoint();
            if (tcbs[i].remained_T <= 0) {

                tcbs[i].remained_C = tcbs[i].C;
                tcbs[i].remained_T = tcbs[i].T;
                tcbs[i].status = RUNNABLE;
            }
        }
    }
}

// The scheduler only for 0_0 - 0_3
tcb_t* round_robin_scheduler(void) {
    tcb_t* next_thread;
    round_robin_idx++;
    round_robin_idx = round_robin_idx % created_threads_num;
    next_thread = &tcbs[round_robin_idx];
    return next_thread;
}

tcb_t* RMS_scheduler(void) {
    // should directlly go to the thread which has the waiting mutex lock
    // must trigger by lock action
    if (waiting_mutex_id != 32) {
        //breakpoint();
        for (int32_t i = 0; i < allowed_threads_num; i++) {
            tcb_t* cur = &tcbs[i];
            for (uint32_t j = 0; j < 20; j++) {
                // find the thread that has the waiting mutex.
                // if find, switch to the thread directly.
                if (cur->mutexes_arr[j] == waiting_mutex_id) {
                    if (!change_prio) {
                        cur->d_priority = cur_thread->priority;
                        change_prio = true;    
                    }     
                    //lock_trigger = false;
                    //breakpoint();
                    return cur;
                }
            }
        }
    } 

    if (cur_thread->remained_C <= 0) {
        cur_thread->status = WAITING;
    }

    uint32_t max_prio = 18;
    tcb_t* next_thread = &tcbs[IDLE_THREAD_ID];
    for (int32_t i = 0; i < allowed_threads_num; i++) {
        if (tcbs[i].priority < max_prio && tcbs[i].status == RUNNABLE && tcbs[i].locked_by_mutex_id == 32) {
            if (tcbs[i].locked_by_mutex_id == 32) {
                next_thread = &tcbs[i];
                max_prio = tcbs[i].priority;
            }
        }
    }

    next_thread->status = RUNNING;
    return next_thread;
}


void default_idle_thread(void) {
    while (1) {
        // wait_for_interrupt();
        // printk("idle\n");
    };
}

int sys_create_path(char* path, int is_file) {
    kernel_create_path(path, is_file);
    return 0;
}

int sys_write_file(char* path, char* data) {
    return kernel_write_file(path, data);
}

uint32_t sys_read_file(char* path) {
    return kernel_read_file(path);
}

int sys_copy_path(char* path1, char* path2) {
    return kernel_copy_path(path1, path2);
}

int sys_delete_path(char* path) {
    return kernel_delete_path(path);
}

void sys_init_file_system(void) {
    kernel_init_file_system();
}