#include "FormatIn.hpp"

#include <stdexcept>

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include "../log/log.hpp"
#include "../configure/conf.hpp"

#include "ffmpeg/property/VideoProp.hpp"
#include "ffmpeg/data/CodedData.hpp"
#include "ffmpeg/data/FrameData.hpp"

using namespace logif;

namespace ffwrapper{
	FormatIn::FormatIn()
	:ctx_(NULL)
	,vs_idx_(-1){

	}

	FormatIn::~FormatIn()
	{
		if(ctx_){
			for (int i = 0; i < ctx_->nb_streams; ++i)
			{
				avcodec_close(ctx_->streams[i]->codec);
			}
			avformat_close_input(&ctx_);
			avformat_free_context(ctx_);
			ctx_ = NULL;
		}
	}

	FormatIn::FormatIn(VideoProp &prop)
	:ctx_(NULL)
	,vs_idx_(-1)
	{
		bool flag = true;
		AVDictionary *avdic = prop.optsFormat();
		if (!avdic){
			flag = open(prop.url().c_str(), NULL) && flag;
		}else{
			flag = open(prop.url().c_str(), &avdic) && flag;
			av_dict_free(&avdic);
		}
		flag = findStreamInfo(NULL) && flag;

		if(prop.transcode_){
		    flag = openCodec(AVMEDIA_TYPE_VIDEO, NULL) && flag;
		}
		if (!flag){
			throw std::runtime_error("FormatIn Init Failed!");
		}
	}
/////////////////////////////////////////////////////////////////////////
	bool FormatIn::open(const char *filename, AVDictionary **options){

		const int ret = avformat_open_input(&ctx_, filename, NULL, options);
		if(ret < 0){
			logIt("open %s failed:%s",filename,
					getAVErrorDesc(ret).c_str()); 

			return false;
		}

		return true;
	}

	bool FormatIn::findStreamInfo(AVDictionary **options){

		const int ret = avformat_find_stream_info(ctx_, options);
		if(ret < 0){
			logIt("find %s stream info failed:%s",
					ctx_->filename,getAVErrorDesc(ret).c_str()); 

			return false;
		}

		for (int i = 0; i < ctx_->nb_streams; ++i)
		{
			switch(ctx_->streams[i]->codec->codec_type){
				case AVMEDIA_TYPE_VIDEO:
					vs_idx_ = i;
					break;

				default:
					break;
			}
		}
		return true;
	}

	bool FormatIn::openCodec(const int type, AVDictionary **options){
		int stream_index = -1;
		switch(type){
			case AVMEDIA_TYPE_VIDEO:
				stream_index = vs_idx_;
				break;
			default:
				break;
		}
		if(stream_index < 0){
			logIt("open input %s codec need correct stream",
										ctx_->filename); 

			return false;
		}

		AVStream *s = ctx_->streams[stream_index];
		AVCodecContext *ctx = s->codec;

		ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		const int ret = avcodec_open2(ctx, 
						avcodec_find_decoder(ctx->codec_id), options);

		if(ret < 0){
			logIt("open input %s codec failed:%s",
					ctx_->filename,getAVErrorDesc(ret).c_str()); 

			return false;
		}
		return true;
	}

	AVStream *FormatIn::getStream(int type){
		return ctx_->streams[vs_idx_];
	}

	AVCodecContext *FormatIn::getCodecContext(int type){
		return ctx_->streams[vs_idx_]->codec;
	}
	
	bool FormatIn::readPacket(AVPacket &pkt_out, int stream_index){

		bool founded = false;
		while (!founded){
			const int ret = av_read_frame(ctx_, &pkt_out);
			if(ret < 0){
				logIt("read frame from %s failed:%s",
						ctx_->filename,getAVErrorDesc(ret).c_str()); 
	
				return false;
			}
			if(pkt_out.stream_index == stream_index){
				founded = true;
			}else{
				av_free_packet(&pkt_out);
				av_init_packet(&pkt_out);
				pkt_out.data = NULL;
				pkt_out.size = 0;
			}
		}
		pkt_out.stream_index = 0;
		return true;

	}

	bool FormatIn::readPacket(std::shared_ptr<CodedData> &data, int stream_index){

		AVPacket &pkt(data->getAVPacket());
		return readPacket(pkt, getStream()->index);
	}

	int FormatIn::decode(AVFrame* &frame, AVPacket &pkt){
		AVStream *in = getStream();
		AVCodecContext *dec_ctx = in->codec;

		if(!dec_ctx){
			logIt("FormatIn decode packet error"); 
			return -1;
		}

		av_packet_rescale_ts(&pkt, in->time_base, dec_ctx->time_base);

		int got_picture = 0;

		int ret = avcodec_decode_video2(dec_ctx, frame, &got_picture, &pkt);
		if(ret < 0){
			logIt("decode frame failed:%s",
					getAVErrorDesc(ret).c_str()); 
			return -1;
		}
	
		if(got_picture){
			return 1;
		}
		return 0;
	}

	int FormatIn::decode(std::shared_ptr<FrameData> &frame_data,
					std::shared_ptr<CodedData> &data){

		AVFrame *frame = frame_data->getAVFrame();
		AVPacket &pkt(data->getAVPacket());

		return decode(frame, pkt);
	}

	int FormatIn::readFrame(AVFrame* &frame){

		auto data(std::make_shared<CodedData>());
		if(!readPacket(data)){
			return -1;
		}

		AVPacket &pkt(data->getAVPacket());

		return decode(frame, pkt);
	}

	int FormatIn::readFrame(std::shared_ptr<FrameData> &frame_data){

		AVFrame *frame(frame_data->getAVFrame());

		return readFrame(frame);
	}
}