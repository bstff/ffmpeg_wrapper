#include "VideoProp.hpp"

extern "C"{
#include <libavutil/opt.h>
}

namespace ffwrapper{
	VideoProp::VideoProp()
	:url_("")
	,transcode_(false)
	,rtsp_tcp_(false)
	,fps_(12)
	,bit_rate_(400)
	,width_(960)
	,height_(720)
	,quality_(3)
	,udp_port_x_(5000)
	,udp_port_y_(65000)
	{}

	VideoProp::~VideoProp()
	{}

	bool VideoProp::rtspStream(){
		if(url_.find("rtsp://") != 0){
			return false;
		}
		return true;
	}

	AVDictionary* VideoProp::optsFormat(){
		if(url_.find("rtsp://") != 0){
			return NULL;
		}

		AVDictionary *avdic = NULL;
		char option_key2[]="max_delay";
		char option_value2[]="5000000";
		av_dict_set(&avdic,option_key2,option_value2,0);

		char option_key[]="rtsp_transport";

		if(rtsp_tcp_){
		    char option_value[]="tcp";
		    av_dict_set(&avdic,option_key,option_value,0);
		}else{
			char option_value[]="udp";
		    av_dict_set(&avdic,option_key,option_value,0);

		    char opt_key1[] = "min_port";
        	av_dict_set_int(&avdic, opt_key1, udp_port_x_,0);
        	char opt_key3[] = "max_port";
        	av_dict_set_int(&avdic, opt_key3, udp_port_y_,0);
		}

		return avdic;
	}

	std::string VideoProp::preset(){

		char *preset = "veryfast";
    	switch(quality_){
        case 1:
            preset = "ultrafast";
            break;
        case 2:
            preset = "superfast";
            break;
        case 3:
            preset = "veryfast";
            break;
        case 4:
            preset = "faster";
            break;
        case 5:
            preset = "fast";
            break;
        case 6:
            preset = "medium";
            break;
        case 7:
            preset = "slow";
            break;
        case 8:
            preset = "slower";
            break;
        case 9:
            preset = "veryslow";
            break;
        default:
            break;
    	}

    	return preset;
	}

	std::string VideoProp::filter_desc(){
		char spec[64] = {0};

		if(rtspStream()){
			snprintf(spec, sizeof(spec),"scale=%d:%d,fps=fps=%d",
					width_, height_, fps_);
		}else{
			snprintf(spec, sizeof(spec),"scale=%d:%d,setpts=2*PTS",
					width_, height_);
		}

		return std::string(spec);
	}

}