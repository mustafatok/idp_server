#ifndef __H264ENCODER_H
#define __H264ENCODER_H

#include <functional>
#include <stdint.h>
#include "encoder.h"

extern "C" {
#include <x264.h>
}

#include "../tools/timer.h"

class H264Encoder : public Encoder{
public:
        explicit H264Encoder();
        virtual ~H264Encoder();

        virtual void pushFrame(int id, uint8_t** framePlanes, int* framePlaneSizes, int planes);
        virtual void pushFrame(int id, uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes) {}
        virtual void printStats(int id, int code);
        virtual void setSize(int id, int width, int height);
        void close();

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

#if X264_BUILD < 76
#define	DATA_SIZE 1000000 // 1MB - if client crashes it might be because old x264 versions (older than 17 Sep 2009)
                          // did not check the size of the target buffer when writing the nalu units (check nal_encode function in x264 source code)
	uint8_t data[DATA_SIZE];
#endif
};


#endif // __H264ENCODER_H
