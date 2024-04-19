#include <iostream>
extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/bsf.h>
#include <libavcodec/avcodec.h>
}

//解码音频 aac-->pcm or 主流格式-->pcm
class DecodeAudio{
private:
    //输入和输出的文件名
    char* m_inFileName=nullptr; 
    char* m_outFileName=nullptr;
    AVFormatContext* m_inFmtCtx=nullptr;    //输入文件句柄
    //保存输出文件
    FILE* m_outFp=nullptr;
    //packet
    AVPacket* m_pkt=nullptr;
    //frame
    AVFrame* m_frame=nullptr;
public:
    DecodeAudio(char* argv1,char* argv2){
        m_inFileName=argv1;
        m_outFileName=argv2;
    }
    ~DecodeAudio(){
        if(m_inFmtCtx){
            avformat_close_input(&m_inFmtCtx);
            m_inFmtCtx=nullptr;
            av_log(NULL,AV_LOG_DEBUG,"m_inFmtCtx已被析构！\n");
        }
        if(m_outFp){
            fclose(m_outFp);
            m_outFp=nullptr;
            av_log(NULL,AV_LOG_DEBUG,"m_outFp已被析构！\n");
        }
    }
    //打开输入文件
    bool openInput(){
        int ret=avformat_open_input(&m_inFmtCtx,m_inFileName,NULL,NULL);
        if(ret<0){
            av_log(NULL,AV_LOG_ERROR,"avformat_open_input failed!\n");
            return false;
        }
        ret=avformat_find_stream_info(m_inFmtCtx,NULL);
        if(ret<0){
            av_log(NULL,AV_LOG_ERROR,"avformat_find_stream_info failed!\n");
            return false;
        }
        return 0;
    };
    //音频解码
    bool decodeVideo(){
        //打开输出文件
        m_outFp=fopen(m_outFileName,"wb");
        if(!m_outFp){
            av_log(NULL,AV_LOG_ERROR,"open m_outFp failed!\n");
            return false;
        }

        //查找音频流
        int audioIndex=av_find_best_stream(m_inFmtCtx,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0); //查找音频流
        if(audioIndex==-1){
            av_log(NULL,AV_LOG_ERROR,"find audioIndex failed!\n");
            return false;
        }
        //查找音频解码器
        const AVCodec* codec=nullptr;
        //根据id查找源文件的解码器
        codec=avcodec_find_decoder(m_inFmtCtx->streams[audioIndex]->codecpar->codec_id);
        if(!codec){
            av_log(NULL,AV_LOG_ERROR,"find codec failed!\n");
            return false;
        }
        //解码器句柄
        AVCodecContext* m_codecCxt=nullptr;
        m_codecCxt=avcodec_alloc_context3(codec);
        if(!m_codecCxt){
            av_log(NULL,AV_LOG_ERROR,"avcodec_alloc_context3 failed!\n");
            return false;
        }
        //将源文件解码参数设置到解码器句柄
        avcodec_parameters_to_context(m_codecCxt,m_inFmtCtx->streams[audioIndex]->codecpar);

        //打开解码器
        int ret=avcodec_open2(m_codecCxt,codec,NULL);
        if(ret<0){
            av_log(NULL,AV_LOG_ERROR,"avcodec_open2 failed!\n");
            return false;
        }

        m_pkt=av_packet_alloc();
        m_frame=nullptr;
        frame=av_frame_alloc();
        while(1){
            //读数据包
            ret=av_read_frame(m_inFmtCtx,m_pkt); //读取一帧数据
            if(ret<0){
                break;
            }
            //音频流处理
            if(m_pkt->stream_index==audioIndex){
                savePCMDecode(m_codecCxt,m_pkt,frame,m_outFp,audioIndex);
            }
            av_packet_unref(m_pkt);
        }
        //刷新
        savePCMDecode(m_codecCxt,NULL,frame,m_outFp,audioIndex);
        return true;
    }

//发送数据包解码，将解码后的数据写入输出文件
void savePCMDecode(AVCodecContext* codecCtx, AVPacket* pkt, AVFrame* frame, FILE* file,int audioIndex) {
    //发送数据包去进行解析获得帧数据
    if (avcodec_send_packet(codecCtx, pkt) >= 0) {
        //接收解码后的帧数据
        while (avcodec_receive_frame(codecCtx, frame) >= 0) {
            /*
              Planar（平面），其数据格式排列方式为 (特别记住，该处是以点nb_samples采样点来交错，不是以字节交错）:
              [LLLLLLRRRRRR][LLLLLLRRRRRR][LLLLLLRRRRRR]...
              每个LLLLLLRRRRRR为一个音频帧
              而非Planar的数据格式（即交错排列）排列方式为：
              LRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRL...（每个LR为一个音频样本）
            */
            if (av_sample_fmt_is_planar(codecCtx->sample_fmt)) {    //判断是否为平面格式
                //pcm播放时是LRLRLR格式，所以要交错保存数据
                int numBytes = av_get_bytes_per_sample(codecCtx->sample_fmt);    //计算每个样本数占用的字节数  假设样本格式为 16 位有符号整数AV_SAMPLE_FMT_S16，那么占用2字节
                for (int i = 0; i < frame->nb_samples; i++) {
                    for (int ch = 0; ch < codecCtx->ch_layout.nb_channels; ch++) {
                        fwrite(frame->data[ch] + numBytes * i, 1, numBytes, file);
                    }
                }
            }else {
                fwrite(frame->data[0], 1, frame->linesize[0], file);
            }
        }
    }
}
};

// int main(int argc,char** argv){
//     DecodeAudio da(argv[1],argv[2]);
/      da.openInput();
//     da.decodeVideo();
//     return 0;
// }

