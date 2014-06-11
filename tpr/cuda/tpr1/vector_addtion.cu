#include "helper_timer.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cuda.h>
#define out
#define null NULL;


__global__ void vector_add_kernel (int *a, int *b, int *c, int N) {
	int tid = blockIdx.x * blockDim.x + threadIdx.x;	
	if (tid < N) {
		c[tid] = a[tid] + b[tid];
	}
}

void gpu_vector_add(int *a, int *b, int *c, int N, 
		out float* kernel_time, out float* transfer_time) {

	int SIZE = N * sizeof(int);
	StopWatchInterface* transfer_timer = null;
	StopWatchInterface* kernel_timer = null;
	sdkCreateTimer(&transfer_timer);
	sdkCreateTimer(&kernel_timer);
	
	kernel_timer->reset();
	transfer_timer->reset();
	
	int *dev_a, *dev_b, *dev_c;

	transfer_timer->start();
	cudaMalloc((void**)&dev_a, SIZE);
	cudaMalloc((void**)&dev_b, SIZE);
	cudaMalloc((void**)&dev_c, SIZE);
           
	cudaMemcpy(dev_a, a, SIZE, cudaMemcpyHostToDevice);
	cudaMemcpy(dev_b, b, SIZE, cudaMemcpyHostToDevice);
	cudaMemcpy(dev_c, c, SIZE, cudaMemcpyHostToDevice);	
	
	
	const int BLOCK_SIZE = 1024;
	const int MAX_BLOCK_SIZE = 65536;
	int k = ceil(((double)N) / BLOCK_SIZE);
	
	
	dim3 thredsPerBlock(BLOCK_SIZE, 1, 1);
    dim3 numBlock(
			k <= MAX_BLOCK_SIZE ? k : MAX_BLOCK_SIZE, 
			ceil(((double)k) / MAX_BLOCK_SIZE),
			1); 
	transfer_timer->stop();
			
	kernel_timer->start();
	vector_add_kernel<<<numBlock, thredsPerBlock>>>(dev_a, dev_b, dev_c, N);
	cudaThreadSynchronize();
	kernel_timer->stop();
	
	transfer_timer->start();
	cudaMemcpy(c, dev_c, SIZE, cudaMemcpyDeviceToHost);
	
	cudaFree(dev_a);
	cudaFree(dev_b);
	cudaFree(dev_c);
	transfer_timer->stop();
	
	*kernel_time = kernel_timer->getTime();
	*transfer_time = transfer_timer->getTime();
	sdkDeleteTimer(&transfer_timer);
	sdkDeleteTimer(&kernel_timer);
}


void cpu_vector_add(int *a, int *b, int *c, int N, 
		out float* time) {
	StopWatchInterface* timer = null;
	sdkCreateTimer(&timer);
	timer->reset();
	timer->start();
	
	for ( int i = 0; i < N; ++i) {
		c[i] = a[i] + b[i];
	}
	timer->stop();
	*time = timer->getTime();
	sdkDeleteTimer(&timer);
}

bool compare(int *a, int *b, int N) {
	for (int i = 0; i < N; ++i) {
		if (a[i] != b[i]) {
			return a[i] - b[i];
		}
	}
	return 0;
}

int main(void) {
	const int TESTS = 100;
	int size;
	int *a, *b, *c_cpu, *c_gpu;
	float gpu_kernel, gpu_transfer, cpu;
	float t_gpu_kernel, t_gpu_transfer, t_cpu;
	
	int tests[] = {1, 
		2,		4,		8,		16,		32,		64,		128,	256,	512,	1024,
		2000,	4000,	8000,	16000,	32000,	64000,	128000,	256000,	512000,	1024000,
		2000000,4000000,8000000,16000000 };
	
	for (int k = 0; k < sizeof(tests) / sizeof (int); ++k) {
		size = tests[k];
	/*while (true) {
		std::cin >> size;
		
		if (size <= 0) {
			break;
		}*/
	
		a = new int[size];
		b = new int[size];
		c_cpu = new int[size];
		c_gpu = new int[size];
			
		for (int i = 0; i < size; ++i) {
			a[i] = i;
			b[i] = i * i;
		}

		gpu_kernel = gpu_transfer = cpu = 0.0f;
		for (int i = 0; i < TESTS; ++i) {
			gpu_vector_add(a, b, c_gpu, size, &t_gpu_kernel, &t_gpu_transfer);
			cpu_vector_add(a, b, c_cpu, size, &t_cpu);
			cpu += t_cpu;
			gpu_kernel += t_gpu_kernel;
			gpu_transfer += t_gpu_transfer;			
		}
		
		gpu_transfer /= TESTS;
		gpu_kernel /= TESTS;
		cpu /= TESTS;
		
		if (size == 1) continue;
		/*std::cout << (compare(c_cpu, c_gpu, size)? "  INVALID !!!: " : "valid: ") 
				<< "size: " << size
				<< " cpu_time: "	<< cpu << " [ms] gpu_kernel: " << gpu_kernel 
				<< "[ms] gpu_transfer: " << gpu_transfer << "[ms] gpu_all: "
				<< gpu_kernel + gpu_transfer << "[ms]\n";		*/
		std::cout << gpu_kernel << "\n";
	}
	
	return 0;
}