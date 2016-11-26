package com.example.ffmpeg;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.PixelFormat;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.Display;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.LinearLayout.LayoutParams;
import android.widget.RelativeLayout;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

public class MainActivity extends Activity implements OnClickListener, Callback, OnSeekBarChangeListener {
	
	private Button btn_protocol;
	private Button btn_format;
	private Button btn_codec;
	private Button btn_filter;
	private Button btn_config;
	private TextView tv_content;
	private Button btn_start;
	private Button btn_stop;
	
	private Button btn_fwd;
	private Button btn_back;

	private SurfaceView surfaceview;
	private int second;
	private SeekBar seekbar;
	private TextView tv_progress;
	
	private Button btn_music;
	private Button btn_find;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		btn_protocol = (Button) findViewById(R.id.btn_protocol);
		btn_format = (Button) findViewById(R.id.btn_format);
		btn_codec = (Button) findViewById(R.id.btn_codec);
		btn_filter = (Button) findViewById(R.id.btn_filter);
		btn_config = (Button) findViewById(R.id.btn_config);
		tv_content = (TextView) findViewById(R.id.tv_content);
		btn_start = (Button) findViewById(R.id.btn_start);
		btn_stop = (Button) findViewById(R.id.btn_stop);
		surfaceview = (SurfaceView) findViewById(R.id.surfaceview);
		
		btn_fwd = (Button) findViewById(R.id.btn_fwd);
		btn_back = (Button) findViewById(R.id.btn_back);
		seekbar = (SeekBar) findViewById(R.id.seekbar);
		tv_progress = (TextView) findViewById(R.id.tv_progress);
		
		btn_music = (Button) findViewById(R.id.btn_music);
		btn_find = (Button) findViewById(R.id.btn_find);
		
		surfaceview.getHolder().setFormat(PixelFormat.RGBA_8888);
		btn_protocol.setOnClickListener(this);
		btn_format.setOnClickListener(this);
		btn_codec.setOnClickListener(this);
		btn_filter.setOnClickListener(this);
		btn_config.setOnClickListener(this);
		btn_start.setOnClickListener(this);
		btn_stop.setOnClickListener(this);
		surfaceview.getHolder().addCallback(this);
		
		btn_fwd.setOnClickListener(this);
		btn_back.setOnClickListener(this);
		
		seekbar.setOnSeekBarChangeListener(this);
		btn_music.setOnClickListener(this);
		btn_find.setOnClickListener(this);
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		String content = null;
		switch (v.getId()) {
		case R.id.btn_protocol:
			content = FFmpegNative.urlprotocolinfo();
			break;
		case R.id.btn_format:
			content = FFmpegNative.avformatinfo();
			break;
		case R.id.btn_codec:
			content = FFmpegNative.avcodecinfo();
			break;
		case R.id.btn_filter:
			content = FFmpegNative.avfilterinfo();
			break;
		case R.id.btn_config:
			content = FFmpegNative.configurationinfo();
			break;
		case R.id.btn_start:
//			new Thread(new Runnable() {
//				
//				@Override
//				public void run() {
//					// TODO Auto-generated method stub
//					FFmpegNative.test(MainActivity.this);
//				}
//			}).start();
			second = FFmpegNative.init("/mnt/sdcard/test.mp4");
			System.out.println("init:"+second);
			seekbar.setMax(second);
			int[] size = FFmpegNative.getVideoSize();
			int[] screenSize = getScreenRes();
			int width, height;
			float widthScaledRatio = screenSize[0]*1.0F/size[0];
			float heightScaledRatio = screenSize[1]*1.0F/size[1];
			if(widthScaledRatio > heightScaledRatio){
				width = (int) (size[0]*heightScaledRatio);
				height = screenSize[1];
			}else{
				width = screenSize[0];
				height = (int) (size[1]*widthScaledRatio);
			}
			System.out.println(width+"*"+height);
			updateSurfaceView(width, height);
			FFmpegNative.setup(width, height);
			FFmpegNative.play();
			break;
		case R.id.btn_stop:
			FFmpegNative.stop();
			break;
		case R.id.btn_fwd:
			FFmpegNative.seekAt(500);
			break;
		case R.id.btn_back:
			FFmpegNative.seekAt(-5);
			break;
		case R.id.btn_music:
			Intent intent = new Intent(MainActivity.this, MusicPlay.class);
			startActivity(intent);
			break;
		case R.id.btn_find:
			System.out.println(FFmpegNative.findDecoder(28));
			break;
		default:
			break;
		}
		if(!TextUtils.isEmpty(content)){
			tv_content.setText(content);
		}
	}
	
	private int[] getScreenRes() {
		int[] res = new int[2];
		Display display = getWindowManager().getDefaultDisplay();
		res[0] = display.getWidth();  // deprecated
		res[1] = display.getHeight();  // deprecated
		return res;
	}
	
	private void updateSurfaceView(int pWidth, int pHeight) {
		//update surfaceview dimension, this will cause the native window to change
		LinearLayout.LayoutParams params = (LayoutParams) surfaceview.getLayoutParams();
		params.width = pWidth;
		params.height = pHeight;
		surfaceview.setLayoutParams(params);
	}
	
	private void saveFrameToPath(Bitmap bitmap, String pPath) {
		int BUFFER_SIZE = 1024 * 8;
	    try {
	    	File file = new File(pPath);
	        file.createNewFile();
	        FileOutputStream fos = new FileOutputStream(file);
	        final BufferedOutputStream bos = new BufferedOutputStream(fos, BUFFER_SIZE);
	        bitmap.compress(CompressFormat.JPEG, 100, bos);
	        bos.flush();
	        bos.close();
	        fos.close();
	    } catch (FileNotFoundException e) {
	        e.printStackTrace();
	    } catch (IOException e) {
	    	e.printStackTrace();
	    }
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		// TODO Auto-generated method stub
		FFmpegNative.setSurface(holder.getSurface());
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		// TODO Auto-generated method stub
		FFmpegNative.setSurface(null);
	}

	@Override
	public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
		// TODO Auto-generated method stub
		tv_progress.setText(progress+"s");
	}

	@Override
	public void onStartTrackingTouch(SeekBar seekBar) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onStopTrackingTouch(SeekBar seekBar) {
		// TODO Auto-generated method stub
		int progress = seekBar.getProgress();
		System.out.println(progress);
		FFmpegNative.seekAt(progress);
	}
}
