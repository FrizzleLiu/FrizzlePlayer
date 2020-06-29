package com.frizzle.frizzleplayer;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * 将绘制工作交给NDK
 * 但是不能将SurfaceView对象交给NDK,因为C++中没有SurfaceView,实现SurfaceHolder.Callback接口
 */
public class FrizzlePlayer implements SurfaceHolder.Callback{
    static {
        System.loadLibrary("frizzleplayer");
    }

    private SurfaceHolder surfaceHolder;

    public void setSurfaceView(SurfaceView surfaceView){
        if (null != surfaceHolder){
            surfaceHolder.removeCallback(this);
        }
        surfaceHolder=surfaceView.getHolder();
        surfaceHolder.addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {
        this.surfaceHolder=surfaceHolder;
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

    }


    /**
     * @param absolutePath  //surface是SurfaceView的画布,需要将surface传递给NDK
     *                      播放
     */
    public void start(String absolutePath) {
        native_start(absolutePath,surfaceHolder.getSurface());
    }

    public native void native_start(String absolutePath, Surface surface);
}
