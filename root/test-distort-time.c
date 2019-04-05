#include <unistd.h>
#include <lib.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <sys/wait.h>

struct timeval print_time(int p) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	printf("%d: <%ld.%06ld>\n", p, (long int)(tv.tv_sec), (long int)(tv.tv_usec));
	if (p == 1) {
		printf("\n");
	}
	return tv;
}

void P2(pid_t parent) {
	distort_time(parent, 2);

	print_time(2);

	sleep(5);

	print_time(2);

	sleep(15);

	distort_time(parent, 4);
}

void P1() {
	pid_t child, parent = getpid();

	child = fork();
	if (child == 0) {
		P2(parent);
		return;
	}

	distort_time(child, 3);

	print_time(1);

	sleep(5);

	print_time(1);

	sleep(10);

	print_time(1);

	sleep(3);

	print_time(1);

	sleep(5);

	print_time(1);

	sleep(10);

	print_time(1);

	wait(0);
}

void P3() {
	pid_t pid;

	pid = fork();
	if (pid == 0) {
		P1();
		return;
	}

	print_time(3);

	sleep(5);

	struct timeval tv = print_time(3);

	sleep(9);

	settimeofday(&tv, NULL);
	print_time(3);

	wait(0);
}

void base_errors() {
	printf("same process: %d\n", distort_time(getpid(), 2));
	printf("doesn't exist: %d\n", distort_time(-1, 2));
	//printf("init: %d\n", distort_time(1, 2));
}

int main(int argc, char** argv)
{
	base_errors();

	P3();

	return 0;
}