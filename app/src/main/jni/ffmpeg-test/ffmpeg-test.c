#include "logjam.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <pthread.h>


#include <libavcodec/avcodec.h>
#include <libavcodec/jni.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

struct timespec timespec1, timespec2, diff;

// http://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/
struct timespec timespec_diff(struct timespec start, struct timespec end) {
    struct timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}

#define start_time_measure() \
        clock_gettime(CLOCK_MONOTONIC, &timespec1);

#define stop_time_measure(title) \
        clock_gettime(CLOCK_MONOTONIC, &timespec2); \
        diff = timespec_diff(timespec1, timespec2); \
        LOGI("%-30s timediff: %d.%09ld", title, (int) diff.tv_sec, (long int)diff.tv_nsec);


jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	LOGI("Loading native library compiled at " __TIME__ " " __DATE__);

    //Required for MediaCodec HW decoder
	if (av_jni_set_java_vm(vm, NULL) != 0) {
	    LOGI("av_jni_set_java_vm failed!");
	}

	return JNI_VERSION_1_6;
}


ANativeWindow* window;
ANativeWindow_Buffer window_buffer;

JNIEXPORT void JNICALL Java_pl_pachciarek_ffmpeg_1android_1example_FFmpegTest_setSurface(JNIEnv *env, jobject obj, jobject surface) {
    window = ANativeWindow_fromSurface(env, surface);
}


char *filePath = NULL;

JNIEXPORT void JNICALL Java_pl_pachciarek_ffmpeg_1android_1example_FFmpegTest_setFilePath(JNIEnv *env, jobject obj, jstring filePathJString) {
    if (filePath != NULL) {
        free(filePath);
    }

    if (filePathJString != NULL) {
        filePath = (char *) (*env)->GetStringUTFChars(env, filePathJString, 0);
        (*env)->DeleteLocalRef(env, filePathJString);
    }

    LOGI("File path: %s", filePath);
}


AVDictionary *avFormatOptions = NULL;
AVDictionary *avCodecOptions = NULL;


bool taskRunning;

JNIEXPORT void JNICALL Java_pl_pachciarek_ffmpeg_1android_1example_FFmpegTest_endNativeRendering(JNIEnv *env, jobject obj) {
    taskRunning = false;
}


