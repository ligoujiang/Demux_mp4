#include <iostream>
extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}


// class ResampleAudio{
// private:
//     //输入参数
//     int64_t in_ch_layout;
//     int in_rate;
//     enum AVSampleFormat in_samplefmt;
//     int in_nb_channels;
//     uint8_t **in_data=nullptr;
//     int in_linesize;
//     int in_nb_samples;
//     //输出参数
//     int64_t out_ch_layout;
//     int out_rate;
//     enum AVSampleFormat out_samplefmt;
//     int out_nb_channels;
//     uint8_t** out_data=nullptr;
//     int out_linesize;
//     int out_nb_samples;
//     int max_out_nb_samples;
//     //输出文件
//     const char* outFileName=nullptr;
//     FILE* outFile=nullptr;
//     //重采样句柄
//     SwrContext* swrCtx=nullptr;
//     AVAudioFifo* audio_fifo=nullptr;//采样点的缓存
//     int64_t start_pts;
//     int64_t cur_pts;
//     uint8_t** resampled_data=nullptr; //缓存重采样后的数据
//     int resampled_data_size; //重采样后的点数
//     int in_channels;
//     int out_channels;
//     int64_t total_resampled_num;
// public:
//     ResampleAudio(){

//     }
//     ~ResampleAudio(){
//         if(swrCtx){
//             swrCtx=nullptr;
//             swr_free(&swrCtx);
//         }
//         if(outFile){
//             outFile=nullptr;
//             fclose(outFile);
//         }
//     }
//     bool resampleAudio(){
//         //打开输出文件
//         outFile=fopen(outFileName,"wb");
//         if(!outFile){
//             av_log(NULL,AV_LOG_ERROR,"open outFile failed!\n");
//             return false;
//         }
//         //创建重采样器
//         swrCtx=swr_alloc();
//         if(!swrCtx){
//             av_log(NULL,AV_LOG_ERROR,"swr_alloc failed!\n");
//             return false;
//         }
//         //设置重采样参数
//         //输入参数
//         av_opt_set_int(swrCtx,"in_channel_layout",in_ch_layout,0);
//         av_opt_set_int(swrCtx,"in_sample_rate",in_rate,0);
//         av_opt_set_sample_fmt(swrCtx,"in_sample_fmt",in_samplefat,0);
//         //输出参数
//         av_opt_set_int(swrCtx,"in_channel_layout",out_ch_layout,0);
//         av_opt_set_int(swrCtx,"in_sample_rate",out_rate,0);
//         av_opt_set_sample_fmt(swrCtx,"in_sample_fmt",out_samplefat,0);
//         //初始化重采样
//         int ret=swr_init(swrCtx);
//         if(ret<0){
//             av_log(NULL,AV_LOG_ERROR,"swr_init failed!\n");
//             return false;
//         }

//         //计算输入源的通道数量
//         in_nb_channels=av_get_channel_layout_nb_channels(in_ch_layout);
//         //给输入源分配空间
//         ret=av_samples_alloc_array_and_samples(&in_data,in_linesize,in_nb_channels,in_nb_samples,in_samplefmt,0);
//         if(ret<0){
//             av_log(NULL,AV_LOG_ERROR,"av_samples_alloc_array_and_samples failed!\n");
//             return false;
//         }

//         //计算输出采样数量
//         max_out_nb_samples=out_nb_samples=av_rescale_rnd(in_nb_samples,out_rate,in_rate,AV_ROUND_UP);
//         //计算输出的通道数量
//         out_nb_channels=av_get_channel_layout_nb_channels(out_ch_layout);
//         //分配输出缓存内存
//         ret=av_samples_alloc_array_and_samples(&out_data,out_linesize,out_nb_channels,out_nb_samples,out_samplefmt,0);
//         if(ret<0){
//             av_log(NULL,AV_LOG_ERROR,"av_samples_alloc_array_and_samples failed!\n");
//             return false;
//         }


//     }
// };


// //音频重采样
// int main(int argc,char** argv){
//     return 0;
// }
