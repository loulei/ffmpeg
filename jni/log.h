#ifndef _LOG_H_
#define _LOG_H_
#include <android/log.h>

#define LOGTAG "ffmpeg_native"

#define LOGV(fmt, ...) __android_log_print(ANDROID_LOG_VERBOSE, LOGTAG, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, LOGTAG, fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) __android_log_print(ANDROID_LOG_INFO, LOGTAG, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) __android_log_print(ANDROID_LOG_WARN, LOGTAG, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, LOGTAG, fmt, ##__VA_ARGS__)
#endif
