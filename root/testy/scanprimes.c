#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define CYCLES_COUNT 20
#define SCAN_COUNT   20
#define BUFFER_SIZE  1
#define STDOUT       1

char buf[BUFFER_SIZE];

void scan(char* file)
{
	int fd, bytes;
	fd = open(file, O_RDONLY);
	while ((bytes = read(fd, buf, BUFFER_SIZE)))
		write(STDOUT, buf, bytes);
	close(fd);
}

#define PRIMES_COUNT 10000000

char sieve[PRIMES_COUNT];

void run_sieve(void)
{
	int i, j;
	int count = 0;
	for (i = 0; i < PRIMES_COUNT; ++i)
	{
		sieve[i] = 0;
	}
	for (i = 2; i * i <= PRIMES_COUNT; ++i)
	{
		if (!sieve[i])
		{
			for(j = i+i; j < PRIMES_COUNT; j += i)
			{
				sieve[j] = 1;
			}
		}
	}
	for (i = 2; i < PRIMES_COUNT; ++i)
	{
		count += (1 - sieve[i]);
	}
	printf("Liczb pierwszych: %d\n", count);
	fflush(stdout);
}

int main(int argc, char** argv)
{
	int i, j, k;
	for (i = 1; i < argc; ++i)
		for (j = 0; j < CYCLES_COUNT; ++j)
		{
			for (k = 0; k < SCAN_COUNT; ++k)
				scan(argv[i]);
			run_sieve();
		}
	return 0;
}
