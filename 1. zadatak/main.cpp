#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <utility>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "timing.h"
using namespace cl;


char SOURCE_FILE[] = "./prime.cl";
char KERNEL_NAME[] = "num_of_primes";
int N = 1 << 19;
int G = 1 << 17;
int L = 1<<5;


int main(int argc, char** argv)
{
	// argumenti
	std::vector<int> numbers(N, 0), results(G, 0);
	for (int i = 0; i < N; i++)
		numbers[i] = i;
	Clock clock;

	std::cout << "Jedna dretva:" << std::endl;
	clock.start();

	bool is_prime;
	int primes_in_task = 0;
	double serial_time;

	for (int i = 0; i < N; i++)
	{
		if (i < 2)
			continue;
		is_prime = true;
		// loop to check if n is prime
		for (int j = 2; j <= i / 2; j++) {
			if (i % j == 0) {
				is_prime = false;
				break;
			}
		}
		if (is_prime)
		{
			primes_in_task++;
		}
	}

	serial_time = clock.stop();

	std::cout << "Broj prostih brojeva u intervalu (0, " << N << ") je: " << primes_in_task << std::endl;
	std::cout << "Trajanje: " << serial_time << " s" << std::endl << std::endl;

	std::cout << G << " dretvi:" << std::endl;
	clock.start();

	Program program;	// izdvojeno zbog catch(...)
	double parallel_time;

	try {
		// Ucitaj tekst programa
		std::ifstream sourceFile(SOURCE_FILE);
		std::string sourceCode(
			std::istreambuf_iterator<char>(sourceFile),
			(std::istreambuf_iterator<char>()));
		Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));

		// Dostupne platforme
		std::vector<Platform> platforms;
		Platform::get(&platforms);

		// Odabir platforme i stvaranje konteksta
		cl_context_properties cps[3] =
		{ CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(), 0 };
		Context context(CL_DEVICE_TYPE_GPU, cps);

		// Popis OpenCL uredjaja
		std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

		// Stvori naredbeni red za prvi uredjaj
		CommandQueue queue = CommandQueue(context, devices[0]);

		// Stvori programski objekt
		program = Program(context, source);

		// Prevedi programski objekt za zadani uredjaj
		program.build(devices);

		// Stvori jezgrene funkcije
		Kernel kernel(program, KERNEL_NAME);

		// Stvori buffer za input
		Buffer number_buffer = Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, N * sizeof(int), &numbers[0]);
		// enqueueWriteBuffer nije potrebno ako smo maloprije stavili | CL_MEM_COPY_HOST_PTR:
		//queue.enqueueWriteBuffer(A, CL_TRUE, 0, N * sizeof(int), &a[0]);

		// Stvori buffer za rezultate
		Buffer result_buffer = Buffer(context, CL_MEM_WRITE_ONLY, G * sizeof(int));

		// Postavi argumente jezgrenih funkcija
		kernel.setArg(0, number_buffer);
		kernel.setArg(1, result_buffer);
		kernel.setArg(2, N);
		kernel.setArg(3, G);

		// Definiraj velicinu radnog prostora i radne grupe
		NDRange global(G, 1);	// ukupni broj dretvi
		NDRange local(L, 1);	// velicina radne grupe

		// Pokreni jezgrenu funkciju
		queue.enqueueNDRangeKernel(kernel, NullRange, global, local);

		queue.finish();

		// Procitaj rezultatenqueue
		queue.enqueueReadBuffer(result_buffer, CL_TRUE, 0, G * sizeof(int), &results[0]);

		int total = 0;
		for (auto result : results)
			total += result;

		parallel_time = clock.stop();

		std::cout << "Broj prostih brojeva u intervalu (0, " << N << ") je: " << total << std::endl;
		std::cout << "Trajanje: " << parallel_time << " s, ubrzanje: " << serial_time / parallel_time << std::endl;

	}
	catch (Error error) {
		std::cout << error.what() << "(" << error.err() << ")" << std::endl;
		std::cout << "Build Status: " << program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(cl::Device::getDefault()) << std::endl;
		std::cout << "Build Options:\t" << program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(cl::Device::getDefault()) << std::endl;
		std::cout << "Build Log:\t " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(cl::Device::getDefault()) << std::endl;
	}

	return 0;
}
