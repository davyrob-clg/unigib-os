    #include <stdio.h>
    #include <time.h>
    #include <sys/time.h> // For gettimeofday

    int main() {
        struct timeval start_time, end_time;
        long long iterations = 100000000; // Number of loop iterations
	double result=0.0;
	long long i;

	while(1)
	{
        gettimeofday(&start_time, NULL); // Start timing

        for (long long i = 0; i < iterations; i++) {
            // Code to be benchmarked
            // Example: a simple arithmetic operation
            volatile int temp = i * 2; 
        }
	

	for (i = 0; i < 1000000000; i++) { // 1 billion iterations
		result += (double)i / 3.1415926535; // Example floating-point operation
		result *= 1.000000001; // Another operation
    	}

        gettimeofday(&end_time, NULL); // End timing

        // Calculate and print the duration
        double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + 
                              (double)(end_time.tv_usec - start_time.tv_usec) / 1000000.0;

        printf("UniGib Processors Tester: %lld iterations took %.6f seconds.\n", iterations, elapsed_time);
	}

        return 0;
    }
