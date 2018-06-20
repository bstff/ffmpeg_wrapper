#include "CodedData.hpp"


namespace ffwrapper{
	CodedData::CodedData()
	:extradata_(NULL)
	,extradata_size_(0)
	{
		init_packet();
	}

	CodedData::~CodedData()
	{
		free_packet();
	}

	AVPacket& CodedData::newAVPacket(){
		free_packet();
		init_packet();

		return packet_;
	}


	void CodedData::init_packet(){
		av_init_packet(&packet_);
		initialize();
	}

	void CodedData::free_packet(){
		if(packet_.data && packet_.size){
			av_free_packet(&packet_);
		}

		initialize();
	}

	void CodedData::initialize(){
		packet_.data = NULL;
		packet_.size = 0;

		extradata_ = NULL;
		extradata_size_ = 0;
	}
}