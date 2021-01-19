/******************************************/
/*
  duplex.cpp
  by Gary P. Scavone, 2006-2007.

  This program opens a duplex stream and passes
  input directly through to the output.
*/
/******************************************/

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
#include <AudioData.cpp>

using namespace std;

#define FORMAT RTAUDIO_FLOAT64

int logger = 0;
string convType;


typedef struct
{
  int L; /* length of the input buffer */
  int M; /* length of the impulse audio_data */
  int ML;
  DATA_VEC* impulse;
  unsigned int bufferBytes;
  DATA_VEC* overlapBuffer;   //overlap part size = M
  DATA_VEC* currentBuffer;   // buffer after convolution size L+M-1
  DATA_VEC* outBuffer;  //output with size = L
  DATA_VEC* reBuffer;   // for conv_freq real part of buffer
  DATA_VEC* imBuffer;   // for conv_freq imaginary part of buffer
  DATA_VEC* reImpulse; // reel part of impulse audio_data
  DATA_VEC* imImpulse; // im part of impulse audio_data
} AudioData;

void conv_freq(AudioData* audio_data, DATA_VEC* inbuff);
void conv_t(AudioData* audio_data, DATA_VEC* inbuff);

int inout( void *outputBuffer, void *inputBuffer, unsigned int /*nBufferFrames*/, double /*streamTime*/, RtAudioStreamStatus status, void *data ) {
  
  double t1 = 1000*get_process_time();

  if (status) cout << "Stream over/underflow detected." << endl;

  //Choose to perform temporal or frequential conv
  AudioData *currAudioData = (AudioData*) data;
  if(convType == "f") {
  	conv_freq(currAudioData, (DATA_VEC*)inputBuffer);
  } else if(convType == "t") {
  	conv_t(currAudioData, (DATA_VEC*)inputBuffer);
  } else {
  	cout << "Conv type unrecognized" << endl;
  	exit(1);
  }

  // overlap add
  for(int i = 0; i < currAudioData->M; i++) {
  	currAudioData->currentBuffer[i] = currAudioData->currentBuffer[i] + currAudioData->overlapBuffer[i];
  }

  for(int i = currAudioData->L; i < currAudioData->M + currAudioData->L; i++) {
  	currAudioData->overlapBuffer[i-currAudioData->L] = currAudioData->currentBuffer[i];
  }

  for(int i = 0; i < currAudioData->L; i++) {
  	currAudioData->outBuffer[i] = currAudioData->currentBuffer[i];
  }
  memcpy(outputBuffer, currAudioData->outBuffer, currAudioData->bufferBytes);
  /*
  int j = 0;
  for (int k = 0; k < 2 * currAudioData->bufferBytes-1; k = k+2){
  	*outputBuffer[k] = currAudioData->outBuffer[j];
  	*outputBuffer[k+1] = currAudioData->outBuffer[j];
  	j++;
  }
  */
  double t2 = 1000*get_process_time();
  if(logger % 100 == 0)
  	cout << "process time: " << t2-t1 << " ms" << endl;
  logger++;

  return 0;
}

