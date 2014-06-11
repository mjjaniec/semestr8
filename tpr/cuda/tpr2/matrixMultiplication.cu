// Matrix multiplication by parts
// Elements stored in row-major order

using namespace std;
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cuda.h>
#include <math.h>
#include <cuda_runtime.h>

#define BLOCK_SIZE 16

typedef struct
{	int width;
	int height;
	float *elements;
} Matrix;

// Forward declaration of matrix mult
__global__ void MatMulKernel (const Matrix, const Matrix, Matrix);

// Host code
void GpuMatMul(const Matrix A, const Matrix B, Matrix C)
{
	// Load matrices A and B to device memory
	Matrix d_A;
	d_A.width = A.width; d_A.height = A.height;
	size_t size = A.width * A.height * sizeof(float);
	cudaMalloc((void**) &d_A.elements, size);
	cudaMemcpy(d_A.elements, A.elements, size, cudaMemcpyHostToDevice);
	
	Matrix d_B;
	d_B.width = B.width; d_B.height = B.height;
	size = B.width * B.height * sizeof(float);
	cudaMalloc((void**) &d_B.elements, size);
	cudaMemcpy(d_B.elements, B.elements, size, cudaMemcpyHostToDevice);
	
	// allocate C in device
	Matrix d_C;
	d_C.width = C.width; d_C.height = C.height;
	size = d_C.width * d_C.height * sizeof(float);
	cudaMalloc((void**) &d_C.elements, size);
	
	// call kernel
    dim3 thredsPerBlock(BLOCK_SIZE, BLOCK_SIZE, 1); // threads per block?
    dim3 numBlock(ceil(((double)A.width)/BLOCK_SIZE), ceil(((double)A.width)/BLOCK_SIZE), 1); // number of blocks?
	MatMulKernel<<<numBlock, thredsPerBlock>>>(d_A, d_B, d_C);
	cudaThreadSynchronize();
	
	// copy C to host
	cudaMemcpy(C.elements, d_C.elements, size, cudaMemcpyDeviceToHost);
	
	// free device memory
	cudaFree(d_A.elements);
	cudaFree(d_B.elements);
	cudaFree(d_C.elements);
}

//matrix multiplication kernel
__global__ void MatMulKernel(Matrix A, Matrix B, Matrix C)
{
	// each thread computes one element of C and acumulates results to Cvalue 
	float Cvalue = 0; 
	int row = blockIdx.y * blockDim.y + threadIdx.y; 
	int col = blockIdx.x * blockDim.x + threadIdx.x;
	if ((row>=A.height) || (col>=B.width)){
		return;
	}
	for (int e=0; e<A.width; e++) {
		Cvalue += A.elements[row*A.width + e] * B.elements[e*B.width + col];
	}
	C.elements[row*C.width + col] = Cvalue;
}

void CpuMatMul(const Matrix A, const Matrix B, Matrix C)
{
	float sum;
	int N = A.width;
	for (int row = 0; row < N; ++row)
	{
		for (int column = 0; column < N; ++column) 
		{
			sum = 0.0f;
			for (int k = 0; k < N; ++k) {
				sum += A.elements[row * N + k] * B.elements[k * N + column];
			}			
			C.elements[row * N + column] = sum;
		}
	}
}

int main(int argc, char ** argv)
{	
	int tests[] = {452};
	for (int test = 0; test < sizeof(tests)/sizeof(int); ++ test)
	{
		int Width = tests[test];
		
		Matrix A;
		Matrix B;
		Matrix C;
		
		A.width = Width;
		B.width = Width;
		C.width = Width;
		
		A.height = Width;
		B.height = Width;
		C.height = Width;
		
		A.elements = new float[Width*Width];
		B.elements = new float[Width*Width];
		C.elements = new float[Width*Width];
		
		//fill matrices
		std::ifstream A_input;
		std::ifstream B_input;
		
		float a, b;
		for (int i = 0, max = Width * Width; i < max; ++i)
		{		
			A.elements[i] = rand();
			B.elements[i] = rand();
		}
		A_input.close();
		B_input.close();
		
		const int N = 100;
		float gpuTime, cpuTime;
		cudaEvent_t start, stop;
		
		cudaEventCreate(&start);
		cudaEventCreate(&stop);
		cudaEventRecord(start, 0);

		for (int i = 0; i < N; ++i) 
		{
			GpuMatMul(A, B, C);		
		}
		
		cudaEventRecord(stop, 0);
		cudaEventSynchronize(stop);
		cudaEventElapsedTime(&gpuTime, start, stop);
		gpuTime /= N;
		
		
		cudaEventCreate(&start);
		cudaEventCreate(&stop);
		cudaEventRecord(start, 0);
		
		/*for (int i = 0; i < N; ++i)
		{
			CpuMatMul(A, B, C);
		}*/
		
		cudaEventRecord(stop, 0);
		cudaEventSynchronize(stop);
		cudaEventElapsedTime(&cpuTime, start, stop);
		cpuTime /= N;
		
		/*
		CpuMatMul(A, B, C);
		std::ofstream C_output;
		C_output.open("C.txt");
		for (int i=0; i<Width; i++)
		{	for (int j=0; j<Width; j++)
				C_output<<C.elements[i*Width+j]<<"\t";
			C_output<<endl;
		}
		*/
		
		cout << "matrixSize: " << Width << " gpuTime: " << gpuTime << "[ms] cpuTime: " << cpuTime << "[ms]\n";
	}
}
