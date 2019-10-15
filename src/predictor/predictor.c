#include "include/predictor.h"
#include "localdiffrences/include/localdiffrences.h"
#include "utils/include/utilities.h"
#include "localsum/include/localsum.h"
#include "weights/include/weights.h"

#include "../cliparser/include/cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void predict(struct arguments * parameters, int * inputSample, unsigned short int * residuals) {
	unsigned int sMin = 0;
    unsigned int sMax = (0x1 << parameters->dynamicRange) - 1;
    unsigned int sMid = 0x1 << (parameters->dynamicRange - 1);
	int maxmimumError = 0; // Zero means lossless
	int sampleDamping = 0;
	int sampleOffset = 0;
	
	// Init Stuff
	int * localsum = (int*) calloc(parameters->xSize*parameters->ySize*parameters->zSize, sizeof(int));
	if(localsum == NULL) {
        fprintf(stderr, "Error in allocating the localsum\n");
        exit(EXIT_FAILURE);
    }
	
	int * sampleRep = (int*) malloc(parameters->xSize*parameters->ySize*parameters->zSize*sizeof(int));

	int ** weights = (int **) malloc((parameters->mode != REDUCED ? 4 : 1) * sizeof(int *));
    for (int i=0; i<(parameters->mode != REDUCED ? 4 : 1); i++) {
         weights[i] = (int *)calloc((i == 4 ? parameters->precedingBands : 1), sizeof(int)); 
    }
	int ** diffVector = (int **) malloc((parameters->mode != REDUCED ? 4 : 1) * sizeof(int *));
    for (int i=0; i<(parameters->mode != REDUCED ? 4 : 1); i++) { 
         diffVector[i] = (int *)calloc((i == 4 ? parameters->precedingBands : 1), sizeof(int)); 
    }

    if(weights == NULL){
        fprintf(stderr, "Error in allocating the weights\n");
        exit(EXIT_FAILURE);
    }

	int clippedBin = 0;
	long long doubleResPredSample = 0;
	long long highResSample = 0;
	int predictedSample = 0;
	int quantizedValue = 0;

	// Compute predictions
	for (int z = 0; z < parameters->zSize; z++)
	{
		initWeights(weights, z, parameters);
		for (int y = 0; y < parameters->ySize; y++)
		{
			for (int x = 0; x < parameters->xSize; x++)
			{
				printf("At X: %d, Y: %d, Z: %d, \n",x,y,z);
				quantizedValue = quantization(inputSample, predictedSample, maxmimumError, x, y, z, parameters);
				/* 
					Quantization and turning samples into sample represantation stage
				 */
				printf("Quantization value is %d \n", quantizedValue);

				sampleRep[offset(x,y,z, parameters)] = sampleRepresentation(inputSample, &clippedBin, predictedSample, quantizedValue, maxmimumError, highResSample, sampleDamping, sampleOffset, x, y, z, sMin, sMax, parameters);
				printf("sample rep is %d \n", sampleRep[offset(x,y,z, parameters)]);
				/* 
					Local Sum Calculations
				 */
				
				wideNeighborLocalSum(sampleRep,localsum,x,y,z,parameters);
				
				BuildDiffVector(sampleRep,localsum,diffVector,x,y,z,parameters);

				highResSample = computeHighResPredSample(localsum, weights, diffVector, sMid, sMin, sMax, x, y, z, parameters);
				printf("High resolution Sample is %d \n", highResSample);
				predictedSample = computePredictedSample(sampleRep, &doubleResPredSample, localsum, weights, diffVector, highResSample, sMid, sMin, sMax, x, y, z, parameters);
				printf("predicted value is %d \n", predictedSample);
				
				int doubleResError = (2 * clippedBin) - doubleResPredSample; 
				updateWeightVector(weights, diffVector, doubleResError, x, y, z, parameters);
				printf("---------------------------\n");

			}
		}
	}
	printf("Samples \n");
	printArray(inputSample, parameters);
	printf("Sample representation \n");
	printArray(sampleRep, parameters);
	printf("Local Sum\n");
	printArray(localsum, parameters);

	// Free up stuff
	free(weights);
	free(diffVector);
	free(localsum);
}

