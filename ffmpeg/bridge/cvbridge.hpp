#ifndef _CV_FFMPEG_DATA_BRIDGE_H_
#define _CV_FFMPEG_DATA_BRIDGE_H_

#include <stdlib.h>
#include <stdint.h>

struct AVFrame;

namespace ffwrapper{

	class swscale_wrapper;
	class PicData;

	class cvbridge
	{
	public:
		cvbridge(int srcW, int srcH, int srcFmt, 
					 int dstW, int dstH, int dstFmt, int flags=4,
					 void *srcFilter=NULL,
					 void *dstFilter=NULL,
					 double *param=NULL);
		~cvbridge();
	
	public:
		bool copyPicture(uint8_t *out, AVFrame *in);
	private:
		ffwrapper::swscale_wrapper 		*scale_;
		ffwrapper::PicData 				*pic_;
	};
	
}
#endif