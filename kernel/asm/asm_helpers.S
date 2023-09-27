.cpu cortex-m4
.syntax unified
.thumb

.section .text

.thumb_func
.global enter_user_mode
enter_user_mode:
  bkpt
  ldr r0, =__psp_stack_base
  msr psp, r0
  mov r1, #0x3
  msr CONTROL, r1
  isb
  bl launch_main
  @ cannot allow this to return or the wrong stack will be used...
