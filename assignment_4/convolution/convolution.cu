#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include "timer.h"

#define filter_height 5
#define filter_width 5

#define border_height ((filter_height/2)*2)
#define border_width ((filter_width/2)*2)

#define block_size_x 32
#define block_size_y 16

#define SEED 1234

using namespace std;

void convolutionSeq(float *output, float *input, float *filter,
                    long int image_height, long int image_width,
                    long int input_height, long int input_width) {
    timer sequentialTime = timer("Sequential");
    
    sequentialTime.start();

    // For each pixel in the output image
    for (int y=0; y < image_height; y++) {
        for (int x=0; x < image_width; x++) { 
            output[y*image_width+x]=0;
            
            // For each filter weight
            for (int i=0; i < filter_height; i++) {
                for (int j=0; j < filter_width; j++) {
                    output[y*image_width+x] += input[(y+i)*input_width+x+j] * filter[i*filter_width+j];
                }
            }

	        output[y*image_width+x] /= 35;
        }
    }
  
    sequentialTime.stop(); 
    cout << "convolution (sequential): \t\t" << sequentialTime << endl;

}

__global__ void convolution_kernel_naive(float *output, float *input, float *filter,
                                         long int image_height, long int image_width,
                                         long int input_height, long int input_width) {
    // TODO: Determine x and y based on Block ID and Grid ID.
    size_t y = blockIdx.y * blockDim.y + threadIdx.y;
    size_t x = blockIdx.x * blockDim.x + threadIdx.x;

    // For each filter weight
    for (size_t i=0; i < filter_height; i++) {
        for (size_t j=0; j < filter_width; j++) {
            output[y*image_width+x] += input[(y+i)*input_width+x+j] * filter[i*filter_width+j];
        }
    }
    
    output[y*image_width+x] /= 35;
}

void convolutionCUDA(float *output, float *input, float *filter,
                    long int image_height, long int image_width,
                    long int input_height, long int input_width) {
    float *d_input; float *d_output; float *d_filter;
    cudaError_t err;
    timer kernelTime = timer("kernelTime");
    timer memoryTime = timer("memoryTime");

    // memory allocation
    err = cudaMalloc((void **)&d_input, input_height*input_width*sizeof(float));
    if (err != cudaSuccess) { fprintf(stderr, "Error in cudaMalloc d_input: %s\n", cudaGetErrorString( err )); }
    err = cudaMalloc((void **)&d_output, image_height*image_width*sizeof(float));
    if (err != cudaSuccess) { fprintf(stderr, "Error in cudaMalloc d_output: %s\n", cudaGetErrorString( err )); }
    err = cudaMalloc((void **)&d_filter, filter_height*filter_width*sizeof(float));
    if (err != cudaSuccess) { fprintf(stderr, "Error in cudaMalloc d_filter: %s\n", cudaGetErrorString( err )); }

    memoryTime.start();
    // host to device 
    err = cudaMemcpy(d_input, input, input_height*input_width*sizeof(float), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) { fprintf(stderr, "Error in cudaMemcpy host to device input: %s\n", cudaGetErrorString( err ));  }
    err = cudaMemcpy(d_filter, filter, filter_height*filter_width*sizeof(float), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) { fprintf(stderr, "Error in cudaMemcpy host to device filter: %s\n", cudaGetErrorString( err ));  }
    
    // zero the result array 
    err = cudaMemset(d_output, 0, image_height*image_width*sizeof(float));
    if (err != cudaSuccess) { fprintf(stderr, "Error in cudaMemset output: %s\n", cudaGetErrorString( err ));  }
    memoryTime.stop();
    //setup the grid and thread blocks
    //thread block size
    dim3 threads(block_size_x, block_size_y);
    //problem size divided by thread block size rounded up
    dim3 grid(int(ceilf(image_width/(float)threads.x)), int(ceilf(image_height/(float)threads.y)) );

    //measure the GPU function
    kernelTime.start();
    convolution_kernel_naive<<<grid, threads>>>(d_output, d_input, d_filter,
                                                image_height, image_width,
                                                input_height, input_width);
    cudaDeviceSynchronize();
    kernelTime.stop();
 
    //check to see if all went well
    err = cudaGetLastError();
    if (err != cudaSuccess) { fprintf(stderr, "Error during kernel launch convolution_kernel: %s\n", cudaGetErrorString( err )); }

    //copy the result back to host memory
    memoryTime.start();
    err = cudaMemcpy(output, d_output, image_height*image_width*sizeof(float), cudaMemcpyDeviceToHost);
    memoryTime.stop();
    if (err != cudaSuccess) { fprintf(stderr, "Error in cudaMemcpy device to host output: %s\n", cudaGetErrorString( err )); }
 
    err = cudaFree(d_input);
    if (err != cudaSuccess) { fprintf(stderr, "Error in freeing d_input: %s\n", cudaGetErrorString( err )); }
    err = cudaFree(d_output);
    if (err != cudaSuccess) { fprintf(stderr, "Error in freeing d_output: %s\n", cudaGetErrorString( err )); }
    err = cudaFree(d_filter);
    if (err != cudaSuccess) { fprintf(stderr, "Error in freeing d_filter: %s\n", cudaGetErrorString( err )); }

    cout << "convolution (kernel): \t\t" << kernelTime << endl;
    cout << "convolution (memory): \t\t" << memoryTime << endl;

}

