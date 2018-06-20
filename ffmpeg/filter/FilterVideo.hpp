#ifndef _FFMPEG_FILTER_VIDEO_H_
#define _FFMPEG_FILTER_VIDEO_H_

#include <functional>

struct AVFilterContext;
struct AVFilterGraph;
struct AVCodecContext;
struct AVFrame;

namespace ffwrapper{

typedef std::function<bool(AVFrame*)> FUNC_OUT_FILTERED_FRAME;

class FilterVideo
{
public:
	FilterVideo();
	~FilterVideo();
	
	FilterVideo(AVCodecContext *dec, AVCodecContext *enc, 
				const char *filter_spec, FUNC_OUT_FILTERED_FRAME cb);
public:
	void setCallbackFunc(FUNC_OUT_FILTERED_FRAME cb){func_out_filtered_frame_ = cb;}

	bool initFilterGraph(AVCodecContext *dec_ctx, AVCodecContext *enc_ctx);
	bool configureFilterGraph(const char *filter_spec);

	bool filterFrame(AVFrame *frame);
public:
	AVFilterContext *in(){return src_;}
	AVFilterContext *out(){return out_;}
private:
	AVFilterGraph  		*graph_;
	AVFilterContext 	*src_;
	AVFilterContext 	*out_;

	FUNC_OUT_FILTERED_FRAME 	func_out_filtered_frame_;
};

}

#endif