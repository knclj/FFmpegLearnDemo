package com.clj.ffmpeglearndemo;

import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.FragmentActivity;

import android.Manifest;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceView;
import android.widget.TextView;
import android.widget.Toast;

import com.tbruyelle.rxpermissions2.RxPermissions;

import java.io.File;

import io.reactivex.functions.Consumer;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";

    private NativePlayer player;
    private TextView stateTv;
    private SurfaceView mSurfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        stateTv = findViewById(R.id.tv_state);
        mSurfaceView = findViewById(R.id.surfaceView);
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
                runOnUiThread(() -> {
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
}