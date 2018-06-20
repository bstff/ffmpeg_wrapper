#ifndef _FFMPEG_DATA_AVPACKET_H_
#define _FFMPEG_DATA_AVPACKET_H_

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace ffwrapper{
	class CodedData
	{
	public:
		CodedData();
		~CodedData();
	
	public:
		AVPacket& newAVPacket();

		void refExtraData(unsigned char *data, const int size){
			extradata_ = data;
			extradata_size_ = size;
		}

		AVPacket& getAVPacket(){return packet_;}

		unsigned char *getExtraData(){return extradata_;}
		const int getExtraDataSize() const {return extradata_size_;}
	private:
		void init_packet();
		void free_packet();
		inline void initialize();
	private:
		AVPacket 		packet_;

		unsigned char 	*extradata_;
		int 			extradata_size_;
	};
}
#endif