#ifndef __H264ENCODER_H
#define __H264ENCODER_H

#include <functional>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "encoder.h"
#include "../observer/inputobserver.h"

extern "C" {
#include <x264.h>
}

#include "../tools/timer.h"

class H264Encoder : public Encoder, public InputObserver{
public:
		explicit H264Encoder();
		explicit H264Encoder(int bitRate);
		virtual ~H264Encoder();

		// Observer Methods.
		virtual void onSizeChanged(int id, int width, int height);
		virtual void onColorSpaceChanged(int id, int csp) { Encoder::setColorspace(csp); }
        virtual void onFrameReceived(int id, uint8_t** framePlanes, int* framePlaneSizes, int planes);
        virtual void onFrameReceived(int id, uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes){}
        virtual void onStatsCodeReceived(int id, int code);

		void close();
        void stop();

		virtual void setBitRate(int bitRate){
			_bitRate = bitRate;
        	_parameters.rc.i_bitrate = bitRate; // 10kb

			if(_encoder != nullptr) {
        		//_encoder = x264_encoder_open(&_parameters);


				std::cout << " Reconfig " << std::endl;
				// x264_t* tmp = _encoder;
				
				x264_param_t* tmpparam = new x264_param_t();
        		memcpy(tmpparam, &_parameters, sizeof(x264_param_t));

        		x264_t* encoder1 = x264_encoder_open(tmpparam);
        		// TODO : FREE
        		_encoder = encoder1;

				// x264_encoder_encode(tmp, &_nalFrameUnits, &a, nullptr, &op);
        		// x264_encoder_close(tmp);
        		// free(tmp);

			}
		}

		virtual void setFps(int fps){
			this->_fps = fps;
		}
		
		int fps(){
			return this->_fps;
		}



private:
		void init();

		x264_param_t _parameters;
		x264_picture_t _inputPicture;
		x264_picture_t _outputPicture;
		x264_t* _encoder = nullptr;

		x264_nal_t* _nalHeaderUnits = nullptr;
		x264_nal_t* _nalFrameUnits = nullptr;
		int _nalHeaderCount;
		int _nalFrameCount;

		int pts = 0;

		Timer _timer;

		int64_t frameCount = 0;
		int64_t encodeCount = 0;

		int _bitRate = 1000, _fps = 25;

#if X264_BUILD < 76
#define	DATA_SIZE 1000000 // 1MB - if client crashes it might be because old x264 versions (older than 17 Sep 2009)
						  // did not check the size of the target buffer when writing the nalu units (check nal_encode function in x264 source code)
	uint8_t data[DATA_SIZE];
#endif
};


#endif // __H264ENCODER_H
