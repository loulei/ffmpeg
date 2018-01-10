#include "com_example_ffmpeg_FFmpegNative.h"
#include "native-audio-jni.h"

ANativeWindow *window;
char *videoFileName;
AVFormatContext *formatCtx;
int videoStream;
int audioStream;
AVCodecContext *codecCtx;
AVCodecContext *aCodecCtx;
AVFrame *decodedFrame;
AVFrame *frameRGBA;
AVFrame *aFrame;
jobject bitmap;
void *buffer;
struct SwsContext *sws_ctx;
int width;
int height;
int stop;
JavaVM *g_jvm;

JNIEXPORT jstring JNICALL Java_com_example_ffmpeg_FFmpegNative_urlprotocolinfo(
		JNIEnv *env, jobject obj) {
	char info[40000] = { 0 };
	av_register_all();
	struct URLProtocol *pup = NULL;
	struct URLProtocol **p_temp = &pup;
	avio_enum_protocols((void**) p_temp, 0);
	while ((*p_temp) != NULL) {
		sprintf(info, "%s[In ][%10s]\n", info,
				avio_enum_protocols((void**) p_temp, 0));
	}
	pup = NULL;
	avio_enum_protocols((void**) p_temp, 1);
	while ((*p_temp) != NULL) {
		sprintf(info, "%s[Out ][%10s]\n", info,
				avio_enum_protocols((void**) p_temp, 1));
	}
	return (*env)->NewStringUTF(env, info);
}

JNIEXPORT jstring JNICALL Java_com_example_ffmpeg_FFmpegNative_avformatinfo(
		JNIEnv *env, jobject obj) {
	char info[40000] = { 0 };
	av_register_all();
	AVInputFormat *if_temp = av_iformat_next(NULL);
	AVOutputFormat *of_temp = av_oformat_next(NULL);

	while (if_temp != NULL) {
		sprintf(info, "%s[In][%10s]\n", info, if_temp->name);
		if_temp = if_temp->next;
	}

	while (of_temp != NULL) {
		sprintf(info, "%s[Out][%10s]\n", info, of_temp->name);
		of_temp = of_temp->next;
	}
	return (*env)->NewStringUTF(env, info);
}

JNIEXPORT jstring JNICALL Java_com_example_ffmpeg_FFmpegNative_avcodecinfo(
		JNIEnv *env, jobject obj) {
	char info[40000] = { 0 };
	av_register_all();
	AVCodec *c_temp = av_codec_next(NULL);
	while (c_temp != NULL) {
		if (c_temp->decode != NULL) {
			sprintf(info, "%s[Dec]", info);
		} else {
			sprintf(info, "%s[Enc]", info);
		}
		switch (c_temp->type) {
		case AVMEDIA_TYPE_VIDEO:
			sprintf(info, "%s[Video]", info);
			break;
		case AVMEDIA_TYPE_AUDIO:
			sprintf(info, "%s[Audio]", info);
			break;
		default:
			sprintf(info, "%s[Other]", info);
			break;
		}
		sprintf(info, "%s[%10s]\n", info, c_temp->name);
		c_temp = c_temp->next;
	}
	return (*env)->NewStringUTF(env, info);
}

JNIEXPORT jstring JNICALL Java_com_example_ffmpeg_FFmpegNative_avfilterinfo(
		JNIEnv *env, jobject obj) {
	char info[40000] = { 0 };
	av_register_all();
	AVFilter *f_temp = (AVFilter*) avfilter_next(NULL);
	while (f_temp != NULL) {
		printf("filter:%s\n", f_temp->name);
		sprintf(info, "%s[%10s]\n", info, f_temp->name);
	}
	return (*env)->NewStringUTF(env, info);
}

JNIEXPORT jstring JNICALL Java_com_example_ffmpeg_FFmpegNative_configurationinfo(
		JNIEnv *env, jobject obj) {
	char info[40000] = { 0 };
	sprintf(info, "%s\n", avcodec_configuration());
	return (*env)->NewStringUTF(env, info);
}

void custom_log(void *ptr, int level, const char* fmt, va_list vl) {
	FILE *fp = fopen("/storage/emulated/0/av_log.txt", "a+");
	if (fp) {
		vfprintf(fp, fmt, vl);
		fflush(fp);
		fclose(fp);
	}
}

