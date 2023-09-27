/* linker_template.lds -- linker for bootloader component
 * linker script contains 4 main components:
 * |--- memory layout
 * |--- sections
 * |--- options
 * |--- symbols
 *
 * It defines following symbols, which code can use without definition:
 *   __isr_vector_start
 *   __isr_vector_end
 *   __kernel_text_start
 *   __kernel_text_end
 *   __svc_stub_start
 *   __svc_stub_end
 *   __user_text_start
 *   __user_text_end
 *   __text_end
 *   __kernel_rodata_start
 *   __kernel_rodata_end
 *   __user_rodata_start
 *   __user_rodata_end
 *   __exidx_start
 *   __exidx_end
 *   __idata_start
 *   __data_start
 *   __data_end
 *   __kernel_data_start
 *   __kernel_data_end
 *   __user_data_start
 *   __user_data_end
 *   __rtt_start
 *   __rtt_end
 *   __bss_start
 *   __bss_end
 *   __kernel_bss_start
 *   __kernel_bss_end
 *   __user_bss_start
 *   __user_bss_end
 *   __heap_base
 *   __heap_limit
 *   __psp_stack_limit
 *   __psp_stack_base
 *   __msp_stack_limit
 *   __msp_stack_base
 *   __thread_u_stacks_limit
 *   __thread_u_stacks_base
 *   __thread_k_stacks_limit
 *   __thread_k_stacks_base
 *
 * do not re-distribute
 */

MEMORY
{
  flash       (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00100000
  ram         (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00040000
}

SECTIONS
{
/* sections to include:
 * .text for code & constants
 * .bss for unintialized data
 * .heap for our heap
 * .stack for our stack
 * .data for initialized data
 */

  .kernel_text :
  {
    /* vector table entries */
    __isr_vector_start = .;
    KEEP(*(.isr_vector))
    __isr_vector_end = .;
    __kernel_text_start = .;
    <K_OBJ_DIR>/*.o (.text*)        /* template */
    __kernel_text_end = .;
  } > flash

  .app_text :
  {
    /* userspace syscall stubs */
    . = ALIGN(16 * 1024); /* Align to 16K boundary */
    __svc_stub_start = .; 
    KEEP( *( .svc_stub ) )
    __svc_stub_end = .;
    __user_text_start = .;
    <U_OBJ_DIR>/*.o (.text*)        /* template */
    __user_text_end = .;
  } > flash
  
  __text_end = .;
  
  .rodata :
  {
    /* read-only kernel data -- 16kb aligned */
    . = ALIGN(16*1024);
    __kernel_rodata_start = .;
    <K_OBJ_DIR>/*.o (.rodata*);        /* template */
    __kernel_rodata_end = .;
    /* read-only user data -- 2kb aligned */
    . = ALIGN(2*1024);
    __user_rodata_start = .;
    <U_OBJ_DIR>/*.o (.rodata*);        /* template */
    *( .rodata );
    __user_rodata_end = .;
  } > flash

  PROVIDE_HIDDEN (__exidx_start = .);
  .ARM.exidx : ALIGN(4)
  {
      *(.ARM.exidx* .gnu.linkonce.armexidx.*)
  } > flash
  PROVIDE_HIDDEN (__exidx_end = .);

  . = ALIGN(2*1024);
  __idata_start = .;
  
  
  .data : AT (__idata_start)
  {
    . = ALIGN(2*1024);
    /* initialized data sections in ram */
    __data_start = .;
    __rtt_start = .;
    . = . + 168; /* RTT control block is 168 bytes long */
    __rtt_end = .;
    __kernel_data_start = .;
    <K_OBJ_DIR>/*.o (.data*);        /* template */
    __kernel_data_end = .;
    . = ALIGN(1024);
    __user_data_start = .;
    <U_OBJ_DIR>/*.o (.data*);        /* template */
    *( .data );
    __user_data_end = .;
    __data_end = .;
    . = ALIGN(1024);
  } > ram

  .bss :
  {
    /* uninitialized data sections in ram */
    . = ALIGN(1024);
    __bss_start = .;
    __kernel_bss_start = .;
    <K_OBJ_DIR>/*.o (.bss*);        /* template */
    <K_OBJ_DIR>/*.o (COMMON*);      /* template */
    __kernel_bss_end = .;
    . = ALIGN(1024);
    __user_bss_start = .;
    <U_OBJ_DIR>/*.o (.bss*);        /* template */
    <U_OBJ_DIR>/*.o (COMMON*);      /* template */
    * ( .bss ) * ( COMMON ) ;
    __user_bss_end = .;
    __bss_end = .;
    . = ALIGN(1024);
  } > ram

  .misc :
  {
    . = ALIGN(8*1024);
    
    __heap_base = .;
    . = . + (8*1024);      /* 8k heap for _sbrk */
    __heap_limit = .;
    
    __psp_stack_limit = .;
    . = . + (2*1024);       /* 2k main thread stack */
    __psp_stack_base = .;

    __msp_stack_limit = .;
    . = . + (2*1024);       /* 2k kernel stack */
    __msp_stack_base = .;
    
    . = ALIGN(32*1024);
    __thread_u_stacks_limit = .;
    . = . + (32*1024);      /* 32k for thread user stacks */
    __thread_u_stacks_base = .;
    
    __thread_k_stacks_limit = .;
    . = . + (32*1024);      /* 32k for thread kernel stacks */
    __thread_k_stacks_base = .;    
  } > ram

  __end = .;
}