int main(int argc, char *argv[]) {
  unsigned int inputChannels, fs, bufferBytes, oDevice = 0, iDevice = 0, iOffset = 0, oOffset = 0, outputChannels = 1;
  unsigned int bufferFrames = 512; // control the size of the input buffer
  AudioData audio_data;
  //File & Data variables
  FILE *pFile;
  DATA_VEC* buffer;
  unsigned long lSize;
  size_t result;

  //Reading parameters
  string channelsArg = getCmdOption(argc, argv, "-ic=");
  string sampleRateArg = getCmdOption(argc, argv, "-fs=");
  string iDeviceArg = getCmdOption(argc, argv, "-input=");
  string oDeviceArg = getCmdOption(argc, argv, "-output=");
  string convTypeArg = getCmdOption(argc, argv, "-conv=");
  string oChannelsArg = getCmdOption(argc, argv, "-oc=");
  string bufferFramesArg = getCmdOption(argc, argv, "-bs=");

  if(!channelsArg.empty()) {
  	inputChannels = (unsigned int) atoi(channelsArg.c_str());
  } else {
  	cout << "No input channels specified error" << endl;
  	exit(1);
  }
  if(!oChannelsArg.empty()) {
  	outputChannels = (unsigned int) atoi(channelsArg.c_str());
  }
  if(!sampleRateArg.empty()) {
  	fs = (unsigned int) atoi(sampleRateArg.c_str());
  } else {
  	cout << "No sampling rate specified error" << endl;
  	exit(1);
  }
  if(!iDeviceArg.empty()) {
  	iDevice = (unsigned int) atoi(iDeviceArg.c_str());	
  }
  if(!oDeviceArg.empty()) {
  	oDevice = (unsigned int) atoi(oDeviceArg.c_str());
  }
  if(!convTypeArg.empty()) {
  	convType = convTypeArg;
  } else {
  	convType = "f";
  }
  if(!bufferFramesArg.empty()) {
  	bufferFrames = (unsigned int) atoi(bufferFramesArg.c_str());
  }

  //Reading impres data
  pFile = fopen("impres", "rb");
  if (pFile == NULL) {fputs ("File error\n", stderr); exit(1);}
  // obtain file size:
  fseek(pFile, 0, SEEK_END);
  lSize = ftell(pFile);
  rewind(pFile);
  // allocate memory to contain the whole file:
  buffer = (DATA_VEC*) malloc (sizeof(DATA_VEC)*lSize);
  if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

  // copy the file into the buffer:
  result = fread (buffer, 1, lSize, pFile);
  if (result != lSize) {fputs ("Reading error",stderr); exit (3);}

  // the whole file is now loaded in the memory buffer.
  cout << "\nsize of file : " << lSize << endl;
  // terminate
  fclose (pFile);

  //Fill audio_data structure
  int M = 50000;
  int L = bufferFrames;
  int ML = M + L;
  audio_data = new AudioData(L, M, get_nextpow2(ML),
  							 buffer; //IMpulse
  							 (DATA_VEC*)calloc(M, sizeof(DATA_VEC)), //overlapBuffer
  							 (DATA_VEC*)calloc(L + M, sizeof(DATA_VEC)), //currentBuffer
  							 (DATA_VEC*)calloc(L, sizeof(DATA_VEC)), //outBuffer
  							 (DATA_VEC*)calloc(ML, sizeof(DATA_VEC)), //reBuffer
  							 (DATA_VEC*)calloc(ML, sizeof(DATA_VEC)), //imBuffer
  							 (DATA_VEC*)calloc(ML, sizeof(DATA_VEC)), //reImpulse
  							 (DATA_VEC*)calloc(ML, sizeof(DATA_VEC)), //imImpulse
  							 )

  cout << " \n size : " << audio_data.M << "\n";

  RtAudio adac;

  if(adac.getDeviceCount() < 1) {
    cout << "\nNo audio devices found!\n";
    exit(1);
  }

  // Let RtAudio print messages to stderr.
  adac.showWarnings(true);

  // Set the same number of channels for both input and output.
  RtAudio::StreamParameters iParams, oParams;
  iParams.deviceId = iDevice;
  iParams.nChannels = inputChannels;
  iParams.firstChannel = iOffset;
  oParams.deviceId = oDevice;
  oParams.nChannels = outputChannels;
  oParams.firstChannel = oOffset;

  if(iDevice == 0)
    iParams.deviceId = adac.getDefaultInputDevice();
  if(oDevice == 0)
    oParams.deviceId = adac.getDefaultOutputDevice();

  RtAudio::StreamOptions options;
  cout << "\n Number of buffers : " << options.numberOfBuffers;

  bufferBytes = bufferFrames * inputChannels * sizeof(DATA_VEC);
  audio_data.bufferBytes = bufferBytes;
  for(int i = 0; i < audio_data.ML; i++) {
    audio_data.imImpulse[i] = 0;
    if(i < audio_data.M) {
      audio_data.reImpulse[i] = audio_data.impulse[i];
    }
    else {
      audio_data.reImpulse[i] = 0;
    }
  }

  fftr(audio_data.reImpulse, audio_data.imImpulse, audio_data.ML);
  try {
    adac.openStream(&oParams, &iParams, FORMAT, fs, &bufferFrames, &inout, (void*) &audio_data, &options);
  }
  catch (RtAudioError &e) {
    cout << '\n' << e.getMessage() << endl;
    exit(1);
  }

  // Test RtAudio functionality for reporting latency.
  cout << "\nStream latency = " << adac.getStreamLatency() << " frames" << endl;

  try {
    adac.startStream();

    char input;
    cout << "\nRunning ... press <enter> to quit (buffer frames = " << bufferFrames << ").\n";
    cin.get(input);

    // Stop the stream.
    adac.stopStream();
  }
  catch (RtAudioError &e) {
    cout << '\n' << e.getMessage() << '\n' << endl;
    goto cleanup;
  }

 cleanup:
  if (adac.isStreamOpen()) adac.closeStream();
  free(audio_data.currentBuffer);
  free(audio_data.overlapBuffer);
  free(audio_data.outBuffer);
  free(audio_data.impulse);
  return 0;
}

void conv_freq(AudioData* audio_data, DATA_VEC* inbuff) {
  //Frequency convolution
  for(int i = 0 ; i < audio_data->ML; i++){
    audio_data->imBuffer[i] = 0;
    if (i < audio_data->L){
      audio_data->reBuffer[i] = inbuff[i];
    }
    else{
      audio_data->reBuffer[i] = 0;
    }
  }

  fftr(audio_data->reBuffer, audio_data->imBuffer, audio_data->ML);

  for(int i = 0; i < audio_data->ML; i++) {
    audio_data->reBuffer[i] = audio_data->reBuffer[i]*audio_data->reImpulse[i] - audio_data->imBuffer[i]*audio_data->imImpulse[i];
    audio_data->imBuffer[i] = audio_data->imBuffer[i]*audio_data->reImpulse[i] + audio_data->reBuffer[i]*audio_data->imImpulse[i];
  }
  ifft(audio_data->reBuffer, audio_data->imBuffer, audio_data->ML);
  for(int i = 0; i < audio_data->M + audio_data->L; i++) {
      audio_data->currentBuffer[i] = audio_data->reBuffer[i];
  }
}

void conv_t(AudioData* audio_data, DATA_VEC* inbuff) {
	//Time convolution
	int L = audio_data->L;
	int M = audio_data->M;
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
			tmp = tmp + inbuff[k]*audio_data->impulse[i-k+1];
      	}
      	audio_data->currentBuffer[i] = tmp;
	}
}