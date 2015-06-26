#ifndef __INPUTOBSERVER_H
#define __INPUTOBSERVER_H
#include <stdint.h>

class InputObserver {
public:
        virtual void onSizeChanged(int id, int width, int height) = 0;
        virtual void onColorSpaceChanged(int id, int csp){};
        virtual void onFrameReceived(int id, uint8_t** framePlanes, int* framePlaneSizes, int planes) = 0;
        virtual void onFrameReceived(int id, uint8_t** lframePlanes, int* lframePlaneSizes, int lplanes, uint8_t** rframePlanes, int* rframePlaneSizes, int rplanes) = 0;
        virtual void onStatsCodeReceived(int id, int code) = 0;
};


#endif // __INPUTOBSERVER_H
