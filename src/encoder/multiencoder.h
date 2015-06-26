#ifndef __MULTIENCODER_H
#define __MULTIENCODER_H

#include <functional>
#include <stdint.h>

extern "C" {
#include <x264.h>
}

#include "../global.h"
#include "../tools/timer.h"
#include "h264encoder.h"

class MultiH264Encoder : public Encoder, public InputObserver, public EncoderObserver {
public:
	explicit MultiH264Encoder(std::string mode);
	virtual ~MultiH264Encoder();
    void stop();
	
	// Input Observer Methods.
	virtual void onSizeChanged(int id, int width, int height);
    virtual void onColorSpaceChanged(int id, int csp) { id == LEFT ? _encoders[0].onColorSpaceChanged(id, csp) : _encoders[1].onColorSpaceChanged(id, csp); }
    virtual void onFrameReceived(int id, uint8_t** framePlanes, int* framePlaneSizes, int planes);
    virtual void onFrameReceived(int id, uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes);
    virtual void onStatsCodeReceived(int id, int code);

    // Encoder Observer Methods.
    virtual void onEncoderDataReceived(uint8_t type, uint8_t* data, int size);

protected:
	void mergedOutput(uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes);

private:
	H264Encoder _encoders[2];
	std::string _mode;
};


#endif // __MULTIENCODER_H