void SaveFrame(JNIEnv *pEnv, jobject pObj, jobject pBitmap, int width,
		int height, int iFrame) {
	char szFilename[200];
	jmethodID sSaveFrameMID;
	jclass mainActCls;
	sprintf(szFilename, "/mnt/sdcard/testframe/frame%d.jpg", iFrame);
	mainActCls = (*pEnv)->GetObjectClass(pEnv, pObj);
	sSaveFrameMID = (*pEnv)->GetMethodID(pEnv, mainActCls, "saveFrameToPath",
			"(Landroid/graphics/Bitmap;Ljava/lang/String;)V");
	LOGI("call java method to save frame %d", iFrame);
	jstring filePath = (*pEnv)->NewStringUTF(pEnv, szFilename);
	(*pEnv)->CallVoidMethod(pEnv, pObj, sSaveFrameMID, pBitmap, filePath);
	LOGI("call java method to save frame %d done", iFrame);
	(*pEnv)->DeleteLocalRef(pEnv, mainActCls);
	(*pEnv)->DeleteLocalRef(pEnv, filePath);
//	(*pEnv)->DeleteLocalRef(pEnv, sSaveFrameMID);
}

jobject createBitmap(JNIEnv *pEnv, int pWidth, int pHeight) {
	int i;
	//get Bitmap class and createBitmap method ID
	jclass javaBitmapClass = (jclass)(*pEnv)->FindClass(pEnv,
			"android/graphics/Bitmap");
	jmethodID mid = (*pEnv)->GetStaticMethodID(pEnv, javaBitmapClass,
			"createBitmap",
			"(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
	//create Bitmap.Config
	//reference: https://forums.oracle.com/thread/1548728
	const wchar_t* configName = L"ARGB_8888";
	int len = wcslen(configName);
	jstring jConfigName;
	if (sizeof(wchar_t) != sizeof(jchar)) {
		//wchar_t is defined as different length than jchar(2 bytes)
		jchar* str = (jchar*) malloc((len + 1) * sizeof(jchar));
		for (i = 0; i < len; ++i) {
			str[i] = (jchar) configName[i];
		}
		str[len] = 0;
		jConfigName = (*pEnv)->NewString(pEnv, (const jchar*) str, len);
	} else {
		//wchar_t is defined same length as jchar(2 bytes)
		jConfigName = (*pEnv)->NewString(pEnv, (const jchar*) configName, len);
	}
	jclass bitmapConfigClass = (*pEnv)->FindClass(pEnv,
			"android/graphics/Bitmap$Config");
	jobject javaBitmapConfig = (*pEnv)->CallStaticObjectMethod(pEnv,
			bitmapConfigClass,
			(*pEnv)->GetStaticMethodID(pEnv, bitmapConfigClass, "valueOf",
					"(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;"),
			jConfigName);
	//create the bitmap
	jobject localbitmap = (*pEnv)->CallStaticObjectMethod(pEnv, javaBitmapClass,
			mid, pWidth, pHeight, javaBitmapConfig);
	(*pEnv)->GetJavaVM(pEnv, &g_jvm);
	bitmap = (*pEnv)->NewGlobalRef(pEnv, localbitmap);
}

JNIEXPORT jint JNICALL Java_com_example_ffmpeg_FFmpegNative_test(JNIEnv *env,
		jobject obj, jobject activity) {
	AVFormatContext *pFormatCtx;
	int i, videoindex;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;
	AVFrame *pFrame, *pFrameRGBA;
	AVPacket *packet;
	int y_size;
	int ret, got_picture;
	struct SwsContext *img_convert_ctx;
	FILE *fp_yuv;
	int frame_cnt;
	clock_t time_start, time_finish;
	double time_duration = 0.0;
	jobject bitmap;
	void *buffer;

	char *input_str = "/mnt/sdcard/test.flv";
	char *output_str = "/mnt/sdcard/testframe.data";
	char info[1000] = { 0 };

	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	if (avformat_open_input(&pFormatCtx, input_str, NULL, NULL) != 0) {
		LOGE("Couldn't open input stream.\n");
		return -1;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		LOGE("Couldn't find stream information.\n");
		return -1;
	}

	videoindex = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
			break;
		}
	if (videoindex == -1) {
		LOGE("Couldn't find a video stream.\n");
		return -1;
	}
	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		LOGE("Couldn't find Codec.\n");
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		LOGE("Couldn't open codec.\n");
		return -1;
	}

	pFrame = av_frame_alloc();
	pFrameRGBA = av_frame_alloc();
	bitmap = createBitmap(env, pCodecCtx->width, pCodecCtx->height);
	if (AndroidBitmap_lockPixels(env, bitmap, &buffer) < 0)
		return -1;
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
			pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
			PIX_FMT_RGBA, SWS_BILINEAR, NULL, NULL, NULL);
	avpicture_fill((AVPicture*) pFrameRGBA, buffer, AV_PIX_FMT_RGBA,
			pCodecCtx->width, pCodecCtx->height);
	i = 0;
	packet = (AVPacket *) av_malloc(sizeof(AVPacket));
	time_start = clock();

	while (av_read_frame(pFormatCtx, packet) >= 0) {
		if (packet->stream_index == videoindex) {
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,
					packet);
			if (ret < 0)
				return -1;
			if (got_picture) {
				sws_scale(img_convert_ctx,
						(const uint8_t* const *) pFrame->data, pFrame->linesize,
						0, pCodecCtx->height, pFrameRGBA->data,
						pFrameRGBA->linesize);
				++i;
				SaveFrame(env, activity, bitmap, pCodecCtx->width,
						pCodecCtx->height, i);
				LOGD("save frame %d", i);
			}
		}
		av_free_packet(packet);
	}

	time_finish = clock();
	time_duration = (double) (time_finish - time_start);

	sprintf(info, "%s[Time      ]%fms\n", info, time_duration);
	sprintf(info, "%s[Count     ]%d\n", info, frame_cnt);
	LOGD("%s\n", info);
	sws_freeContext(img_convert_ctx);
	AndroidBitmap_unlockPixels(env, bitmap);

	av_frame_free(&pFrameRGBA);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;

}

