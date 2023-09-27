/* startup.s
 * Assembler startup file for the nRF52840
 *
 *
 * do not re-distribute
 */

    .syntax unified
    .arch armv7e-m
    .cpu cortex-m4
    .fpu fpv4-sp-d16
    .thumb

    .section .isr_vector
    .align 2
    .globl __isr_vector
__isr_vector:
    .long   __msp_stack_base            /* initial kernel stack pointer */
    .long   Reset_Handler
    .long   NMI_Handler
    .long   HardFault_Handler
    .long   MemoryManagement_Handler
    .long   BusFault_Handler
    .long   UsageFault_Handler
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   svc_asm_handler
    .long   DebugMon_Handler
    .long   0                           /*Reserved */
    .long   pendsv_asm_handler
    .long   SysTick_Handler

  /* External Interrupts */
    .long   POWER_CLOCK_IRQHandler
    .long   RADIO_IRQHandler
    .long   UARTE0_UART0_IRQHandler
    .long   SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler
    .long   SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler
    .long   NFCT_IRQHandler
    .long   GPIOTE_IRQHandler
    .long   SAADC_IRQHandler
    .long   TIMER0_IRQHandler
    .long   TIMER1_IRQHandler
    .long   TIMER2_IRQHandler
    .long   RTC0_IRQHandler
    .long   TEMP_IRQHandler
    .long   RNG_IRQHandler
    .long   ECB_IRQHandler
    .long   CCM_AAR_IRQHandler
    .long   WDT_IRQHandler
    .long   RTC1_IRQHandler
    .long   QDEC_IRQHandler
    .long   COMP_LPCOMP_IRQHandler
    .long   SWI0_EGU0_IRQHandler
    .long   SWI1_EGU1_IRQHandler
    .long   SWI2_EGU2_IRQHandler
    .long   SWI3_EGU3_IRQHandler
    .long   SWI4_EGU4_IRQHandler
    .long   SWI5_EGU5_IRQHandler
    .long   TIMER3_IRQHandler
    .long   TIMER4_IRQHandler
    .long   PWM0_IRQHandler
    .long   PDM_IRQHandler
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   MWU_IRQHandler
    .long   PWM1_IRQHandler
    .long   PWM2_IRQHandler
    .long   SPIM2_SPIS2_SPI2_IRQHandler
    .long   RTC2_IRQHandler
    .long   I2S_IRQHandler
    .long   FPU_IRQHandler
    .long   USBD_IRQHandler
    .long   UARTE1_IRQHandler
    .long   QSPI_IRQHandler
    .long   CRYPTOCELL_IRQHandler
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   PWM3_IRQHandler
    .long   0                           /*Reserved */
    .long   SPIM3_IRQHandler
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */
    .long   0                           /*Reserved */

    .size __isr_vector, . - __isr_vector


/* Reset Handler */

    .text
    .thumb
    .thumb_func
    .align 1
    .globl Reset_Handler
    .type Reset_Handler, %function
Reset_Handler:

/* @TODO: here's where you'll define your Reset_Handler functionality for your bootloader
 *
 * |----- allocate and zero space for uninitialized variables in RAM
 * |----- allocate space and copy values of initialized global variables to RAM
 *
 * afterward, it should jump to the kernel_main function defined in the obfuscated kernel
 * and loop forever as a safeguard in case kernel_main returns
 * 
 */

ldr r0, =__idata_start
    ldr r1, =__data_start
    ldr r2, =__data_end

L1: cmp r1, r2
    ldr r3, [r0], #4
    str r3, [r1], #4
    blt L1

    ldr r4, =__bss_start
    ldr r5, =__bss_end
    mov r6, 0

L2: cmp r4, r5
    str r6, [r4], #4
    blt L2


/* your changes for Lab 1 should stop here */

    bl kernel_main
    b .

    .pool
    .size   Reset_Handler,.-Reset_Handler


    .section ".text"

/* Dummy Exception Handlers (infinite loops which can be modified) */

    .weak   NMI_Handler
    .type   NMI_Handler, %function
NMI_Handler:
    b       .
    .size   NMI_Handler, . - NMI_Handler


    .weak   HardFault_Handler
    .type   HardFault_Handler, %function
