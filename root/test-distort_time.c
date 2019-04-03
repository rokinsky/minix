#include <unistd.h>
#include <lib.h>
#include <errno.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	printf("same process: %d\n", distort_time(getpid(), 2));
	printf("doesn't exist: %d\n", distort_time(-1, 2));
	printf("init: %d\n", distort_time(1, 2));

	return 0;
}