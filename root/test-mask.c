#include <stdio.h>
#include <inttypes.h>

#define DT_NORMAL     0
#define DT_DISTORTED  1
#define DT_BENCHMARK  2

#define DT_ANTECEDENT 4
#define DT_DESCENDANT 8

#define DT_CHECK(flag,state) (((flag) & (state)) == (state))

int main() {
  printf("DT_DISTORTED\n");
  uint8_t flag = DT_NORMAL;
  printf("false: %u\n", DT_CHECK(flag,DT_DISTORTED));
  flag = DT_DISTORTED;
  printf("true: %u\n", DT_CHECK(flag,DT_DISTORTED));
  flag |= DT_ANTECEDENT;
  printf("true: %u\n", DT_CHECK(flag,DT_DISTORTED));
  flag |= DT_BENCHMARK;
  printf("true: %u\n", DT_CHECK(flag,DT_DISTORTED));
  printf("DT_BENCHMARK\n");
  printf("true: %u\n", DT_CHECK(flag,DT_BENCHMARK));
  printf("DT_ANTECEDENT\n");
  printf("true: %u\n", DT_CHECK(flag,DT_ANTECEDENT));
  printf("DT_DESCENDANT\n");
  printf("false: %u\n", DT_CHECK(flag,DT_DESCENDANT));
}