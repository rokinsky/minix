#include <unistd.h>
#include <lib.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

void print_time(struct timeval tv) {
	time_t nowtime;
	struct tm *nowtm;
	char tmbuf[64], buf[64];
	nowtime = tv.tv_sec;
	nowtm = localtime(&nowtime);
	strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
	snprintf(buf, sizeof buf, "%s.%06ld", tmbuf, (long) tv.tv_usec);
	printf("%u: %s\n", getpid(), buf);
}

int main(int argc, char** argv)
{
	printf("same process: %d\n", distort_time(getpid(), 2));
	printf("doesn't exist: %d\n", distort_time(-1, 2));
	printf("init: %d\n", distort_time(1, 2));

	struct timeval tv;
	gettimeofday(&tv, NULL);
	print_time(tv);

	return 0;
}