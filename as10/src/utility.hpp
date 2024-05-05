#pragma once

#include "godot_cpp/classes/engine.hpp"
#define DO_NOT_RUN_IN_EDITOR(...) if(godot::Engine::get_singleton()->is_editor_hint()) return __VA_ARGS__

#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/multiplayer_api.hpp"
#include "godot_cpp/classes/multiplayer_peer.hpp"
#include "godot_cpp/classes/node.hpp"
#include <concepts>

namespace godot {
	struct RPC: public Callable {
		using Callable::Callable;
		using Callable::operator=;
		RPC(RPC &&other) : Callable(std::move(other)) {}

		// This method allows an arbirary RPC to registered
		static Error setup_rpc(godot::Node* node, godot::StringName method, 
			godot::MultiplayerAPI::RPCMode rpc_mode = godot::MultiplayerAPI::RPC_MODE_AUTHORITY, 
			bool call_local = false, 
			int channel = 0, 
			godot::MultiplayerPeer::TransferMode transfer_mode = godot::MultiplayerPeer::TRANSFER_MODE_RELIABLE
		){
			if(node == nullptr) return godot::Error::ERR_DOES_NOT_EXIST;
			if(!node->has_method(method)) return godot::Error::ERR_DOES_NOT_EXIST;

			godot::Dictionary rpc_config_dict;
			rpc_config_dict["rpc_mode"] = rpc_mode;
			rpc_config_dict["channel"] = channel;
			rpc_config_dict["call_local"] = call_local;
			rpc_config_dict["transfer_mode"] = transfer_mode;
			node->rpc_config(method, rpc_config_dict);
			return godot::Error::OK;
		}

		// This method needs to be called in _ready to setup this specific RPC!
		Error setup_in_ready(
			godot::MultiplayerAPI::RPCMode rpc_mode = godot::MultiplayerAPI::RPC_MODE_AUTHORITY, 
			bool call_local = false, 
			int channel = 0, 
			godot::MultiplayerPeer::TransferMode transfer_mode = godot::MultiplayerPeer::TRANSFER_MODE_RELIABLE
		) {
			return setup_rpc(get_object()->cast_to<godot::Node>(), get_method(), rpc_mode, call_local, channel, transfer_mode);
		}

		template<class... Args>
		void operator()(const Args&... args) const {
			rpc(args...);
		}
	};
}