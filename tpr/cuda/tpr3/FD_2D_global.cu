/*** Calculating a derivative with CD ***/
#include "helper_timer.h"
#include <iostream>
#include <fstream>
#include <cmath>

float get_time() {
	static StopWatchInterface* timer = NULL;
	if (!timer) {
		sdkCreateTimer(&timer);
		timer->reset();
		timer->start();
	}
	return timer->getTime();
}

void checkErrors(char *label)
{
	// we need to synchronise first to catch errors due to
	// asynchroneous operations that would otherwise
	// potentially go unnoticed
	cudaError_t err;
	err = cudaThreadSynchronize();
	if (err != cudaSuccess)
	{
		char *e = (char*) cudaGetErrorString(err);
		fprintf(stderr, "CUDA Error: %s (at %s)\n", e, label);
	}
	err = cudaGetLastError();
	if (err != cudaSuccess)
	{
		char *e = (char*) cudaGetErrorString(err);
		fprintf(stderr, "CUDA Error: %s (at %s)\n", e, label);
	}
}

__global__ void copy_array(float *u, float *u_prev, int N, int BSZ)
{
	int i = threadIdx.x;
	int j = threadIdx.y;
	int I = blockIdx.y*BSZ*N + blockIdx.x*BSZ + j*N + i;
	if (I>=N*N){return;}	
	u_prev[I] = u[I];
}

// GPU kernel
__global__ void update (float *u, float *u_prev, int N, float h, float dt, float alpha, int BSZ)
{
	// Setting up indices
	int i = threadIdx.x;
	int j = threadIdx.y;
	int I = blockIdx.y*BSZ*N + blockIdx.x*BSZ + j*N + i;
	
	if (I>=N*N)
	{
		return;
	}	
	//if (()>=N || j>){return;}	

	
	// if not boundary do
	if ( (I>N) && (I< N*N-1-N) && (I%N!=0) && (I%N!=N-1)) 
	{	
		u[I] = u_prev[I] + alpha*dt/(h*h) * (u_prev[I+1] + u_prev[I-1] + u_prev[I+N] + u_prev[I-N] - 4*u_prev[I]);
	}
	
	// Boundary conditions are automatically imposed
	// as we don't touch boundaries
}

int main()
{
	int tests[] = {4, 32, 64, 96, 128, 192, 256, 384, 512};
	std::cout<<"size overall kernel transport\n";
	for(int N : tests) {
		// Allocate in CPU
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

		// Allocate in GPU
		float *u_d, *u_prev_d;
		float kernel_start, kernel_stop;
		
		double start = get_time();
		cudaMalloc( (void**) &u_d, N*N*sizeof(float));
		cudaMalloc( (void**) &u_prev_d, N*N*sizeof(float));

		// Copy to GPU
		cudaMemcpy(u_d, u, N*N*sizeof(float), cudaMemcpyHostToDevice);

		// Loop 
		dim3 dimGrid(int((N-0.5)/BLOCKSIZE)+1, int((N-0.5)/BLOCKSIZE)+1);
		dim3 dimBlock(BLOCKSIZE, BLOCKSIZE);
		kernel_start = get_time();
		for (int t=0; t<steps; t++)
		{	
			copy_array <<<dimGrid, dimBlock>>> (u_d, u_prev_d, N, BLOCKSIZE);
			update <<<dimGrid, dimBlock>>> (u_d, u_prev_d, N, h, dt, alpha, BLOCKSIZE);
		}
		kernel_stop = get_time();
		checkErrors("update");
		
				// Free device
		
		if (N < 10) continue;
		// Copy result back to host
		cudaMemcpy(u, u_d, N*N*sizeof(float), cudaMemcpyDeviceToHost);
		cudaFree(u_d);
		cudaFree(u_prev_d);
		
		double stop = get_time();
		
		
		double elapsed = stop - start;
		double kernel_elapsed = kernel_stop - kernel_start;
		std::cout<<N<<" "<<elapsed<<" "<<kernel_elapsed<<" "<<elapsed-kernel_elapsed<<std::endl;
	}
}
