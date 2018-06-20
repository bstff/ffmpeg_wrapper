#include "PicData.hpp"

#include <stdexcept>

extern "C" {
#include <libavcodec/avcodec.h>
}

#include "../log/log.hpp"
using namespace logif;

namespace ffwrapper{
	PicData::PicData(const int width, const int height, const int pix_fmt)
	:frame_(nullptr)
	,data_(nullptr)
	,data_size_(0)
	,pix_fmt_(pix_fmt)
	,width_(width)
	,height_(height)
	{
		bool flag = init_frame();
		flag = init_AVPicture() && flag;
		if(!flag){
			throw std::runtime_error("init PicData error");
		}
	}


	PicData::~PicData(){
		free_frame();
	}

	bool PicData::init_frame(){
		frame_ = av_frame_alloc();

		return !!frame_;
	}

	void PicData::free_frame(){
		avpicture_free((AVPicture *)frame_);
		frame_ = NULL;
		data_ = NULL;

	}

	bool PicData::init_AVPicture(){
		data_size_ = avpicture_get_size((AVPixelFormat)pix_fmt_,
										 width_, height_);
		if(data_size_ < 0){
			logIt("avpicture_get_size error");
			return false;
		}
		data_ = (uint8_t*)av_malloc(data_size_);
        const int ret = avpicture_fill((AVPicture *)frame_, data_, 
        	(AVPixelFormat)pix_fmt_, width_, height_);

        if(ret < 0){
        	logIt("avpicture_fill error");
			return false;

        }

        return true;
	}
}