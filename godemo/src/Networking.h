#pragma once
// SPDX-License-Identifier: Unlicense

#include "godot_cpp/classes/object.hpp"

namespace godot
{
    class ClassDB;
};

class GDExtensionTemplate : public godot::Object
{
    GDCLASS( GDExtensionTemplate, godot::Object )

public:
    godot::Control* game;
    godot::VBoxContainer* address_entry;
    godot::Label* label;

    static godot::String version();

    void _ready() override;

private:
    static void _bind_methods();
};
