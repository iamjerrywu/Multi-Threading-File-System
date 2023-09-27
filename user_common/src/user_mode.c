#include <usr_arg.h>
#include <stdlib.h>

extern int main(int argc, const char* argv[]);

/** NOTE: this is really not a good way to switch to userspace code. An RTOS with
 *  memory protection would launch a userspace program using an exception return, 
 *  which provides an atomic mode and control switch, which prevents an MPU fault.
 *  For simplicity, we're switching to user mode and then launching the program. 
 *
 *  Production code should never ever ever do this...
 */

void __attribute__((noinline)) launch_main() {
  int ret_val = main(user_argc, user_argv);
  // need to handle any memory ops, flush any buffers, etc. before you go?
  exit(ret_val);
}

void _usr_mod() {
  __asm volatile("mov r0, #3");
  __asm volatile("msr control, r0");
  __asm volatile("isb");
  // cannot inline call to user main because it would return to wrong stack
  launch_main();
}
