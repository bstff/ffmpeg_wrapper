#include "conf.hpp"

extern "C"{
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavutil/error.h>
}


namespace ffwrapper{

	void makeTheWorld(){
		av_register_all();
    	avfilter_register_all();
    	avformat_network_init();

    	av_log_set_level(AV_LOG_ERROR);
	}

	std::string getAVErrorDesc(const int code){
		char err[64];
    	av_strerror(code, err, sizeof(err));
    	return std::string(err);
	}
}