#include "multiencoder.h"

#include <assert.h>
#include <string.h>
#include <iostream>

#include "../global.h"
#include "../tools/psnr.h"

using namespace std;

//#define ANALYZER
#define LOGTIME

MultiH264Encoder::MultiH264Encoder()
{

}

MultiH264Encoder::~MultiH264Encoder()
{
}

	
void MultiH264Encoder::pushFrame(int id, uint8_t** framePlanes, int* framePlaneSizes, int planes){

}
void MultiH264Encoder::pushFrame(int id, uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes) {

}
void MultiH264Encoder::printStats(int id, int code){

}