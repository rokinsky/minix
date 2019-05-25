	./primes > /dev/null &
	./primes > /dev/null &
	./primes > /dev/null &
	./primes > /dev/null &
	./primes > /dev/null &
	echo "SCANFILES" > tests
	time ./scanfiles scanfiles.c > /dev/null 2>>tests
	time ./scanfiles scanfiles.c > /dev/null 2>>tests
	time ./scanfiles scanfiles.c > /dev/null 2>>tests
	time ./scanfiles scanfiles.c > /dev/null 2>>tests
	time ./scanfiles scanfiles.c > /dev/null 2>>tests
	time ./scanfiles scanfiles.c > /dev/null 2>>tests
	time ./scanfiles scanfiles.c > /dev/null 2>>tests
	time ./scanfiles scanfiles.c > /dev/null 2>>tests
	time ./scanfiles scanfiles.c > /dev/null 2>>tests
	time ./scanfiles scanfiles.c > /dev/null 2>>tests
	echo "SCANPRIMES" >> tests
	time ./scanprimes scanfiles.c > /dev/null 2>>tests
	time ./scanprimes scanfiles.c > /dev/null 2>>tests
	time ./scanprimes scanfiles.c > /dev/null 2>>tests
	time ./scanprimes scanfiles.c > /dev/null 2>>tests
	time ./scanprimes scanfiles.c > /dev/null 2>>tests
	time ./scanprimes scanfiles.c > /dev/null 2>>tests
	time ./scanprimes scanfiles.c > /dev/null 2>>tests
	time ./scanprimes scanfiles.c > /dev/null 2>>tests
	time ./scanprimes scanfiles.c > /dev/null 2>>tests
	time ./scanprimes scanfiles.c > /dev/null 2>>tests
	cat tests
