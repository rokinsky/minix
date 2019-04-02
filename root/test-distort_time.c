#include <unistd.h>
#include <lib.h>

int main(int argc, char** argv)
{
	int res = distort_time(getpid(), 2);

	if (res == OK)
		printf("OK\n");
	else if (res == EINVAL)
		printf("EINVAL\n")
	else if (res == EPERM)
		printf("EPERM\n");

	return 0;
}