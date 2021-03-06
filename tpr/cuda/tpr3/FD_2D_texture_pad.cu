/*** Calculating a derivative with CD ***/
#include "helper_timer.h"
#include <iostream>
#include <fstream>
#include <cmath>

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

float get_time() {
	static StopWatchInterface* timer = NULL;
	if (!timer) {
		sdkCreateTimer(&timer);
		timer->reset();
		timer->start();
	}
	return timer->getTime();
}



texture<float, 2> tex_u;
texture<float, 2> tex_u_prev;

// GPU kernels
__global__ void copy_kernel (float *u, float *u_prev, int N, int BSZ, int N_max)
{
	// Setting up indices
	int i = threadIdx.x;
	int j = threadIdx.y;
	int x = i + blockIdx.x*BSZ;
	int y = j + blockIdx.y*BSZ;
	int I = x + y*N_max;
	

	//if (I>=N*N){return;}	
	//if ((x>=N) || (y>=N)){return;}	
	float value = tex2D(tex_u, x, y);

	u_prev[I] = value;

}

__global__ void update (float *u, float *u_prev, int N, float h, float dt, float alpha, int BSZ, int N_max)
{
	// Setting up indices
	int i = threadIdx.x;
	int j = threadIdx.y;
	int x = i + blockIdx.x*BSZ;
	int y = j + blockIdx.y*BSZ;
	int I = x + y*N_max;
	
	//if (I>=N*N){return;}	
	//if ((x>=N) || (y>=N)){return;}	
	

	float t, b, r, l, c;
	c = tex2D(tex_u_prev, x, y);	
	t = tex2D(tex_u_prev, x, y+1);	
	b = tex2D(tex_u_prev, x, y-1);	
	r = tex2D(tex_u_prev, x+1, y);	
	l = tex2D(tex_u_prev, x-1, y);


	//if ( (I>N) && (I< N*N-1-N) && (I%N!=0) && (I%N!=N-1))
	if ( (x!=0) && (y!=0) && (x!=N-1) && (y!=N-1))
	{	u[I] = c + alpha*dt/h/h * (t + b + l + r - 4*c);	
	}
}

int main()
{
	// Allocate in CPU
	int tests[] = {32, 64, 96, 128, 192, 256, 384, 512};
	for(int N : tests) {
		int BLOCKSIZE = 16;	// 32. As I will be using BLOCKSIZE to be a multiple of 8
							// I'll just look for the closest multiple of BLOCKSIZE (N_max)

		int N_max = (int((N-0.5)/BLOCKSIZE) + 1) * BLOCKSIZE;

		float xmin 	= 0.0f;
		float xmax 	= 3.5f;
		float ymin 	= 0.0f;
		//float ymax 	= 2.0f;
		float h   	= (xmax-xmin)/(N-1);
		float dt	= 0.00001f;	
		float alpha	= 0.645f;
		float time 	= 0.4f;

		int steps = (int)ceil(time/dt);
		int I, J;

		float *x  	= new float[N*N]; 
		float *y  	= new float[N*N]; 
		float *u  	= new float[N_max*N_max];
		float *u_prev  	= new float[N*N];

		// Initialize
		for (int j=0; j<N_max; j++)
		{	for (int i=0; i<N_max; i++)
			{	I = N_max*j + i;
				u[I] = 0.0f;
				if ( ((i==0) || (j==0)) && (j<N) && (i<N)) 
					{u[I] = 200.0f;}
			}
		}	

		// Generate mesh and intial condition
		for (int j=0; j<N; j++)
		{	for (int i=0; i<N; i++)
			{	I = N*j + i;
				x[I] = xmin + h*i;
				y[I] = ymin + h*j;
			}
		}

		// Allocate in GPU
		float *u_d, *u_prev_d;
		double start = get_time();
		double kernel_start, kernel_stop;
		
		cudaMalloc( (void**) &u_d, N_max*N_max*sizeof(float));
		cudaMalloc( (void**) &u_prev_d, N_max*N_max*sizeof(float));

		// Bind textures
		cudaChannelFormatDesc desc = cudaCreateChannelDesc<float>();
		// Textures are aligned in memory. First NULL parameter is place where evetual offset would be placed
		cudaBindTexture2D(NULL, tex_u, u_d, desc, N_max, N_max, sizeof(float)*N_max);
		cudaBindTexture2D(NULL, tex_u_prev, u_prev_d, desc, N_max, N_max, sizeof(float)*N_max);

		// Copy to GPU
		cudaMemcpy(u_d, u, N_max*N_max*sizeof(float), cudaMemcpyHostToDevice);

		// Loop 
		dim3 dimGrid(int((N_max-0.5)/BLOCKSIZE)+1, int((N_max-0.5)/BLOCKSIZE)+1);
		dim3 dimBlock(BLOCKSIZE, BLOCKSIZE);
		kernel_start = get_time();
		for (int t=0; t<steps; t++)
		{	// The transfer of u to u_prev needs to be in separate kernel
			// as it's read only
			copy_kernel <<<dimGrid, dimBlock>>> (u_d, u_prev_d, N, BLOCKSIZE, N_max);
			update <<<dimGrid, dimBlock>>> (u_d, u_prev_d, N, h, dt, alpha, BLOCKSIZE, N_max);
		}
		kernel_stop = get_time();
		checkErrors("update");

		// Copy result back to host
		cudaMemcpy(u, u_d, N_max*N_max*sizeof(float), cudaMemcpyDeviceToHost);


		// Free device
		cudaUnbindTexture(tex_u);
		cudaUnbindTexture(tex_u_prev);
		cudaFree(u_d);
		cudaFree(u_prev_d);
		
		double stop = get_time();
		
		if (N < 10) continue;
		
		double elapsed = stop - start;
		double kernel_elapsed = kernel_stop - kernel_start;
		std::cout<<N<<" "<<elapsed<<" "<<kernel_elapsed<<" "<<elapsed-kernel_elapsed<<std::endl;
	}
}
