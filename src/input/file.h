#ifndef __FILE_H
#define __FILE_H

#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
}

#include "input.h"

class FileInput : public Input {
public:
        explicit FileInput(std::string inputFile);
        virtual ~FileInput();

        /*!
         * This function starts the decoding process.
         * The data will be reported by the callbacks.
         * The operator () is used, so that this class can be used with std::thread.
         */
        void operator()();

        void stop() { stopped = true; }

        
private:
        int32_t decodeFile();
        int32_t decodePacket(int32_t *gotFrame, bool cached);
        int32_t openCodecContext(int *streamId, AVFormatContext *formatContext, enum AVMediaType type);

        std::string _inputFile;

        AVCodec *codec;
        AVCodecContext *codecContext = nullptr;
        AVFormatContext *formatContext = nullptr;
        AVFrame *frame = nullptr;
        AVPacket packet;
        AVStream *stream = nullptr;
        int32_t streamId = 0;
        int32_t frameCount = 0;
        uint8_t *videoData[4] = {nullptr};
        int videoDataLinesize[4];
        int videoDataBuffersize;
        bool stopped = false;
};

#endif // __FILE_H
