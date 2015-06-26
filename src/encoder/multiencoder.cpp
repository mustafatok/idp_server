#include "multiencoder.h"

#include <assert.h>
#include <string.h>
#include <iostream>

#include "../global.h"
#include "../tools/psnr.h"

extern "C" {
#include <libavutil/imgutils.h>
}

using namespace std;

//#define ANALYZER
#define LOGTIME

MultiH264Encoder::MultiH264Encoder(std::string mode)
{
	this->_mode = mode;
	_encoders[0].setEncoderObserver(this);
	_encoders[1].setEncoderObserver(this);
}

MultiH264Encoder::~MultiH264Encoder()
{
}

void MultiH264Encoder::stop(){
    _encoders[0].stop();
    _encoders[1].stop();
}
	
void MultiH264Encoder::onFrameReceived(int id, uint8_t** framePlanes, int* framePlaneSizes, int planes){

}
void MultiH264Encoder::onFrameReceived(int id, uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes) {
	if(_mode == "mergedOutput"){
		this->mergedOutput(lframePlanes, lframePlaneSizes,  lplanes, rframePlanes,  rframePlaneSizes, rplanes);
	}else{

	}
}

void MultiH264Encoder::mergedOutput(uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes){
	uint8_t *tFramePlanes[4];
	int outputRowSizes[4] { 0 };

	av_image_alloc(static_cast<uint8_t**>(tFramePlanes), outputRowSizes, _width, _height*2, PIX_FMT_YUV420P, 1);

	int *tFramePlaneSizes = (int *) malloc(lplanes * sizeof(int));

	tFramePlaneSizes[0] = lframePlaneSizes[0];

	for (int i = 0; i < lplanes; ++i)
	{
		tFramePlaneSizes[i] = lframePlaneSizes[i];
		int size = lframePlaneSizes[i]*_height;
		if(i != 0){
			size /= 2;
		}
		int j;
		for (j = 0; j < size; ++j)
		{
			tFramePlanes[i][j] = lframePlanes[i][j];
		}
		for (; j < 2*size; ++j)
		{
			tFramePlanes[i][j] = rframePlanes[i][j-size];
		}
	}

	_encoders[0].onFrameReceived(0, tFramePlanes, tFramePlaneSizes, lplanes);

	// TODO CLEANUP MEMORY LEAKS
}

void MultiH264Encoder::onStatsCodeReceived(int id, int code){

}

void MultiH264Encoder::onEncoderDataReceived(uint8_t type, uint8_t* data, int size){
	_observer->onEncoderDataReceived(type, data, size);
}


void MultiH264Encoder::onSizeChanged(int id, int width, int height) { 
	Encoder::setSize(width, height);
	std::cout << "TTT - " << width << "  -  " << height << std::endl;
	if(_mode == "mergedOutput"){
		_encoders[0].onSizeChanged(id, width, height*2);
	}else{
		id == LEFT ? _encoders[0].onSizeChanged(id, width, height) : _encoders[1].onSizeChanged(id, width, height);
	}
}
