//#include <utility>
//#include <iostream>
//#include <fstream>
//#include <string>
//#include <vector>
//#include "timing.h"
//
//int N = 1 << 19;
//
//int main(int argc, char** argv)
//{
//	// argumenti
//	std::vector<int> numbers(N, 0);
//	for (int i = 0; i < N; i++)
//		numbers[i] = i;
//
//	Clock clock;
//	std::cout << "Start..." << std::endl;
//	clock.start();
//
//	bool is_prime;
//	int primes_in_task = 0;
//	for (int i = 0; i < N; i++)
//	{
//		if (i < 2)
//			continue;
//		is_prime = true;
//		// loop to check if n is prime
//		for (int j = 2; j <= i / 2; j++) {
//			if (i % j == 0) {
//				is_prime = false;
//				break;
//			}
//		}
//		if (is_prime)
//		{
//			primes_in_task++;
//		}
//	}
//
//	std::cout << "Broj prostih brojeva u intervalu (0, " << N << ") je: " << primes_in_task << std::endl;
//	std::cout << "Trajanje: " << clock.stop() << " s" << std::endl;
//
//	return 0;
//}
