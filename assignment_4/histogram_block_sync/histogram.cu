#include <stdio.h> 
#include <stdlib.h> 
#include <math.h> 
#include <string.h>
#include <iostream>
#include "timer.h"
#include <unistd.h>
#include <getopt.h>

#define BLOCK_SIZE 64

using namespace std;

/* Utility function, use to do error checking.

   Use this function like this:

   checkCudaCall(cudaMalloc((void **) &deviceRGB, imgS * sizeof(color_t)));

   And to check the result of a kernel invocation:

   checkCudaCall(cudaGetLastError());
*/
void die(const char *msg){
    if (errno != 0) 
        perror(msg);
    else
        fprintf(stderr, "error: %s\n", msg);
    exit(1);
}   

void generate_image(int num_rows, int num_cols, unsigned char * image){
    for (int i = 0; i < num_cols * num_rows; ++i)
    {
        image[i] = (unsigned char) (rand() % 256); //255 + 1 for num bins
    }
}

void read_image(const char * image_path, int num_rows, int num_cols, unsigned char * image){
	char format[3];
    FILE *f;
    unsigned imgw, imgh, maxv, v;
    size_t i;

	printf("Reading PGM data from %s...\n", image_path);

	if (!(f = fopen(image_path, "r"))) die("fopen");

	fscanf(f, "%2s", format);
    if (format[0] != 'P' || format[1] != '2') die("only ASCII PGM input is supported");
    
    if (fscanf(f, "%u", &imgw) != 1 ||
        fscanf(f, "%u", &imgh) != 1 ||
        fscanf(f, "%u", &maxv) != 1) die("invalid input");

    if (imgw != num_cols || imgh != num_rows) {
        fprintf(stderr, "input data size (%ux%u) does not match cylinder size (%zux%zu)\n",
                imgw, imgh, num_cols, num_rows);
        die("invalid input");
    }

    for (i = 0; i < num_cols * num_rows; ++i)
    {
        if (fscanf(f, "%u", &v) != 1) die("invalid data");
        image[i] = (unsigned char) (((int)v * 255) / maxv); //255 for num bins
    }
    fclose(f);
}

static void checkCudaCall(cudaError_t result) {
    if (result != cudaSuccess) {
        cerr << "cuda error: " << cudaGetErrorString(result) << endl;
        exit(1);
    }
}


inline __device__ void increment_private_histo(unsigned char pixel)
{
    extern __shared__ unsigned int s_histogram[];
    
    int index  = pixel >> 2;
    int offset = 8 * (pixel & 3);

    s_histogram[index * BLOCK_SIZE + threadIdx.x] += 1 << offset;
}

__device__ void merge_output(unsigned int *histogram)
{
    extern __shared__ unsigned int s_histogram[];

    unsigned int cnt02 = 0;
    unsigned int cnt13 = 0;
    
    int idx;
    unsigned int cnt;
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        idx = (i + threadIdx.x) & (BLOCK_SIZE - 1);
        cnt = s_histogram[threadIdx.x * BLOCK_SIZE + idx];

        cnt02 += cnt & 0xff00ff;
        cnt >>= 8;
        cnt13 += cnt & 0xff00ff;
    }

    unsigned int cnt0 = cnt02 & 0xffff;
    unsigned int cnt1 = cnt13 & 0xffff;
    unsigned int cnt2 = cnt02 >> 16;
    unsigned int cnt3 = cnt13 >> 16;
    
    atomicAdd(&histogram[threadIdx.x * 4 + 0], cnt0);
    atomicAdd(&histogram[threadIdx.x * 4 + 1], cnt1);
    atomicAdd(&histogram[threadIdx.x * 4 + 2], cnt2);
    atomicAdd(&histogram[threadIdx.x * 4 + 3], cnt3);
}

__global__ void histogramKernel(unsigned char* image, long img_size, unsigned int* histogram, int hist_size) 
{
    extern __shared__ unsigned int s_histogram[];

    int gid = blockIdx.x * blockDim.x + threadIdx.x;
    
    for (int i = threadIdx.x; i < BLOCK_SIZE * blockDim.x; i += blockDim.x ) 
    {
        s_histogram[i] = 0;
    }

    __syncthreads();
    
    for (int i = gid; i < img_size; i += blockDim.x * gridDim.x) 
    {
	    increment_private_histo(image[i]);
    }

    __syncthreads();
    
    merge_output(histogram);
}

