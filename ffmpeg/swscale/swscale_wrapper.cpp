#include "swscale_wrapper.hpp"

extern "C" {      
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>  
#include <libavutil/opt.h>  
#include <libavutil/imgutils.h>  
}

#include "../log/log.hpp"
#include "../configure/conf.hpp"

using namespace logif;

namespace ffwrapper{
	swscale_wrapper::swscale_wrapper()
	:ctx_(nullptr)
	,srcW_(0)
	,srcH_(0)
	,srcFmt_(0)
	,dstW_(0)
	,dstH_(0)
	,dstFmt_(0)
	,flags_(0)
	{

	}

	swscale_wrapper::~swscale_wrapper(){
		if(ctx_){
			sws_freeContext(ctx_);
		}
	}

	bool swscale_wrapper::initContext(int srcW, int srcH, int srcFmt, 
					 int dstW, int dstH, int dstFmt, int flags,
					 SwsFilter *srcFilter/*=NULL*/,
					 SwsFilter *dstFilter/*=NULL*/,
					 double *param/*=NULL*/){

		ctx_ = sws_getContext(srcW, srcH, (AVPixelFormat)srcFmt, 
			dstW, dstH, ( AVPixelFormat )dstFmt, flags, 
			srcFilter, dstFilter, param);

		if(!ctx_){
			logIt("sws_getContext failed");
			return false;
		}

		srcW_ = srcW;
		srcH_ = srcH;
		srcFmt_ = srcFmt;
		dstW_ = dstW;
		dstH_ = dstH;
		dstFmt_ = dstFmt;
		flags_ = flags;
		
		return true;
	}

	bool swscale_wrapper::scaleFrame(AVFrame *in, AVFrame *out){
		const int ret = sws_scale(ctx_, in->data, in->linesize, 0,
                 in->height, out->data, out->linesize);

		if (ret < 0){
			logIt("sws_scale error:%s", getAVErrorDesc(ret).c_str());
			return false;
		}
		return true;
	}
}