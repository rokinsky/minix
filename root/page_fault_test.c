#include <stdio.h>
#include <stdlib.h>

static void printf_usage(char const *program_name) {
  printf("Usage: %s r|w|x hex_address\n", program_name);
  exit(1);
}

int main(int argc, char *argv[]) {
  volatile char *p;
  volatile char c;
  void (* volatile f)();
  unsigned long address;

  if (argc != 3)
    printf_usage(argv[0]);

  if (1 != sscanf(argv[2], "%lx", &address))
    printf_usage(argv[0]);

  c = 'a';
  p = (char *)address;
  f = (void (*)())address;

  switch (argv[1][0]) {
    case 'r':
      c = *p;
      break;
    case 'w':
      *p = c;
      break;
    case 'x':
      f();
      break;
    default:
      printf_usage(argv[0]);
  }

  return 0;
}