JNIEXPORT jint JNICALL Java_com_example_ffmpeg_FFmpegNative_init(JNIEnv *env,
		jobject obj, jstring filename) {
	AVCodec *pCodec;
	AVCodec *aCodec;
	int i;
	int timeInSec = -1;
	videoFileName = (char*) (*env)->GetStringUTFChars(env, filename, NULL);
	LOGD("filename : %s", videoFileName);
	av_register_all();
	avformat_network_init();
	formatCtx = avformat_alloc_context();
	if (avformat_open_input(&formatCtx, videoFileName, NULL, NULL) != 0)
		return -1;
	if (avformat_find_stream_info(formatCtx, NULL) < 0)
		return -1;

	if (formatCtx->duration != AV_NOPTS_VALUE) {
		int hours, mins, secs, us;
		secs = formatCtx->duration / AV_TIME_BASE;
		timeInSec = secs;
		us = formatCtx->duration % AV_TIME_BASE;
		mins = secs / 60;
		secs %= 60;
		hours = mins / 60;
		mins %= 60;
		LOGD("total time : %02d:%02d:%02d.%02d", hours, mins, secs,
				(100*us)/AV_TIME_BASE);
	} else {
		LOGD("total time : N/A");
	}

	videoStream = -1;
	audioStream = -1;
	for (i = 0; i < formatCtx->nb_streams; i++) {
		if (formatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO
				&& videoStream < 0) {
			videoStream = i;
		}
		if (formatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO
				&& audioStream < 0) {
			audioStream = i;
		}
	}
	if (videoStream == -1)
		return -1;
	if (audioStream == -1)
		return -1;
	codecCtx = formatCtx->streams[videoStream]->codec;
	aCodecCtx = formatCtx->streams[audioStream]->codec;
	pCodec = avcodec_find_decoder(codecCtx->codec_id);
	aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
	if (codecCtx == NULL || aCodecCtx == NULL) {
		LOGD("unsupported codec");
		return -1;
	}
	if (avcodec_open2(codecCtx, pCodec, NULL) < 0)
		return -1;
	if (avcodec_open2(aCodecCtx, aCodec, NULL) < 0)
		return -1;
	decodedFrame = av_frame_alloc();
	aFrame = av_frame_alloc();
	frameRGBA = av_frame_alloc();
//	createEngine1();
	return timeInSec;
}

