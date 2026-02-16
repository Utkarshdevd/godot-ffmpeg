#include "core/demuxer.hpp"
#include <cstdlib>
#include <iostream>

int main() {
    godot_ffmpeg::Demuxer demuxer;
    bool success = demuxer.open("tests/assets/master_4k.mp4");

    if (!success) {
        std::cout << "FAILED: Could not open file\n";
        return EXIT_FAILURE;
    }

    std::cout << "PASSED\n";
    return EXIT_SUCCESS;
}
