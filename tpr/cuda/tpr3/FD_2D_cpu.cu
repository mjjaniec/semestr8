/*** Calculating a derivative with CD ***/
#include "helper_timer.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <omp.h>

float get_time() {
	static StopWatchInterface* timer = NULL;
	if (!timer) {
		sdkCreateTimer(&timer);
		timer->reset();
		timer->start();
	}
	return timer->getTime();
}


void update (float *u, float *u_prev, int N, float h, float dt, float alpha, int BSZ)
{
	int MAX = N*N-1-N;
	#pragma omp parallel for
	for(int I = N+1; I < MAX; ++I) {
		if ( (I%N!=0) && (I%N!=N-1)) {	
			u[I] = u_prev[I] + alpha*dt/(h*h) * (u_prev[I+1] + u_prev[I-1] + u_prev[I+N] + u_prev[I-N] - 4*u_prev[I]);
		}
	}
	
	// Boundary conditions are automatically imposed
	// as we don't touch boundaries
}

int main()
{
	omp_set_num_threads(4);
	// Allocate in CPU
	int tests[] = {32, 64, 96, 128, 192, 256};
	for(int N : tests) {
		int BLOCKSIZE = 16;

		float xmin 	= 0.0f;
		float xmax 	= 3.5f;
		float ymin 	= 0.0f;
		//float ymax 	= 2.0f;
		float h   	= (xmax-xmin)/(N-1);
		float dt	= 0.00001f;	
		float alpha	= 0.645f;
		float time 	= 0.4f;

		int steps = ceil(time/dt);
		int I;

		float *x  	= new float[N*N]; 
		float *y  	= new float[N*N]; 
		float *u  	= new float[N*N];
		float *u_prev	= new float[N*N];


		// Generate mesh and intial condition
		for (int j=0; j<N; j++)
		{	
			for (int i=0; i<N; i++)
			{	
				I = N*j + i;
				x[I] = xmin + h*i;
				y[I] = ymin + h*j;
				u[I] = 0.0f;
				if ( (i==0) || (j==0)) 
				{
					u[I] = 200.0f;
				}
			}
		}

		
		double start = get_time();
		for (int t=0; t<steps; t++)
		{	
			memcpy(u_prev, u, N * sizeof(float));
			update(u, u_prev, N, h, dt, alpha, BLOCKSIZE);
		}
		double stop = get_time();
		
		double elapsed = stop - start;
		std::cout<<N<<" "<<elapsed<<std::endl;

	}
	
}
