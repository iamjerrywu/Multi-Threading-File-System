/** @file   kernel.c
 *
 *  @brief  starting point for kernel-space operations
 *  @note   Not for public release, do not share
 *
 *  @date   last modified 25 October 2022
 *  @author CMU 14-642
**/

#include <arm.h>
#include <printk.h>
#include <rtt.h>
#include <timer.h>
#include <mpu.h>

extern void enter_user_mode();
extern uint32_t __svc_stub_start;
extern uint32_t __user_rodata_start;
extern uint32_t __user_data_start;
extern uint32_t __bss_start;
extern uint32_t __heap_base;
extern uint32_t __psp_stack_limit;

extern char __isr_vector_start;
extern char __isr_vector_end;
extern char __kernel_text_start;
extern char __kernel_text_end;
// extern char __svc_stub_start;
extern char __svc_stub_end;
extern char __user_text_start;
extern char __user_text_end;

extern char __kernel_bss_start;
extern char __kernel_bss_end;
extern char __user_bss_start;
extern char __user_bss_end;



/** @brief mpu control register */
#define MPU_CTRL 0xE000ED94
/** @brief mpu region number register */
#define MPU_RNR 0xE000ED98
/** @brief mpu region base address register */
#define MPU_RBAR 0xE000ED9C
/** @brief mpu region attribute and size register */
#define MPU_RASR 0xE000EDA0

/** @brief system handler control and state register */
#define SCB_SHCRS 0xE000ED24
/** @brief configurable fault status register */
#define SCB_CFSR 0xE000ED28
/** @brief mem mgmt fault address register */
#define SCB_MMFAR 0xE000ED34

#define MPU_CTRL_ENABLE_BG  (1 << 2)
#define MPU_CTRL_ENABLE     (1 << 0)

int kernel_main() {
    init_align_prio();          // <-- do not remove
    // breakpoint();
    rtt_init();
    breakpoint();
    // while (1) {
    //     systick_start(1000);
    // }
    enable_fpu();

    // enable mpu
    REG_WRITE(SCB_SHCRS, REG_READ(SCB_SHCRS) | 1 << 16);    
    // REG_WRITE(MPU_CTRL, (MPU_CTRL_ENABLE_BG | MPU_CTRL_ENABLE));
    
    // user code
    mm_region_enable(0, &__isr_vector_start, mm_log2ceil_size(32 * 1024), 1, 1);
    
    // user readonly
    mm_region_enable(1, &__user_rodata_start, mm_log2ceil_size(2 * 1024), 0, 0); //try 0, 0
    // data
    mm_region_enable(2, &__user_data_start, mm_log2ceil_size(1 * 1024), 0, 1); // try 0, 1
    // bss
    mm_region_enable(3, &__user_bss_start, mm_log2ceil_size(1 * 1024), 1, 1); // try 0, 1
    // heap
    mm_region_enable(4, &__heap_base, mm_log2ceil_size(8 * 1024), 1, 1); // try 0, 1
    // main user stack
    mm_region_enable(5, &__psp_stack_limit, mm_log2ceil_size(2 * 1024), 1, 1); // try 0, 1
    enter_user_mode();
    BUSY_LOOP(1);
    return 0;
}