JNIEXPORT jint JNICALL Java_pl_pachciarek_ffmpeg_1android_1example_FFmpegTest_startNativeRendering(JNIEnv *env, jobject obj) {
    if (taskRunning == true) {
        LOGI("Task running...");
        return 1;
    }

    taskRunning = true;


    int               err, i, videoStreamId;
    AVDictionaryEntry *e;
    AVFormatContext   *pFormatCtx = NULL;
    AVCodecContext    *pCodecCtx = NULL;
    AVCodec           *pCodec = NULL;
    AVFrame           *pFrame = NULL;
    AVFrame           *pFrameRGB = NULL;
    AVPacket          packet;
    int               frameFinished;


    start_time_measure();

    av_register_all();

    stop_time_measure("av_register_all");


    start_time_measure();

    if((err = avformat_open_input(&pFormatCtx, filePath, NULL, &avFormatOptions)) != 0) {
        LOGI("Couldn't open file. Error code: %d", err);
        taskRunning = false;
        return 2;
    }

    stop_time_measure("avformat_open_input");

    e = NULL;
    while ((e = av_dict_get(avFormatOptions, "", e, AV_DICT_IGNORE_SUFFIX))) {
        LOGI("avformat_open_input: option \"%s\" not recognized", e->key);
    }

    start_time_measure();

    if((err = avformat_find_stream_info(pFormatCtx, NULL)) < 0) {
        LOGI("Couldn't find stream information. Error code: %d", err);
        taskRunning = false;
        return 3;
    }

    stop_time_measure("avformat_find_stream_info");

    videoStreamId = -1;
    for(i = 0; i < pFormatCtx->nb_streams; i++) {
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamId = i;
            break;
        }
    }

    if(videoStreamId == -1) {
        LOGI("Can't find video stream");
        taskRunning = false;
        return 4;
    }

    start_time_measure();

    //Get codec context for the video stream and find decoder
    pCodecCtx = pFormatCtx->streams[videoStreamId]->codec;
    //pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    pCodec = avcodec_find_decoder_by_name("h264_mediacodec"); //HW acceleration works only if codec is selected manually

    stop_time_measure("avcodec_find_decoder");

    if(pCodec == NULL) {
        LOGI("Can't find codec for input stream");
        taskRunning = false;
        return 6;
    }

    LOGI("Codec: %s", pCodec->name);

    start_time_measure();

    if((err = avcodec_open2(pCodecCtx, pCodec, &avCodecOptions)) < 0) {
        LOGI("Can't open codec. Error code: %d", err);
        taskRunning = false;
        return 7;
    }

    stop_time_measure("avcodec_open2");

    e = NULL;
    while ((e = av_dict_get(avCodecOptions, "", e, AV_DICT_IGNORE_SUFFIX))) {
        LOGI("avcodec_open2: option \"%s\" not recognized", e->key);
    }

    start_time_measure();

    //Allocate video frame YUV and RGB
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    if(pFrame == NULL || pFrameRGB == NULL) {
        LOGI("Can't alloc memory for video frames");
        taskRunning = false;
        return 8;
    }

    stop_time_measure("av_frame_alloc");

    start_time_measure();

    //Initialize nativewindow buffer with stream size
    ANativeWindow_acquire(window);
    ANativeWindow_setBuffersGeometry(window, pCodecCtx->width, pCodecCtx->height, WINDOW_FORMAT_RGBA_8888);

    stop_time_measure("ANativeWindow init");

    start_time_measure();

    //Initialize scaler/YUV->RGB converter context
    struct SwsContext *sws_ctx = sws_getContext(
            pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, //convert from
            pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGBA, //convert to
            SWS_POINT, //https://lists.libav.org/pipermail/libav-user/2008-December/001931.html
            NULL,
            NULL,
            NULL
    );

    stop_time_measure("sws_getContext");

    LOGI("Initialized. Entering main loop...");

    i = 0;

    while (taskRunning && (av_read_frame(pFormatCtx, &packet) >= 0)) {
        //Is this a packet from the video stream?
        if(packet.stream_index == videoStreamId) {
            start_time_measure();

            //Decode video frame
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet); //== 0 is ok, else decoding error

            //Didn't get a video frame?
            if(!frameFinished) {
                continue;
            }

            //Lock native window
            if (ANativeWindow_lock(window, &window_buffer, NULL) != 0) {
                continue;
            }

            //Fill buffer
            av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, window_buffer.bits, AV_PIX_FMT_RGBA, window_buffer.width, window_buffer.height, 1);

            pFrameRGB->data[0] = window_buffer.bits;
            pFrameRGB->linesize[0] = window_buffer.stride * 4;

            //Convert the image from its native format to RGB
            sws_scale(
                    sws_ctx,
                    (const uint8_t * const *) pFrame->data,
                    pFrame->linesize,
                    0,
                    pCodecCtx->height,
                    pFrameRGB->data,
                    pFrameRGB->linesize
            );

            ANativeWindow_unlockAndPost(window);

            i++;

            clock_gettime(CLOCK_MONOTONIC, &timespec2);
            diff = timespec_diff(timespec1, timespec2);
            LOGI("decode and render frame %d timediff: %d.%09ld", i, (int) diff.tv_sec, (long int) diff.tv_nsec);
        }

        //Free the packet that was allocated by av_read_frame
        av_packet_unref(&packet);
    }

    LOGI("Leaving main loop. Cleanup...");

    // Free the RGB and YUV frame
    av_free(pFrameRGB);
    av_free(pFrame);

    // Close the codec
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);

    ANativeWindow_release(window);

    taskRunning = false;

	return 0;
}
