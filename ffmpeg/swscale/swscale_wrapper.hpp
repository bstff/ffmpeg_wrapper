#ifndef _FFMPEG_SWSCALE_WRAPPER_H_
#define _FFMPEG_SWSCALE_WRAPPER_H_

#include <stdlib.h>

struct SwsContext;
struct SwsFilter;
struct AVFrame;

namespace ffwrapper{
	class swscale_wrapper
	{
	public:
		swscale_wrapper();
		~swscale_wrapper();
	bool initContext(int srcW, int srcH, int srcFmt, 
					 int dstW, int dstH, int dstFmt, int flags,
					 SwsFilter *srcFilter=NULL,
					 SwsFilter *dstFilter=NULL,
					 double *param=NULL);

	bool scaleFrame(AVFrame *in, AVFrame *out);

	private:
		SwsContext 		*ctx_;
		int 			srcW_,srcH_,srcFmt_;
		int 			dstW_,dstH_,dstFmt_;
		int 			flags_;
	};
}
#endif