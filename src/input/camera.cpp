#include "camera.h"

#include <arv.h>
#include <stdlib.h>
#include <iostream>
#include <mutex>

extern "C" {
#include <libavutil/avconfig.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/log.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}

#include "../global.h" 

using namespace std;

std::mutex mtxCRead;
uint cntCRead = 0;

#define XBINNING ((SENSORWIDTH-WIDTH)/2)
#define YBINNING ((SENSORHEIGHT-HEIGHT)/2)

CameraInput::CameraInput(std::string inputCamera) : Input()
{
	cam_id = inputCamera;
	_xbinning = (SENSORWIDTH-WIDTH)/2;
	_ybinning = (SENSORHEIGHT-HEIGHT)/2;
}
CameraInput::CameraInput() : Input()
{

}

CameraInput::~CameraInput()
{

}

void CameraInput::operator()()
{
	mtxCRead.lock();
	stopped = false;
	if (strncmp(cam_id.c_str(), "Fake_1", 4) == 0) {
		arv_enable_interface("Fake");
	}

	camera = arv_camera_new(cam_id.c_str());

	if (camera == nullptr) {
		cerr << "Could not find a GenICam based camera called: " << cam_id << endl;
		return;
	}
	cout << cam_id << " " << camera << endl;
	arv_camera_set_region(camera, _xbinning, _ybinning, WIDTH, HEIGHT);
	_observer->onColorSpaceChanged(_id, PIX_FMT_YUV420P);
	_observer->onSizeChanged(_id, WIDTH, HEIGHT);
#if X264_BUILD < 100
	arv_camera_set_frame_rate(camera, 25);
#else
	arv_camera_set_frame_rate(camera, 25);
#endif
	arv_camera_set_pixel_format(camera, ARV_PIXEL_FORMAT_YUV_422_PACKED);

	double exposureTime = arv_camera_get_exposure_time (camera);
	cout << cam_id << " "<< "Exposure Time: " << exposureTime << endl;

	guint bufferSize = arv_camera_get_payload(camera);
	cout << cam_id << " " << "bufferSize: " << bufferSize << endl;

	stream = arv_camera_create_stream(camera, nullptr, nullptr);

	if (stream == nullptr) {
		cerr << cam_id << " " << "Could not create stream." << endl;
		return;
	}
	for (int i = 0; i < 50; i++) {
		arv_stream_push_buffer(stream, arv_buffer_new(bufferSize, nullptr));
	}

	if(bufferSize==0) exit(1);

	arv_camera_set_acquisition_mode(camera, ARV_ACQUISITION_MODE_CONTINUOUS);

	ArvBuffer *buffer = nullptr;

	SwsContext *swsContext = sws_getCachedContext(nullptr, WIDTH, HEIGHT, AV_PIX_FMT_UYVY422, WIDTH, HEIGHT, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

	int inputRowSize = WIDTH * (ARV_PIXEL_FORMAT_BIT_PER_PIXEL (ARV_PIXEL_FORMAT_YUV_422_PACKED) / 8);
	int inputRowSizes[] { inputRowSize, 0, 0, 0 };
	uint8_t *inputPlanes[] { nullptr, nullptr, nullptr, nullptr };

	int outputRowSizes[4] { 0 };
	uint8_t* outputPlanes[] { nullptr, nullptr, nullptr, nullptr };

	if (av_image_alloc(static_cast<uint8_t**>(outputPlanes), outputRowSizes, WIDTH, HEIGHT, PIX_FMT_YUV420P, 1) < 0) {
		cerr << "Could not allocate temporary image." << endl;
		return;
	}

	stopped = false;
	cntCRead++;
	mtxCRead.unlock();
	
	cout << "cntCRead" << " " << cntCRead << endl;

	while(cntCRead != 2){
		usleep(1000);
	}


	arv_camera_start_acquisition(camera);

	while (!stopped) {
		// cout << _id << " - R - 1 " << endl;

		buffer = arv_stream_timeout_pop_buffer(stream, 1000000);
		// cout << _id << " - R - 2 " << endl;


		if (buffer == nullptr) {
			cout << _id << " AQ " << endl;

			arv_camera_stop_acquisition(camera);
			arv_camera_start_acquisition(camera);

			continue;
		}

		if (arv_buffer_get_status(buffer) == ARV_BUFFER_STATUS_SUCCESS) { // the buffer contains a valid image
			size_t size;
			inputPlanes[0] = (uint8_t*) arv_buffer_get_data(buffer, &size);
			sws_scale(swsContext, static_cast<uint8_t**>(inputPlanes), inputRowSizes, 0, HEIGHT, static_cast<uint8_t**>(outputPlanes), outputRowSizes);
			_observer->onFrameReceived(_id, outputPlanes, outputRowSizes, 3);
			// cout << _id << " - R - 3 " << endl;

		}else{
			// cout << _id << " - R - 4 " << endl;

		}
		arv_stream_push_buffer(stream, buffer);
	}

	arv_camera_stop_acquisition (camera);
	g_object_unref (stream);
	g_object_unref (camera);
	sws_freeContext(swsContext);
	_observer->onStatsCodeReceived(_id, STATUS_INPUT_END);
	cntCRead = 0;

}



