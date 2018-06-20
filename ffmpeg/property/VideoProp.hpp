#ifndef _FFMPEG_VIDEO_ENCODE_PROP_H_
#define _FFMPEG_VIDEO_ENCODE_PROP_H_

#include <string>

struct AVDictionary;

namespace ffwrapper{
	class VideoProp
	{
	public:
		VideoProp();
		~VideoProp();
	
	public:
		bool rtspStream();
		AVDictionary *optsFormat();
		std::string preset();
		std::string filter_desc();
		std::string url(){return url_;}
	public:
		std::string 	url_;
		bool			transcode_;
		bool 			rtsp_tcp_;

		int 			fps_;
		int 			bit_rate_;
		int 			width_;
		int 			height_;

		int 			quality_;

		int 			udp_port_x_;
		int 			udp_port_y_;

	};
}
#endif