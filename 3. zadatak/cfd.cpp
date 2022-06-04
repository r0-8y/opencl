#define __CL_ENABLE_EXCEPTIONS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "arraymalloc.h"
#include "boundary.h"
#include "jacobi.h"
#include "cfdio.h"
#include <CL/cl.hpp>
#include <iostream>
#include <fstream>
#include <utility>

using namespace cl;

char SOURCE_FILE[] = "./cfd.cl";
char KERNEL_NAME[] = "jacobistep";
int G = 1 << 5;
int L = 1 << 5;

int main(int argc, char **argv)
{
	int printfreq=1000; //output frequency
	double error = 0.0, bnorm;
	double tolerance=0.00125534; //tolerance for convergence. <=0 means do not check

	//main arrays
	double *psi;
	//temporary versions of main arrays
	double *psitmp;

	//command line arguments
	int scalefactor, numiter;

	//simulation sizes
	int bbase=10;
	int hbase=15;
	int wbase=5;
	int mbase=32;
	int nbase=32;

	int irrotational = 1, checkerr = 0;

	int m,n,b,h,w;
	int iter;
	int i,j;

	double tstart, tstop, ttot, titer;

	//do we stop because of tolerance?
	if (tolerance > 0) {checkerr=1;}

	//check command line parameters and parse them

	if (argc <3|| argc >4) {
		printf("Usage: cfd <scale> <numiter>\n");
		return 0;
	}

	scalefactor=atoi(argv[1]);
	numiter=atoi(argv[2]);

	if(!checkerr) {
		printf("Scale Factor = %i, iterations = %i\n",scalefactor, numiter);
	}
	else {
		printf("Scale Factor = %i, iterations = %i, tolerance= %g\n",scalefactor,numiter,tolerance);
	}

	printf("Irrotational flow\n");

	//Calculate b, h & w and m & n
	b = bbase*scalefactor;
	h = hbase*scalefactor;
	w = wbase*scalefactor;
	m = mbase*scalefactor;
	n = nbase*scalefactor;
	G = m;

	printf("Running CFD on %d x %d grid in parallel\n",m,n);

	//allocate arrays
	psi    = (double *) malloc((m+2)*(n+2)*sizeof(double));
	psitmp = (double *) malloc((m+2)*(n+2)*sizeof(double));

	//zero the psi array
	for (i=0;i<m+2;i++) {
		for(j=0;j<n+2;j++) {
			psi[i*(m+2)+j]=0.0;
		}
	}

	//set the psi boundary conditions
	boundarypsi(psi,m,n,b,h,w);

	//compute normalisation factor for error
	bnorm=0.0;

	for (i=0;i<m+2;i++) {
			for (j=0;j<n+2;j++) {
			bnorm += psi[i*(m+2)+j]*psi[i*(m+2)+j];
		}
	}
	bnorm=sqrt(bnorm);

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

		// Stvori buffere
		Buffer psi_buffer = Buffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, (m + 2) * (n + 2) * sizeof(double), psi);
		Buffer psitmp_buffer = Buffer(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, (m + 2) * (n + 2) * sizeof(double));

		// Postavi argumente jezgrenih funkcija
		kernel.setArg(0, psitmp_buffer);
		kernel.setArg(1, psi_buffer);
		kernel.setArg(2, m);
		kernel.setArg(3, n);

		// Definiraj velicinu radnog prostora i radne grupe
		NDRange global(G, 1);	// ukupni broj dretvi
		NDRange local(L, 1);	// velicina radne grupe

		//begin iterative Jacobi loop
		printf("\nStarting main loop...\n\n");
		tstart = gettime();


		for (iter = 1; iter <= numiter; iter++) {

			//calculate psi for next iteration
			// jacobistep(psitmp, psi, m, n);

			queue.enqueueWriteBuffer(psi_buffer, CL_TRUE, 0, (m + 2)* (n + 2) * sizeof(double), psi);

			// Pokreni jezgrenu funkciju
			queue.enqueueNDRangeKernel(kernel, NullRange, global, local);
			queue.finish();

			// Procitaj rezultatenqueue
			queue.enqueueReadBuffer(psitmp_buffer, CL_TRUE, 0, (m + 2) * (n + 2) * sizeof(double), psitmp);

			//calculate current error if required
			if (checkerr || iter == numiter) {
				error = deltasq(psitmp, psi, m, n);

				error = sqrt(error);
				error = error / bnorm;
			}

			// printf("%lf\n", error);

			//quit early if we have reached required tolerance
			if (checkerr) {
				if (error < tolerance) {
					printf("Converged on iteration %d\n", iter);
					break;
				}
			}

			//copy back
			for (i = 1; i <= m; i++) {
				for (j = 1; j <= n; j++) {
					psi[i * (m + 2) + j] = psitmp[i * (m + 2) + j];
				}
			}

			//print loop information
			if (iter % printfreq == 0) {
				if (!checkerr) {
					printf("Completed iteration %d\n", iter);
				}
				else {
					printf("Completed iteration %d, error = %g\n", iter, error);
				}
			}
		}	// iter

		if (iter > numiter) iter = numiter;

		tstop = gettime();

		ttot = tstop - tstart;
		titer = ttot / (double)iter;

		//print out some stats
		printf("\n... finished\n");
		printf("After %d iterations, the error is %g\n", iter, error);
		printf("Time for %d iterations was %g seconds\n", iter, ttot);
		printf("Each iteration took %g seconds\n", titer);

		//output results
		//writedatafiles(psi,m,n, scalefactor);
		//writeplotfile(m,n,scalefactor);

		//free un-needed arrays
		free(psi);
		free(psitmp);
		printf("... finished\n");
	}
	catch (Error error) {
		std::cout << error.what() << "(" << error.err() << ")" << std::endl;
		std::cout << "Build Status: " << program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(cl::Device::getDefault()) << std::endl;
		std::cout << "Build Options:\t" << program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(cl::Device::getDefault()) << std::endl;
		std::cout << "Build Log:\t " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(cl::Device::getDefault()) << std::endl;
	}

	return 0;
}
