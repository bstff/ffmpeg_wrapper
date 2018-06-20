#ifndef _FFMPEG_FORMAT_CONTEXT_IN_H_
#define _FFMPEG_FORMAT_CONTEXT_IN_H_

#include <memory>

struct AVFormatContext;
struct AVDictionary;
struct AVStream;
struct AVCodecContext;
struct AVPacket;
struct AVFrame;

namespace ffwrapper{

	class VideoProp;
	class CodedData;
	class FrameData;

	class FormatIn
	{
	public:
		FormatIn();
		~FormatIn();
		
		explicit FormatIn(VideoProp &prop);

	public:
		bool open(const char *filename, AVDictionary **options);
		bool findStreamInfo(AVDictionary **options);

		bool openCodec(const int type, AVDictionary **options);

		bool readPacket(AVPacket &pkt_out, int stream_index = 0);
		bool readPacket(std::shared_ptr<CodedData> &data, int stream_index = 0);

		int decode(AVFrame* &frame, AVPacket &pkt);
		int decode(std::shared_ptr<FrameData> &frame_data,
					std::shared_ptr<CodedData> &data);
		
		int readFrame(AVFrame* &frame);
		int readFrame(std::shared_ptr<FrameData> &frame_data);
	public:
		AVStream *getStream(int type = 0);
		AVCodecContext *getCodecContext(int type = 0);
	private:
	 	AVFormatContext 	*ctx_;
	 	int 				vs_idx_;	
	};
}

#endif