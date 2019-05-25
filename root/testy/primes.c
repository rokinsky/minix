#include <stdio.h>

#define PRIMES_COUNT 10000000
#define CYCLES 500

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
	int i;
	for (i = 0; i < CYCLES; ++i)
		run_sieve();
	return 0;
}
