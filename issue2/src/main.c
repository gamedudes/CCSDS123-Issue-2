#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include "cliparser/include/cli.h"
#include "predictor/include/predictor.h"
#include "encoder/include/encoder.h"
#include "utils/include/utilities.h"
#include <sys/time.h>

int main(int argc, char **argv) {
	double start;
	double computeTime=0, predictionTime = 0, encoderTime = 0;
	double readTime=0;
  	double writeTime=0;

	/* 
		Parse Arguments 
	*/ 
	struct arguments parameters;
	parseArguments(argc, argv, &parameters);
	/*
		Mallocs related to compressions
	*/
	uint32_t * sample = malloc(parameters.xSize*parameters.ySize*parameters.zSize*sizeof(uint32_t));
	uint32_t * sampleRep = malloc(parameters.xSize*parameters.ySize*parameters.zSize*sizeof(uint32_t));
	//Predictor
	int32_t * weights = malloc((parameters.mode != REDUCED ? parameters.precedingBands+3 : parameters.precedingBands) * sizeof(int32_t ));
	int32_t * diffVector = malloc((parameters.mode != REDUCED ? parameters.precedingBands+3 : parameters.precedingBands) * sizeof(int32_t ));
	// Encoder
    uint16_t * counter = malloc(sizeof(uint16_t)*parameters.zSize);
    uint64_t * accumulator = malloc(sizeof(uint64_t)*parameters.zSize);

	/* 
		Mallocs for decompression
	*/
	uint32_t * decompressedSamples = malloc(parameters.xSize*parameters.ySize*parameters.zSize*sizeof(uint32_t));

	/* 
		Files to open for writing/reading
	 */
	FILE * residuals_file = NULL;
	FILE * deltafile = NULL;
	if (parameters.debugMode)
	{
		deltafile = fopen("Encoded.bin.delta", "wb");
	}
	FILE * decompressedFile = NULL;
	if(parameters.compressionMode == DECOMPRESS) {
		decompressedFile = fopen("DecompressedFile", "wb");
		residuals_file = fopen("Encoded.bin", "rb"); // Uncertain if rb/wb changes anything, but it seems to fix the problem from creating a new file.
	} else {
		residuals_file = fopen("Encoded.bin", "wb");
	}
	// Read from file into sample
	if(parameters.compressionMode == COMPRESS) {
		readIntSamples(&parameters, parameters.fileName, sample);
	}
	printf("Computing \n");
	/* 
		Begin Compression
	 */
	unsigned int numWrittenBits = 0;
	unsigned int totalWrittenBytes = 0;
	start = walltime();
	if (parameters.compressionMode == COMPRESS) {
		if(parameters.encodeOrder == BSQ) {
			for (uint16_t z = 0; z < parameters.zSize; z++) {
				for (uint16_t y = 0; y < parameters.ySize; y++) {
					for (uint16_t x = 0; x < parameters.xSize; x++) {
						uint32_t tempResidual = predict(sample, x, y, z, &parameters, sampleRep, diffVector, weights, 
						0, 0, 0, 0, 0);
						if(parameters.debugMode) {
							fwrite(&tempResidual, 4, 1, deltafile);
						}
						//Currently only BSQ encoding mode
						encodeSampleAdaptive(tempResidual, counter, accumulator, x, y, z, &totalWrittenBytes, &numWrittenBits, residuals_file, &parameters);
					}
				}
			}
		} 
		else if (parameters.encodeOrder == BIP) {
			for (uint16_t y = 0; y < parameters.ySize; y++) {
				for (uint16_t x = 0; x < parameters.xSize; x++) {
					for (uint16_t z = 0; z < parameters.zSize; z++) {
						uint32_t tempResidual = predict(sample, x, y, z, &parameters, sampleRep, diffVector, weights, 0, 0, 0, 0, 0);
						// Currently only BSQ encoding mode
						fwrite((&tempResidual), 4, 1, deltafile);
						encodeSampleAdaptive(tempResidual, counter, accumulator, x, y, z, &totalWrittenBytes, &numWrittenBits, residuals_file, &parameters);
					}
				}
			}
		} else if (parameters.encodeOrder == BIL) {
			for (uint16_t y = 0; y < parameters.ySize; y++)	{
				for (uint16_t z = 0; z < parameters.zSize; z++)	{
					for (uint16_t x = 0; x < parameters.xSize; x++)	{
						uint32_t tempResidual = predict(sample, x, y, z, &parameters, sampleRep, diffVector, weights, 0, 0, 0, 0, 0);
						// Currently only BSQ encoding mode
						fwrite((&tempResidual), 4, 1, deltafile);
						encodeSampleAdaptive(tempResidual, counter, accumulator, x, y, z, &totalWrittenBytes, &numWrittenBits, residuals_file, &parameters);
					}
				}
			}
		}
	} else {
		if(parameters.encodeOrder == BSQ) {
			for (uint16_t z = 0; z < parameters.zSize; z++) {
				for (uint16_t y = 0; y < parameters.ySize; y++) {
					for (uint16_t x = 0; x < parameters.xSize; x++) {
						uint32_t tempResidual = decodeSampleAdaptive(counter, accumulator, x, y, z, residuals_file, &parameters);
						decompressedSamples[offset(x,y,z,&parameters)] = unPredict(tempResidual, decompressedSamples, x, y, z, &parameters, diffVector, weights, 0, 0, 0, 0, 0);
						fwrite(&decompressedSamples[offset(x,y,z,&parameters)], 2, 1, decompressedFile);
					}
				}
			}
		} 
	}

	computeTime += walltime() - start;

	printf("\n");
	printf("Compute time:          %4.6f s\n",computeTime);
	printf("Average time per pixel(compute time/total pixels):          %7.4f ms\n",(computeTime*1e3)/(double)(parameters.xSize*parameters.ySize*parameters.zSize));
	printf("Average time per frame(compute time/z pixels):          %7.4f ms\n",(computeTime*1e3)/(double)(parameters.zSize));

	/*
		END OF COMPUTATION/WRITING
	*/

	fclose(residuals_file);
	if(parameters.debugMode) {
		fclose(deltafile);
	}

	// Free up stuff
	free(accumulator);
	free(counter);
	free(sampleRep);
	free(sample);
 	free(weights);
	free(diffVector);

	return 0;
}