void histogramCuda(unsigned char* image, long img_size, unsigned int* histogram, int hist_size) 
{
    // allocate the vectors on the GPU
    unsigned char* deviceImage = NULL;
    checkCudaCall(cudaMalloc((void **) &deviceImage, img_size * sizeof(unsigned char)));
    if (deviceImage == NULL) {
        cout << "could not allocate memory!" << endl;
        return;
    }
    unsigned int* deviceHisto = NULL;
    checkCudaCall(cudaMalloc((void **) &deviceHisto, hist_size * sizeof(unsigned int)));
    if (deviceHisto == NULL) {
        checkCudaCall(cudaFree(deviceImage));
        cout << "could not allocate memory!" << endl;
        return;
    }

    timer kernelTime1 = timer("kernelTime1");
    timer memoryTime = timer("memoryTime");

    // copy the original vectors to the GPU
    memoryTime.start();
    checkCudaCall(cudaMemcpy(deviceImage, image, img_size*sizeof(unsigned char), cudaMemcpyHostToDevice));
    memoryTime.stop();

    // execute kernel
    kernelTime1.start();
    histogramKernel<<<img_size / BLOCK_SIZE + 1, BLOCK_SIZE, BLOCK_SIZE * 256>>>(deviceImage, img_size, deviceHisto, hist_size);
    cudaDeviceSynchronize();
    kernelTime1.stop();

    // check whether the kernel invocation was successful
    checkCudaCall(cudaGetLastError());

    // copy result back
    memoryTime.start();
    checkCudaCall(cudaMemcpy(histogram, deviceHisto, hist_size * sizeof(unsigned int), cudaMemcpyDeviceToHost));
    memoryTime.stop();

    checkCudaCall(cudaFree(deviceImage));
    checkCudaCall(cudaFree(deviceHisto));

    cout << "histogram (kernel): \t\t" << kernelTime1  << endl;
    cout << "histogram (memory): \t\t" << memoryTime << endl;
}

void histogramSeq(unsigned char* image, long img_size, unsigned int* histogram, int hist_size) {
  int i; 

  timer sequentialTime = timer("Sequential");
  
  for (i=0; i<hist_size; i++) histogram[i]=0;

  sequentialTime.start();
  for (i=0; i<img_size; i++) {
	histogram[image[i]]++;
  }
  sequentialTime.stop();
  
  cout << "histogram (sequential): \t\t" << sequentialTime << endl;

}

int main(int argc, char* argv[]) {
    int c;
    int seed = 42;
    const char *image_path = 0;
    image_path ="../../images/pat1_100x150.pgm";
    int gen_image = 0;
    int debug = 0;

    int num_rows = 150;
    int num_cols = 100;

    /* Read command-line options. */
    while((c = getopt(argc, argv, "s:i:rp:n:m:g")) != -1) {
        switch(c) {
            case 's':
                seed = atoi(optarg);
                break;
            case 'i':
            	image_path = optarg;
            	break;
            case 'r':
            	gen_image = 1;
            	break;
            case 'n':
            	num_rows = strtol(optarg, 0, 10);
            	break;
            case 'm':
				num_cols = strtol(optarg, 0, 10);
				break;
			case 'g':
				debug = 1;
				break;
            case '?':
                fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
                return -1;
            default:
                return -1;
        }
    }

    int hist_size = 256;
    long img_size = num_rows*num_cols;

    unsigned char *image = (unsigned char *)malloc(img_size * sizeof(unsigned char)); 
    unsigned int *histogramS = (unsigned int *)malloc(hist_size * sizeof(unsigned int));     
    unsigned int *histogram = (unsigned int *)malloc(hist_size * sizeof(unsigned int));

    /* Seed such that we can always reproduce the same random vector */
    if (gen_image){
    	srand(seed);
    	generate_image(num_rows, num_cols, image);
    }else{
    	read_image(image_path,num_rows, num_cols, image);
    }

    histogramSeq(image, img_size, histogramS, hist_size);
    histogramCuda(image, img_size, histogram, hist_size);
    
    // verify the resuls
    for(int i=0; i<hist_size; i++) {
	  if (histogram[i]!=histogramS[i]) {
            cout << "error in results! Bin " << i << " is "<< histogram[i] << ", but should be " << histogramS[i] << endl; 
            exit(1);
        }
    }
    cout << "results OK!" << endl;
     
    free(image);
    free(histogram);
    free(histogramS);         
    
    return 0;
}
