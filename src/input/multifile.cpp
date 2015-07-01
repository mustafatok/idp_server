/*
 * Based on code written by Stefano Sabatini
 * http://www.ffmpeg.org/doxygen/trunk/demuxing_decoding_8c-example.html
 */

#include "multifile.h"

#include <cmath>
#include <functional>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <string.h>
#include <mutex>
#include <condition_variable>
#include <unistd.h>


extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
}

#include "../global.h"

using namespace std;

std::mutex mtx;
std::mutex mtxM;
std::condition_variable cv;
std::condition_variable cvM;
bool readyL = false;
bool readyR = false;


MultiFileInput::MultiFileInput(std::string type, std::string inputFileL, std::string inputFileR) : Input()
{
	_waitingQ = 0;

	if(type == "file"){
		_fileInputL = new FileInput(inputFileL);
		_fileInputR = new FileInput(inputFileR);
	}else if(type == "camera"){
		_fileInputL = new CameraInput(inputFileL);
		_fileInputR = new CameraInput(inputFileR);
	}else{
		cerr << "Type is not valid!" << endl;
		exit(1);
	}

	_fileInputL->setInputObserver(LEFT, this);
	_fileInputR->setInputObserver(RIGHT, this);

}
void MultiFileInput::stop() {
		Input::stop();

		_fileInputL->stop();
		_fileInputR->stop();

		if(_mThread != nullptr){
			_mThread->join();
			delete _mThread;
		}
		if(_lThread != nullptr){
			_lThread->join();
			delete _lThread;
		}
		if(_rThread != nullptr){
			_rThread->join();
			delete _rThread;
		}
		_mThread = _rThread = _lThread = nullptr;

		readyL = false;
		readyR = false;
		_waitingQ = 0;

	

}
MultiFileInput::~MultiFileInput()
{
		delete _fileInputL;
		delete _fileInputR;
		delete _lThread;
		delete _rThread;
		delete _mThread;
}

void MultiFileInput::operator()()
{
	stopped = false;


	if (_lThread == nullptr) {
		_lThread = new thread([&](){
			(*_fileInputL)();
		});
	}
	if (_rThread == nullptr) {
		_rThread = new thread([&](){
			(*_fileInputR)();
		});
	}
	if (_mThread == nullptr) {
		_mThread = new thread(&MultiFileInput::postFrame, this);
	}
}

void MultiFileInput::setLeftSize(int width, int height)
{
	_lWidth = width;
	_lHeight = height;
	cout << "Left-Video-Width: " << _lWidth << endl;
	cout << "Left-Video-Height: " << _lHeight << endl;
}

void MultiFileInput::setRightSize(int width, int height)
{
	_rWidth = width;
	_rHeight = height;
	cout << "Right-Video-Width: " << _rWidth << endl;
	cout << "Right-Video-Height: " << _rHeight << endl;
}

void MultiFileInput::pushLeftFrame(uint8_t** framePlanes, int* framePlaneSizes, int planes){
	_lFramePlanes = framePlanes;
	_lFramePlaneSizes = framePlaneSizes;
	_lPlanes = planes;
	_waitingQ++;

	cvM.notify_one();

	std::unique_lock<std::mutex> lck(mtx);
	while (!readyL) cv.wait(lck);
	readyL = false;

	// TODO Delete
	usleep(30 * 1000);

}
void MultiFileInput::pushRightFrame(uint8_t** framePlanes, int* framePlaneSizes, int planes){
	_rFramePlanes = framePlanes;
	_rFramePlaneSizes = framePlaneSizes;
	_rPlanes = planes;
	_waitingQ++;

	cvM.notify_one();
	
	std::unique_lock<std::mutex> lck(mtx);
	while (!readyR) cv.wait(lck);
	readyR = false;

	// TODO Delete
	usleep(30 * 1000);

}

/**
This function assumes that the left image and the right image has the same width and height values.
**/
void MultiFileInput::postFrame()
{
	while(!stopped){
		std::unique_lock<std::mutex> lck(mtxM);
		while (_waitingQ != 2) cvM.wait(lck);
		
		_observer->onFrameReceived(this->_id, _lFramePlanes, _lFramePlaneSizes, _lPlanes, _rFramePlanes, _rFramePlaneSizes, _rPlanes);

		readyL = readyR = true;
		_waitingQ = 0;
		cv.notify_all();
	}

}

// TODO Take a look later
void MultiFileInput::printStats(int code)
{
	//statusCallback(STATUS_INPUT_END);
}




