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
	this->_type = type;
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

		_fileInputL->stop();
		_fileInputR->stop();

		Input::stop();
		cvM.notify_one();
		
		if(_mThread != nullptr){
			_mThread->join();
			delete _mThread;
			cout << "MJOIN" << endl;
			cv.notify_all();

		}
		Input::stop();

		if(_lThread != nullptr){
			_lThread->join();
			delete _lThread;
			cout << "LJOIN" << endl;
			cv.notify_all();

		}
		Input::stop();
		
		if(_rThread != nullptr){
			_rThread->join();
			delete _rThread;
			cout << "RJOIN" << endl;
			cv.notify_all();


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
}

void MultiFileInput::setRightSize(int width, int height)
{
	_rWidth = width;
	_rHeight = height;
}

void MultiFileInput::pushLeftFrame(uint8_t** framePlanes, int* framePlaneSizes, int planes){
    // cout << " PL " << endl;

	if(stopped) return;

	_lFramePlanes = framePlanes;
	_lFramePlaneSizes = framePlaneSizes;
	_lPlanes = planes;
	_waitingQ++;

	cvM.notify_one();

	std::unique_lock<std::mutex> lck(mtx);
	while (!readyL && !stopped) cv.wait(lck);
	readyL = false;

	if(_type == "file")
		usleep(40 * 1000);
    // cout << " PLE " << endl;


}
void MultiFileInput::pushRightFrame(uint8_t** framePlanes, int* framePlaneSizes, int planes){
    // cout << " PR " << endl;

	if(stopped) return;
	
	_rFramePlanes = framePlanes;
	_rFramePlaneSizes = framePlaneSizes;
	_rPlanes = planes;
	_waitingQ++;

	cvM.notify_one();
	
	std::unique_lock<std::mutex> lck(mtx);
	while (!readyR && !stopped) cv.wait(lck);
	readyR = false;

	if(_type == "file")
		usleep(40 * 1000);
    // cout << " PRE " << endl;


}

/**
This function assumes that the left image and the right image has the same width and height values.
**/
void MultiFileInput::postFrame()
{
	while(!stopped){
		std::unique_lock<std::mutex> lck(mtxM);
		while (_waitingQ != 2 && !stopped) cvM.wait(lck);
		
		if(stopped) return;
		_observer->onFrameReceived(this->_id, _lFramePlanes, _lFramePlaneSizes, _lPlanes, _rFramePlanes, _rFramePlaneSizes, _rPlanes);
	
		readyL = readyR = true;
		_waitingQ = 0;
		cv.notify_all();
	}

}



