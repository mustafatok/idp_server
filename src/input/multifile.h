#ifndef __MULTIFILE_H
#define __MULTIFILE_H

#include <string>
#include <thread>
#include <atomic>

#include "file.h"
#include "camera.h"


class MultiFileInput : public Input {
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
	void setColorspace(int csp);
	void pushLeftFrame(uint8_t** framePlanes, int* framePlaneSizes, int planes);
	void pushRightFrame(uint8_t** framePlanes, int* framePlaneSizes, int planes);

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
	int _csp;
	uint8_t **_lFramePlanes, **_rFramePlanes;
	int *_lFramePlaneSizes, *_rFramePlaneSizes;
	int _lPlanes, _rPlanes;

	void postFrame();
	bool stopped = false;

};

#endif // __MULTIFILE_H
