#include <iostream>
extern "C"{
#include <libavformat/avformat.h>
}

using namespace std;

//demux mp4 ->aac+h264
class Demux{
private:
    //输入输出参数1
    char* m_inFileName=nullptr;
    char* m_outFileName_H264=nullptr;
    char* m_outFileName_Aac=nullptr;
    //解封装句柄
    AVFormatContext* m_inFmtCtx=nullptr;
    //输出文件句柄
    FILE* m_fp_h264=nullptr;
    FILE* m_fp_aac=nullptr;
    AVPacket* m_pkt=nullptr;
    AVFrame* m_freame=nullptr;
public:
    Demux(char* argv1){
        m_inFileName=argv1;
    }
    Demux(char* argv1,char* argv2,char*argv3){
        m_inFileName=argv1;
        m_outFileName_H264=argv2;
        m_outFileName_Aac=argv3;
    }
    ~Demux(){
        if(m_inFmtCtx){
            avformat_close_input(&m_inFmtCtx);
            av_log(NULL,AV_LOG_INFO,"m_inFmtCtx已被析构！\n");
        }
        if(m_fp_h264){
            fclose(m_fp_h264);
            av_log(NULL,AV_LOG_DEBUG,"m_fp_h264已被析构！\n");
        }
        if(m_fp_aac){
            fclose(m_fp_aac);
            av_log(NULL,AV_LOG_DEBUG,"m_fp_acc已被析构！\n");
        }
        if(m_pkt){
            av_packet_free(&m_pkt);
            av_log(NULL,AV_LOG_DEBUG,"m_pkt已被析构！\n");
        }
    }
    bool DemuxMP4(){
        //设置打印级别
        //av_log_set_level(AV_LOG_DEBUG);
        //打开输入文件
        int ret =avformat_open_input(&m_inFmtCtx,m_inFileName,NULL,NULL);
        if(ret<0){
            av_log(NULL,AV_LOG_ERROR,"avformat_open_input failed!\n");
            return false;
        }
        ret=avformat_find_stream_info(m_inFmtCtx,NULL);
        if(ret<0){
            av_log(NULL,AV_LOG_ERROR,"avformat_find_stream_info failed!\n");
            return false;
        }
        //打印输入文件信息
        av_dump_format(m_inFmtCtx,0,m_inFileName,0);

        //打开输出文件
        m_fp_h264=fopen(m_outFileName_H264,"wb");
        if(!m_fp_h264){
            av_log(NULL,AV_LOG_ERROR,"open_m_fp_h264 failed!\n");
            return false;
        }
        m_fp_aac=fopen(m_outFileName_Aac,"wb");
        if(!m_fp_aac){
            av_log(NULL,AV_LOG_ERROR,"open_m_fp_aac failed!\n");
            return false;
        }
        //查找视频流
        int videoIndex=av_find_best_stream(m_inFmtCtx,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0); //查找视频流
        if(videoIndex==-1){
            av_log(NULL,AV_LOG_ERROR,"find videoIndex failed!\n");
            return false;
        }
        //查找音频流
        int audioIndex=av_find_best_stream(m_inFmtCtx,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0); //查找视频流
        if(audioIndex==-1){
            av_log(NULL,AV_LOG_ERROR,"find audioIndex failed!\n");
            return false;
        }

        //发包
        m_pkt=av_packet_alloc();
        while(true){
            ret=av_read_frame(m_inFmtCtx,m_pkt);
            if(ret<0){
                av_log(NULL,AV_LOG_ERROR,"av_read_frame failed!\n");
                break;
            }
            if(m_pkt->stream_index==AVMEDIA_TYPE_VIDEO){
                //处理视频
                av_packet_unref(m_pkt);
            }else if(m_pkt->stream_index==AVMEDIA_TYPE_AUDIO){
                //处理音频
                av_packet_unref(m_pkt);
            }
            av_packet_unref(m_pkt);
        }
        return true;
    }
};


int main(int argc,char** argv)
{
    Demux de(argv[1]);
    de.DemuxMP4();

    return 0;
}
