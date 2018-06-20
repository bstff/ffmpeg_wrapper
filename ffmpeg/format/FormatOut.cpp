#include "FormatOut.hpp"

#include <stdexcept>
#include <vector>

#include <unistd.h>
#include <sys/time.h>

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

#include "../log/log.hpp"
#include "../configure/conf.hpp"

#include "ffmpeg/property/VideoProp.hpp"
#include "ffmpeg/data/CodedData.hpp"
#include "ffmpeg/data/FrameData.hpp"

using namespace logif;

namespace ffwrapper{
	FormatOut::FormatOut()
	:ctx_(NULL)
	,v_s_(NULL)
    ,sync_opts_(0)
    ,record_(false)
    ,fps_(0.0f)
    ,format_name_("mp4")
	{}

	FormatOut::~FormatOut()
	{
		clear();
	}

    void FormatOut::clear(){
        if(ctx_){
            closeResource();

            for (int i = 0; i < ctx_->nb_streams; ++i)
            {
                avcodec_close(ctx_->streams[i]->codec);
            }
            avformat_free_context(ctx_);
            ctx_ = NULL;
        }
        v_s_ = NULL;
        sync_opts_ = 0;

    }

    FormatOut::FormatOut(VideoProp &prop, AVStream *in, 
        const char *filename, char *format_name/*=NULL*/)
        :FormatOut(){

        bool flag = open(filename, format_name);
        flag = openCodecFromIn(in, prop)  && flag;

        if(!flag){
            throw std::runtime_error("FormatOut Init Failed!");
        }
    }
///////////////////////////////////////////////////////////////////////
	bool FormatOut::open(const char *filename, const char *format_name){

		const int ret = avformat_alloc_output_context2(&ctx_, NULL, format_name, filename);
		if(ret < 0){
			logIt("open %s failed:%s",filename,
					getAVErrorDesc(ret).c_str()); 

			return false;
		}
        
		return true;
	}

	bool FormatOut::openCodecFromIn(AVStream *in, VideoProp &prop){

		AVOutputFormat *ofmt = ctx_->oformat;

		AVCodecID codec_id = AV_CODEC_ID_H264;
		AVCodec *codec = avcodec_find_encoder(codec_id);
		if(!codec){
			logIt("can't find %d encoder", codec_id); 

			return false;
		}

		v_s_ = avformat_new_stream(ctx_, codec);

		AVCodecContext *enc_ctx = v_s_->codec;
		AVCodecContext *dec_ctx = in->codec;

		enc_ctx->codec_id = codec_id;
    	enc_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    	enc_ctx->height = (prop.height_ & 0x01) ? prop.height_-1 : prop.height_;
    	enc_ctx->width = (prop.width_ & 0x01) ? prop.width_ - 1 : prop.width_;

    	enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
    	
    	v_s_->time_base.num = in->time_base.num;
    	v_s_->time_base.den = in->time_base.den;
    	enc_ctx->time_base.num = 1;
    	enc_ctx->time_base.den = prop.fps_;
	
    	
    	enc_ctx->gop_size = dec_ctx->gop_size > 0 ? dec_ctx->gop_size : prop.fps_ * 2;
    	enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    	enc_ctx->max_b_frames = 0;
    	
    	
    	av_opt_set(enc_ctx->priv_data, "preset", prop.preset().c_str(), 0);
    	av_opt_set(enc_ctx->priv_data, "tune", "zerolatency", 0);
    	av_opt_set(enc_ctx->priv_data, "profile", "baseline", 0);
	
    	enc_ctx->thread_count = 4;
	
    	
    	enc_ctx->refs = 4;
    	if(!prop.rtspStream()){
    	    
    	    enc_ctx->me_range = 16;
    	    enc_ctx->max_qdiff = 4;
    	    enc_ctx->qmin = 10;
    	    enc_ctx->qmax = 31;
    	    enc_ctx->qcompress = 0.6;
	
    	}else{
    	    enc_ctx->flags |= AV_CODEC_FLAG_QSCALE;
            enc_ctx->bit_rate = prop.bit_rate_ * 1000;

    	    enc_ctx->rc_min_rate = enc_ctx->bit_rate / 2;
    	    enc_ctx->rc_max_rate = enc_ctx->bit_rate * 2 - enc_ctx->rc_min_rate;
    	    enc_ctx->rc_buffer_size = enc_ctx->rc_max_rate * 10;
    	}
	
    	if(ofmt->flags & AVFMT_GLOBALHEADER)
    	{
    	    enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    	}
    	
    	ofmt->video_codec = codec_id;
    	
    	int err =avcodec_open2(enc_ctx, codec, NULL);
    	if( err< 0)
    	{
    	    logIt("can't open output codec: %s",
    	    		getAVErrorDesc(err).c_str());
    	    return false;
    	}

    	return true;
	}

	AVCodecContext *FormatOut::getCodecContext(){
		return getStream()->codec;
	}

    int FormatOut::encode(AVPacket &pkt, AVFrame *frame){

        AVStream *out = getStream();
        AVCodecContext *enc_ctx = out->codec;

        int got_packet = 0;
    
        frame->quality = enc_ctx->global_quality;
        frame->pict_type = AV_PICTURE_TYPE_NONE;
    
        pkt.data = NULL;
        pkt.size = 0;
    
        int ret = avcodec_encode_video2(enc_ctx, &pkt, frame, &got_packet);
        if(ret < 0){
            logIt("encode filtered frame failed:%s",
                    getAVErrorDesc(ret).c_str()); 
    
            return -1;
        }
    
        if(got_packet){
            if(pkt.pts == AV_NOPTS_VALUE 
               && !(enc_ctx->codec->capabilities & AV_CODEC_CAP_DELAY))
            {
                pkt.pts = sync_opts_++;
            }
            
            av_packet_rescale_ts(&pkt, enc_ctx->time_base, out->time_base);
        
            return 1;
        }
        
        return 0;
    }