JNIEXPORT jintArray JNICALL Java_com_example_ffmpeg_FFmpegNative_getVideoSize(
		JNIEnv *env, jobject obj) {
	jintArray lRes;
	if (NULL == codecCtx) {
		return NULL;
	}
	lRes = (*env)->NewIntArray(env, 2);
	if (lRes == NULL)
		return NULL;
	jint lVideoRes[2];
	lVideoRes[0] = codecCtx->width;
	lVideoRes[1] = codecCtx->height;
	(*env)->SetIntArrayRegion(env, lRes, 0, 2, lVideoRes);
	return lRes;
}

JNIEXPORT void JNICALL Java_com_example_ffmpeg_FFmpegNative_setSurface(JNIEnv *env, jobject obj, jobject surface) {
	if(0 != surface) {
		window = ANativeWindow_fromSurface(env, surface);
		ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBA_8888);
	} else {
		ANativeWindow_release(window);
	}
}

JNIEXPORT jint JNICALL Java_com_example_ffmpeg_FFmpegNative_setup(JNIEnv *env,
		jobject obj, jint jwidth, jint jheight) {
	width = jwidth;
	height = jheight;
	bitmap = createBitmap(env, width, height);
	if (AndroidBitmap_lockPixels(env, bitmap, &buffer) < 0)
		return -1;
	sws_ctx = sws_getContext(codecCtx->width, codecCtx->height,
			codecCtx->pix_fmt, jwidth, jheight, AV_PIX_FMT_RGBA, SWS_BILINEAR,
			NULL, NULL, NULL);
	avpicture_fill((AVPicture*) frameRGBA, buffer, AV_PIX_FMT_RGBA, jwidth,
			jheight);
	return 0;
}

void finish(JNIEnv *env) {
	sws_freeContext(sws_ctx);
	//02-01 11:31:58.976: A/libc(7663): Fatal signal 11 (SIGSEGV), code 1, fault addr 0x98 in tid 8018 (.example.ffmpeg)
	if ((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) < 0) {
		AndroidBitmap_unlockPixels(env, bitmap);
	}
	//free buffer ...
	av_frame_free(&frameRGBA);
	av_frame_free(&decodedFrame);
	av_frame_free(&aFrame);
	avcodec_close(aCodecCtx);
	avcodec_close(codecCtx);
	avformat_close_input(&formatCtx);
	(*g_jvm)->DetachCurrentThread(g_jvm);
}

void* decodeAndRender(void *data) {
	JNIEnv *env = data;
	ANativeWindow_Buffer windowBuffer;
	AVPacket packet;
	int i = 0;
	int frameFinished;
	int lineCnt;
//	createBufferQueueAudioPlayer(aCodecCtx->sample_rate, aCodecCtx->channels, SL_PCMSAMPLEFORMAT_FIXED_16);
	while (av_read_frame(formatCtx, &packet) >= 0 && !stop) {
		if (packet.stream_index == videoStream) {
			avcodec_decode_video2(codecCtx, decodedFrame, &frameFinished,
					&packet);
			if (frameFinished) {
				sws_scale(sws_ctx, (const uint8_t * const *) decodedFrame->data,
						decodedFrame->linesize, 0, codecCtx->height,
						frameRGBA->data, frameRGBA->linesize);
				if (ANativeWindow_lock(window, &windowBuffer, NULL) < 0) {
					LOGE("lock window fail");
				} else {
//					LOGI("copy buffer %d:%d:%d", width, height, width*height*4);
//					LOGI("window buffer: %d:%d:%d", windowBuffer.width,
//							windowBuffer.height, windowBuffer.stride);
//					switch(decodedFrame->pict_type){
//					case AV_PICTURE_TYPE_I:
//						LOGD("frame type : I");
//						break;
//					case AV_PICTURE_TYPE_B:
//						LOGD("frame type : B");
//						break;
//					case AV_PICTURE_TYPE_P:
//						LOGD("frame type : P");
//						break;
//					}
					if (windowBuffer.width >= windowBuffer.stride) {
						memcpy(windowBuffer.bits, buffer, width * height * 4);
					} else {
						int j;
						for (j = 0; j < height; j++) {
							memcpy(
									windowBuffer.bits
											+ windowBuffer.stride * j * 4,
									buffer + width * j * 4, width * 4);
						}
					}

					ANativeWindow_unlockAndPost(window);
					++i;
				}
			}
		} else if (packet.stream_index == audioStream) {
//			avcodec_decode_audio4(aCodecCtx, aFrame, &frameFinished, &packet);
//			if (frameFinished) {
//				int data_size = av_samples_get_buffer_size(aFrame->linesize,
//						aCodecCtx->channels, aFrame->nb_samples,
//						aCodecCtx->sample_fmt, 1);
//				LOGD("size:%d channel:%d sample:%d rate:%d", data_size, aCodecCtx->channels, aFrame->nb_samples, aCodecCtx->sample_rate);
//				(*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, aFrame->data[0], data_size);
//			}
		}
//		usleep(5000);
		av_free_packet(&packet);
	}
	LOGD("total frames : %d", i);

	//crash
	finish(env);
}

