#ifndef GODOT_AV_AV_TEST_NODE_H
#define GODOT_AV_AV_TEST_NODE_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {

class AVTestNode : public Node {
    GDCLASS(AVTestNode, Node)

public:
    AVTestNode();
    ~AVTestNode();

    void _ready() override;

protected:
    static void _bind_methods();
};

}

#endif // GODOT_AV_AV_TEST_NODE_H
