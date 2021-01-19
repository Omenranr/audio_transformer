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

class AudioTransformer {
	virtual void transform(DATA_VEC* inputBuffer) = 0; 
};

