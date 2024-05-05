// SPDX-License-Identifier: Unlicense

#include "Networking.h"
#include "godot_cpp/core/class_db.hpp"

#include "Version.h"
#include <godot_cpp/wariant/variant.hpp>

#include "utility.hpp"

void Networking::_read() {
    game = get_node<godot::Control>("../Game");
    //address_entry = $PanelContainer/VBoxContainer/IPAddress;
    //label = $"../Game/Label";

    //godot::RPC::setup_rpc(godot::Node* node, godot::StringName method);
    multiplayer()->multiplayer_peer();
    
    godot::print("hello");
}

godot::String Networking::version()
{
    return VersionInfo::VERSION_STR.data();
    godot::Variant v;
}

/// Bind our methods so GDScript can access them.
void Networking::_bind_methods()
{
    godot::ClassDB::bind_static_method( "Networking", godot::D_METHOD( "version" ),
                                        &Networking::version );
}
