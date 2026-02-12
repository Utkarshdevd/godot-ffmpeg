#include "av_test_node.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

extern "C" {
#include <libavutil/avutil.h>
}

namespace godot {

void AVTestNode::_bind_methods() {}

void AVTestNode::_ready() {
    const char *version_cstr = av_version_info();
    String version = version_cstr ? String(version_cstr) : String("(unknown)");
    UtilityFunctions::print("[godot_av] FFmpeg version: ", version);
}

AVTestNode::AVTestNode() {}

AVTestNode::~AVTestNode() {}

}
