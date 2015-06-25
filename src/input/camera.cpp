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

// const char* CAMERA = "Allied Vision Technologies-50-0536874357";
// const char* CAMERA = "Allied Vision Technologies-50-0536872642";
// const char* CAMERA = "Fake_1";
const int WIDTH = 640;
const int HEIGHT = 480;
const int SENSORWIDTH = 2048;
const int SENSORHEIGHT = 1088;

std::mutex mtxRead;


#define XBINNING ((SENSORWIDTH-WIDTH)/2)
#define YBINNING ((SENSORHEIGHT-HEIGHT)/2)

CameraInput::CameraInput(std::string inputCamera) : Input()
{
        cam_id = inputCamera;
}
CameraInput::CameraInput() : Input()
{

}

CameraInput::~CameraInput()
{

}

void CameraInput::processFrame(ArvBuffer *buffer)
{

}

void CameraInput::newBuffer(ArvStream* stream)
{

}

void CameraInput::operator()()
{
        mtxRead.lock();
        if (strncmp(cam_id.c_str(), "Fake_1", 4) == 0) {
                arv_enable_interface("Fake");
        }

        camera = arv_camera_new(cam_id.c_str());

        if (camera == nullptr) {
                cerr << "Could not find a GenICam based camera called: " << cam_id << endl;
                return;
        }
        cout << cam_id << " " << camera << endl;
        arv_camera_set_region(camera, 0, 0, WIDTH, HEIGHT);
        arv_camera_set_binning (camera, XBINNING, YBINNING);
        _encoder->setColorspace(_id, CSP_YUV420PLANAR);
        _encoder->setSize(_id, WIDTH, HEIGHT);
#if X264_BUILD < 100
        arv_camera_set_frame_rate(camera, 10);
#else
	arv_camera_set_frame_rate(camera, 50);
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
        //arv_camera_set_timestamp_control_reset(camera);

        arv_camera_set_acquisition_mode(camera, ARV_ACQUISITION_MODE_CONTINUOUS);
        arv_camera_start_acquisition(camera);

        //arv_camera_set_timestamp_control_reset(camera);
        //arv_camera_set_timestamp_control_latch(camera);
        //int timestamp = arv_camera_get_timestamp_value(camera);


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
        mtxRead.unlock();


        while (!stopped) {

                // usleep(100); // 100 us


                buffer = arv_stream_pop_buffer(stream);
                //arv_camera_set_timestamp_control_latch(camera);
                //cout << "Frame, GevTimestampValue:" << arv_camera_get_timestamp_value(camera) - timestamp << endl;

                if (buffer == nullptr) {
                        continue;
                }

                if (buffer->status == ARV_BUFFER_STATUS_SUCCESS) { // the buffer contains a valid image
                        inputPlanes[0] = reinterpret_cast<uint8_t*>(buffer->data);
                        sws_scale(swsContext, static_cast<uint8_t**>(inputPlanes), inputRowSizes, 0, HEIGHT, static_cast<uint8_t**>(outputPlanes), outputRowSizes);
                        _encoder->pushFrame(_id, outputPlanes, outputRowSizes, 3);
                }

                arv_stream_push_buffer(stream, buffer);

        }

        arv_camera_stop_acquisition (camera);
        g_object_unref (stream);
        g_object_unref (camera);
        sws_freeContext(swsContext);
        _encoder->printStats(_id, STATUS_INPUT_END);

}



