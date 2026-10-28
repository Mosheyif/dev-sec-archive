/*********************************************************
*
* Description: A single thread program that calculates the
* sum of divisors for a 10-12 digit non negative number
* Author: Moshe Yifrah
*
* Compile with: gcc single_thread_sum_of_divs.c 
*
*********************************************************/

#include <stdio.h>   /* printf */

/*********************************************************/

int main()
{
	size_t i = 1;
	size_t number = 1000000000UL;
	size_t sum_of_divisors = 0;
	int round = 0, rounds = 10;

	for(round = 0; round < rounds; ++round)
	{
		for(i = 1; i <= number; ++i)
		{
			if (0 == number % i)
			{
				sum_of_divisors += i;
			}
		}
	}
	
	printf("Sum of divisors of %lu is: %lu\n", number, sum_of_divisors / round);
	
	return 0;
}


/*********************************************************/


