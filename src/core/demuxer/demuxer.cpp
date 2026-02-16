#include "demuxer.hpp"
#include "../logger/logger.hpp"

extern "C" {
#include <libavformat/avformat.h>
}

namespace godot_ffmpeg {

Demuxer::Demuxer()
    : fmt_ctx(nullptr)
    , video_stream_index(-1)
    , audio_stream_index(-1)
{
}

Demuxer::~Demuxer() {
    close();
}

bool Demuxer::open(const std::string& path) {
    // Open input file
    int ret = avformat_open_input(&fmt_ctx, path.c_str(), nullptr, nullptr);
    if (ret < 0) {
        LOG_ERR("Could not open file: %s (error code: %d)", path.c_str(), ret);
        return false;
    }

    // Read stream information
    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        LOG_ERR("Could not find stream info for: %s (error code: %d)", path.c_str(), ret);
        close();
        return false;
    }

    // Find video and audio stream indices
    video_stream_index = -1;
    audio_stream_index = -1;

    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && video_stream_index == -1) {
            video_stream_index = static_cast<int>(i);
        } else if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && audio_stream_index == -1) {
            audio_stream_index = static_cast<int>(i);
        }
    }

    LOG_INFO("Opened file: %s", path.c_str());
    if (video_stream_index >= 0) {
        LOG_INFO("Found video stream at index: %d", video_stream_index);
    }
    if (audio_stream_index >= 0) {
        LOG_INFO("Found audio stream at index: %d", audio_stream_index);
    }

    return true;
}

bool Demuxer::read_packet(AVPacket* packet) {
    if (!fmt_ctx) {
        return false;
    }

    int ret = av_read_frame(fmt_ctx, packet);
    return ret >= 0;
}

void Demuxer::close() {
    if (fmt_ctx) {
        avformat_close_input(&fmt_ctx);
        fmt_ctx = nullptr;
    }
    video_stream_index = -1;
    audio_stream_index = -1;
}

} // namespace godot_ffmpeg
