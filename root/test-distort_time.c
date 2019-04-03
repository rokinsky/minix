#include <unistd.h>
#include <lib.h>
#include <errno.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	int res = distort_time(getpid(), 2);

	printf("%d\n", res);

	return 0;
}