#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#define WITH_PRINT ( 0 )

/*
   Lorant SZABO
   
   Build with:
    `gcc -Wall -Wextra -pedantic -O0 -ggdb -o branch_test branch_test.c'
   Inspect assembly code for the critical section (e.g. compiler doesn't optimized 
   away important steps):
    `objdump -d -S (-M intel) branch_test'
   Run with perf (usually perf packed with 'linux-tools-generic' / 'linux-tools-common')
    `perf stat ./branch_test 17 3'
   And watch for branch-misses!

   E.g.:
    ``
*/

int main(int argc, char const *argv[]) {
	register uint64_t shr;
	register volatile uint64_t x = 0;
	int shr_len = 32;
	register int shr_len_minus_1 = 31;
	register uint64_t mask = (1UL << shr_len) - 1;

	/* run for only 1 sec */
	alarm(1);
	signal(SIGALRM, exit);

/* 
 * To test every 0-1 cases which are rotary invariant but the 0s and 1s' order matters,
 * simply odd numbers should be used as input. To test **only** the different 
 * integer partitions of 0s and 1s, A194602 series could be used (from oeis.org) 
 * - this ommit the 0s and 1s groups to be varible (e.g. 0111011011 and 0110111011
 * are not used in that series at the same time).
 */

	if (argc == 3) {
		shr_len = atoi(argv[1]);
		shr_len_minus_1 = shr_len - 1;
		if (shr_len == 64)
			mask = 0xffffffffffffffffUL;
		else
			mask = (1UL << shr_len) - 1;

		shr = atol(argv[2]);
		shr &= mask;
	} else {
		printf("Usage: %s <shr_length> <shr_init_value>\n", argv[0]);
		exit(-1);
	}

#if WITH_PRINT
	printf("shr_len: %d, shr: 0x%0*lx\n", shr_len, ((shr_len + 3) >> 2), shr);
#endif

	while (1) {
		if (shr & 1) {
			/* dummy, unreductible instructions to (hopefully) fill the pipeline */
			++x;
			x &= 0x1234567890111111UL;
			x ^= 0x1edcba9876111111UL;
			++x;
			x &= 0x2234567890222222UL;
			x ^= 0x2edcba9876222222UL;
			++x;
			x &= 0x3234567890333333UL;
			x ^= 0x3edcba9876333333UL;
		} else {
			--x;
			x &= 0x4234567890444444UL;
			x ^= 0x4edcba9876444444UL;
			--x;
			x &= 0x5234567890555555UL;
			x ^= 0x5edcba9876555555UL;
			--x;
			x &= 0x6234567890666666UL;
			x ^= 0x6edcba9876666666UL;
		}

		/* rotate around shift register with the initial content */
		shr = ( shr >> (shr_len_minus_1) ) | ( shr << 1 );
		shr &= mask;
	}

	return 0;
}