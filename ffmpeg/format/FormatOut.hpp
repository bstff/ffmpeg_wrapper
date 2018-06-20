#ifndef _FFMPEG_FORMAT_CONTEXT_OUT_H_
#define _FFMPEG_FORMAT_CONTEXT_OUT_H_

#include <stdlib.h>
#include <memory>
#include <string>

struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
struct AVDictionary;

namespace ffwrapper{
	class VideoProp;
	class CodedData;
	class FrameData;

	class FormatOut
	{
	public:
		FormatOut();
		~FormatOut();
	
		FormatOut(VideoProp &prop, AVStream *in, 
				const char *filename, char *format_name = NULL);

		FormatOut(AVStream *in, const char *format_name);

		void clear();
	public:
		bool open(const char *filename, const char *format_name);
		bool openCodecFromIn(AVStream *in, VideoProp &prop);

		int encode(AVPacket &pkt, AVFrame *frame);
		int encode(std::shared_ptr<CodedData> &data,
					std::shared_ptr<FrameData> &frame_data);
		int encode(std::shared_ptr<CodedData> &data,AVFrame *frame);

	public:
		bool copyCodecFromIn(AVStream *in);
		bool openResource(const char *filename, const int flags);
		bool closeResource();

		bool startWriter(const char *filename);
		bool writeFrame(AVPacket &pkt, const int64_t &frame_cnt, bool interleaved = true);
		bool endWriter();

		bool writeHeader(AVDictionary **options = NULL);
		bool writeFrame(AVPacket &pkt, bool interleaved = true);
		bool writeTrailer();
	public:
		AVStream *getStream(){return v_s_;}
		AVCodecContext *getCodecContext();

		const double getFPS()const;
		const char* getFormatName() const{return format_name_.c_str();}
		const char* getFileName() const;//{return ctx_->filename;}
	private:
		AVStream 				*in_;
		AVFormatContext 		*ctx_;	
		AVStream 				*v_s_;

		int64_t				 	sync_opts_;

		bool 					record_;

		double 					fps_;
		std::string 			format_name_;
	};
}
#endif