int compare_arrays(float *a1, float *a2, int n) {
    int errors = 0;
    int print = 0;

    for (int i=0; i<n; i++) {

        if (isnan(a1[i]) || isnan(a2[i])) {
            errors++;
            if (print < 10) {
                print++;
                fprintf(stderr, "Error NaN detected at i=%d,\t a1= %10.7e \t a2= \t %10.7e\n",i,a1[i],a2[i]);
            }
        }

        float diff = (a1[i]-a2[i])/a1[i];
        if (diff > 1e-6f) {
            errors++;
            if (print < 10) {
                print++;
                fprintf(stderr, "Error detected at i=%d, \t a1= \t %10.7e \t a2= \t %10.7e \t rel_error=\t %10.7e\n",i,a1[i],a2[i],diff);
            }
        }

    }

    return errors;
}

void die(const char *msg){
    if (errno != 0) 
        perror(msg);
    else
        fprintf(stderr, "error: %s\n", msg);
    exit(1);
}

static void readpgm_float(const char *fname,
                          long int height, long int width, float *input) {
    char format[3];
    FILE *f;
    unsigned imgw, imgh, maxv, v;
    size_t i;

    if (!(f = fopen(fname, "r"))) die("fopen");

    fscanf(f, "%2s", format);
    if (format[0] != 'P' || format[1] != '2') die("only ASCII PGM input is supported");

    if (fscanf(f, "%u", &imgw) != 1 ||
        fscanf(f, "%u", &imgh) != 1 ||
        fscanf(f, "%u", &maxv) != 1) die("invalid input");

    if (imgw != width || imgh != height) {
        fprintf(stderr, "input data size (%ux%u) does not match cylinder size (%zux%zu)\n",
                imgw, imgh, width, height);
        die("invalid input");
    }

    for (i = 0; i < width * height; ++i)
    {
        if (fscanf(f, "%u", &v) != 1) die("invalid data");
        input[i] = 0.0 + (float)v * 1.0 / maxv;
    }

    fclose(f);
}

int main(int argc, char *argv[]) {
    int c;
    int i; 
    int errors=0;

    const char *image_path = 0;
    image_path ="../../images/pat1_100x150.pgm";
    int gen_image = 0;

    long int image_height = 1024;
    long int image_width = 1024;

    /* Read command-line options. */
    while((c = getopt(argc, argv, "i:r:h:w")) != -1) {
        switch(c) {
            case 'i':
                image_path = optarg;
            	break;
            case 'r':
            	gen_image = 1;
            	break;
            case 'h':
                image_height = strtol(optarg, 0, 10);
            	break;
            case 'w':
                image_width = strtol(optarg, 0, 10);
				break;
            case '?':
                fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
                return -1;
            default:
                return -1;
        }
    }

    long int input_height = (image_height + border_height);
    long int input_width  = (image_width  + border_width);

    //allocate arrays and fill them
    float *input = (float *) malloc(input_height * input_width * sizeof(float));
    float *output1 = (float *) calloc(image_height * image_width, sizeof(float));
    float *output2 = (float *) calloc(image_height * image_width, sizeof(float));
    float *filter = (float *) malloc(filter_height * filter_width * sizeof(float));

    if (gen_image) {
        for (i=0; i< input_height * input_width; i++) {
            input[i] = (float) (i % SEED);
        }
    } else {
        readpgm_float(image_path, image_height, image_width, input);
    }

    //This is specific for a W==H smoothing filter, where W and H are odd.
    for (i=0; i<filter_height * filter_width; i++) { 
      filter[i] = 1.0;
    }

    for (i=filter_width+1; i<(filter_height - 1) * filter_width; i++) {
	    if (i % filter_width > 0 && i % filter_width < filter_width-1)
            filter[i]+=1.0; 
    }

    filter[filter_width*filter_height/2]=3.0;
    //end initialization
   
    //measure the CPU function
    convolutionSeq(output1, input, filter, image_height, image_width, input_height, input_width);

    //measure the GPU function
    convolutionCUDA(output2, input, filter, image_height, image_width, input_height, input_width);


    //check the result
    errors += compare_arrays(output1, output2, image_height*image_width);
    if (errors > 0) {
        printf("TEST FAILED! %d errors!\n", errors);
    } else {
        printf("TEST PASSED!\n");
    }

    free(filter);
    free(input);
    free(output1);
    free(output2);

    return 0;
}


