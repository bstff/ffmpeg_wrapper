#include "cvbridge.hpp"

#include <string.h>
#include <stdexcept>

#include "ffmpeg/swscale/swscale_wrapper.hpp"
#include "ffmpeg/data/PicData.hpp"

#include "../configure/conf.hpp"

using namespace ffwrapper;

namespace ffwrapper{
	cvbridge::cvbridge(int srcW, int srcH, int srcFmt, 
				 int dstW, int dstH, int dstFmt, int flags/*=4*/,
				 void *srcFilter/*=NULL*/,
				 void *dstFilter/*=NULL*/,
				 double *param/*=NULL*/)
	:scale_(new swscale_wrapper)
	,pic_(new PicData(dstW, dstH, dstFmt))
	{
		bool flag = !!scale_ && !!pic_;
		flag = scale_->initContext(srcW, srcH, srcFmt, dstW, dstH, dstFmt, flags) && flag;

		if(!flag){
			throw std::runtime_error("init swscale error");
		}
	}

	cvbridge::~cvbridge(){
		if(scale_){
			delete scale_;
		}
		if(pic_){
			delete pic_;
		}
	}

	bool cvbridge::copyPicture(uint8_t *out, AVFrame *in){
		if(!scale_ || !pic_){
			return false;
		}

		if(!scale_->scaleFrame(in, pic_->getAVFrame())){
			return false;
		}

		memcpy(out, pic_->getAVPictureData(), 
					pic_->getAVPictureSize());

		return true;		
	}
}
