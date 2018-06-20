#ifndef _FFMPEG_DATA_AVPICTURE_H_
#define _FFMPEG_DATA_AVPICTURE_H_

#include <stdint.h>

struct AVFrame;

namespace ffwrapper{
	class PicData
	{
	public:
		PicData(const int width, const int height, const int pix_fmt);
		~PicData();
	
	public:
		AVFrame* getAVFrame(){return frame_;}

		const uint8_t* getAVPictureData()const{return data_;}
		const int getAVPictureSize()const{return data_size_;}
		const int getPixelFormat()const{return pix_fmt_;}
		const int getPictureWidth()const{return width_;}
		const int getPictureHeight()const{return height_;}
	private:
		bool init_frame();
		void free_frame();

		bool init_AVPicture();
	private:
		AVFrame 		*frame_;
		uint8_t 		*data_;
		int 			data_size_;
		int 			pix_fmt_;
		int 			width_;
		int 			height_;
	};
}
#endif