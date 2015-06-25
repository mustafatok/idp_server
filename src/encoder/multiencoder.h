#ifndef __MULTIENCODER_H
#define __MULTIENCODER_H

#include <functional>
#include <stdint.h>

extern "C" {
#include <x264.h>
}

#include "../tools/timer.h"
#include "h264encoder.h"

class MultiH264Encoder : public Encoder {
public:
	explicit MultiH264Encoder();
	virtual ~MultiH264Encoder();
	
    virtual void pushFrame(int id, uint8_t** framePlanes, int* framePlaneSizes, int planes);
    virtual void pushFrame(int id, uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes);
    virtual void printStats(int id, int code);

private:
	H264Encoder _encoders[2];
};


#endif // __MULTIENCODER_H
