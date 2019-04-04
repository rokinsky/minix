#include <unistd.h>
#include <lib.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <sys/wait.h>

void print_time(int p, struct timeval tv) {
	time_t nowtime;
	struct tm *nowtm;
	char tmbuf[64], buf[64];
	nowtime = tv.tv_sec;
	nowtm = localtime(&nowtime);
	strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
	snprintf(buf, sizeof buf, "%s.%06ld", tmbuf, (long) tv.tv_usec);
	printf("%u: %s\n", p, buf);
}

void P2(pid_t parent) {
	struct timeval tv_start, tv_end;
	distort_time(parent, 2);

	gettimeofday(&tv_start, NULL);
	print_time(2, tv_start);

	sleep(5);

	gettimeofday(&tv_end, NULL);
	print_time(2, tv_end);

	sleep(15);

	distort_time(parent, 4);
}

void P1() {
	pid_t child, parent = getpid();
	struct timeval tv_start, tv_end;

	child = fork();
	if (child == 0)
		P2(parent);

	distort_time(child, 2);

	gettimeofday(&tv_start, NULL);
	print_time(1, tv_start);

	sleep(5);

	gettimeofday(&tv_end, NULL);
	print_time(1, tv_end);

	sleep(10);
	printf("%d: new test\n", 1);

	gettimeofday(&tv_start, NULL);
	print_time(1, tv_start);

	sleep(3);

	gettimeofday(&tv_end, NULL);
	print_time(1, tv_end);

	sleep(5);

	gettimeofday(&tv_start, NULL);
	print_time(1, tv_start);

	sleep(10);

	gettimeofday(&tv_end, NULL);
	print_time(1, tv_end);

	wait(0);
}

void P3() {
	pid_t pid;
	struct timeval tv_start, tv_end;

	pid = fork();
	if (pid == 0)
		P1();

	gettimeofday(&tv_start, NULL);
	print_time(3, tv_start);

	sleep(5);

	gettimeofday(&tv_end, NULL);
	print_time(3, tv_end);

	sleep(9);

	settimeofday(&tv_end, NULL);

	wait(0);
}

void base_errors() {
	printf("same process: %d\n", distort_time(getpid(), 2));
	printf("doesn't exist: %d\n", distort_time(-1, 2));
	printf("init: %d\n", distort_time(1, 2));
}

int main(int argc, char** argv)
{
	base_errors();

	P3();

	return 0;
}