JNIEXPORT void JNICALL Java_com_example_ffmpeg_FFmpegNative_play(JNIEnv *env, jobject obj) {
	pthread_t decodeThread;
	stop = 0;
	pthread_create(&decodeThread, NULL, decodeAndRender, env);
}

JNIEXPORT void JNICALL Java_com_example_ffmpeg_FFmpegNative_stop(JNIEnv *env, jobject obj) {
	stop = 1;
}

int seek_frame(int tsms) {
	int64_t frame;
	frame = av_rescale(tsms, formatCtx->streams[videoStream]->time_base.den,
			formatCtx->streams[videoStream]->time_base.num);
	frame /= 1000;
//	if(avformat_seek_file(formatCtx, videoStream, 0, frame, frame, AVSEEK_FLAG_FRAME) < 0){
//		return 0;
//	}
	if (av_seek_frame(formatCtx, videoStream, frame, AVSEEK_FLAG_FRAME) < 0) {
		return 0;
	}
	avcodec_flush_buffers(codecCtx);
	return 1;
}

JNIEXPORT void JNICALL Java_com_example_ffmpeg_FFmpegNative_seekAt(JNIEnv *env, jobject obj, jint secs) {
	seek_frame(secs*1000);
}

JNIEXPORT jstring JNICALL Java_com_example_ffmpeg_FFmpegNative_getMusicInfo(JNIEnv *env, jobject obj, jstring filename){
	int m, s;
	char info[1000];
	av_register_all();
//	avformat_network_init();
	formatCtx = avformat_alloc_context();
	AVCodec *aCodec;
	const char *file = (*env)->GetStringUTFChars(env, filename, 0);
	if(avformat_open_input(&formatCtx, file, NULL, NULL) != 0){
		LOGD("avformat_open_input error");
		return NULL;
	}
	if(avformat_find_stream_info(formatCtx, NULL) < 0){
		LOGD("avformat_find_stream_info error");
		return NULL;
	}

	int ret = av_find_best_stream(formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &aCodec, 0);
	LOGD("ret:%d", ret);
	aCodecCtx = formatCtx->streams[ret]->codec;

	if (formatCtx->duration != AV_NOPTS_VALUE) {
		s = formatCtx->duration / AV_TIME_BASE;
		LOGD("sec:%d", s);
		m = s / 60;
		s %= 60;
	} else {
		LOGD("total time : N/A");
	}
//	AVRational rr = formatCtx->streams[ret]->time_base;
//	d = (formatCtx->streams[ret]->duration) * av_q2d(rr);
//	m = (d % 3600)/60;
//	s = d % 60;
	snprintf(info, sizeof(info), "filename:%s\n"
								"duration:%d:%02d\n"
								"codec:%s\n"
								"channel:%d\n"
								"rate:%dHz\n"
								"bit:%dKbps",
								formatCtx->filename,
								m, s,
								aCodec->name,
								aCodecCtx->channels,
								aCodecCtx->sample_rate,
								aCodecCtx->bit_rate/1000);
	avcodec_close(aCodecCtx);
	avformat_close_input(&formatCtx);
	(*env)->ReleaseStringUTFChars(env, filename, file);
	return (*env)->NewStringUTF(env, info);
}



#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio
#define out_sample_fmt AV_SAMPLE_FMT_S16
SwrContext *au_convert_ctx;
uint8_t *out_buffer;
int out_buffer_size;
int out_sample_rate;
int out_channels;
AVPacket *ppacket;
int stream_index;
int isPause = 0;
int frameFinished = 0;
int initFinish = 1;
int isPlaying = 0;

