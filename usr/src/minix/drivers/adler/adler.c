#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <minix/syslib.h>

int main(int argc, char **argv) {
    sef_startup();
    printf("Hello, World!\n");
    sleep(10);
    return EXIT_SUCCESS;
}