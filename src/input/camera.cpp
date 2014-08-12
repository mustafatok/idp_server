#include "camera.h"

#include <arv.h>
#include <stdlib.h>
#include <iostream>

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

const char* CAMERA = "Allied Vision Technologies-50-0536872642";
//const char* CAMERA = "Fake_1";
const int WIDTH = 1280;
const int HEIGHT = 720;
const int SENSORWIDTH = 2048;
const int SENSORHEIGHT = 1088;

#define XBINNING ((SENSORWIDTH-WIDTH)/2)
#define YBINNING ((SENSORHEIGHT-HEIGHT)/2)

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
        if (strncmp(CAMERA, "Fake_1", 4) == 0) {
                arv_enable_interface("Fake");
        }

        camera = arv_camera_new(nullptr);

        if (camera == nullptr) {
                cerr << "Could not find a GenICam based camera called: " << CAMERA << endl;
                return;
        }

        arv_camera_set_region(camera, 0, 0, WIDTH, HEIGHT);
        arv_camera_set_binning (camera, XBINNING, YBINNING);
        colorspaceCallback(CSP_YUV420PLANAR);
        sizeCallback(WIDTH, HEIGHT);
#if X264_BUILD < 100
        arv_camera_set_frame_rate(camera, 10);
#else
	arv_camera_set_frame_rate(camera, 50);
#endif
        arv_camera_set_pixel_format(camera, ARV_PIXEL_FORMAT_YUV_422_PACKED);

        double exposureTime = arv_camera_get_exposure_time (camera);
        cout << "Exposure Time: " << exposureTime << endl;

        guint bufferSize = arv_camera_get_payload(camera);

        stream = arv_camera_create_stream(camera, nullptr, nullptr);

        if (stream == nullptr) {
                cerr << "Could not create stream." << endl;
                return;
        }
        cout << "bufferSize: " << bufferSize << endl;
        for (int i = 0; i < 50; i++) {
                arv_stream_push_buffer(stream, arv_buffer_new(bufferSize, nullptr));
        }

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


        while (!stopped) {
                usleep(100); // 100 us


                buffer = arv_stream_pop_buffer(stream);
                //arv_camera_set_timestamp_control_latch(camera);
                //cout << "Frame, GevTimestampValue:" << arv_camera_get_timestamp_value(camera) - timestamp << endl;

                if (buffer == nullptr) {
                        continue;
                }

                if (buffer->status == ARV_BUFFER_STATUS_SUCCESS) { // the buffer contains a valid image
                        inputPlanes[0] = reinterpret_cast<uint8_t*>(buffer->data);
                        sws_scale(swsContext, static_cast<uint8_t**>(inputPlanes), inputRowSizes, 0, HEIGHT, static_cast<uint8_t**>(outputPlanes), outputRowSizes);
                        frameCallback(outputPlanes, outputRowSizes, 3);
                }

                arv_stream_push_buffer(stream, buffer);
        }

        arv_camera_stop_acquisition (camera);
        g_object_unref (stream);
        g_object_unref (camera);
        sws_freeContext(swsContext);
        statusCallback(STATUS_INPUT_END);
}



