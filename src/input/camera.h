#ifndef __CAMERA_H
#define __CAMERA_H

#include <arv.h>

#include "input.h"

const int WIDTH = 960;
const int HEIGHT = 544;
const int SENSORWIDTH = 2048;
const int SENSORHEIGHT = 1088;

class CameraInput : public Input {
public:
        explicit CameraInput(std::string inputCamera);
        explicit CameraInput();

        virtual ~CameraInput();

        void operator()();
        void setInputOffset(int x, int y){
        	_xbinning = (SENSORWIDTH-WIDTH)/2 + x;
        	_ybinning = (SENSORHEIGHT-HEIGHT)/2 + y;
			arv_camera_set_region(camera, _xbinning, _ybinning, WIDTH, HEIGHT);

        }
        void setInputOffsetX(int x){
                _xbinning = (SENSORWIDTH-WIDTH)/2 + x;
                if(camera != nullptr)arv_camera_set_region(camera, _xbinning, _ybinning, WIDTH, HEIGHT);
        }

private:

        ArvCamera *camera = nullptr;
        ArvStream *stream;
        ArvBuffer *buffer;

        int _xbinning;
        int _ybinning;

        std::string cam_id;
};

#endif // __CAMERA_H