package com.example.ffmpeg;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;

public class MusicPlay extends Activity implements OnClickListener {
	
	private ListView lv_music;
	private Button btn_play;
	private Button btn_stop;
	
	private List<File> musicList;
	private MusicAdapter musicAdapter;
	private static final String MUSIC_FOLDER = Environment.getExternalStorageDirectory().getPath() + File.separator + "music";
	
	private static final String MUSIC_SUBNAME[]={
		".ape",
		".flac",
		".wav",
		".mp3",
		".mp2",
		".aac",
		".wav",
		".wma",
	};
	
	private static final int PROC_INIT_MUSIC = 1;
	private static final int PROC_INIT_LIST = 2;

	private Handler handler = new Handler(){
		public void handleMessage(android.os.Message msg) {
			switch (msg.what) {
			case PROC_INIT_MUSIC:
				initMusic();
				break;
				
			case PROC_INIT_LIST:
				FFmpegNative.initSL();
				initList();
				break;

			default:
				break;
			}
		};
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_music);
		
		lv_music = (ListView) findViewById(R.id.lv_music);
		btn_play = (Button) findViewById(R.id.btn_play);
		btn_stop = (Button) findViewById(R.id.btn_stop);
		

		btn_play.setOnClickListener(this);
		btn_stop.setOnClickListener(this);
		handler.sendEmptyMessage(PROC_INIT_MUSIC);
	}
	
	class MusicAdapter extends BaseAdapter{
		
		private OnMusicClickListener onMusicClickListener;

		@Override
		public int getCount() {
			// TODO Auto-generated method stub
			return musicList == null ? 0 : musicList.size();
		}

		@Override
		public Object getItem(int position) {
			// TODO Auto-generated method stub
			return musicList.get(position);
		}

		@Override
		public long getItemId(int position) {
			// TODO Auto-generated method stub
			return position;
		}

		@Override
		public View getView(final int position, View convertView, ViewGroup parent) {
			// TODO Auto-generated method stub
			ViewHolder holder = null;
			if(convertView == null){
				holder = new ViewHolder();
				convertView = LayoutInflater.from(MusicPlay.this).inflate(R.layout.item_music, null);
				holder.tv_music = (TextView) convertView.findViewById(R.id.tv_music);
				holder.btn_info = (Button) convertView.findViewById(R.id.btn_info);
				convertView.setTag(holder);
			}else{
				holder = (ViewHolder) convertView.getTag();
			}
			holder.tv_music.setText(musicList.get(position).getName());
			holder.btn_info.setOnClickListener(new OnClickListener() {
				
				@Override
				public void onClick(View v) {
					// TODO Auto-generated method stub
					if(onMusicClickListener != null){
						onMusicClickListener.onMusicClicked(musicList.get(position), v);
					}
				}
			});
			
			holder.tv_music.setOnClickListener(new OnClickListener() {
				
				@Override
				public void onClick(View v) {
					// TODO Auto-generated method stub
					if(onMusicClickListener != null){
						onMusicClickListener.onMusicClicked(musicList.get(position), v);
					}
				}
			});
			return convertView;
		}
		
		public OnMusicClickListener getOnMusicClickListener() {
			return onMusicClickListener;
		}

		public void setOnMusicClickListener(OnMusicClickListener onMusicClickListener) {
			this.onMusicClickListener = onMusicClickListener;
		}

		class ViewHolder{
			public TextView tv_music;
			public Button btn_info;
		}
		
	}
	
	public interface OnMusicClickListener{
		void onMusicClicked(File music, View view);
	}
	
	private void initMusic(){
		final File musicFolder = new File(MUSIC_FOLDER);
		musicList = new ArrayList<File>();
		handler.post(new Runnable() {
			@Override
			public void run() {
				// TODO Auto-generated method stub
				if(musicFolder.exists()){
					loadMusic(musicFolder);
					handler.sendEmptyMessage(PROC_INIT_LIST);
				}
			}
		});
	}
	
	private void initList(){
		musicAdapter = new  MusicAdapter();
		musicAdapter.setOnMusicClickListener(new OnMusicClickListener(){
			@Override
			public void onMusicClicked(File music, View view) {
				// TODO Auto-generated method stub
				System.out.println(music.getName());
				if(view instanceof Button){
					String info = FFmpegNative.getMusicInfo(music.getAbsolutePath());
					new AlertDialog.Builder(MusicPlay.this).setMessage(info).create().show();
				}else if(view instanceof TextView){
					System.out.println("text click");
					FFmpegNative.playMusic(music.getAbsolutePath());
				}
				
			}
		});
		lv_music.setAdapter(musicAdapter);
	}
	
	private void loadMusic(File root){
		for(File file : root.listFiles()){
			if(file.isDirectory()){
				loadMusic(file);
			}else{
				for(String sub : MUSIC_SUBNAME){
					if(file.getName().endsWith(sub)){
						musicList.add(file);
					}
				}
			}
		}
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
		case R.id.btn_play:
			FFmpegNative.resume();
			break;
		case R.id.btn_stop:
			FFmpegNative.pause();
			break;

		default:
			break;
		}
	}
	
	
}
