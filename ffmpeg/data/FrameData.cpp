#include "FrameData.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace ffwrapper{
	FrameData::FrameData()
	:frame_(NULL)
	{
		init_frame();
	}


	FrameData::~FrameData()
	{
		free_frame();
	}


	void FrameData::init_frame(){
		frame_ = av_frame_alloc();
	}

	void FrameData::free_frame(){
		av_frame_free(&frame_);
		frame_ = NULL;
	}
}