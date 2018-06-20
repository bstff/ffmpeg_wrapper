#ifndef _FFMPEG_INIT_CONFIG_H_
#define _FFMPEG_INIT_CONFIG_H_

#include <string>

namespace ffwrapper{
	void makeTheWorld();
	std::string getAVErrorDesc(const int code);
}
#endif