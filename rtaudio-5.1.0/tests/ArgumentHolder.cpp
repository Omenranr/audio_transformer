#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <sys/resource.h>

class ArgumentHolder {
	public:
		unsigned int fs;
		unsigned int inputChannels = 1;
		unsigned int outputChannels = 1;
		unsigned int iDevice = 0;
		unsigned int oDevice = 0;
		unsigned int bufferFrames = 512;
		unsigned int iOffset = 0;
		unsigned int oOffset = 0;
		unsigned int M = 50000;
		string convType = "f";
		string inputFile = "impres";
		string getCmdOption(int argc, char* argv[], const string& option)
		{
		    string cmd;
		     for( int i = 0; i < argc; ++i)
		     {
		          string arg = argv[i];
		          if(0 == arg.find(option))
		          {
		               size_t found = arg.find_last_of(option);
		               cmd =arg.substr(found + 1);
		               return cmd;
		          }
		     }
		     return cmd;
		}
		int init(int argc, char *argv[]) {
		  	//Reading parameters
		  	string channelsArg = getCmdOption(argc, argv, "-ic=");
		  	string sampleRateArg = getCmdOption(argc, argv, "-fs=");
		  	string iDeviceArg = getCmdOption(argc, argv, "-input=");
		  	string oDeviceArg = getCmdOption(argc, argv, "-output=");
		  	string convTypeArg = getCmdOption(argc, argv, "-conv=");
		    string oChannelsArg = getCmdOption(argc, argv, "-oc=");
		    string bufferFramesArg = getCmdOption(argc, argv, "-bs=");
		    string inputFileArg = getCmdOption(argc, argv, "-file=");
		    string mArg = getCmdOption(argc, argv, "-m=");
		    //filling parameters
		    if(!channelsArg.empty()) {
		  	  this->inputChannels = (unsigned int) atoi(channelsArg.c_str());
		    }
		    if(!oChannelsArg.empty()) {
		  	  this->outputChannels = (unsigned int) atoi(channelsArg.c_str());
		    }
		    if(!sampleRateArg.empty()) {
		    	this->fs = (unsigned int) atoi(sampleRateArg.c_str());
		    } else {
		  	  	cout << "No sampling rate specified error" << endl;
		  	  	return -1;
		    }
		    if(!iDeviceArg.empty()) {
		  	  	this->iDevice = (unsigned int) atoi(iDeviceArg.c_str());	
		    }
		    if(!oDeviceArg.empty()) {
		  	  	this->oDevice = (unsigned int) atoi(oDeviceArg.c_str());
		    }
		    if(!convTypeArg.empty()) {
		  	  	this->convType = convTypeArg;
		    }
		    if(!bufferFramesArg.empty()) {
		    	this->bufferFrames = (unsigned int) atoi(bufferFramesArg.c_str());
		    }
		    if(!inputFileArg.empty()) {
		    	this->inputFile = inputFileArg;
		    }
		    if(!mArg.empty()) {
		    	this->M = (unsigned int) atoi(mArg.c_str());
		    }
		    return 0;
		}
		void showHelp(int argc, char *argv[]) {
			string helpArg = getCmdOption(argc, argv, "-help=");
			if(!helpArg.empty()) {
				cout << "Show help and exit: -help=1 [OPTIONAL]" << endl;
				cout << "Input channels: -ic=INPUT_CHANNELS, [OPTIONAL] default: 1" << endl;
				cout << "Output channels: -oc=OUTPUT_CHANNELS, [OPTIONAL] default: 1" << endl;
				cout << "Sample rate: -fs=SAMPLING_RATE [REQUIRED]" << endl;
				cout << "Input device: -input=INPUT_ID [OPTIONAL] default: 0" << endl;
				cout << "Output device: -output=OUTPUT_ID [OPTIONAL] default: 0" << endl;
				cout << "Convolution type: -conv=f or t [OPTIONAL] default: f" << endl;
				cout << "Buffer frames: -bs=BUFFER_FRAMES [OPTIONAL] default: 512" << endl;
				cout << "Impulse file name: -file=PATH [OPTIONAL] default: impres" << endl;
				cout << "M: size of impulse vector -m=SIZE_VECTOR [OPTIONAL] default: 50000" << endl;
				exit(0);
			}
		}
};