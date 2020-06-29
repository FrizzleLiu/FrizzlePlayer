package com.frizzle.frizzleplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    private Button btnOpen;
    private SurfaceView surfaceView;
    private SeekBar seekBar;
    private FrizzlePlayer frizzlePlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        // Example of a call to a native method
        btnOpen = findViewById(R.id.btn_open);
        surfaceView = findViewById(R.id.surface_view);
        seekBar = findViewById(R.id.seek_bar);
        frizzlePlayer = new FrizzlePlayer();
        frizzlePlayer.setSurfaceView(surfaceView);

        btnOpen.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                requestPerms();
            }
        });
    }

    private void requestPerms() {
        //权限,简单处理下
        if (Build.VERSION.SDK_INT>Build.VERSION_CODES.N) {
            String[] perms= {Manifest.permission.WRITE_EXTERNAL_STORAGE};
            if (checkSelfPermission(perms[0]) == PackageManager.PERMISSION_DENIED) {
                requestPermissions(perms,200);
            }else {
                openFile();
            }
        }
    }

    /**
     * 从sdk取文件,交给FrizzlePlayer处理
     */
    private void openFile() {
        File file = new File(Environment.getExternalStorageDirectory(), "input.mp4");
        frizzlePlayer.start(file.getAbsolutePath());
    }
}
