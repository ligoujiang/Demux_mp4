#include <iostream>
extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/bsf.h>
}

using namespace std;


const int sampling_frequencies[] = {
    96000,  // 0x0
    88200,  // 0x1
    64000,  // 0x2
    48000,  // 0x3
    44100,  // 0x4
    32000,  // 0x5
    24000,  // 0x6
    22050,  // 0x7
    16000,  // 0x8
    12000,  // 0x9
    11025,  // 0xa
    8000   // 0xb
    // 0xc d e f是保留的
};
//adts格式头部文件
int adts_header(char * const p_adts_header, const int data_length,
                const int profile, const int samplerate,
                const int channels)
{

    int sampling_frequency_index = 3; // 默认使用48000hz
    int adtsLen = data_length + 7;
    //ADTS不是单纯的data，是hearder+data的，所以加7这个头部hearder的长度，这里7是因为后面protection absent位设为1，不做校验，所以直接加7，如果做校验，可能会是9

    int frequencies_size = sizeof(sampling_frequencies) / sizeof(sampling_frequencies[0]);
    int i = 0;
    for(i = 0; i < frequencies_size; i++)   //查找采样率
    {
        if(sampling_frequencies[i] == samplerate)
        {
            sampling_frequency_index = i;
            break;
        }
    }
    if(i >= frequencies_size)
    {
        printf("unsupport samplerate:%d\n", samplerate);
        return -1;
    }

    p_adts_header[0] = 0xff;         //syncword:0xfff                          高8bits
    p_adts_header[1] = 0xf0;         //syncword:0xfff                          低4bits
    p_adts_header[1] |= (0 << 3);    //MPEG Version:0 for MPEG-4,1 for MPEG-2  1bit
    p_adts_header[1] |= (0 << 1);    //Layer:0                                 2bits
    p_adts_header[1] |= 1;           //protection absent:1                     1bit

    p_adts_header[2] = (profile)<<6;            //profile:profile               2bits
    p_adts_header[2] |= (sampling_frequency_index & 0x0f)<<2; //sampling frequency index:sampling_frequency_index  4bits
    p_adts_header[2] |= (0 << 1);             //private bit:0                   1bit
    p_adts_header[2] |= (channels & 0x04)>>2; //channel configuration:channels  高1bit

    p_adts_header[3] = (channels & 0x03)<<6; //channel configuration:channels 低2bits
    p_adts_header[3] |= (0 << 5);               //original：0                1bit
    p_adts_header[3] |= (0 << 4);               //home：0                    1bit
    p_adts_header[3] |= (0 << 3);               //copyright id bit：0        1bit
    p_adts_header[3] |= (0 << 2);               //copyright id start：0      1bit
    p_adts_header[3] |= ((adtsLen & 0x1800) >> 11);           //frame length：value   高2bits

    p_adts_header[4] = (uint8_t)((adtsLen & 0x7f8) >> 3);     //frame length:value    中间8bits
    p_adts_header[5] = (uint8_t)((adtsLen & 0x7) << 5);       //frame length:value    低3bits
    p_adts_header[5] |= 0x1f;                                 //buffer fullness:0x7ff 高5bits
    p_adts_header[6] = 0xfc;      //11111100       //buffer fullness:0x7ff 低6bits
    // number_of_raw_data_blocks_in_frame：
    //    表示ADTS帧中有number_of_raw_data_blocks_in_frame + 1个AAC原始帧。

    return 0;
}


//demux ->aac+h264
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
    //过滤器句柄
    AVBSFContext *m_bsfCtx = nullptr;
