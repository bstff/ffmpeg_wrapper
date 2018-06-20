#ifndef _FFMPEG_DATA_AVFRAME_H_
#define _FFMPEG_DATA_AVFRAME_H_

struct AVFrame;

namespace ffwrapper{
	class FrameData
	{
	public:
		FrameData();
		~FrameData();
	
	public:
		AVFrame* getAVFrame(){return frame_;}
	private:
		void init_frame();
		void free_frame();
	private:
		AVFrame 		*frame_;	
	};
}
#endif