    int FormatOut::encode(std::shared_ptr<CodedData> &data,
                    std::shared_ptr<FrameData> &frame_data){

        AVStream *out = getStream();
        AVCodecContext *enc_ctx = out->codec;
        data->refExtraData(enc_ctx->extradata, enc_ctx->extradata_size);

        AVPacket &pkt(data->getAVPacket());
        AVFrame *frame = frame_data->getAVFrame();

        return encode(pkt, frame);
    }

    int FormatOut::encode(std::shared_ptr<CodedData> &data,AVFrame *frame){
        
        AVStream *out = getStream();
        AVCodecContext *enc_ctx = out->codec;
        data->refExtraData(enc_ctx->extradata, enc_ctx->extradata_size);

        AVPacket &pkt(data->getAVPacket());

        return encode(pkt, frame);
    }

//////////////////////////////////////////////////////////////////////////
    FormatOut::FormatOut(AVStream *in,const char *format_name)
    :FormatOut(){
        in_ = in;
        record_ = true;

        format_name_ = format_name;
        fps_ = getFPS();

    }

    
    bool FormatOut::copyCodecFromIn(AVStream *in){

        v_s_ = avformat_new_stream(ctx_, in->codec->codec);
        if(!v_s_){
            return false;
        }

        int ret = avcodec_copy_context(v_s_->codec, in->codec);
        if (ret < 0){
            logIt("can't copy codec from in error:%s",
                    getAVErrorDesc(ret).c_str()); 

            return false;
        }
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    
        if (ctx_->oformat->flags & AVFMT_GLOBALHEADER)  
        {  
            v_s_->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;  
        }  
        return true;
    }

    bool FormatOut::openResource(const char *filename, const int flags){

        if(record_ && (ctx_->flags & AVFMT_NOFILE) != AVFMT_NOFILE){
            const int err = avio_open2(&ctx_->pb, filename, flags, NULL, NULL);
            if(err < 0)
            {
                logIt("can't save to %s error:%s",filename,
                        getAVErrorDesc(err).c_str()); 

                return false;
            }        
        }
        return true;
    }

    bool FormatOut::closeResource(){
        if(record_){
            return avio_close(ctx_->pb) == 0;
        }
        return true;
    }

    bool FormatOut::writeHeader(AVDictionary **options/* = NULL*/){

        const int ret = avformat_write_header(ctx_, options);
        if(ret < 0){
            logIt("write header to file failed:%s",
                    getAVErrorDesc(ret).c_str()); 
            return false;
        }
        return true;
    }


    bool FormatOut::writeFrame(AVPacket &pkt, bool interleaved/* = true*/){
        
        int ret = 0;
        if(interleaved)
            ret = av_interleaved_write_frame(ctx_, &pkt);
        else
        {
            // returns 1 if flushed and there is no more data to flush
            ret = av_write_frame(ctx_, &pkt);
        }
    
        if(ret < 0)
        {
            logIt("write packet to file failed:%s",
                    getAVErrorDesc(ret).c_str()); 
            return false;
        }
        return true;
    }

    bool FormatOut::writeTrailer(){
        const int ret = av_write_trailer(ctx_);
        if(ret != 0)
        {
            logIt("write trailer to file failed:%s",
                    getAVErrorDesc(ret).c_str()); 
            return false;
        }
        
        return true;
    }

    bool FormatOut::startWriter(const char *filename){
        if(ctx_){
            clear();
        }

        bool flag = open(filename, format_name_.c_str());
        flag = copyCodecFromIn(in_) && flag;
        flag = openResource(filename, AVIO_FLAG_WRITE);

        flag = writeHeader() && flag;

        return flag;
    }

    bool FormatOut::writeFrame(AVPacket &pkt, const int64_t &frame_cnt,
                              bool interleaved/* = true*/){

        int64_t time_stamp = frame_cnt;
        
        pkt.pos = -1;  
        pkt.stream_index = 0;

        //Write PTS
        AVRational time_base = getStream()->time_base;
        
        AVRational time_base_q = { 1, AV_TIME_BASE };
        //Duration between 2 frames (us)
        int64_t calc_duration = (double)(AV_TIME_BASE)*(1 / fps_);  //内部时间戳
        //Parameters
        pkt.pts = av_rescale_q(time_stamp*calc_duration, time_base_q, time_base);
        pkt.dts = pkt.pts;
        pkt.duration = av_rescale_q(calc_duration, time_base_q, time_base); //(double)(calc_duration)*(double)(av_q2d(time_base_q)) / (double)(av_q2d(time_base));
        
        return writeFrame(pkt, interleaved);
    }

    bool FormatOut::endWriter(){

        return writeTrailer();
    }

    const double FormatOut::getFPS()const{

        double fps = 25.0f;
        if(in_->avg_frame_rate.den > 0){
            fps = av_q2d(in_->avg_frame_rate);
        }else if(in_->r_frame_rate.den > 0){
            fps = av_q2d(in_->r_frame_rate);
        }

        return fps;
    }

    const char* FormatOut::getFileName() const{
        return ctx_->filename;
    }

    
}