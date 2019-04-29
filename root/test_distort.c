// Krzysztof Małysa
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <minix/config.h>

struct timeval subtract(struct timeval a, struct timeval b) {
	struct timeval res = {a.tv_sec - b.tv_sec, a.tv_usec - b.tv_usec};
	if (res.tv_usec < 0) {
		--res.tv_sec;
		res.tv_usec += 1000000;
	}

	return res;
}

void mysleep(double seconds) {
	// sleep() and usleep() call nanosleep() with second argument causing nanosleep() to call gettimeofday()
	struct timespec ts;
	ts.tv_sec = seconds;
	ts.tv_nsec = (seconds - (int)seconds) * 1e9;
	assert(0 == nanosleep(&ts, NULL)); // This does not call gettimeofday()
}

void print_current_time_from_tv(struct timeval tv) {
	printf("current time: %.6lf\n", tv.tv_sec + tv.tv_usec / 1e6);
	fflush(stdout);
}

void print_current_time() {
	struct timeval tv;
	assert(0 == gettimeofday(&tv, NULL));
	print_current_time_from_tv(tv);
}

void do_check_diff_impl(struct timeval beg, struct timeval end, double expected_diff, const char* msg) {
	struct timeval diff = subtract(end, beg);
	double ddiff = diff.tv_sec + diff.tv_usec / 1e6;
	printf("%s: %.6lf\n", msg, ddiff);
	print_current_time_from_tv(end);
	fflush(stdout);
	assert(fabs(ddiff - expected_diff) < 0.04);
}

#define check_diff(beg, end, exp_df) do_check_diff(beg, end, exp_df, __LINE__)
void do_check_diff(struct timeval beg, struct timeval end, double expected_diff, int line) {
	printf("======= check from line: %i =======\n", line);
	do_check_diff_impl(beg, end, expected_diff, "time diff");
}

#define sleep_for(sec, ed) do_sleep_for(sec, ed, __LINE__)
void do_sleep_for(double seconds, double expected_diff, int line) {
	printf("======= check from line: %i =======\n", line);
	fflush(stdout);
	struct timeval before, after;
	assert(0 == gettimeofday(&before, NULL));
	print_current_time_from_tv(before);
	printf("sleeping for %.6lf seconds...\n", seconds);
	fflush(stdout);
	mysleep(seconds);

	assert(0 == gettimeofday(&after, NULL));
	do_check_diff_impl(before, after, expected_diff, "awaken after");
}

void assert_wait() {
	int status;
	assert(wait(&status) != -1);
	assert(WIFEXITED(status));
	assert(WEXITSTATUS(status) == 0);
}

void distort_my_time_from_child(int scale) {
	if (fork() == 0) {
		assert(distort_time(getppid(), scale) == 0);
		_exit(0);
	}

	assert_wait();
}

void distort_time_of(pid_t pid, int scale) {
	assert(distort_time(pid, scale) == 0);
}

pid_t some_nonexistent_pid() {
	pid_t p = 1;
	while ((errno = 0, kill(p, 0)) == 0 || errno != ESRCH)
		++p;

	return p;
}

// Checks argument checking
void test1() {
	sleep_for(1, 1);
	assert(distort_time(getpid(), 2) == EPERM);

	assert(distort_time(some_nonexistent_pid(), 2) == EINVAL);

	assert(distort_time(5, 2) == EPERM); // 5 == pid of /service/pm

	assert(distort_time(getppid(), 2) == 0);
}

// Checks distorting time of ancestor
void test2() {
	distort_my_time_from_child(4);

	sleep_for(4, 1);
	sleep_for(1, 0.25);

	distort_my_time_from_child(0);
	sleep_for(1, 0);
	sleep_for(2, 0);

	distort_my_time_from_child(255);
	sleep_for(12, 12.0/255);

	distort_my_time_from_child(256); // Overflow
	sleep_for(1, 0);

	distort_my_time_from_child(1);
	sleep_for(1, 1);
}

void resettimeofday() {
	struct timeval curr;
	assert(0 == gettimeofday(&curr, NULL));
	assert(0 == settimeofday(&curr, NULL));
}

// Checks distorting time of parent
void test3() {
	distort_my_time_from_child(1);

	pid_t pid = fork();
	if (pid == 0) { // Child
		pid = fork();
		if (pid == 0) { // Child
			// Sync 0
			sleep_for(0.2, 0.2); // Tricky: gets current time, then after 0.1 sec time gets distorted - time stops, then on the second read time the time point will be set
			// Sync 1
			sleep_for(1, 0);
			// Sync 2
			sleep_for(1, 2.0); // Tricky: gets current time (frozen for 1 second, then waits 1 second but in the meanwhile settimeofday() is called and time point is reset, so after sleeping for 1 second, the second read of current time will return current time and set time point to its value)
			// Sync 3
			mysleep(0.2);
			// Sync 4
			sleep_for(1, 4);
			// Sync 5
			mysleep(0.2);
			// Sync 6
			sleep_for(1, 255);
			// Sync 7
			mysleep(0.2);
			// Sync 8
			sleep_for(2, 0);
			// Sync 9
			distort_my_time_from_child(1);
			sleep_for(1, 1);
			// Sync 10

			exit(0);
		}

		assert_wait();
		exit(0);
	}

	pid += 1; // A dirty hack to get the child's child's pid
	// Sync 0
	mysleep(0.1);
	distort_time_of(pid, 0);
	mysleep(0.1);
	// Sync 1
	mysleep(1);
	// Sync 2
	mysleep(0.5);
	resettimeofday();
	mysleep(0.5);
	// Sync 3
	mysleep(0.1);
	distort_time_of(pid, 4);
	mysleep(0.1);
	// Sync 4
	mysleep(1);
	// Sync 5
	mysleep(0.1);
	distort_time_of(pid, 255);
	mysleep(0.1);
	// Sync 6
	mysleep(1);
	// Sync 7
	mysleep(0.1);
	distort_time_of(pid, 256); // Overflow
	mysleep(0.1);
	// Sync 8
	mysleep(2);
	// Sync 9
	mysleep(1);
	// Sync 10

	assert_wait();
}

