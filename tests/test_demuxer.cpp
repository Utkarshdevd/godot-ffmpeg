#include "../src/core/demuxer/demuxer.hpp"
#include "../src/core/logger/logger.hpp"
#include <cstdlib>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

int main() {
    godot_ffmpeg::Demuxer demuxer;
    bool success = demuxer.open("tests/assets/master_4k.mp4");

    if (!success) {
        LOG_ERR("FAILED: Could not open file");
        return EXIT_FAILURE;
    }

    const int v_index = demuxer.get_video_stream_index();
    const int a_index = demuxer.get_audio_stream_index();

    if (v_index < 0 || a_index < 0) {
        LOG_ERR("FAILED: Missing video or audio stream (video=%d, audio=%d)", v_index, a_index);
        return EXIT_FAILURE;
    }

    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        LOG_ERR("FAILED: Could not allocate packet");
        return EXIT_FAILURE;
    }

    int video_count = 0;
    int audio_count = 0;
    bool seen_keyframe = false;

    while (demuxer.read_packet(packet)) {
        // Assert: packet must belong to video or audio stream
        if (packet->stream_index != v_index && packet->stream_index != a_index) {
            LOG_ERR("FAILED: Packet stream_index %d is neither video (%d) nor audio (%d)",
                    packet->stream_index, v_index, a_index);
            av_packet_unref(packet);
            av_packet_free(&packet);
            demuxer.close();
            return EXIT_FAILURE;
        }

        // Assert: packet has data
        if (packet->size <= 0) {
            LOG_ERR("FAILED: Packet has non-positive size: %d", packet->size);
            av_packet_unref(packet);
            av_packet_free(&packet);
            demuxer.close();
            return EXIT_FAILURE;
        }

        // Assert: packet has valid timestamp (video files usually have PTS)
        if (packet->pts == AV_NOPTS_VALUE) {
            LOG_ERR("FAILED: Packet has AV_NOPTS_VALUE for pts");
            av_packet_unref(packet);
            av_packet_free(&packet);
            demuxer.close();
            return EXIT_FAILURE;
        }

        if (packet->stream_index == v_index) {
            video_count++;
            if (packet->flags & AV_PKT_FLAG_KEY) {
                seen_keyframe = true;
            }
        } else {
            audio_count++;
        }

        av_packet_unref(packet);  // Must free every iteration
    }

    av_packet_free(&packet);
    demuxer.close();

    // Assert: we must have seen at least one keyframe (I-Frame)
    if (!seen_keyframe) {
        LOG_ERR("FAILED: No keyframe (I-Frame) found in video stream");
        return EXIT_FAILURE;
    }

    // Assert: must have both video and audio packets
    if (video_count <= 0 || audio_count <= 0) {
        LOG_ERR("FAILED: Expected video and audio packets (video=%d, audio=%d)", video_count, audio_count);
        return EXIT_FAILURE;
    }

    LOG_INFO("Video: %d packets, Audio: %d packets", video_count, audio_count);
    LOG_INFO("PASSED");
    return EXIT_SUCCESS;
}
