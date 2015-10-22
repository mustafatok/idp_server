CXX    ?= g++
CFLAGS  = -Wall -std=c++11 -pthread -Wno-literal-suffix -Wl,--no-undefined
LIBS    = -L/usr/lib/x86_64-linux-gnu -lm -lgio-2.0 -lgobject-2.0 -lxml2 -lgthread-2.0 -lglib-2.0 -lz -ldl -lbz2 -lcrypto -lopencv_core -lopencv_imgproc -lopencv_highgui
#LIBS   += $(shell pkg-config --cflags --libs libavcodec libavutil libavformat libavfilter libswscale)
LIBS   += $(shell export PKG_CONFIG_PATH=`pwd`/build/stage/lib/pkgconfig/ && pkg-config --cflags aravis-0.4 x264 libavcodec libavutil libavfilter libswscale opencv)
SRC	= src/
OBJS	= build/stage/lib/libaravis-0.4.a build/stage/lib/libx264.a build/stage/lib/libavformat.a \
          build/stage/lib/libavcodec.a build/stage/lib/libavdevice.a build/stage/lib/libavfilter.a \
          build/stage/lib/libavutil.a build/stage/lib/libswresample.a \
          build/stage/lib/libswscale.a build/stage/lib/libavformat.a

ARAVIS_SRC = git://git.gnome.org/aravis

# choose x264 version, 
# 0: original x264 rate control, 
# 1: Rho domain RC from Fan Zhang,
# 2: Rho domain RC from Fan Zhang adaptive,
# 3: Rho domain RC 
X264RC=1

X264_SRC   = git://git.videolan.org/x264.git
X264_VER   = a5831aa		# newest version on May, 22nd 2014
X264_FLAGS = --enable-static #--enable-debug

FFMPEG_SRC = git://git.videolan.org/ffmpeg.git
FFMPEG_VER = n2.2.4 #version currently on fedora 20 is 2.1.x
FFMPEG_FLAGS = --enable-static --disable-shared #--disable-pthreads

SOURCE  = $(SRC)*.cpp \
          $(SRC)input/*.cpp \
          $(SRC)output/*.cpp \
          $(SRC)encoder/*.cpp \
          $(SRC)tools/*.cpp

BINARY  = serverd
BINARYD = debug_serverd

all: libs server
clean: server_clean libs_clean

server:
	$(CXX) $(SOURCE) $(OBJS) $(CFLAGS) -o build/$(BINARY) -O3 $(LIBS) 

server_debug:
	$(CXX) $(SOURCE) $(OBJS) $(CFLAGS) -o build/$(BINARYD) -DDEBUG -g $(LIBS)

libs:
	mkdir -p build/stage
	mkdir -p build/src
	# aravis
	if [ ! -f build/src/aravis/autogen.sh ]; then \
		cd build/src/ && git clone $(ARAVIS_SRC) && cd aravis; \
	fi
	if [ ! -f build/stage/lib/libaravis-0.4.a ]; then \
		cd build/src/aravis/ && ./autogen.sh --prefix=`pwd`/../../stage/ && \
		   make && make install; \
	fi
	# ffmpeg
	if [ ! -f build/src/ffmpeg/configure ]; then \
		cd build/src/ && git clone $(FFMPEG_SRC) && cd ffmpeg && git checkout $(FFMPEG_VER); \
	fi
	if [ ! -f build/stage/lib/libavcodec.a ]; then \
		cd build/src/ffmpeg && ./configure --prefix=`pwd`/../../stage/ $(FFMPEG_FLAGS) && make && make install; \
	fi
	# x264
	if [ $(X264RC) -eq 0 ] && [ ! -f build/src/x264/configure ]; then \
		cd build/src/ && git clone $(X264_SRC) && cd x264 && git checkout $(X264_VER); \
	fi
	if [ $(X264RC) -eq 0 ]; then \
		cd build/src/x264/ && ./configure --prefix=`pwd`/../../stage/ $(X264_FLAGS) && make && make install; \
	fi
	if [ $(X264RC) -eq 1 ]; then \
		cd build/src/x264_rho_min/ && ./configure --prefix=`pwd`/../../stage/ $(X264_FLAGS) && make && make install; \
	fi
	if [ $(X264RC) -eq 2 ]; then \
		cd build/src/x264_rho_min_Ch_adaptive/ && ./configure --prefix=`pwd`/../../stage/ $(X264_FLAGS) && make && make install; \
	fi
	if [ $(X264RC) -eq 3 ]; then \
		cd build/src/RC_Intraframe_CSVT2009/ && ./configure --prefix=`pwd`/../../stage/ $(X264_FLAGS) && make && make install; \
	fi

libs_clean:
	# rm -rf ./build

server_clean:
	rm -f build/$(BINARY) build/$(BINARYD)

