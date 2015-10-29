#ifndef __MULTIFILE_H
#define __MULTIFILE_H

#include <string>
#include <thread>
#include <atomic>

#include "file.h"
#include "camera.h"

class MultiFileInput : public Input, public InputObserver {
public:
	explicit MultiFileInput(std::string type, std::string inputFileL, std::string inputFileR);
	virtual ~MultiFileInput();

	/*!
	 * This function starts the decoding process.
	 * The data will be reported by the callbacks.
	 * The operator () is used, so that this class can be used with std::thread.
	 */
	void operator()();

	void setLeftSize(int width, int height);
	void setRightSize(int width, int height);
	void pushLeftFrame(uint8_t** framePlanes, int* framePlaneSizes, int planes);
	void pushRightFrame(uint8_t** framePlanes, int* framePlaneSizes, int planes);

	// Observer methods override.
    virtual void onFrameReceived(int id, uint8_t** framePlanes, int* framePlaneSizes, int planes){
    	if(id == LEFT)
    		pushLeftFrame(framePlanes, framePlaneSizes, planes);
    	else
    		pushRightFrame(framePlanes, framePlaneSizes, planes);
    }
    virtual void onFrameReceived(int id, uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes) {}
    virtual void onSizeChanged(int id, int width, int height) {
        this->_lWidth = this->_rWidth = width;
        this->_lHeight = this->_rHeight = height;
		_observer->onSizeChanged(id, width , height);
    }
    virtual void onColorSpaceChanged(int id, int csp){
   		_observer->onColorSpaceChanged(id, PIX_FMT_YUV420P);
    };

	virtual void onStatsCodeReceived(int id, int code){}
    virtual void printStats(int code) {}

	virtual void stop();
	virtual void start(){
		_fileInputL->start();
		_fileInputR->start();
		stopped = false;
	}
	virtual void setInputOffset(int x, int y){
		if(x < 0 || y < 0) return;
		if(x + WIDTH > SENSORWIDTH || y + HEIGHT > SENSORHEIGHT) return;

        _fileInputL->setInputOffset(x, y);
        _fileInputR->setInputOffset(x, y);
	}
	virtual void setInputOffsetX(int x1, int x2){
        _fileInputL->setInputOffsetX(x1);
        _fileInputR->setInputOffsetX(x2);
	}

	std::string type(){return _type;}
private:

	Input* _fileInputL;
	Input* _fileInputR;

	std::thread *_lThread = nullptr;
	std::thread *_rThread = nullptr;
	std::thread *_mThread = nullptr;
	std::atomic_uint _waitingQ;

	int _lWidth, _lHeight;
	int _rWidth, _rHeight;
	uint8_t **_lFramePlanes, **_rFramePlanes;
	int *_lFramePlaneSizes, *_rFramePlaneSizes;
	int _lPlanes, _rPlanes;

	void postFrame();

protected:
	std::string _type;

};

#endif // __MULTIFILE_H
