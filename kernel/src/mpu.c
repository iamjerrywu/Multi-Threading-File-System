/** @file   mpu.c
 *
 *  @brief  implementation of memory protection for lab 4
 *  @note   Not for public release, do not share
 *
 *  @date   last modified 25 October 2022
 *  @author CMU 14-642
**/

#include <arm.h>
#include <mpu.h>
#include <printk.h>
#include <syscall.h>
#include <syscall_thread.h>
#include <unistd.h>

/** @brief mpu control register */
#define MPU_CTRL *((uint32_t*)0xE000ED94)
/** @brief mpu region number register */
#define MPU_RNR *((uint32_t*)0xE000ED98)
/** @brief mpu region base address register */
#define MPU_RBAR *((uint32_t*)0xE000ED9C)
/** @brief mpu region attribute and size register */
#define MPU_RASR *((uint32_t*)0xE000EDA0)

/** @brief MPU CTRL register flags */
//@{
#define MPU_CTRL_ENABLE_BG  (1 << 2)
#define MPU_CTRL_ENABLE     (1 << 0)
//@}

/** @brief MPU RNR register flags */
//@{
#define MPU_RNR_REGION      (0xFF)
#define MPU_RNR_REGION_MAX  (7)
//@}

/** @brief MPU RASR register masks */
//@{
#define MPU_RASR_XN         (1 << 28)
#define MPU_RASR_SIZE       (0x3E)
#define MPU_RASR_ENABLE     (1 << 0)
#define MPU_RASR_USER_RO    (2 << 24)
#define MPU_RASR_USER_RW    (3 << 24)
#define MPU_RASR_USER_NO    (0 << 24)
//@}

/** @brief system handler control and state register */
#define SCB_SHCRS *((uint32_t*)0xE000ED24)
/** @brief configurable fault status register */
#define SCB_CFSR *((uint32_t*)0xE000ED28)
/** @brief mem mgmt fault address register */
#define SCB_MMFAR *((uint32_t*)0xE000ED34)

/** @brief SCB_SHCRS mem mgmt fault enable bit */
#define SHCRS_MEMFAULTENA   (1 << 16)

/** @brief stacking error */
#define SCB_CFSR_STKERR     (1 << 4)
/** @brief unstacking error */
#define SCB_CFSR_UNSTKERR   (1 << 3)
/** @brief data access error */
#define SCB_CFSR_DACCVIOL   (1 << 1)
/** @brief instruction access error */
#define SCB_CFSR_IACCVIOL   (1 << 0)
/** @brief indicates the MMFAR is valid */
#define SCB_CFSR_MMFARVALID (1 << 7)

#define SCB_SHCSR

extern void* thread_kill;
extern tcb_t tcbs[16];
extern tcb_t* cur_thread;
extern uint32_t thread_stack_size;

void hardfault_handler(void) {
    breakpoint();
    printk("hard fault shit!\n");
}

void mm_c_handler(void *psp) {
    int status = SCB_CFSR & 0xFF;

    psp_t* p = (psp_t*)psp;
    (void)p;

    // Attempt to print cause of fault
    printk("%s: ", __func__);
    printk("Memory Protection Fault\n");
    // breakpoint();
    if(status & SCB_CFSR_STKERR) printk("Stacking Error\n");
    if(status & SCB_CFSR_UNSTKERR) printk("Unstacking Error\n");
    if(status & SCB_CFSR_DACCVIOL) printk("Data access violation\n");
    if(status & SCB_CFSR_IACCVIOL) printk("Instruction access violation\n");
    if(status & SCB_CFSR_MMFARVALID) printk("Faulting Address = %x\n", SCB_MMFAR);

    // unrecoverable stack overflow
    
    uint32_t pc = ((psp_t*)psp)->pc;
    uint32_t limit_addr = (uint32_t)tcbs[cur_thread->priority].user_base + thread_stack_size;
    if(pc > limit_addr) {
        printk("stack overflow --> aborting\n");
        sys_exit(-1);
    }

    // Other errors can be recovered from by killing the offending thread.
    // Standard thread killing rules apply. You should halt if the thread
    // is the main or idle thread, but otherwise up to you. Manually set
    // the pc? Call a function? Context swap? TODO
    // breakpoint();
    // breakpoint();
    sys_thread_kill();
}



/** @brief  enables an aligned memory protection region
 *
 *  @param  region_number       region number to enable
 *  @param  base_address        region's base address
 *  @param  size_log2           log[2] of the region's size
 *  @param  exec_never          indicator if region is NOT user-executable
 *  @param  user_write_access   indicator if region is user-writable
 *
 *  @return 0 if successful, -1 on failure
 */
int mm_region_enable(uint32_t region_number, void* base_address, uint8_t size_log2, int execute, int user_write_access) {
    if(region_number > MPU_RNR_REGION_MAX) {
        printk("Invalid region number\n");
        return -1;
    }

    if((uint32_t)base_address & ((1 << size_log2) - 1)) {
        printk_int((uint32_t)base_address);
        printk("Misaligned region\n");
        return -1;
    }

    if(size_log2 < 5) {
        printk("Region too small\n");
        return -1;
    }

    MPU_RNR = region_number & MPU_RNR_REGION;
    MPU_RBAR = (uint32_t)base_address;

    uint32_t size = ((size_log2 - 1) << 1) & MPU_RASR_SIZE;
    uint32_t ap = user_write_access ? MPU_RASR_USER_RW : MPU_RASR_USER_RO;
    // uint32_t ap = user_write_access ? MPU_RASR_USER_RW : MPU_RASR_USER_NO;
    
    uint32_t xn = execute ? 0 : MPU_RASR_XN;

    MPU_RASR = size | ap | xn | MPU_RASR_ENABLE;

    return 0;
}

/**
 * @brief  Disables a memory protection region.
 *
 * @param  region_number      The region number to disable.
 */
void mm_region_disable(uint32_t region_number) {
    MPU_RNR = region_number & MPU_RNR_REGION;
    MPU_RASR &= ~MPU_RASR_ENABLE;
}

/**
 * @brief  Returns ceiling (log_2 n).
 */
uint32_t mm_log2ceil_size(uint32_t n) {
    uint32_t ret = 0;
    while(n > (1U << ret)) {
        ret++;
    }
    return ret;
}

void enable_mpu() {
    REG_WRITE(SCB_SHCRS, REG_READ(SCB_SHCRS) | 1 << 16);    
    REG_WRITE(MPU_CTRL, (MPU_CTRL_ENABLE_BG | MPU_CTRL_ENABLE));
}

void disable_mpu() {
    REG_WRITE(MPU_CTRL, 0);
}