#include "pm.h"
#include "mproc.h"
#include <stdio.h>
#include <sys/types.h>

int do_distort_time() {
  pid_t caller = m_in.m1_i1;
  pid_t contender = m_in.m1_i2;
  uint8_t scale = m_in.m1_i3;

  printf("Hello world!\n");
  return 0; // OK;
}
