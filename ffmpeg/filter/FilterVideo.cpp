#include "FilterVideo.hpp"

#include <stdexcept>

extern "C"{
#include <libavutil/opt.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}

#include "../log/log.hpp"
#include "../configure/conf.hpp"

using namespace logif;

namespace ffwrapper{
    
FilterVideo::FilterVideo()
:graph_(NULL)
,src_(NULL)
,out_(NULL)
,func_out_filtered_frame_(nullptr)
{}

FilterVideo::~FilterVideo()
{
    if (graph_)
    {
        for (int i = 0; i < graph_->nb_filters; ++i)
        {
            avfilter_free(graph_->filters[i]);
        }
        avfilter_graph_free(&graph_);
        graph_ = NULL;
    }
    
}

FilterVideo::FilterVideo(AVCodecContext *dec, AVCodecContext *enc, 
    const char *filter_spec, FUNC_OUT_FILTERED_FRAME cb)
:graph_(NULL)
,src_(NULL)
,out_(NULL)
,func_out_filtered_frame_(cb)
{
    bool flag = initFilterGraph(dec, enc);
    flag = configureFilterGraph(filter_spec) && flag;
    if(!flag){
        throw std::runtime_error("FilterVideo Init Failed!");
    }
}
///////////////////////////////////////////////////////////////

bool FilterVideo::initFilterGraph(AVCodecContext *dec_ctx, AVCodecContext *enc_ctx){
    graph_ = avfilter_graph_alloc();
    if(!graph_){
        logIt("alloc filter graph error"); 

        return false;
    }

    AVFilter* buffersrc = (AVFilter*)avfilter_get_by_name("buffer");
    AVFilter* buffersink = (AVFilter*)avfilter_get_by_name("buffersink");
    if(!buffersrc || !buffersink){
        logIt("can't get \"buffer/sink\" filters"); 

        return false;
    }

    char args[512];

    snprintf(args, sizeof(args),
            "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
            dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
            dec_ctx->time_base.num, dec_ctx->time_base.den,
            dec_ctx->sample_aspect_ratio.num,
            dec_ctx->sample_aspect_ratio.den);
    int ret = avfilter_graph_create_filter(&src_, buffersrc, "in",
                                           args, NULL, graph_);
    if(ret < 0)
    {
        logIt("create in filter failed:%s",
                getAVErrorDesc(ret).c_str()); 

        return false;
    } 

    ret = avfilter_graph_create_filter(&out_, buffersink, "out",
                                        NULL, NULL, graph_);
    if(ret < 0)
    {
        logIt("create out filter failed:%s",
                getAVErrorDesc(ret).c_str());
        return false;
    }
    
    
    ret = av_opt_set_bin(out_, "pix_fmts",(uint8_t *)&enc_ctx->pix_fmt, 
                        sizeof(enc_ctx->pix_fmt),AV_OPT_SEARCH_CHILDREN);
    if(ret < 0)
    {
        logIt("set out filter opt failed:%s",
                getAVErrorDesc(ret).c_str());
        return false;
    }

    return true;
}

bool FilterVideo::configureFilterGraph(const char *filter_spec){

    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();

    if(!outputs || !inputs){
        logIt("create filter in/out failed:{}");
        return false;
    }

    /* Endpoints for the filter graph. */
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = src_;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;
    
    inputs->name       = av_strdup("out");
    inputs->filter_ctx = out_;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;

    int ret = avfilter_graph_parse_ptr(graph_, filter_spec,
                                       &inputs, &outputs, NULL);
    if(ret < 0){
        logIt("parse filter graph failed:%s",
                getAVErrorDesc(ret).c_str());
        return false;
    }

    ret = avfilter_graph_config(graph_, NULL);
    if(ret < 0){
        logIt("config filter graph failed:%s",
                getAVErrorDesc(ret).c_str());
        return false;
    }

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return true;
}

bool FilterVideo::filterFrame(AVFrame *frame){

    frame->pts = av_frame_get_best_effort_timestamp(frame);

    int ret = av_buffersrc_add_frame_flags(in(),frame,
                                        AV_BUFFERSRC_FLAG_PUSH);
    if(ret < 0){
        logIt("push frame to filter failed:%s",
                getAVErrorDesc(ret).c_str()); 

        return false;
    }

    AVFrame *filtered_frame;

    bool flag = true;

    while(true){
        filtered_frame = av_frame_alloc();
        if(!filtered_frame){
            logIt("alloc filtered frame failed");

            break;
        }

        ret = av_buffersink_get_frame(out(),filtered_frame);
        if(ret < 0){
            if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
                ret = 0;
            }else{
                logIt("pull frame from filter failed:%s",
                            getAVErrorDesc(ret).c_str()); 
            }
            av_frame_free(&filtered_frame);

            break;
        }

        if(func_out_filtered_frame_){
            flag = func_out_filtered_frame_(filtered_frame);
        }

        av_frame_free(&filtered_frame);

    }
    return flag;
}

}
