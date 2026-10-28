/*********************************************************
*
* Description: A multithreaded program that calculates the
* sum of divisors for a 10-12 digit non negative number
* using the openMP API
* Author: Moshe Yifrah
*
* Compile with: gcc open_mp_sum_of_divs.c -fopenmp
*
*********************************************************/

#include <stdio.h>   /* printf */
#include <omp.h>     /* openMP API */

/*********************************************************/

int main()
{
	size_t i = 1;
	size_t number = 1000000000UL;
	size_t sum_of_divisors = 0;
	int round = 0, rounds = 10;
	size_t round_sum = 0;

	for(round = 0; round < rounds; ++round)
	{
		round_sum = 0;
		
		#pragma omp parallel for reduction(+:round_sum)
		for(i = 1; i <= number; ++i)
		{
			if (0 == number % i)
			{
				round_sum += i;
			}
		}
		
		sum_of_divisors += round_sum;
	}
	
	printf("Sum of divisors of %lu is: %lu\n", number, sum_of_divisors / round);
	
	return 0;
}


/*********************************************************/


