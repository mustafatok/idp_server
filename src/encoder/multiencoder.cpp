#include "multiencoder.h"

#include <assert.h>
#include <string.h>
#include <iostream>
#include <cv.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "../global.h"
#include "../tools/psnr.h"

extern "C" {
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

using namespace std;

//#define ANALYZER
#define LOGTIME

MultiH264Encoder::MultiH264Encoder(int mode, int defaultWidth, int defaultHeight)
{
	this->_mode = mode;
	_encoders[0].setEncoderObserver(LEFT, this);
	_encoders[1].setEncoderObserver(RIGHT, this);

	_lWidth = (mode == (int)MODE_LEFTRESIZED) ? defaultWidth / 2 : defaultWidth;
	_lHeight = (mode == (int)MODE_LEFTRESIZED) ? defaultHeight / 2 : defaultHeight;
	_rWidth = (mode == (int)MODE_RIGHTRESIZED) ? defaultWidth / 2 : defaultWidth;
	_rHeight = (mode == (int)MODE_RIGHTRESIZED) ? defaultHeight / 2 : defaultHeight;
}

MultiH264Encoder::~MultiH264Encoder()
{
	this->stop();
}

void MultiH264Encoder::stop(){
    _encoders[0].stop();
    _encoders[1].stop();
}
	
void MultiH264Encoder::onFrameReceived(int id, uint8_t** framePlanes, int* framePlaneSizes, int planes) {
	std::cerr << "MultiH264Encoder onFrameReceived called for single frame!!" << std::endl;
	exit(1);
}
void MultiH264Encoder::onFrameReceived(int id, uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes) {
	if(_mode == (int) MODE_VERTICALCONCAT){
		this->verticalConcat(lframePlanes, lframePlaneSizes, lplanes, rframePlanes,  rframePlaneSizes, rplanes);
	}else if(_mode == (int) MODE_LEFTRESIZED){
		this->resize(lframePlanes, lframePlaneSizes, lplanes, true, rframePlanes,  rframePlaneSizes, rplanes, false);
	}else if(_mode == (int) MODE_RIGHTRESIZED){
		this->resize(lframePlanes, lframePlaneSizes, lplanes, false, rframePlanes,  rframePlaneSizes, rplanes, true);
	}else if(_mode == (int) MODE_LEFTBLURRED){
		this->blur(lframePlanes, lframePlaneSizes, lplanes, true, rframePlanes,  rframePlaneSizes, rplanes, false);
	}else if(_mode == (int) MODE_RIGHTBLURRED){
		this->blur(lframePlanes, lframePlaneSizes, lplanes, false, rframePlanes,  rframePlaneSizes, rplanes, true);
	}else if(_mode == (int) MODE_INTERLEAVING){
		this->interleaving(lframePlanes, lframePlaneSizes, lplanes, rframePlanes,  rframePlaneSizes, rplanes);
	}
}

void MultiH264Encoder::verticalConcat(uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes){
	uint8_t *tFramePlanes[4];
	int outputRowSizes[4] { 0 };

	av_image_alloc(static_cast<uint8_t**>(tFramePlanes), outputRowSizes, _width, _height*2, PIX_FMT_YUV420P, 1);

	for (int i = 0; i < lplanes; ++i)
	{
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

	_encoders[0].onFrameReceived(0, tFramePlanes, lframePlaneSizes, lplanes);

	// TODO CLEANUP MEMORY LEAKS
	// av_freep(&tFramePlanes[0])
}
void MultiH264Encoder::interleaving(uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes){
	
	if(leftFlagInterleaving == 0){
		_encoders[1].onFrameReceived(0, rframePlanes, rframePlaneSizes, rplanes);
		leftFlagInterleaving = 1;
	}else if(leftFlagInterleaving == 1){
		_encoders[0].onFrameReceived(0, lframePlanes, lframePlaneSizes, lplanes);
		leftFlagInterleaving = 0;
	}else{
		_encoders[0].onFrameReceived(0, lframePlanes, lframePlaneSizes, lplanes);
		_encoders[1].onFrameReceived(0, rframePlanes, rframePlaneSizes, rplanes);
	}
}

void MultiH264Encoder::blur(uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, bool lBlur, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes, bool rBlur){
	// Make it free afterwards...
	SwsContext *swsctx = sws_getContext(_width, _height,
                      PIX_FMT_YUV420P,
                      _width, _height,
                      PIX_FMT_BGR24,
                      0, 0, 0, 0);

	AVFrame* frame2 = av_frame_alloc();
	int num_bytes = avpicture_get_size(PIX_FMT_BGR24, _width, _height);
	uint8_t* frame2_buffer = (uint8_t *)av_malloc(num_bytes*sizeof(uint8_t));
	avpicture_fill((AVPicture*)frame2, frame2_buffer, PIX_FMT_BGR24, _width, _height);

	if(lBlur){
		sws_scale(swsctx, lframePlanes, lframePlaneSizes, 0, _height, frame2->data, frame2->linesize);
	}else if(rBlur){
		sws_scale(swsctx, rframePlanes, rframePlaneSizes, 0, _height, frame2->data, frame2->linesize);
	}else{
		_encoders[0].onFrameReceived(0, lframePlanes, lframePlaneSizes, lplanes);
		_encoders[1].onFrameReceived(0, rframePlanes, rframePlaneSizes, rplanes);
		return;
		// TODO cleanup.
	}

	cv::Mat src(_height,
			_width,
			CV_8UC3,
			frame2->data[0]);


	cv::Mat dst;
	cv::GaussianBlur( src, dst, cv::Size( 5, 5 ), 0, 0 );
 //  	cv::namedWindow( "Display window1", cv::WINDOW_AUTOSIZE );
	// cv::imshow("Display window1", dst);  	
	// cv::namedWindow( "Display window2", cv::WINDOW_AUTOSIZE );
	// cv::imshow("Display window2", src);
	
	frame2->data[0] = dst.data;
	// frame2->linesize[0] = src.step;
	// avpicture_fill((AVPicture*)frame2, dst.data, PIX_FMT_BGR24, _width, _height);

 	// cv::waitKey(0); 
	SwsContext *swsctx2 = sws_getContext(_width, _height,
                      PIX_FMT_BGR24,
                      _width, _height,
                      PIX_FMT_YUV420P,
                      0, 0, 0, 0);

	AVFrame* frame3 = av_frame_alloc();
	num_bytes = avpicture_get_size(PIX_FMT_YUV420P, _width, _height);
	uint8_t* frame3_buffer = (uint8_t *)av_malloc(num_bytes*sizeof(uint8_t));
	avpicture_fill((AVPicture*)frame3, frame3_buffer, PIX_FMT_YUV420P, _width, _height);
	sws_scale(swsctx2, frame2->data, frame2->linesize, 0, _height, frame3->data, frame3->linesize);


	if(lBlur){
		_encoders[0].onFrameReceived(0, frame3->data, frame3->linesize, lplanes);
		_encoders[1].onFrameReceived(0, rframePlanes, rframePlaneSizes, rplanes);
	}else if(rBlur){
		_encoders[0].onFrameReceived(0, lframePlanes, lframePlaneSizes, lplanes);
		_encoders[1].onFrameReceived(0, frame3->data, frame3->linesize, rplanes);
	}else{
		_encoders[0].onFrameReceived(0, lframePlanes, lframePlaneSizes, lplanes);
		_encoders[1].onFrameReceived(0, rframePlanes, rframePlaneSizes, rplanes);
	}


	// TODO CLEANUP MEMORY LEAKS
	av_free(frame2_buffer);
	av_frame_free(&frame2);
	av_free(frame3_buffer);
	av_frame_free(&frame3);

}

void MultiH264Encoder::resize(uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, bool lResize, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes, bool rResize){
	// Make it free afterwards...
	SwsContext *swsctx = sws_getContext(_width, _height,
                      PIX_FMT_YUV420P,
                      _width / 2, _height / 2,
                      PIX_FMT_YUV420P,
                      0, 0, 0, 0);

	AVFrame* frame2 = av_frame_alloc();
	int num_bytes = avpicture_get_size(PIX_FMT_YUV420P, _width / 2, _height / 2);
	uint8_t* frame2_buffer = (uint8_t *)av_malloc(num_bytes*sizeof(uint8_t));
	avpicture_fill((AVPicture*)frame2, frame2_buffer, PIX_FMT_YUV420P, _width / 2, _height / 2);

	if(lResize){
		sws_scale(swsctx, lframePlanes, lframePlaneSizes, 0, _height, frame2->data, frame2->linesize);
		_encoders[0].onFrameReceived(0, frame2->data, frame2->linesize, lplanes);
		_encoders[1].onFrameReceived(0, rframePlanes, rframePlaneSizes, rplanes);
	}else if(rResize){
		sws_scale(swsctx, rframePlanes, rframePlaneSizes, 0, _height, frame2->data, frame2->linesize);
		_encoders[0].onFrameReceived(0, lframePlanes, lframePlaneSizes, lplanes);
		_encoders[1].onFrameReceived(0, frame2->data, frame2->linesize, rplanes);
	}else{
		_encoders[0].onFrameReceived(0, lframePlanes, lframePlaneSizes, lplanes);
		_encoders[1].onFrameReceived(0, rframePlanes, rframePlaneSizes, rplanes);
	}

	// TODO CLEANUP MEMORY LEAKS
	av_free(frame2_buffer);
	av_frame_free(&frame2);

}

void MultiH264Encoder::onStatsCodeReceived(int id, int code){

}

// TODO : Make it multithreaded!!!  Do not forget to avoid memory leaks
void MultiH264Encoder::onEncodedDataReceived(int id, uint8_t type, uint8_t* data, int size){
	if(_mode == (int) MODE_VERTICALCONCAT){
		_observer->onEncodedDataReceived(_id, type, data, size);
		return;
	}
	if(id == LEFT){
		_lType = type;
		_lData = data;
		_lSize = size;
		// std::cout << "leftSize : "<< _lSize << std::endl;
	}else if(id == RIGHT){
		_rType = type;
		_rData = data;
		_rSize = size;
		// std::cout << "rightSize : "<< _rSize << std::endl;
	}
	if((++_tmpCnt) == 2){
		if(_mode == (int) MODE_INTERLEAVING){
			if(leftFlagInterleaving == -1){
				serializeAndSend();
				if(_lType != PROTOCOL_TYPE_HEADER && _rType != PROTOCOL_TYPE_HEADER){
					leftFlagInterleaving = 1;
				}else{
					_tmpCnt--;
				}
			}else{
				if(id == LEFT){
					_observer->onEncodedDataReceived(_id,  PROTOCOL_TYPE_LFRAME, _lData, _lSize);
				}else{
					_observer->onEncodedDataReceived(_id,  PROTOCOL_TYPE_RFRAME, _rData, _rSize);
				}
			}
			_tmpCnt--;
		}else{
			serializeAndSend();
			_tmpCnt = 0;
		}

	}

}

void MultiH264Encoder::serializeAndSend(){
	uint8_t* data;
	// TODO size can overflow??
	uint32_t size = 	(_lSize + _rSize) + 2 + 2 * sizeof(uint32_t);

	// std::cout << "size : "<< size << std::endl;

	data = new uint8_t[size];
	
	data[0] = _lType;
	data[1] = _rType;

	uint32_t* leftSize = reinterpret_cast<uint32_t*>(data + 2);
	*leftSize = _lSize;

	uint32_t* rightSize = reinterpret_cast<uint32_t*>(data + 2 + sizeof(uint32_t));
	*rightSize = _rSize;

	memcpy(data + 2 +  2*sizeof(uint32_t), _lData, _lSize);
	memcpy(data + 2 +  2*sizeof(uint32_t) + _lSize, _rData, _rSize);

	// TODO type?? Don't forget to free the data.
	_observer->onEncodedDataReceived(_id, _lType, data, size);
	delete data;
}

void MultiH264Encoder::onSizeChanged(int id, int width, int height) { 
	Encoder::setSize(width, height);
	if(_mode == (int) MODE_VERTICALCONCAT){
		_encoders[0].onSizeChanged(id, width, height*2);
	}else{
		if(_mode == (int) MODE_LEFTRESIZED){
			id == LEFT ? _encoders[0].onSizeChanged(id, width / 2, height / 2) : _encoders[1].onSizeChanged(id, width, height);
		}else if(_mode == (int) MODE_RIGHTRESIZED){
			id == LEFT ? _encoders[0].onSizeChanged(id, width, height) : _encoders[1].onSizeChanged(id, width / 2, height / 2);
		}else if(_mode == (int)MODE_LEFTBLURRED || _mode == (int) MODE_RIGHTBLURRED){
			id == LEFT ? _encoders[0].onSizeChanged(id, width, height) : _encoders[1].onSizeChanged(id, width, height);
		}else if(_mode == (int) MODE_INTERLEAVING){
			id == LEFT ? _encoders[0].onSizeChanged(id, width, height) : _encoders[1].onSizeChanged(id, width, height);
		}
	}
}
