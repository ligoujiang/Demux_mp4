extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
}
#include <iostream>

class Decodec_Video{
private:
    //输入和输出参数变量
    const char* inFileName=nullptr;
    const char* outFileName=nullptr;
    //输入和输出参数变量
    int outWidth;
    int outHeight;
    //输入和输出文件
    FILE* in_fp=nullptr;
    FILE* out_fp=nullptr;
    //输入和输出句柄
    AVFormatContext* inFmtCtx=nullptr;
    AVFormatContext* outFmtCtx=nullptr;
    //packet
    AVPacket* pkt=nullptr;
    //frame
    AVFrame* frame=nullptr;
    //
    const char* encodecName=nullptr;
    //创建编码器句柄
    AVCodecContext* encodecCtx=nullptr;
public:
    ~Decodec_Video(){
        if(inFmtCtx){
            avformat_close_input(&inFmtCtx);
        }
    }
    //打开输入文件并打印文件信息
    bool openInput(char* inFileName){
        //打开输入文件
        int ret=avformat_open_input(&inFmtCtx,inFileName,NULL,NULL);
        if(ret<0){
            av_log(NULL,AV_LOG_ERROR,"avformat_open_input failed!\n");
            return false;
        }
        //获取文件信息
        ret=avformat_find_stream_info(inFmtCtx,NULL);
        if(ret<0){
            av_log(NULL,AV_LOG_ERROR,"avformat_find_stream_info failed!\n");
            return false;
        }
        //打印输出
        av_dump_format(inFmtCtx,0,inFileName,0);
        return true;
    }
    //打印输入文件的信息
    void getDuration(){
        av_log(NULL,AV_LOG_INFO,"音视频总流数：%d\n",inFmtCtx->nb_streams);
        av_log(NULL,AV_LOG_INFO,"视频流信息\n");
        av_log(NULL,AV_LOG_INFO,"分辨率：%dx%d\n",inFmtCtx->streams[0]->codecpar->width,inFmtCtx->streams[0]->codecpar->height);
        float time=inFmtCtx->duration*av_q2d(AV_TIME_BASE_Q);  //AV_TINE_BASE_Q为ffmpeg内部时间基 av_q2d转换成double类型
        av_log(NULL,AV_LOG_INFO,"时间：%fs\n",time);
        av_log(NULL,AV_LOG_INFO,"编码率：%dkb/s\n",inFmtCtx->streams[0]->codecpar->bit_rate/1000);
        av_log(NULL,AV_LOG_INFO,"音频流信息\n");
        av_log(NULL,AV_LOG_INFO,"采样率：%dHz\n",inFmtCtx->streams[1]->codecpar->sample_rate);
    }
    bool encodecVideo(){
        //指定编码器
        encodecName="AV_CODEC_ID_H264";
        const AVCodec* encodec=avcodec_find_encoder_by_name(encodecName);
        if(encodec==NULL){
            av_log(NULL,AV_LOG_ERROR,"avcodec_find_encoder_by_name failed!\n");
            return false;
        }
        //创建编码器句柄
        encodecCtx=avcodec_alloc_context3(encodec);
        if(encodecCtx==NULL){
            av_log(NULL,AV_LOG_ERROR,"avcodec_alloc_context3 failed!\n");
            return false;
        }
        //给编码器句柄设置参数
        //必设参数
        encodecCtx->width=outWidth;  //设置宽
        encodecCtx->height=outHeight;    //设置高
        encodecCtx->time_base=(AVRational){1,fps}; //时间基

        //获取源视频色彩格式（根据原视频格式指定）
        enum AVPixelFormat pixFmt=AV_PIX_FMT_YUV420P;
        //指定帧率
        int fps=25;
        encodecCtx->codec_type=AVMEDIA_TYPE_VIDEO; //编解码类型
        encodecCtx->pix_fmt=pixFmt; //视频色彩格式
        encodecCtx->framerate=(AVRational){fps,1};
        //encodecCtx->bit_rate=48000; //编码率
        encodecCtx->max_b_frames=0; //b帧数量
        encodecCtx->gop_size=25;   // I帧间隔

        //附加：当解码器为AV_CODEC_ID_H264，设置H264编解码器的一些参数
        if (encodec->id == AV_CODEC_ID_H264) {    
            // 相关的参数可以参考libx264.c的 AVOption options
            // ultrafast all encode time:2270ms
            // medium all encode time:5815ms
            // veryslow all encode time:19836ms
            ret = av_opt_set(codec_ctx->priv_data, "preset", "medium", 0);
            if(ret != 0) {
                printf("av_opt_set preset failed\n");
            }
            ret = av_opt_set(codec_ctx->priv_data, "profile", "main", 0); // 默认是high
            if(ret != 0) {
                printf("av_opt_set profile failed\n");
            }
            //ret = av_opt_set(codec_ctx->priv_data, "tune","zerolatency",0); // 直播时才使用该设置
            ret = av_opt_set(codec_ctx->priv_data, "tune","film",0); //  画质film
            if(ret != 0) {
                printf("av_opt_set tune failed\n");
            }
        }
        
        //初始化编解码句柄
        ret=avcodec_open2(encodecCtx,encodec,NULL);
        if(ret<0){
            av_log(NULL,AV_LOG_ERROR,"avcodec_open2 failed!\n");
            return false;
        }
        pkt=av_packet_alloc();
        if(pkt==nullptr){
            av_log(NULL,AV_LOG_ERROR,"av_packet_alloc failed!\n");
            return false;
        }
        frame=av_frame_alloc();
        if(frame==nullptr){
            av_log(NULL,AV_LOG_ERROR,"av_frame_alloc failed!\n");
            return false;
        }

        //frame必设参数
        frame->width=codecCtx->width;
        frame->height=codecCtx->height;
        frame->format=codecCtx->pix_fmt;

        //设置该参数将导致视频全是I帧，忽略gop_size
        //frame->pict_type=AV_PICTURE_TYPE_I;

        //申请视频帧数据空间
        ret=av_frame_get_buffer(frame,0);
        if(ret){
            av_log(NULL,AV_LOG_ERROR,"av_frame_get_buffer failed!\n");
            return false;
        }
    }
};




//视频编码，从本地读取YUV数据进行H264编码
int main(int argc,char** argv){
    Decodec_Video DV;
    DV.openInput(argv[1]);
    DV.getDuration();
    return 0;
}
