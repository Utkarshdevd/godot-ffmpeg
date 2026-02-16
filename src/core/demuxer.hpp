#pragma once

#include <string>

namespace godot_ffmpeg {

class Demuxer {
public:
    /** Open a media file. Returns true on success, false on failure. */
    bool open(const std::string& path) {
        (void)path;
        return false;  // Stub: TDD Red — not implemented yet.
    }
};

} // namespace godot_ffmpeg
