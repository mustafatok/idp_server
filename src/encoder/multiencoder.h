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
	explicit MultiH264Encoder(int mode, int defaultWidth, int defaultHeight, int resizeFactor);
	virtual ~MultiH264Encoder();
    void stop();
	
	// Input Observer Methods.
	virtual void onSizeChanged(int id, int width, int height);
    virtual void onColorSpaceChanged(int id, int csp) { id == LEFT ? _encoders[0].onColorSpaceChanged(id, csp) : _encoders[1].onColorSpaceChanged(id, csp); }
    virtual void onFrameReceived(int id, uint8_t** framePlanes, int* framePlaneSizes, int planes);
    virtual void onFrameReceived(int id, uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes);
    virtual void onStatsCodeReceived(int id, int code);

    // Encoder Observer Methods.
    virtual void onEncodedDataReceived(int id, uint8_t type, uint8_t* data, int size);

    virtual void setBitRate(int lbitRate){
    	_encoders[0].setBitRate(lbitRate);
    }
    virtual void setBitRate(int lbitRate, int rbitRate){
    	_encoders[0].setBitRate(lbitRate);
    	_encoders[1].setBitRate(rbitRate);
    }
    virtual void setFps(int fps){
    	_encoders[0].setFps(fps);
    	_encoders[1].setFps(fps);
    }
    virtual void setBlurParams(int sizeX, int stdX, int stdY){
        _sizeX = sizeX;
        _stdX = stdX;
        _stdY = stdY;
    }
    virtual void setResizeFactor(int resizeFactor){
        _resizeFactor = resizeFactor;
    }
protected:
    void verticalConcat(uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes);
	void singleFrame(uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes);
	void interleaving(uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes);
	void blur(uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, bool lBlur, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes, bool rBlur);
	void resize(uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, bool lResize, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes, bool rResize);

    void serializeAndSend();

	uint8_t _lType; uint8_t* _lData; int _lSize;
	uint8_t _rType; uint8_t* _rData; int _rSize;
	int _tmpCnt = 0;
    int _lFrameCounter = 0; int _rFrameCounter = 0;
    long _lSizeCounter = 0; long _rSizeCounter = 0;
    Timer timer;
    Timer timer1;

    int _sizeX; int _stdX; int _stdY;
    int _resizeFactor;
private:
	H264Encoder _encoders[2];
	int _mode;
	int leftFlagInterleaving = -1;
};


#endif // __MULTIENCODER_H
