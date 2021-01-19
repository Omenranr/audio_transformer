/******************************************/
/*
  Adapted from the original duplex.cpp by Gary P. Scavone, 2006-2007.

  This code applies a reverb effect to the input audio signal.
*/
/******************************************/

#include "RtAudio.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <somefunc.cpp>
#include <sys/resource.h>
#include <AudioHandler.cpp>
#include <ArgumentHolder.cpp>

using namespace std;

#define FORMAT RTAUDIO_FLOAT64

int logger = 0;

int inout(void *outputBuffer, void *inputBuffer, unsigned int /*nBufferFrames*/, double /*streamTime*/, RtAudioStreamStatus status, void *data) {
  
  double t1 = 1000 * get_process_time();

  if (status) cout << "Stream over/underflow detected." << endl;

  //CHOOSE BETWEEN FREQ OR TEMP CONVOLUTION
  AudioHandler *currAudioHandler = (AudioHandler*) data;

  //APPLYING CONVOLUTION
  currAudioHandler->applyConv(inputBuffer);

  //APPLYING OVERLAP ADD
  currAudioHandler->overlapAdd(outputBuffer);

  double t2 = 1000 * get_process_time();
  if(logger % 100 == 0)
  	cout << "process time: " << t2 - t1 << " ms" << endl;
  logger++;

  return 0;
}

int main(int argc, char *argv[]) {

  //INITIALIZING ARGUMENTS
  ArgumentHolder audioArgs = ArgumentHolder();
  audioArgs.showHelp(argc, argv);
  if(audioArgs.init(argc, argv) == -1) {
  	cout << "Error when reading arguments occured" << endl;
  	exit(1);
  }

  //DEFINING AND CONSTRUCTING audio_data OBJECT
  int ML = get_nextpow2(audioArgs.M + audioArgs.bufferFrames);
  unsigned int bufferBytes = audioArgs.bufferFrames * audioArgs.inputChannels * sizeof(DATA_VEC);
  AudioHandler audio_data(audioArgs.bufferFrames /* L */, audioArgs.M, ML, audioArgs.convType, bufferBytes,
  							 (DATA_VEC*)calloc(audioArgs.M, sizeof(DATA_VEC)), /* overlapBuffer */
  							 (DATA_VEC*)calloc(audioArgs.M + audioArgs.bufferFrames, sizeof(DATA_VEC)), /* currentBuffer */
  							 (DATA_VEC*)calloc(audioArgs.bufferFrames /* L */, sizeof(DATA_VEC)), /* outBuffer */
  							 (DATA_VEC*)calloc(ML, sizeof(DATA_VEC)), /* reBuffer */
  							 (DATA_VEC*)calloc(ML, sizeof(DATA_VEC)), /* imBuffer */
  							 (DATA_VEC*)calloc(ML, sizeof(DATA_VEC)), /* reImpulse */
  							 (DATA_VEC*)calloc(ML, sizeof(DATA_VEC)) /* imImpulse */
  							 );
  audio_data.readImpulse(audioArgs.inputFile);
  audio_data.initializeVectors();

  //INITIALIZING RTAUDIO ADAC STREAM
  RtAudio adac;
  if(adac.getDeviceCount() < 1) {
    cout << "\nNo audio devices found!\n";
    exit(1);
  }
  // Let RtAudio print messages to stderr.
  adac.showWarnings(true);
  cout << "input channels" << audioArgs.inputChannels;

  //INITIALIZING RtAudio StreamParameters
  RtAudio::StreamParameters iParams, oParams;
  iParams.deviceId = audioArgs.iDevice;
  iParams.nChannels = audioArgs.inputChannels;
  iParams.firstChannel = audioArgs.iOffset;
  oParams.deviceId = audioArgs.oDevice;
  oParams.nChannels = audioArgs.outputChannels;
  oParams.firstChannel = audioArgs.oOffset;
  if(audioArgs.iDevice == 0)
    iParams.deviceId = adac.getDefaultInputDevice();
  if(audioArgs.oDevice == 0)
    oParams.deviceId = adac.getDefaultOutputDevice();

  RtAudio::StreamOptions options;
  cout << "\n Number of buffers : " << options.numberOfBuffers;

  //OPENING ADAC STREAM
  try {
    adac.openStream(&oParams, &iParams, FORMAT, audioArgs.fs, &(audioArgs.bufferFrames), &inout, (void*) &audio_data, &options);
  }
  catch (RtAudioError &e) {
    cout << '\n' << e.getMessage() << endl;
    exit(1);
  }

  // Test RtAudio functionality for reporting latency.
  cout << "\nStream latency = " << adac.getStreamLatency() << " frames" << endl;

  //STARTING ADAC STREAM
  try {
    adac.startStream();
    char input;
    cout << "\nRunning ... press <enter> to quit (buffer frames = " << audioArgs.bufferFrames << ").\n";
    cin.get(input);
    // Stop the stream.
    adac.stopStream();
  }
  catch (RtAudioError &e) {
    cout << '\n' << e.getMessage() << '\n' << endl;
    goto cleanup;
  }

  //CLEANING MEMORY AND EXITING
  cleanup:
  if (adac.isStreamOpen()) adac.closeStream();
  free(audio_data.currentBuffer);
  free(audio_data.overlapBuffer);
  free(audio_data.outBuffer);
  free(audio_data.impulse);
  return 0;
}