#ifndef __ENCODER_H
#define __ENCODER_H

#include <functional>
#include <stdint.h>

extern "C" {
#include <x264.h>
}

#include "../tools/timer.h"

typedef void data_callback_t (uint8_t, uint8_t*, int);

class H264Encoder {
public:
        explicit H264Encoder();
        virtual ~H264Encoder();

        void setSize(int width, int height);
        void setColorspace(int csp);
        void pushFrame(uint8_t** framePlanes, int* framePlaneSizes, int planes);

        void printStats(int code);

        void close();

        /*!
         * This callback provides the data to be further processed!
         */
        void setDataCallback(data_callback_t* callback);

        /*!
         * Same functionality, but for member functions.
         */
        template <typename ObjectType>
        void setDataCallback(ObjectType *instance, void (ObjectType::*callback)(uint8_t, uint8_t*, int))
        {
                dataCallback = [=](uint8_t type, uint8_t* data, int size) {
                        (instance->*callback)(type, data, size);
                };
        }


private:
        void init();

        int _width;
        int _height;

        x264_param_t _parameters;
        x264_picture_t _inputPicture;
        x264_picture_t _outputPicture;
        x264_t* _encoder = nullptr;

        x264_nal_t* _nalHeaderUnits = nullptr;
        x264_nal_t* _nalFrameUnits = nullptr;
        int _nalHeaderCount;
        int _nalFrameCount;

        int pts = 0;

        int csp = 0;

        Timer _timer;

        std::function<void(uint8_t, uint8_t*, int)> dataCallback;

        int64_t frameCount = 0;
        int64_t encodeCount = 0;

#if X264_BUILD < 76
#define	DATA_SIZE 1000000 // 1MB - if client crashes it might be because old x264 versions (older than 17 Sep 2009)
                          // did not check the size of the target buffer when writing the nalu units (check nal_encode function in x264 source code)
	uint8_t data[DATA_SIZE];
#endif
};


#endif // __ENCODER_H
