extends Control

@onready var game : Control = $"../Game"
@onready var address_entry : LineEdit = $PanelContainer/VBoxContainer/IPAddress
@onready var label : Label = $"../Game/Label"
@onready var players : Label = $"../Game/PlayersDisplay"

const PORT : int = 6420
var peer : ENetMultiplayerPeer = ENetMultiplayerPeer.new()

func buttonCommon():
	set_visible(false)
	game.set_visible(true);
	
func _on_host_button_pressed(): # private functions start with underscore
	buttonCommon()
	
	peer.create_server(PORT)
	multiplayer.multiplayer_peer = peer
	multiplayer.peer_connected.connect(addPlayer)
	multiplayer.peer_disconnected.connect(removePlayer)
	label.updateValue(0)
	players.updateValue(multiplayer.get_unique_id())
	
func _on_join_button_pressed():
	buttonCommon()
	
	peer.create_client(address_entry.text, PORT)
	multiplayer.multiplayer_peer = peer
	
func addPlayer(peer_id: int):
	print(peer_id, " joined.")
	players.updateValue(peer_id)
	pass

func removePlayer(peer_id: int):
	print(peer_id, " left.")
	pass
