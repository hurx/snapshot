#include "snap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
using namespace std;
//包含库
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "SDL/SDL.h"
#include "libswresample/swresample.h"  
};


//int SnapShot(char* file, char* out_pic, double shot_time);
/*
int main(int argc, char* argv[]){
	char* file_path = "LOL.mp4";//待解码文件
	//char* filepath = "http://1253039488.vod2.myqcloud.com/d8c9fd32vodtransgzp1253039488/1f8f10735285890794368016706/v.f10.mp4";//待解码文件
	char* out_pic = "snapshot.jpg";
	double shot_time = 5.0;
	SnapShot(file_path,out_pic,shot_time);
	return 0;
}
*/
int SnapShot(char* file, char* out_pic, double shot_time)
{
	//解码相关
	char* filepath = file;//待解码文件
	AVFormatContext* pFormatCtx;//解封装格式上下文
	AVCodecContext* pCodecCtx;//解码上下文
	AVCodec			*pCodec;//解码器
	AVPacket packet; //解码前的数据包
	AVFrame *pFrame = av_frame_alloc();//解码后的数据帧
	int videoindex; //视频流编号
	int gotPicture_in = 0;
	unsigned i = 0;
	//编码相关
	AVStream* video_st;
	AVFormatContext* oFormatCtx;//封装格式上下文
	AVCodecContext* oCodecCtx;//编码码上下文
	AVCodec			*oCodec;//编码器
	AVOutputFormat* fmt;//输出封装格式上下文
	AVPacket pkt; //编码后的数据包
	uint8_t* picture_buf;//存放图片数据的缓存
	AVFrame*  picture = av_frame_alloc();
	int got_picture_out = 0;
 
	const char* out_file = out_pic;    //输出文件
 
	int size; //指定色彩格式(例如YUV420P)的一帧图像大小
	int y_size;//输出帧的分辨率,即width*height
 
 
	//----------------------------------------------------------------
	av_register_all();//注册格式库和编解码库
	pFormatCtx = avformat_alloc_context();
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)//打开输入文件并读取文件头
		return -1;
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)//读取流信息
		return -1;
	//找到视频流
	videoindex = -1;
	for (i = 0; i < (pFormatCtx->nb_streams); i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			videoindex = i;
	}
	if (videoindex == -1)
	{
		return -1;
	}
 
	//获取编解码上下文
	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	AVStream *v_stream = pFormatCtx->streams[videoindex];
	int rate_num = v_stream->avg_frame_rate.num;
	int rate_den = v_stream->avg_frame_rate.den;
	//cout << rate_num/rate_den<<endl;
	//cout << v_stream->time_base.num << " " << v_stream->time_base.den << endl;
	//AVRational time_base = v_stream->time_base;
	//获取解码器
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL)
		return -1;
	//打开解码器
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
		return -1;
 
	//编码组件设置部分
	//--------------------------------------------------------
	//根据文件名配分封装格式上下文
	avformat_alloc_output_context2(&oFormatCtx, NULL, NULL, out_file);
	fmt = oFormatCtx->oformat;
	//为输出文件创建输出数据缓存 
	if (avio_open(&oFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0){
		printf("Couldn't open output file.");
		return -1;
	}
	//添加新的流到封装格式上下文中
	//mux的时候，必须在写文件头的前调用，而且必须手动释放分配的内存
	video_st = avformat_new_stream(oFormatCtx, 0);
	if (video_st == NULL){
		return -1;
	}
	//设置编码上下文属性
	oCodecCtx = video_st->codec;
	oCodecCtx->codec_id = fmt->video_codec;
	oCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	oCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;//编码为YUVJ420P格式
	oCodecCtx->width =  pCodecCtx->width;
	oCodecCtx->height = pCodecCtx->height;
	//编码的时基必须由用户指定
	oCodecCtx->time_base.num = 1;
	oCodecCtx->time_base.den = 25;
	//匹配合适编码器
	oCodec = avcodec_find_encoder(oCodecCtx->codec_id);
	if (!oCodec){
		printf("Codec not found.");
		return -1;
	}
	//打开编码器
	if (avcodec_open2(oCodecCtx, oCodec, NULL) < 0){
		printf("Could not open codec.");
		return -1;
	}
	//计算图片大小
	size = avpicture_get_size(oCodecCtx->pix_fmt, oCodecCtx->width, oCodecCtx->height);
	picture_buf = (uint8_t *)av_malloc(size);
	if (!picture_buf)
	{
		return -1;
	}
	//avpicture_fill是让picture的data[0]、data[1]、data[2]等正确的指向av_frame_alloc()分配空间地址，
	//因为av_frame_alloc()分配的空间是一个线性地址（一个连续的缓冲区），而picture的data[]是分别指向
	//不同的平面的，如YUV420P中的Y平面、U平面、V平面，通过avpicture_fill之后，picture的data[]就分别指向
	//这个线性地址的不同位置了。完成avpicture_fill后，你对picture中的data[]进行操作时，实际是操作avcodec_alloc_frame()
	//分配的空间。
	avpicture_fill((AVPicture *)picture, picture_buf, oCodecCtx->pix_fmt, oCodecCtx->width, oCodecCtx->height);
 
	// 写流的头部到输出文件中去
	avformat_write_header(oFormatCtx, NULL);
	y_size = oCodecCtx->width * oCodecCtx->height;//图形大小
	av_new_packet(&pkt, y_size * 3);//为编码后的数据包分配内存大小
	//---------------------------------------------------------------------------------------------------------------
 
	//解码
	//这里是先解码然后再编码，然后封装为jpeg格式。
	//读取视频流	
	while (av_read_frame(pFormatCtx, &packet) >= 0) {
		// Is this a packet from the video stream?
		if (packet.stream_index == videoindex) {
			// Decode video frame
			int ret = avcodec_decode_video2(pCodecCtx, pFrame, &gotPicture_in, &packet);
			if (ret < 0)
				return -1;
			// Did we get a  frame?
			if (gotPicture_in) {
				double currentPos = packet.pts * av_q2d(v_stream->time_base);
				//cout << currentPos << endl;
				//这里选取了第五十个数据帧进行截图
				if (currentPos >= shot_time){
					//cout << packet.pts << endl;
					//cout << packet.dts << endl;
					//编码  
					ret = avcodec_encode_video2(oCodecCtx, &pkt, pFrame, &got_picture_out);
					if (ret < 0){
						printf("Encode Error.\n");
						return -1;
					}
					if (got_picture_out == 1){
						pkt.stream_index = video_st->index;
						ret = av_write_frame(oFormatCtx, &pkt);
					}
					//写文件尾
					av_write_trailer(oFormatCtx);
					printf("Encode Successful.\n");
					break;
				}
			}
		}
	}
	//释放内存
	av_free(picture_buf);
 
	av_free_packet(&packet);
	av_free_packet(&pkt);
 
	av_frame_free(&pFrame);
	av_frame_free(&picture);
 
	avcodec_close(pCodecCtx);
	avcodec_close(video_st->codec);
 
	//avformat_close_input(&pFormatCtx);
	avformat_free_context(pFormatCtx);
	avformat_free_context(oFormatCtx);
	return 0;
}

