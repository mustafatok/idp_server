#ifndef __PSNR_H
#define __PSNR_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
}

class QualityAnalyzer {
public:
        explicit QualityAnalyzer();
        virtual ~QualityAnalyzer();

        void init();
        void cleanup();

        /**
         *  CSP: IYUV420
         *  return: psnr
         */
        double decode_and_compare ( uint8_t **planes, int* planesizes, // source frame
                                    uint8_t *data, int size );         //encoded frame

private:
        AVCodec *codec = nullptr;
        AVCodecContext *codecContext = nullptr;
        AVFrame *picture = nullptr;
        AVPacket packet;
};

#endif
