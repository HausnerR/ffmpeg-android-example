FFMPEG_LIB_PATH := ../../../../../ffmpeg-android-build/android-builds
FFMPEG_LIB_PLATFORM_VERSION := armv7-a-neon


LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ffmpeg-test
LOCAL_SRC_FILES := ffmpeg-test/ffmpeg-test.c

LOCAL_CFLAGS := -Wdeprecated-declarations

ANDROID_LIB := -landroid
LOCAL_LDLIBS += -llog -ljnigraphics -lz -landroid

LOCAL_C_INCLUDES := $(FFMPEG_LIB_PATH)/$(FFMPEG_LIB_PLATFORM_VERSION)/include
LOCAL_STATIC_LIBRARIES := libavdevice libavformat libavfilter libavcodec libwscale libavutil libswresample libswscale libpostproc

include $(BUILD_SHARED_LIBRARY)

$(call import-add-path, $(FFMPEG_LIB_PATH))
$(call import-module, $(FFMPEG_LIB_PLATFORM_VERSION))