// Checks distorting time of parent by 1
void test4() {
	struct timeval beg, end;

	distort_my_time_from_child(2);
	resettimeofday();

	sleep_for(1, 0.5);

	assert(0 == gettimeofday(&beg, NULL));
	distort_my_time_from_child(4);
	assert(0 == gettimeofday(&end, NULL));
	check_diff(beg, end, -0.25);

	distort_my_time_from_child(2);
	resettimeofday();
	sleep_for(1, 0.5);

	assert(0 == gettimeofday(&beg, NULL));
	distort_my_time_from_child(1);
	assert(0 == gettimeofday(&end, NULL));
	check_diff(beg, end, 0.5);
}

// Checks unsuccessful calls to PM_CLOCK_SETTIME
void test5() {
	struct timeval beg, end;

	distort_my_time_from_child(0);
	resettimeofday();

	sleep_for(1, 0);

	assert(0 == gettimeofday(&beg, NULL));
	struct timespec ts;
	ts.tv_sec = beg.tv_sec + 1;
	ts.tv_nsec = beg.tv_usec * 1000;

	assert(-1 == clock_settime(CLOCK_MONOTONIC, &ts));
	assert(errno == EINVAL);
	assert(0 == gettimeofday(&end, NULL)); // Time should not have been reset
	check_diff(beg, end, 0);

	assert(0 == clock_settime(CLOCK_REALTIME, &ts));
	assert(0 == gettimeofday(&end, NULL)); // Time should have been reset
	check_diff(beg, end, 1);
}

// Checks timepoint setting
void test6() {
	if (fork() == 0) {
		// New process is not distorted and no time point is set
		struct timeval beg, end;
		assert(0 == gettimeofday(&beg, NULL));

		sleep_for(0.5, 0.5);
		distort_my_time_from_child(0);
		mysleep(0.5);

		assert(0 == gettimeofday(&end, NULL)); // Time point is set now
		check_diff(beg, end, 1);

		sleep_for(0.5, 0);

		_exit(0);
	}
	assert_wait();

	if (fork() == 0) {
		// New process is not distorted and no time point is set
		struct timeval beg, end;
		assert(0 == gettimeofday(&beg, NULL));

		sleep_for(0.5, 0.5);
		distort_my_time_from_child(1);
		sleep_for(0.5, 0.5);

		assert(0 == gettimeofday(&end, NULL));
		check_diff(beg, end, 1);

		sleep_for(0.5, 0.5);

		distort_my_time_from_child(0); // Time point does not change
		assert(0 == gettimeofday(&end, NULL));
		check_diff(beg, end, 0.5);

		_exit(0);
	}
	assert_wait();

	if (fork() == 0) {
		// New process is not distorted and no time point is set
		struct timeval beg, end;
		assert(0 == gettimeofday(&beg, NULL));

		sleep_for(0.5, 0.5);
		distort_my_time_from_child(2);
		mysleep(0.5);

		assert(0 == gettimeofday(&end, NULL));
		check_diff(beg, end, 1);

		sleep_for(0.5, 0.25);

		_exit(0);
	}
	assert_wait();

	if (fork() == 0) {
		// New process is not distorted and no time point is set
		struct timeval beg, end;

		distort_my_time_from_child(4);
		sleep_for(0.4, 0.1);

		assert(0 == gettimeofday(&beg, NULL));
		resettimeofday(); // Time point is reseted
		
		mysleep(1);

		assert(0 == gettimeofday(&end, NULL)); // Time point is set now
		check_diff(beg, end, 1);

		sleep_for(0.4, 0.1);

		_exit(0);
	}
	assert_wait();
}

void test7() {
	int i;
	for(i = 0; i < NR_PROCS; i++) {
		if (fork() == 0) {
			struct timeval tv;
			distort_my_time_from_child(0);
			assert(0 == gettimeofday(&tv, NULL));
			assert(0 == gettimeofday(&tv, NULL));
			_exit(0);
		} else {
			assert_wait();
		}
	}
	for(i = 0; i < NR_PROCS; i++) {
		if (fork() == 0) {
			struct timeval tv1, tv2;
			assert(0 == gettimeofday(&tv1, NULL));
			mysleep(0.01);
			assert(0 == gettimeofday(&tv2, NULL));
			assert(tv1.tv_usec != tv2.tv_usec || tv1.tv_sec != tv2.tv_sec);
			exit(0);
		} else {
		assert_wait();
		}
	}
}

int main() {
	test1();
	test2();
	test3();
	test4();
	test5();
	test6();
	test7();

	printf("\033[1;32mAll tests passed!\033[m\n");
}