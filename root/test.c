#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <inttypes.h>
#include <assert.h>

// epsilon is given in nanos (10^9 nano = 1s)
// minix's shit kernel has latency around ~16ms afaik
#define EPSILON         1000000 //<this is one milisecond
#define NSEC            1000000000
#define HUSKY           NULL
// you can tweak this
#define SCALE1          64
#define SCALE2          64
#define SCALE3          64
#define WAIT_1_ON_1     5
#define WAIT_2_ON_1     12
#define WAIT_3_ON_1     7

uint64_t convert(time_t sec, long micro) {
  uint64_t time = sec * NSEC;
  time += (uint32_t) micro;
  return time;
}

time_t get_seconds(uint64_t time) {
  time /= NSEC;
  return (time_t) time;
}

long get_micro(uint64_t time) {
  return (uint32_t) (time % NSEC);
}

void add_micro(struct timeval* tv) {
  tv->tv_usec *= 1000;
}

void print_time(pid_t id, struct timeval* k, struct timeval* s) {
  uint64_t time = convert(k->tv_sec, k->tv_usec) - convert(s->tv_sec, s->tv_usec);
  printf("id %d: %lld.", (int) id, (long long) get_seconds(time));
  long micro = get_micro(time);

  if (micro == 0) {
    micro++;
  }

  while (micro < NSEC/10) {
    printf("0");
    micro *= 10;
  }

  printf("%lld\n", (long long) get_micro(time));
}

void print_testing(uint64_t actual, uint64_t users) {
  printf("TESTING: should've waited for [%" PRIu64 "]ns\n", actual);
  printf("TESTING:  actually waited for [%" PRIu64 "]ns\n", users);
}

void assert_time_diff(struct timeval* k, struct timeval* s, uint64_t def) {
  uint64_t diff = convert(k->tv_sec, k->tv_usec) - convert(s->tv_sec, s->tv_usec);
  print_testing(def, diff);

  if (diff > def) {
      assert(diff < def + EPSILON);
  } else {
      assert(def < diff + EPSILON);
  }
}

void assert_multiply(struct timeval* k, struct timeval* s,
  uint64_t def, uint64_t mul) {
  def *= NSEC;
  uint64_t diff = convert(k->tv_sec, k->tv_usec) - convert(s->tv_sec, s->tv_usec);
  uint64_t muled = diff * mul;
  print_testing(def, muled);

  if (muled > def) {
      assert(muled < def + EPSILON);
  } else {
      assert(def < muled + EPSILON);
  }
}

void print_one(const char* text, struct timeval* tv) {
  printf(text);
  printf(" ");

  static struct timeval temp;
  temp.tv_sec = 0;
  temp.tv_usec = 0;

  print_time(getpid(), tv, &temp);
}

int main(int argc, char** argv)
{
  pid_t pid;
  pid_t init = getpid();
  struct timeval start, koniec, start3, koniec4, start1, koniec1, koniec2;
  switch (pid = fork()) {
    case 0:

      pid = getpid();
      printf("I am process 2 with pid %d\n", pid);
      distort_time(init, SCALE1);
      sleep(2);

      gettimeofday(&start1, HUSKY);
      sleep(WAIT_1_ON_1);
      gettimeofday(&koniec2, HUSKY);
      add_micro(&start1);
      add_micro(&koniec2);

      print_time(pid, &koniec2, &start1);
      assert_time_diff(&koniec2, &start1, (uint64_t) SCALE2 * WAIT_1_ON_1 * NSEC);

      sleep(5 + WAIT_2_ON_1 + 1);
      distort_time(init, SCALE3);

      return 0;
    default:

      printf("I am process 1 with pid %d\n", init);
      distort_time(pid, SCALE2);
      sleep(2);


      gettimeofday(&start, HUSKY);
      add_micro(&start);
      print_one("1s: ", &start);

      sleep(WAIT_1_ON_1);

      gettimeofday(&koniec, HUSKY);
      add_micro(&koniec);
      print_one("1k: ", &koniec);
      print_time(init, &koniec, &start);
      assert_multiply(&koniec, &start, WAIT_1_ON_1, SCALE1);
      sleep(5);

      koniec.tv_usec /= 1000; //< sneaky bastard
      settimeofday(&koniec, HUSKY);

      gettimeofday(&start3, HUSKY);
      add_micro(&start3);
      print_one("1s: ", &start3);

      sleep(WAIT_2_ON_1);

      gettimeofday(&koniec4, HUSKY);
      add_micro(&koniec4);
      print_one("1k: ", &koniec4);
      print_time(init, &koniec4, &start3);
      assert_multiply(&koniec4, &start3, WAIT_2_ON_1, SCALE1);

      sleep(5);

      gettimeofday(&start, HUSKY);
      add_micro(&start);
      print_one("1s: ", &start);

      sleep(WAIT_3_ON_1);

      gettimeofday(&koniec, HUSKY);
      add_micro(&koniec);
      print_one("1k: ", &koniec);
      print_time(init, &koniec, &start);
      assert_multiply(&koniec, &start, WAIT_3_ON_1, SCALE3);
  }

  return 0;
}
