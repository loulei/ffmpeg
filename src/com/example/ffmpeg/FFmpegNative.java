package com.example.ffmpeg;

import android.view.Surface;

public class FFmpegNative {
	static {
		System.loadLibrary("avutil-54");
		System.loadLibrary("avcodec-56");
		System.loadLibrary("swresample-1");
		System.loadLibrary("avformat-56");
		System.loadLibrary("swscale-3");
		System.loadLibrary("avfilter-5");
		System.loadLibrary("ffmpeg_codec");
	}

	public static native String urlprotocolinfo();
	public static native String avformatinfo();
	public static native String avcodecinfo();
	public static native String avfilterinfo();
	public static native String configurationinfo();
	public static native int test(MainActivity activity);
	
	public static native int init(String filename);
	public static native int[] getVideoSize();
	public static native void setSurface(Surface surface);
	public static native int setup(int width, int height);
	public static native int finish();
	public static native void play();
	public static native void stop();
	
	public static native void seekAt(int second);
	
	public static native String getMusicInfo(String filename);
	public static native void playMusic(String filename);
	public static native void release();
	public static native void pause();
	public static native void resume();
	public static native void initSL();
	
	public static native int findDecoder(int codeId);
}
























