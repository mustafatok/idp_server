#include "psnr.h"

#include <iostream>

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
#include <libavutil/frame.h>
}

using namespace std;

double do_psnr(AVFrame *frame, uint8_t **refPlanes, int *refSizes)
{
        int64_t total = 0;
        for ( int i = 0; i < 3; ++i ) {
                int64_t c = 0;
                //cout << "refSizes[i]: " << refSizes[i] << ", frame->linesize[i]: " << frame->linesize[i] << endl;
                for (int j = 0; j < refSizes[i]; ++j) {
                        int diff = frame->data[i][j] - refPlanes[i][j];
                        c += diff * diff;
                }
                total += c / refSizes[i];
        }

        return 10.0 * log( 65025.0 / total) / log(10.0);
}

QualityAnalyzer::QualityAnalyzer()
{
        avcodec_register_all();
        init();
}

QualityAnalyzer::~QualityAnalyzer()
{
        if (codecContext != nullptr) {
                avcodec_close(codecContext);
                codecContext = nullptr;
        }
}

void QualityAnalyzer::init()
{
        // init decoder
        picture = av_frame_alloc();
        if (!picture) {
                cerr << "Could not allocate temporary picture for decoder." << endl;
                return;
        }

        codecContext = avcodec_alloc_context3(codec);
        codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        if (!codec) {
                cerr << "Could not find codec." << endl;
                return;
        }

        if (avcodec_open2(codecContext, codec, nullptr) < 0) {
                cerr << "Could not open codec." << endl;
                return;
        }

        av_init_packet(&packet);
}

void QualityAnalyzer::cleanup()
{
        // TODO
}

double QualityAnalyzer::decode_and_compare ( uint8_t **planes, int *planesizes,
                                            uint8_t *data, int size )
{
        // decode compressed frames
        if (codec == nullptr || codecContext == nullptr) {
                cerr << "Decoder not initialized!" << endl;
                return -1.0;
        }

        int gotPicture = 0;

        packet.data = data;
        packet.size = size;

        int res = avcodec_decode_video2(codecContext, picture, &gotPicture, &packet);
        if (res < 0) {
                cerr << "Could not decode frame data!" << endl;
                return -1.0;
        }

        if (gotPicture) {
                // picture is an AVFrame !!
                double psnr = do_psnr(picture, planes, planesizes);
                cout << "PSNR: " << psnr << endl;
                return psnr;
        }

        av_frame_unref(picture);

        return -1.0;
}





