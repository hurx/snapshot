CC = g++
#CFLAGS  := -Wall -O3 -std=c++0x
FFMPEG_INC_PATH = /usr/local/ffmpeg/FFmpeg-4.2.1-build/include
FFMPEG_LIB_PATH = /usr/local/ffmpeg/FFmpeg-4.2.1-build/lib
FFMPEG_INC = -I$(FFMPEG_INC_PATH)
FFMPEG_LIB = -L$(FFMPEG_LIB_PATH)
FFMPEG_LIB_NAME = -lavcodec -lavformat -lswscale -lswresample -lavutil

snap:muxing.c
	#$(CC)  snap.c $(FFMPEG_INC) $(FFMPEG_LIB) $(FFMPEG_LIB_NAME) -o snap
	$(CC)  snap.c $(FFMPEG_INC) $(FFMPEG_LIB) $(FFMPEG_LIB_NAME) -fPIC -shared -o libsnap.so
test:test.c
	$(CC)  test.c $(FFMPEG_INC) -I. $(FFMPEG_LIB) $(FFMPEG_LIB_NAME) -L.  -lsnap -o test
