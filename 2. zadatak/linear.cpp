//#include <math.h>
//#include <stdio.h>
//#include "timing.h"
//#include <iostream>
//
//int main(int argc, char* argv[])
//{
//	int n;
//	double PI25DT = 3.141592653589793238462643;
//	double pi, h, x;
//
//	Clock clock;
//	std::cout << "Start..." << std::endl;
//	clock.start();
//
//	n = 1 << 29;
//	h = 1.0 / (double)n;
//	pi = 0.0;
//
//	for (int i = 1; i <= n; i++)
//	{
//		x = h * ((double)i - 0.5);
//		pi += 4.0 / (1.0 + x * x);
//	}
//
//	pi *= h;
//
//	printf("pi is approximately %.16f, Error is %.16f\n", pi, fabs(pi - PI25DT));
//	std::cout << "Trajanje: " << clock.stop() << " s" << std::endl;
//
//	return 0;
//}