void init_swr(){
	uint64_t out_channel_layout=AV_CH_LAYOUT_STEREO;
	//nb_samples: AAC-1024 MP3-1152
	out_sample_rate=aCodecCtx->sample_rate;
	out_channels=av_get_channel_layout_nb_channels(out_channel_layout);

	out_buffer=(uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE*out_channels);
	  //FIX:Some Codec's Context Information is missing
	int in_channel_layout=av_get_default_channel_layout(aCodecCtx->channels);
	//Swr
	au_convert_ctx = swr_alloc();
	swr_alloc_set_opts(au_convert_ctx,out_channel_layout, out_sample_fmt, out_sample_rate,
					 in_channel_layout, aCodecCtx->sample_fmt, aCodecCtx->sample_rate, 0, NULL);
	if(swr_init(au_convert_ctx)<0){
		au_convert_ctx=NULL;
	}
	createBufferQueueAudioPlayer(2, out_sample_rate);
}

//02-14 10:28:04.595: E/AudioTrack(29069): AudioTrack::set : Exit


void play_callback(){
	if(isPause){
		return;
	}
	if(av_read_frame(formatCtx, ppacket) >= 0){
		isPlaying = 1;
		if(ppacket->stream_index == stream_index){
			int ret = avcodec_decode_audio4(aCodecCtx, aFrame, &frameFinished, ppacket);
			if(frameFinished){
				int r = swr_convert(au_convert_ctx, &out_buffer, MAX_AUDIO_FRAME_SIZE, (const uint8_t **)aFrame->data, aFrame->nb_samples);
				out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, r, out_sample_fmt, 1);
				AudioWrite(out_buffer,out_buffer_size);
			}
		}
		isPlaying = 0;
	}
}

JNIEXPORT void JNICALL Java_com_example_ffmpeg_FFmpegNative_pause(JNIEnv *env, jobject obj){
	isPause = 1;
}

JNIEXPORT void JNICALL Java_com_example_ffmpeg_FFmpegNative_resume(JNIEnv *env, jobject obj){
	isPause = 0;
	play_callback();
}


JNIEXPORT void JNICALL Java_com_example_ffmpeg_FFmpegNative_release(JNIEnv *env, jobject obj){
//	usleep(100*1000);
	while(!initFinish){
		swr_free(&au_convert_ctx);
		av_free(aFrame);
		avcodec_close(aCodecCtx);
		avformat_close_input(&formatCtx);
	}

}


JNIEXPORT void JNICALL Java_com_example_ffmpeg_FFmpegNative_initSL(JNIEnv *env, jobject obj){
	createEngine();
	av_register_all();
	avformat_network_init();
	set_play_callback(play_callback);
}

JNIEXPORT void JNICALL Java_com_example_ffmpeg_FFmpegNative_playMusic(JNIEnv *env, jobject obj, jstring filename){

	Java_com_example_ffmpeg_FFmpegNative_release(env, obj);
	initFinish = 0;
	while(isPlaying){
		usleep(10*1000);
	}
	//call read_frame in callback
	formatCtx = avformat_alloc_context();
	AVCodec *aCodec;
	const char *file = (*env)->GetStringUTFChars(env, filename, 0);
	if(avformat_open_input(&formatCtx, file, NULL, NULL) != 0){
		LOGD("avformat_open_input error");
	}
	if(avformat_find_stream_info(formatCtx, NULL) < 0){
		LOGD("avformat_find_stream_info error");
	}

	stream_index = av_find_best_stream(formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &aCodec, 0);
	LOGD("stream_index:%d", stream_index);
	aCodecCtx = formatCtx->streams[stream_index]->codec;
	if (avcodec_open2(aCodecCtx, aCodec, NULL) < 0)
		LOGD("avcodec_open2 error");
	init_swr();
	initFinish = 1;
	(*env)->ReleaseStringUTFChars(env, filename, file);
	ppacket = (AVPacket*)av_malloc(sizeof(AVPacket));
	aFrame = av_frame_alloc();
	play_callback();
}

JNIEXPORT jint JNICALL Java_com_example_ffmpeg_FFmpegNative_findDecoder(JNIEnv *env, jobject obj, jint codecID){
	AVCodec *codec = NULL;
	av_register_all();
	codec = avcodec_find_decoder(codecID);
	if(codec != NULL){
		return 0;
	}else{
		return -1;
	}
}































