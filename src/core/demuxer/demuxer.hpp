#pragma once

#include <string>

extern "C" {
struct AVFormatContext;
struct AVPacket;
}

namespace godot_ffmpeg {

class Demuxer {
public:
    Demuxer();
    ~Demuxer();

    /** Open a media file. Returns true on success, false on failure. */
    bool open(const std::string& path);

    /** Read the next packet from the file. Returns true on success, false on EOF or error. */
    bool read_packet(AVPacket* packet);

    /** Close the file and clean up resources. */
    void close();

    /** Get the video stream index. Returns -1 if no video stream found. */
    int get_video_stream_index() const { return video_stream_index; }

    /** Get the audio stream index. Returns -1 if no audio stream found. */
    int get_audio_stream_index() const { return audio_stream_index; }

private:
    AVFormatContext* fmt_ctx;
    int video_stream_index;
    int audio_stream_index;
};

} // namespace godot_ffmpeg
