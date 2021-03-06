package com.clj.ffmpeglearndemo;

import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.FragmentActivity;

import android.Manifest;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.tbruyelle.rxpermissions2.RxPermissions;

import java.io.File;

import io.reactivex.functions.Consumer;

public class MainActivity extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener {
    private static final String TAG = "MainActivity";

    private NativePlayer player;
    private TextView stateTv;
    private SurfaceView mSurfaceView;

    private TextView timeTv;
    private SeekBar seekBar;
    private boolean isTouch;
    private int duration;//获取视频的总长度

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        stateTv = findViewById(R.id.tv_state);
        mSurfaceView = findViewById(R.id.surfaceView);
        timeTv = findViewById(R.id.tv_time);
        seekBar = findViewById(R.id.seekBar);
        seekBar.setOnSeekBarChangeListener(this);
        Log.d(TAG,NativePlayer.stringFromJNI());
        initNativePlayer();
        checkPermissionRequest(this);

    }

    @Override
    protected void onResume() {
        super.onResume();

    }

    @Override
    protected void onStop() {
        super.onStop();
        player.stop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        player.release();
    }

    private void initNativePlayer(){
        player = new NativePlayer();
        mSurfaceView.getHolder().addCallback(player);
        player.setDataSource( new File(Environment.getExternalStorageDirectory() + File.separator + "demo.mp4")
                .getAbsolutePath());
        player.setListener(new NativePlayer.OnPreparedListener() {
            @Override
            public void onPrepared() {
                duration = player.getDuration();
                runOnUiThread(() -> {
                    if(duration != 0){
                        timeTv.setText("00:00/"+getMinutes(duration)+":"+getSeconds(duration));
                        timeTv.setVisibility(View.VISIBLE);
                        seekBar.setVisibility(View.VISIBLE);
                    }
                    stateTv.setTextColor(Color.GREEN); // 绿色
                    stateTv.setText("恭喜init初始化成功");
                    Toast.makeText(MainActivity.this,"准备完成",Toast.LENGTH_SHORT).show();
                });
                player.start();
            }
        });
        player.setOnErrorListener(new NativePlayer.OnErrorListener() {
            @Override
            public void onEror(String msg) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        // Toast.makeText(MainActivity.this, "出错了，错误详情是:" + errorInfo, Toast.LENGTH_SHORT).show();

                        stateTv.setTextColor(Color.RED); // 红色
                        stateTv.setText("哎呀,错误啦，错误:" + msg);
                    }
                });
            }
        });
        player.setOnProgressListener(new NativePlayer.OnProgressListener() {
            @Override
            public void onProgress(int progress) {
                if(!isTouch){
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            if(duration != 0){
                                timeTv.setText(getMinutes(progress)+":"+getSeconds(progress)
                                +"/"+getMinutes(duration)+":"+getSeconds(duration));
                                seekBar.setProgress(progress*100/duration);
                            }
                        }
                    });
                }
            }
        });

    }

    public void checkPermissionRequest(FragmentActivity activity){

        RxPermissions rxPermissions = new RxPermissions(activity);
        rxPermissions.setLogging(true);
        rxPermissions.request(Manifest.permission.READ_EXTERNAL_STORAGE,
                Manifest.permission.WRITE_EXTERNAL_STORAGE)
        .subscribe(new Consumer<Boolean>() {
            @Override
            public void accept(Boolean aBoolean) throws Exception {
                Log.d(TAG,"checkPermissionRequest:"+aBoolean);
                if(aBoolean){
                    player.prepare();
                }
            }
        });
    }

    // 119 ---> 1.多一点点
    private String getMinutes(int duration) { // 给我一个duration，转换成xxx分钟
        int minutes = duration / 60;
        if (minutes <= 9) {
            return "0" + minutes;
        }
        return "" + minutes;
    }

    // 119 ---> 60 59
    private String getSeconds(int duration) { // 给我一个duration，转换成xxx秒
        int seconds = duration % 60;
        if (seconds <= 9) {
            return "0" + seconds;
        }
        return "" + seconds;
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if(fromUser){
            timeTv.setText(getMinutes(progress * duration / 100)
                    + ":" +
                    getSeconds(progress * duration / 100) + "/" +
                    getMinutes(duration) + ":" + getSeconds(duration));
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        isTouch = true;
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        isTouch = false;
        int seekBarProgress = seekBar.getProgress();
       int playProgress =  seekBarProgress*duration/100;
       player.seek(playProgress);
    }
}