int quantization(int * sample, int predictedSample, int maximumError, int x, int y, int z, struct arguments * parameters) {
	int predictionResidual = sample[offset(x,y,z,parameters)] - predictedSample;
	if (x == 0 && y == 0)
	{
		return predictionResidual;
	} else {
		return sgn(predictionResidual) * floor((predictionResidual+maximumError)/((maximumError << 1) + 1));
	}
}

int computePredictedSample(int * sample, long long * doubleResPredSample, int * localSum, int ** weightVector, int ** diffVector, long long highResPredSample, int smid, int smin, int smax, int x, int y, int z, struct arguments * parameters) {
	*doubleResPredSample = highResPredSample;
	if(x > 0 || y > 0) {
		if (z > 0) {
			*doubleResPredSample = (*doubleResPredSample) >> (parameters->weightResolution+1);
		}
	} else if (x == 0 && y == 0 && parameters->precedingBands > 0 && z > 0) {
		*doubleResPredSample = 2 * sample[offset(x,y,z-1,parameters)];
	} else if (x == 0 && y == 0 && (parameters->precedingBands == 0 || z == 0)) {
		*doubleResPredSample = 2 * smid;
	} else {
		printf("computePredicted Sample should not arrive here\n");
		exit(EXIT_FAILURE);
	}
	return (int) (*doubleResPredSample) >> 1;
}

int sampleRepresentation(int * sample, int * clippedBinCenter, int predictedSample, int quantizedSample, int maximumError, int highResPredSample, int sampleDamping, int sampleOffset, int x, int y, int z, int smin, int smax, struct arguments * parameters) {
    if( x == 0 && y == 0) {
        return sample[offset(x,y,z,parameters)];
    } else {
        long long doubleResSample = 0;
        *clippedBinCenter = clip(predictedSample + (quantizedSample*((maximumError << 1) + 1)), smin, smax);
        doubleResSample = (4 * ((0x1 << parameters->theta) - sampleDamping)) * ((*clippedBinCenter << parameters->weightResolution) - ((sgn(quantizedSample) * maximumError * sampleOffset) << (parameters->weightResolution - parameters->theta)));
        doubleResSample += (sampleDamping * highResPredSample - (sampleDamping << (parameters->weightResolution + 1)));
        doubleResSample = doubleResSample >> (parameters->weightResolution + parameters->theta + 1);
		doubleResSample = (doubleResSample + 1) >> 1;
        return (int)doubleResSample;
    }
}

long long computeHighResPredSample(int * localSum, int ** weightVector, int ** diffVector, int smid, int smin, int smax, int x, int y, int z, struct arguments * parameters) {
	long long diffPredicted = 0;
	long long predictedSample = 0;
	diffPredicted = innerProduct(weightVector, diffVector, z, parameters);
	predictedSample = modR(diffPredicted + ((localSum[offset(x,y,z, parameters)] - 4 * smid) << parameters->weightResolution), parameters->registerSize);
	predictedSample += (smid << (parameters->weightResolution + 2));
	predictedSample += (0x1 << (parameters->weightResolution + 1));
	predictedSample = clip(predictedSample, (smin << (parameters->weightResolution+2)), (smax << (parameters->weightResolution+2) + (0x1 << parameters->weightResolution+1)  ));
	return predictedSample;
}

long long innerProduct(int ** weightVector, int ** diffVector, int z, struct arguments * parameters) {
	long long diffPredicted = 0;
	int currentPredBands = z < parameters->precedingBands ? z : parameters->precedingBands;
	if (z > 0) {
		for(int i = 0; i < currentPredBands; i++) {
			diffPredicted += diffVector[0][i] * weightVector[0][i];
		}
	}

	if (parameters->mode == FULL) {
		for (int i = 0; i < 3; i++) {
			diffPredicted += diffVector[i][0] * weightVector[i][0];
		}
	}
	return diffPredicted;
}