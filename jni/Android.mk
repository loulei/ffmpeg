LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE :=avcodec-56-prebuilt  
LOCAL_SRC_FILES :=libavcodec-56.so  
include $(PREBUILT_SHARED_LIBRARY)  
   
include $(CLEAR_VARS)  
LOCAL_MODULE :=avdevice-56-prebuilt  
LOCAL_SRC_FILES :=libavdevice-56.so  
include $(PREBUILT_SHARED_LIBRARY)  
   
include $(CLEAR_VARS)  
LOCAL_MODULE :=avfilter-5-prebuilt  
LOCAL_SRC_FILES :=libavfilter-5.so  
include $(PREBUILT_SHARED_LIBRARY)  
   
include $(CLEAR_VARS)  
LOCAL_MODULE :=avformat-56-prebuilt
LOCAL_SRC_FILES :=libavformat-56.so
include $(PREBUILT_SHARED_LIBRARY)
   
include $(CLEAR_VARS)
LOCAL_MODULE :=avutil-54-prebuilt
LOCAL_SRC_FILES :=libavutil-54.so
include $(PREBUILT_SHARED_LIBRARY)  
   
include $(CLEAR_VARS)
LOCAL_MODULE :=  avswresample-1-prebuilt
LOCAL_SRC_FILES :=libswresample-1.so
include $(PREBUILT_SHARED_LIBRARY)
   
include $(CLEAR_VARS)
LOCAL_MODULE :=  swscale-3-prebuilt
LOCAL_SRC_FILES :=libswscale-3.so
include $(PREBUILT_SHARED_LIBRARY)
   
include $(CLEAR_VARS)  
LOCAL_MODULE :=ffmpeg_codec
LOCAL_SRC_FILES :=com_example_ffmpeg_FFmpegNative.c native-audio-jni.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_LDLIBS := -llog -ljnigraphics -lz -landroid
LOCAL_LDLIBS += -lOpenSLES
LOCAL_SHARED_LIBRARIES:= avcodec-56-prebuilt avdevice-56-prebuilt avfilter-5-prebuilt avformat-56-prebuilt avutil-54-prebuilt  avswresample-1-prebuilt swscale-3-prebuilt
include $(BUILD_SHARED_LIBRARY)



























