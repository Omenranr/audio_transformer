/*
This file contains the AudioHandler class
Description:
	The AudioHandler class handles all audio related buffers and operations:
Variables:
	L: length of input;
	M: length of impulse to use;
	ML: closest next power of M + L;
	convType: temporal convolution or frequencial convolution;
	bufferBytes: Number of buffers;
	overlapBuffer: The variable containing overlap buffer;
	outBuffer: Result of the convolution to output;
	currentBuffer: Containing currently processed data;
	reBuffer: Contaning real part of the stft(inputBuffer);
	imBuffer: Contaning imaginary part of the stft(inputBuffer);
	reImpulse: Contaning real part of the stft(impulse);
	imImpulse: Contaning imaginary part of the stft(impulse);
Functions:
	initializeVectors: initializes the imImpulse and reImpulse buffers.
	readImpulse: reads the impulse data.
	conv_freq: calculates frequency convolution.
	conv_t: calculates temporal convolution.
	applyConv: applies specified convolution type in the arguments handler.
	overlapAdd: applies the overlapAdd operation.

*/

#include "RtAudio.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <somefunc.cpp>
#include <sys/resource.h>

typedef double DATA_VEC;

class AudioHandler {
	public:
		int L, M, ML;
		unsigned int bufferBytes;
		string convType = "f";
		DATA_VEC *impulse, *overlapBuffer, *currentBuffer, *outBuffer, *reBuffer, *imBuffer, *reImpulse, *imImpulse;
		AudioHandler(int L, int M, int ML, string convType, unsigned int bufferBytes, DATA_VEC *overlapBuffer, DATA_VEC *currentBuffer, DATA_VEC *outBuffer, DATA_VEC *reBuffer, 
			DATA_VEC *imBuffer, DATA_VEC *reImpulse, DATA_VEC *imImpulse) {
			this->L = L;
			this->M = M;
			this->ML = ML;
			this->convType = convType;
			this->bufferBytes = bufferBytes;
			this->overlapBuffer = overlapBuffer;
			this->outBuffer = outBuffer;
			this->currentBuffer = currentBuffer;
			this->reBuffer = reBuffer;
			this->imBuffer = imBuffer;
			this->reImpulse = reImpulse;
			this->imImpulse = imImpulse;
		} 
		void initializeVectors() {
			try {
				for(int i = 0; i < this->ML; i++) {
					this->imImpulse[i] = 0;
					if(i < this->M) {
						this->reImpulse[i] = this->impulse[i];
					}
					else {
						this->reImpulse[i] = 0;
					}
				}
				fftr(this->reImpulse, this->imImpulse, this->ML);				
			} catch (exception &e) {
				cerr << "Exception caught : " << e.what() << endl;
			}
		}
		void readImpulse(string filename) {

			DATA_VEC *buffer;
		    FILE *pFile;
     		unsigned long lSize;

		    //Reading impres data
		    pFile = fopen(filename.c_str(), "rb");
		    if (pFile == NULL) {
		    	fputs ("File error\n", stderr); 
		    	exit(1);
		    }
		    // obtain file size:
		    fseek(pFile, 0, SEEK_END);
		    lSize = ftell(pFile);
		    rewind(pFile);
		    // allocate memory to contain the whole file:
		    buffer = (DATA_VEC*) malloc (sizeof(DATA_VEC)*lSize);
		    if (buffer == NULL) {
		    	fputs ("Memory error", stderr); 
		    	exit (2);
		    }
		    // copy the file into the buffer:
		    if (fread(buffer, 1, lSize, pFile) != lSize) {fputs ("Reading error",stderr); exit (3);}
		    // the whole file is now loaded in the memory buffer.
		    cout << "\nsize of file : " << lSize << endl;
		    // terminate
		    this->impulse = buffer;
		    fclose(pFile);
		}
		void conv_freq(DATA_VEC* inbuff) {
		  //Frequency convolution
		  for(int i = 0 ; i < this->ML; i++){
		    this->imBuffer[i] = 0;
		    if(i < this->L) {
		      this->reBuffer[i] = inbuff[i];
		    }
		    else {
		      this->reBuffer[i] = 0;
		    }
		  }

		  fftr(this->reBuffer, this->imBuffer, this->ML);

		  for(int i = 0; i < this->ML; i++) {
		    this->reBuffer[i] = this->reBuffer[i]*this->reImpulse[i] - this->imBuffer[i]*this->imImpulse[i];
		    this->imBuffer[i] = this->imBuffer[i]*this->reImpulse[i] + this->reBuffer[i]*this->imImpulse[i];
		    //cout << this->reBuffer[i];
		  }
		  ifft(this->reBuffer, this->imBuffer, this->ML);
		  for(int i = 0; i < this->M + this->L; i++) {
		      this->currentBuffer[i] = this->reBuffer[i];
		  }
		}
		void conv_t(DATA_VEC* inbuff) {
			//Time convolution
			int L = this->L;
			int M = this->M;
			int kmin = 0, kmax = 0;
			double tmp = 0;
		  	for(int i = 0; i < (L+M-1); i++) {
		  		tmp = 0;
		  		if (i >= M) {
		  			kmin = i-M+1;
				} 
				else {
					kmin = 1;
				}
				if(i < L) {
					kmax = i;
				}
				else {
					kmax = L;
				}
				//sum of values from kmin to kmax
				for(int k = kmin; k <= kmax; k++) {
					tmp = tmp + inbuff[k]*this->impulse[i-k+1];
		      	}
		      	this->currentBuffer[i] = tmp;
			}
		}
		void applyConv(void *inputBuffer) {
			if(this->convType == "f") {
		  		this->conv_freq((DATA_VEC*) inputBuffer);
		  	} 
		  	else if(this->convType == "t") {
		  		this->conv_t((DATA_VEC*) inputBuffer);
		  	} 
		  	else {
		  		cout << "Conv type unrecognized" << endl;
		  		exit(1);
		  	}
		}
		void overlapAdd(void *outputBuffer) {
			for(int i = 0; i < this->M; i++) {
				this->currentBuffer[i] = this->currentBuffer[i] + this->overlapBuffer[i];
		  	}
		  	for(int i = this->L; i < this->M + this->L; i++) {
		  		this->overlapBuffer[i-this->L] = this->currentBuffer[i];
		  	}
		  	for(int i = 0; i < this->L; i++) {
		  		this->outBuffer[i] = this->currentBuffer[i];
		  	}

		  	//COPYING RESULT INTO OUTPUT_BUFFER
		  	memcpy(outputBuffer, this->outBuffer, this->bufferBytes);
		 }
};