#include "h264encoder.h"

#include <assert.h>
#include <string.h>
#include <iostream>

#include "../global.h"
#include "../tools/psnr.h"

using namespace std;

//#define ANALYZER
#define LOGTIME

H264Encoder::H264Encoder()
{

}

H264Encoder::~H264Encoder()
{
        close();
}

void H264Encoder::close()
{
        if (_encoder != nullptr) {
                memset(&_inputPicture, 0, sizeof(x264_picture_t));
                memset(&_outputPicture, 0, sizeof(x264_picture_t));

                x264_encoder_close(_encoder);
                _encoder = nullptr;
                encodeCount = 0;
                frameCount = 0;
        }
}

void H264Encoder::stop(){
        close();
}

void H264Encoder::onSizeChanged(int id, int width, int height)
{
        Encoder::setSize(width, height);
        cout << "Video-Width: " << _width << endl;
        cout << "Video-Height: " << _height << endl;
        init();
}

void H264Encoder::init()
{
        if (_encoder != nullptr) {
                // already initialized
                return;
        }

        memset(&_parameters, 0, sizeof(x264_param_t));

#if X264_BUILD < 100
        x264_param_default( &_parameters );
        _parameters.i_threads = 1;
        _parameters.i_width = _width;
        _parameters.i_height = _height;
        _parameters.b_deterministic = 0; // cause for deviation?
        _parameters.i_fps_num = 60;
        _parameters.b_cabac = 0;
        _parameters.rc.i_bitrate = 1000; // 10kb
        _parameters.rc.f_rf_constant = 25;
        _parameters.b_repeat_headers = 1;
        _parameters.rc.i_rc_method = X264_RC_ABR;
        _parameters.analyse.i_me_method = X264_ME_DIA;
        _parameters.analyse.i_subpel_refine = 1;
        _parameters.b_pre_scenecut = 1;

        x264_picture_alloc(&_inputPicture, X264_CSP_I420, _width, _height);
        _encoder = x264_encoder_open(&_parameters);
#else
        x264_param_default_preset(&_parameters, "ultrafast", "zerolatency,fastdecode");
        x264_param_apply_fastfirstpass(&_parameters);
        _parameters.i_threads = 1; //4; don't use threads until RHO-Domain implementation supports it
        _parameters.i_width = _width;
        _parameters.i_height = _height;
        _parameters.b_deterministic = 0;

        // framerate related
        _parameters.i_fps_num = 60; // TODO: set correct framerate
        _parameters.b_intra_refresh = 1;

        _parameters.b_cabac = 0;

        // don't use bframes: (maximum number of b-frames between two reference frames)
        _parameters.i_bframe = 0;

        // use opencl:
        //_parameters.b_opencl = 1;

        // Rate Control algorithm:
        _parameters.rc.i_rc_method = X264_RC_ABR;
        _parameters.rc.b_stat_read = 0; // -> 1pass abr
        _parameters.rc.i_bitrate = 1000; // 10kb
        _parameters.rc.f_rf_constant = 25;
        _parameters.rc.f_rf_constant_max = 35;

        // add headers to every IDR frame, use format from annex b in h264 standard
        _parameters.b_repeat_headers = 1;
        _parameters.b_annexb = 1;

        // load basic profile which is best suited for low latency environment
        x264_param_apply_profile(&_parameters, "baseline");

        if (_csp == CSP_YUV420PLANAR) {
                x264_picture_alloc(&_inputPicture, X264_CSP_I420, _width, _height);
        } else { // CSP_RGB8PACKED
                //x264_picture_alloc(&_inputPicture, X264_CSP_RGB, _width, _height);
        }

        // open encoder context
        _encoder = x264_encoder_open(&_parameters);

#endif

        int ret = x264_encoder_headers(_encoder, &_nalHeaderUnits, &_nalHeaderCount);
        if (ret < 0) {
                cerr << __FILE__ << "Could not retrieve header information" << endl;
                return;
       }

        // sending the frame
#if X264_BUILD > 100
        this->postProcess(PROTOCOL_TYPE_HEADER, reinterpret_cast<uint8_t*>(_nalHeaderUnits[0].p_payload), ret);
#else
        this->postProcess(PROTOCOL_TYPE_HEADER, reinterpret_cast<uint8_t*>(_nalHeaderUnits[0].p_payload), 1);
#endif
        cout << "sent header with payload sizeee: " << ret << endl;
}

void H264Encoder::onFrameReceived(int id, uint8_t** framePlanes, int* framePlaneSizes, int planes)
{
        assert(_encoder != nullptr);
#ifdef ANALYZER
    static QualityAnalyzer qa;
#endif

        _inputPicture.i_pts = pts;
        _inputPicture.img.i_plane = planes;
#if X264_BUILD >= 76
        _inputPicture.param = &_parameters;
#endif

        for (int i = 0; i < planes; ++i) {
                _inputPicture.img.i_stride[i] = framePlaneSizes[i];
                _inputPicture.img.plane[i] = framePlanes[i];
        }
        frameCount++;

        int64_t et = 0;
        _timer.remember();
        int frame_size = x264_encoder_encode(_encoder, &_nalFrameUnits, &_nalFrameCount, &_inputPicture, &_outputPicture);
        et = _timer.diff_ys();

#if X264_BUILD >= 76
        if (frame_size > 0) {
                encodeCount += et;
#ifdef LOGTIME
                cout << "Encoding of frame took " << et << " ys." << endl;
#endif

#ifdef ANALYZER
            qa.decode_and_compare(framePlanes, framePlaneSizes, reinterpret_cast<uint8_t*>(_nalFrameUnits[0].p_payload), frame_size);
#endif
            this->postProcess(PROTOCOL_TYPE_FRAME, reinterpret_cast<uint8_t*>(_nalFrameUnits[0].p_payload), frame_size);
#else
        if (frame_size >= 0) {
                encodeCount += et;
#ifdef LOGTIME
                cout << "Encoding of frame took " << et << " ys." << endl;
#endif

                uint8_t *dataptr = data;
                int datasize = DATA_SIZE;

                for (int i = 0; i < _nalFrameCount; ++i)
                {
                    x264_nal_encode(dataptr, &datasize, 1, &_nalFrameUnits[i]);
                    if ( datasize > 0)
                    {
                        dataptr += datasize;
                    } else {
                        cout << "buffer to small" << endl;
                        return;
                    }
                }
#ifdef ANALYZER
        qa.decode_and_compare(framePlanes, framePlaneSizes, data, datasize);
#endif
        this->postProcess(PROTOCOL_TYPE_FRAME, reinterpret_cast<uint8_t*>(data), datasize); 
#endif
            ++pts;

        } else {
                cout << "Encoding has failed! Error-Code: " << frame_size << endl;
                return;
        }

}

void H264Encoder::onStatsCodeReceived(int id, int code)
{
        if (code == STATUS_INPUT_END && frameCount != 0) {
                cout << "Average encoding time: " << (encodeCount / frameCount) << endl;
                cout << "Total encoding time: " << encodeCount << endl;
                cout << "Total frame count: " << frameCount << endl;
                encodeCount = 0;
                frameCount = 0;
                close();
        }
}
