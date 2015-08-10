/*
 * Based on code written by Stefano Sabatini
 * http://www.ffmpeg.org/doxygen/trunk/demuxing_decoding_8c-example.html
 */

#include "file.h"

#include <cmath>
#include <functional>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <string.h>

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

FileInput::FileInput(std::string inputFile) : Input()
{
        _inputFile = inputFile;
        av_register_all();
}

FileInput::~FileInput()
{
        
}

void FileInput::operator()()
{
        stopped = false;
        decodeFile(); 
}

int32_t FileInput::decodeFile()
{
        int32_t ret = 0;
        int32_t gotFrame = 0;
        stopped = false;

        // open the input file and allocate the format context
        if (avformat_open_input(&formatContext, _inputFile.c_str(), nullptr, nullptr) <0) {
                cerr << "Opening input file failed!" << endl;
                return -1;
        }

        // retrieving stream information
        if (avformat_find_stream_info(formatContext, nullptr) < 0) {
                cerr << "Could not find stream information!" << endl;
                return -1;
        }

        if (openCodecContext(&streamId, formatContext, AVMEDIA_TYPE_VIDEO) >= 0) {
                stream = formatContext->streams[streamId];
                codecContext = stream->codec;
                
                ret = av_image_alloc(videoData, videoDataLinesize, codecContext->width,
                                     codecContext->height, codecContext->pix_fmt, 1);

                if (ret < 0) {
                        cerr << "Could not allocate raw video buffer." << endl;
                        goto end;
                }
                videoDataBuffersize = ret;
                
                _observer->onColorSpaceChanged(_id, PIX_FMT_YUV420P);
                _observer->onSizeChanged(_id, codecContext->width, codecContext->height);
        }

        av_dump_format(formatContext, 0, _inputFile.c_str(), 0);

        if (!stream) {
                cerr << "Could not detect video stream in file." << endl;
                goto end;
        }

        // initialize packet, set data to nullptr, let the demuxer fill it
        av_init_packet(&packet);
        packet.data = nullptr;
        packet.size = 0;

        // read from from the file
        while (av_read_frame(formatContext, &packet) >= 0) {
                AVPacket orig = packet;
                do {
                        ret = decodePacket(&gotFrame, false);
                        if (ret < 0) {
                                break;
                        }
                        packet.data += ret;
                        packet.size -= ret;
                } while(packet.size > 0 && !stopped);
                av_free_packet(&orig);
                if (stopped) {
                    goto end;
                }
        }

        // flush cached frames
        packet.data = nullptr;
        packet.size = 0;
        do {
                decodePacket(&gotFrame, true);
        } while(gotFrame);

        _observer->onStatsCodeReceived(_id, STATUS_INPUT_END);

end:
        // cleanup memory
        avcodec_close(codecContext);
        avformat_close_input(&formatContext);
        av_freep(&videoData[0]);
        stopped = false;
        return ret;
}

int32_t FileInput::decodePacket(int32_t *gotFrame, bool cached)
{
        int32_t ret = 0;
        int32_t decoded = packet.size;

        *gotFrame = 0;

        if (packet.stream_index == streamId) {
                frame = av_frame_alloc();
                ret = avcodec_decode_video2(codecContext, frame, gotFrame, &packet);
                if (ret < 0) {
                        av_frame_unref(frame);
                        cerr << "Error decoding video frame (" << ret << ")" << endl;
                        return ret;
                }

                if (*gotFrame) {
                        if(!stopped){
                            // ouput must  be _un_aligned(!)
                            av_image_copy(videoData, videoDataLinesize,
                                    (const uint8_t **)(frame->data), frame->linesize,
                                    codecContext->pix_fmt, codecContext->width, codecContext->height);
                            _observer->onFrameReceived(_id, static_cast<uint8_t**>(videoData), static_cast<int*>(videoDataLinesize), 3);
                  
                        }
                }
                av_frame_unref(frame);
                
        }

        return decoded;
}

int32_t FileInput::openCodecContext(int *streamId, AVFormatContext *formatContext, enum AVMediaType type)
{
        int32_t result = 0;
        AVStream *stream;
        AVCodecContext *codecContext = nullptr;
        AVCodec *codec = nullptr;
        AVDictionary *opts = nullptr;

        result = av_find_best_stream(formatContext, type, -1, -1, nullptr, 0);
        if (result < 0) {
                cerr << "Could not find " << av_get_media_type_string(type) << " in input file." << endl;
                return result;
        } else {
                *streamId = result;
                stream = formatContext->streams[*streamId];

                // looking for decoder for selected stream
                codecContext = stream->codec;
                codec = avcodec_find_decoder(codecContext->codec_id);
                if (!codec) {
                        cerr << "Failed to find " << av_get_media_type_string(type) << " codec." << endl;
                        return AVERROR(EINVAL);
                }
        
                // init decoders
                av_dict_set(&opts, "refcounted_frames", "1", 0);
                if ((result = avcodec_open2(codecContext, codec, &opts)) < 0) {
                        cerr << "Failed to open " << av_get_media_type_string(type) << " codec." << endl;        
                        return result;
                }
        }

        return 0;
}
