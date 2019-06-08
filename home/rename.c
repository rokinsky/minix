#include <stdio.h>

int main (int ac, char** av) {
   if (ac == 2) rename(av[1], av[1]);
   else return 1;
}