HardFault_Handler:
    bkpt
    b       hardfault_handler
    .size   HardFault_Handler, . - HardFault_Handler


    .weak   MemoryManagement_Handler
    .type   MemoryManagement_Handler, %function
MemoryManagement_Handler:
    /* bkpt */
    mrs r0, psp    // build exception stack frame
    b       mm_c_handler
    .size   MemoryManagement_Handler, . - MemoryManagement_Handler


    .weak   BusFault_Handler
    .type   BusFault_Handler, %function
BusFault_Handler:
    b       .
    .size   BusFault_Handler, . - BusFault_Handler


    .weak   UsageFault_Handler
    .type   UsageFault_Handler, %function
UsageFault_Handler:
    b       .
    .size   UsageFault_Handler, . - UsageFault_Handler


    .thumb_func
    .global svc_asm_handler
svc_asm_handler:
    mrs r0, psp    // build exception stack frame

    b svc_c_handler       
    .size   svc_asm_handler, . - svc_asm_handler

    .weak   DebugMon_Handler
    .type   DebugMon_Handler, %function
DebugMon_Handler:
    b       .
    .size   DebugMon_Handler, . - DebugMon_Handler


    .thumb_func
    .global pendsv_asm_handler
pendsv_asm_handler:
    /* cpsid i */
    push {lr}
    push {r4-r11}
    mrs r0, psp
    push {r0}
    mrs r0, msp
    bl pendsv_c_handler
    msr msp, r0
    pop {r0}
    msr psp, r0
    pop {r4-r11}
    pop {lr}
    /* cpsie i */
    bx  lr
    .size   pendsv_asm_handler, . - pendsv_asm_handler
    

    .weak   SysTick_Handler
    .type   SysTick_Handler, %function
SysTick_Handler:
    mrs r0, psp    // build exception stack frame
    b       systick_c_handler
    .size   SysTick_Handler, . - SysTick_Handler


/* IRQ Handlers */

    .globl  Default_Handler
    .type   Default_Handler, %function
Default_Handler:
    b       .
    .size   Default_Handler, . - Default_Handler

    .macro  IRQ handler
    .weak   \handler
    .set    \handler, Default_Handler
    .endm

    IRQ  POWER_CLOCK_IRQHandler
    IRQ  RADIO_IRQHandler
    IRQ  UARTE0_UART0_IRQHandler
    IRQ  SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler
    IRQ  SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler
    IRQ  NFCT_IRQHandler
    IRQ  GPIOTE_IRQHandler
    IRQ  SAADC_IRQHandler
    IRQ  TIMER0_IRQHandler
    IRQ  TIMER1_IRQHandler
    IRQ  TIMER2_IRQHandler
    IRQ  RTC0_IRQHandler
    IRQ  TEMP_IRQHandler
    IRQ  RNG_IRQHandler
    IRQ  ECB_IRQHandler
    IRQ  CCM_AAR_IRQHandler
    IRQ  WDT_IRQHandler
    IRQ  RTC1_IRQHandler
    IRQ  QDEC_IRQHandler
    IRQ  COMP_LPCOMP_IRQHandler
    IRQ  SWI0_EGU0_IRQHandler
    IRQ  SWI1_EGU1_IRQHandler
    IRQ  SWI2_EGU2_IRQHandler
    IRQ  SWI3_EGU3_IRQHandler
    IRQ  SWI4_EGU4_IRQHandler
    IRQ  SWI5_EGU5_IRQHandler
    IRQ  TIMER3_IRQHandler
    IRQ  TIMER4_IRQHandler
    IRQ  PWM0_IRQHandler
    IRQ  PDM_IRQHandler
    IRQ  MWU_IRQHandler
    IRQ  PWM1_IRQHandler
    IRQ  PWM2_IRQHandler
    IRQ  SPIM2_SPIS2_SPI2_IRQHandler
    IRQ  RTC2_IRQHandler
    IRQ  I2S_IRQHandler
    IRQ  FPU_IRQHandler
    IRQ  USBD_IRQHandler
    IRQ  UARTE1_IRQHandler
    IRQ  QSPI_IRQHandler
    IRQ  CRYPTOCELL_IRQHandler
    IRQ  PWM3_IRQHandler
    IRQ  SPIM3_IRQHandler

  .end
