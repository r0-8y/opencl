#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <utility>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "timing.h"
using namespace cl;

constexpr auto PI25DT = 3.141592653589793238462643;;


char SOURCE_FILE[] = "./pi.cl";
char KERNEL_NAME[] = "calculate_pi";
int G = 1 << 11;
int L = 1 << 5;


int main(int argc, char** argv)
{
	double pi, h, x, serial_times[3]{}, time;
	unsigned int n;
	Clock clock;

	std::cout << "Jedna dretva:" << std::endl;

	for (int p = 27; p < 30; p++)
	{
		clock.start();

		n = static_cast<long>(1) << p;
		h = 1.0 / (double)n;
		pi = 0.0;

		for (int i = 0; i <= n; i++)
		{
			x = h * ((double)i - 0.5);
			pi += 4.0 / (1.0 + x * x);
		}

		pi *= h;

		time = clock.stop();
		serial_times[p - 27] = time;

		std::cout << "N: " << n << ", trajanje: " << time << "s, greska: " << fabs(pi - PI25DT) << std::endl;
	}

	// argumenti
	std::vector<double> results(G, 0);

	std::cout << G << " dretvi:" << std::endl;

	Program program;	// izdvojeno zbog catch(...)

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

		// Stvori buffer za rezultate
		Buffer result_buffer = Buffer(context, CL_MEM_WRITE_ONLY, G * sizeof(double));

		// Postavi argumente jezgrenih funkcija
		kernel.setArg(0, result_buffer);
		kernel.setArg(2, G);

		// Definiraj velicinu radnog prostora i radne grupe
		NDRange global(G, 1);	// ukupni broj dretvi
		NDRange local(L, 1);	// velicina radne grupe

		for (int p = 27; p < 30; p++)
		{
			clock.start();
			n = 1 << p;

			kernel.setArg(1, n);

			// Pokreni jezgrenu funkciju
			queue.enqueueNDRangeKernel(kernel, NullRange, global, local);

			queue.finish();

			// Procitaj rezultat
			queue.enqueueReadBuffer(result_buffer, CL_TRUE, 0, G * sizeof(double), &results[0]);

			double pi = 0.0;
			for (auto result : results)
				pi += result;

			time = clock.stop();

			std::cout << "N: " << n << ", trajanje: " << time << "s, greska: " << fabs(pi - PI25DT) << ", ubrzanje: " << serial_times[p-27] / time << std::endl;
		}
	}
	catch (Error error) {
		std::cout << error.what() << "(" << error.err() << ")" << std::endl;
		std::cout << "Build Status: " << program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(cl::Device::getDefault()) << std::endl;
		std::cout << "Build Options:\t" << program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(cl::Device::getDefault()) << std::endl;
		std::cout << "Build Log:\t " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(cl::Device::getDefault()) << std::endl;
	}

	return 0;
}
