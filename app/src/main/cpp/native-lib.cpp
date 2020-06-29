//#include <jni.h>

//#include <string>
//#include <android/native_window_jni.h>
//#include <zconf.h>
//#include <android/log.h>
//
//extern "C" {
//#include <libavcodec/avcodec.h>
//#include <libavformat/avformat.h>
//#include <libswscale/swscale.h>
//#include <libavutil/imgutils.h>
//}
#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <zconf.h>

extern "C"{
#include "libavcodec/avcodec.h"
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
}

#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"LC",FORMAT,##__VA_ARGS__);

extern "C" JNIEXPORT jstring JNICALL
Java_com_frizzle_frizzleplayer_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    return env->NewStringUTF(av_version_info());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_frizzle_frizzleplayer_FrizzlePlayer_native_1start(JNIEnv *env, jobject thiz,
                                                           jstring filePath, jobject surface) {
    const char *path = env->GetStringUTFChars(filePath, 0);
    //FFmpeg网络模块初始化
    avformat_network_init();
    //拿到AVFormatContext上下文,用于获取文件中的视频流、音频流
    AVFormatContext *avFormatContext = avformat_alloc_context();

    //1、打开URL
    AVDictionary *options = NULL;
    //设置超时3秒
    av_dict_set(&options, "timeout", "3000000", 0);

    //打开视频文件,第一个参数是上下文,第二个参数是路径支持本地路径和url地址,
    // 第三个参数表示输入参数可以填空(表示自动,视频默认),第四个参数是一个字典,相当于一个HashMap,可以存放很多配置参数,比如超时时间等
    int resultCode = avformat_open_input(&avFormatContext, path, NULL, &options);
    //返回值为0表示函数执行成功,其它为失败
    char buf[] = "";
    if (resultCode<0) {
        av_strerror(resultCode, buf, 1024);
        // LOGE("%s" ,inputPath)
        LOGE("Couldn't open file %s: %d(%s)", filePath, resultCode, buf);
        LOGE("打开视频文件失败")
        return;
    }

    int video_stream_index = -1;
    //解析文件中的流
    int code = avformat_find_stream_info(avFormatContext, NULL);
    if(code<0){
        LOGE("获取流信息失败")
    }
    //遍历流
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        //流的类型为视频流,codecpar为解码器
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }
    LOGE("获取视频流信息成功")
    //获取视频流索引,有视频的高度,宽度,延迟时间等信息,解码器可以解码视频和音频
    AVCodecParameters *codecpar = avFormatContext->streams[video_stream_index]->codecpar;
    //获取解码器
    AVCodec *avCodec = avcodec_find_decoder(codecpar->codec_id);
    //解码器的上下文
    AVCodecContext *avCodecContext = avcodec_alloc_context3(avCodec);
    //将解码器参数copy到解码器上下文
    avcodec_parameters_to_context(avCodecContext, codecpar);
    //打开解码器
    if(avcodec_open2(avCodecContext, avCodec, NULL)<0){
        LOGE("打开解码器失败")
    }
    //解码 获取yuv数据,FFmpeg中yuv数据是封装在AVPacket中的
    //申请AVPacket
    AVPacket *avPacket = (AVPacket *) av_malloc(sizeof(AVPacket));   //申请AVPacket
    av_init_packet(avPacket);
    //申请AVFrame
    AVFrame *avFrame = av_frame_alloc();//分配一个AVFrame结构体,AVFrame结构体一般用于存储原始数据，指向解码后的原始帧
    AVFrame *rgb_frame = av_frame_alloc();//分配一个AVFrame结构体，指向存放转换成rgb后的帧

    //缓存区
    uint8_t  *out_buffer= (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_RGBA,
                                                                  avCodecContext->width,avCodecContext->height));
    //与缓存区相关联，设置rgb_frame缓存区
    avpicture_fill((AVPicture *)rgb_frame,out_buffer,AV_PIX_FMT_RGBA,avCodecContext->width,avCodecContext->height);


    SwsContext* swsContext = sws_getContext(avCodecContext->width,avCodecContext->height,avCodecContext->pix_fmt,
                                            avCodecContext->width,avCodecContext->height,AV_PIX_FMT_RGBA,
                                            SWS_BICUBIC,NULL,NULL,NULL);
    //取到nativewindow
    ANativeWindow *aNativeWindow=ANativeWindow_fromSurface(env,surface);
    if (aNativeWindow==0){
        LOGE("获取ANativeWindow窗体失败")
    }

    //输出buffer
    ANativeWindow_Buffer outBuffer;
    int frameCount;
    int h =0;
    //从视频流中读取数据包,FFmpeg内部维持了一个视频流队列,就像拿着碗(Packet)去食堂打饭先把碗给食堂阿姨(send_packet),打好饭自取(receive_frame)
    while (av_read_frame(avFormatContext, avPacket) >= 0) {
        LOGE("解码 %d",avPacket->stream_index)
        LOGE("VideoIndex %d",video_stream_index)
        if (avPacket->stream_index == video_stream_index) {
            LOGE("视频流解码")
            avcodec_decode_video2(avCodecContext, avFrame, &frameCount, avPacket);
            //正在解码
            if (frameCount) {

                //设置底层窗体buffer缓冲区的大小
                ANativeWindow_setBuffersGeometry(aNativeWindow,avCodecContext->width,avCodecContext->height,WINDOW_FORMAT_RGBA_8888);
                //锁,防止其它线程操作
                ANativeWindow_lock(aNativeWindow, &outBuffer, NULL);
                //yuv数据转换成rgb数据
                //转换为rgb格式
                sws_scale(swsContext,(const uint8_t *const *)avFrame->data,avFrame->linesize,0,
                          avFrame->height,rgb_frame->data,
                          rgb_frame->linesize);
                //  rgb_frame是有画面数据
                uint8_t *dst= (uint8_t *) outBuffer.bits;
                //一行数据  拿到一行有多少个字节 RGBA
                int destStride=outBuffer.stride*4;
                //像素数据的首地址
                uint8_t * src=  rgb_frame->data[0];
                //  实际内存一行数量
                int srcStride = rgb_frame->linesize[0];
                for (int i = 0; i < avCodecContext->height; ++i) {
                    //将rgb_frame中每一行的数据复制给nativewindow
                    memcpy(dst + i * destStride,  src + i * srcStride, srcStride);
                }
                //解锁
                ANativeWindow_unlockAndPost(aNativeWindow);
                usleep(1000 * 16);
            }
        }
        av_free_packet(avPacket);
    }
    //释放资源
    ANativeWindow_release(aNativeWindow);
    avcodec_close(avCodecContext);
    avformat_free_context(avFormatContext);
    env->ReleaseStringUTFChars(filePath, path);
}