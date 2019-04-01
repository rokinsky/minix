#include "pm.h"
#include "mproc.h"
#include <stdio.h>
#include <sys/types.h>

int do_distort_time() {
  int caller = m_in.m1_i1;
  int contender = m_in.m1_i2;
  int scale = m_in.m1_i3;

  printf("Hello world!\n");
  return 0; // OK;
}
