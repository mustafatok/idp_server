#ifndef __CAMERA_H
#define __CAMERA_H

#include <arv.h>

#include "input.h"

class CameraInput : public Input {
public:
        explicit CameraInput(std::string inputCamera);
        explicit CameraInput();

        virtual ~CameraInput();

        void operator()();

        void stop() { stopped = true; }
        void processFrame(ArvBuffer* frameBuffer);
        void newBuffer(ArvStream* stream);
private:
        bool stopped = false;

        ArvCamera *camera;
        ArvStream *stream;
        ArvBuffer *buffer;

        std::string cam_id;
};

#endif // __CAMERA_H