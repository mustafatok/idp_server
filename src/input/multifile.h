#ifndef __MULTIFILE_H
#define __MULTIFILE_H

#include <string>
#include <thread>
#include <atomic>

#include "file.h"
#include "camera.h"

#define LEFT 1
#define RIGHT 2

class MultiFileInput : public Input, public Encoder {
public:
	explicit MultiFileInput(std::string type, std::string inputFileL, std::string inputFileR, std::string outputType);
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

    virtual void pushFrame(int id, uint8_t** framePlanes, int* framePlaneSizes, int planes){
    	if(id == LEFT)
    		pushLeftFrame(framePlanes, framePlaneSizes, planes);
    	else
    		pushRightFrame(framePlanes, framePlaneSizes, planes);
    }
    virtual void pushFrame(int id, uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes) {}
    virtual void printStats(int id, int code){}
    virtual void setSize(int id, int width, int height) {
        this->_lWidth = this->_rWidth = width;
        this->_lHeight = this->_rHeight = height;
    }
	void printStats(int code);
	void stop();
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
	void mergedOutput();
	bool stopped = false;

	std::string outputType;
};

#endif // __MULTIFILE_H
