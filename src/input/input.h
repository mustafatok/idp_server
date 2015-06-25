#ifndef __INPUT_H
#define __INPUT_H

#include <functional>
#include <stdlib.h>

typedef void size_callback_t (int, int);
typedef void frame_callback_t (uint8_t** , int*, int);
typedef void status_callback_t (int);
typedef void colorspace_callback_t (int);

class Input {
protected:
        std::function<void(int,int)> sizeCallback;
        std::function<void(uint8_t**, int*, int)> frameCallback;
        std::function<void(int)> statusCallback;
        std::function<void(int)> colorspaceCallback;
public:
        explicit Input() = default;
        virtual ~Input() = default;
        virtual void operator()(){}
        /*!
         * This callback provides the size according to the header of your video file!
         */
        void setSizeCallback(size_callback_t* callback)
        {
                sizeCallback = callback;
        }

        template <typename ObjectType>
        void setSizeCallback(ObjectType *instance, void (ObjectType::*callback)(int, int))
        {
                sizeCallback = [=](int w, int h) {
                        (instance->*callback)(w, h);
                };
        }

        /*!
         * This callback provides the single frames!
         *
         * If you use this object in a separate thread you also
         * have to know that the thread loop (= decoding process)
         * won't continue until you return from the callback-handler.
         * -> You have to take care of thread synchronization on your own!
         *
         * The frame will be in YUV420p format!
         */
        void setFrameCallback(frame_callback_t* callback)
        {
                frameCallback = callback;
        }

        template <typename ObjectType>
        void setFrameCallback(ObjectType *instance, void (ObjectType::*callback)(uint8_t**, int*, int))
        {
                frameCallback = [=](uint8_t** d, int* s, int l) {
                        (instance->*callback)(d, s, l);
                };
        }

        void setStatusCallback(status_callback_t* callback)
        {
                statusCallback = callback;
        }

        template <typename ObjectType>
        void setStatusCallback(ObjectType *instance, void (ObjectType::*callback)(int))
        {
                statusCallback = [=](int code) {
                        (instance->*callback)(code);
                };
        }

        void setColorspaceCallback(colorspace_callback_t* callback)
        {
                colorspaceCallback = callback;
        }

        template <typename ObjectType>
        void setColorspaceCallback(ObjectType *instance, void (ObjectType::*callback)(int))
        {
                colorspaceCallback = [=](int csp) {
                        (instance->*callback)(csp);
                };
        }
};

#endif // __INPUT_H