public:
    Demux(char* argv1){
        m_inFileName=argv1;
    }
    Demux(char* argv1,char* argv2){
        m_inFileName=argv1;
        m_outFileName_H264=argv2;
    }
    Demux(char* argv1,char* argv2,char*argv3){
        m_inFileName=argv1;
        m_outFileName_H264=argv2;
        m_outFileName_Aac=argv3;
    }
    ~Demux(){
        if(m_inFmtCtx){
            avformat_close_input(&m_inFmtCtx);
            m_inFmtCtx=nullptr;
            av_log(NULL,AV_LOG_DEBUG,"m_inFmtCtx已被析构！\n");
        }
        if(m_fp_h264){
            fclose(m_fp_h264);
            m_fp_h264=nullptr;
            av_log(NULL,AV_LOG_DEBUG,"m_fp_h264已被析构！\n");
        }
        if(m_fp_aac){
            fclose(m_fp_aac);
            m_fp_aac=nullptr;
            av_log(NULL,AV_LOG_DEBUG,"m_fp_acc已被析构！\n");
        }
        if(m_pkt){
            av_packet_free(&m_pkt);
            m_pkt=nullptr;
            av_log(NULL,AV_LOG_DEBUG,"m_pkt已被析构！\n");
        }
        if(m_bsfCtx){
            av_bsf_free(&m_bsfCtx);
            m_bsfCtx=nullptr;
            av_log(NULL,AV_LOG_DEBUG,"m_bsfCtx已被析构！\n");
        }
    }
    bool DemuxMP4(){
        //设置打印级别
        av_log_set_level(AV_LOG_DEBUG);
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
        //av_dump_format(m_inFmtCtx,0,m_inFileName,0);

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
        int audioIndex=av_find_best_stream(m_inFmtCtx,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0); //查找音频流
        if(audioIndex==-1){
            av_log(NULL,AV_LOG_ERROR,"find audioIndex failed!\n");
            return false;
        }

        //主流封装格式转换成h264_mp4toannexb
        const AVBitStreamFilter *bsfilter = av_bsf_get_by_name("h264_mp4toannexb");
        //初始化过滤器上下文
        av_bsf_alloc(bsfilter, &m_bsfCtx); //AVBSFContext;
        //添加解码器属性
        avcodec_parameters_copy(m_bsfCtx->par_in, m_inFmtCtx->streams[videoIndex]->codecpar);
        av_bsf_init(m_bsfCtx);

        //发包
        m_pkt=av_packet_alloc();
        while(true){
            ret=av_read_frame(m_inFmtCtx,m_pkt);
            if(ret<0){
                av_log(NULL,AV_LOG_ERROR,"av_read_frame fnish!\n");
                break;
            }
            if(m_pkt->stream_index==videoIndex){

                //这一步是为了转成h264_mp4toannexb
                //----------------------------------------
                //处理视频
                ret=av_bsf_send_packet(m_bsfCtx,m_pkt);
                if(ret<0){
                    continue;
                }
                while(true){
                    ret=av_bsf_receive_packet(m_bsfCtx,m_pkt);
                    if(ret!=0){
                        break;
                    }
                    fwrite(m_pkt->data,1,m_pkt->size,m_fp_h264);
                    av_packet_unref(m_pkt);
                }
                //-----------------------------------------
            }else if(m_pkt->stream_index==audioIndex){
                //处理音频              
                    char adtsHeader[7]={0};
                    adts_header(adtsHeader,m_pkt->size,
                                m_inFmtCtx->streams[audioIndex]->codecpar->profile,
                                m_inFmtCtx->streams[audioIndex]->codecpar->sample_rate,
                                m_inFmtCtx->streams[audioIndex]->codecpar->ch_layout.nb_channels);
                    //先写头数据
                    //注意！！ ts格式不需要写头数据，自带有，写ts格式时，注释下面这行代码即可！
                    fwrite(adtsHeader,1,sizeof(adtsHeader),m_fp_aac);
                    //写音频数据
                    fwrite(m_pkt->data,1,m_pkt->size,m_fp_aac);
                    av_packet_unref(m_pkt);
            }
            av_packet_unref(m_pkt);
        }
        return true;
    }
};


int main(int argc,char** argv)
{
    Demux de(argv[1],argv[2],argv[3]);
    de.DemuxMP4();

    return 0;
}
