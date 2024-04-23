extends Control

@onready var game : Control = $"../Game"
@onready var address_entry : VBoxContainer = $PanelContainer/VBoxContainer/IPAddress
@onready var label : Label = $"../Game/Label"

const PORT : int = 6420
var peer : ENetMultiplayerPeer = ENetMultiplayerPeer.new()

func buttonCommon():
	hide()
	game.show()
	
func _on_host_button_pressed(): # private functions start with underscore
	buttonCommon()
	
	peer.create_server(PORT)
	multiplayer.multiplayer_peer = peer
	multiplayer.peer_connected.connect(addPlayer)
	multiplayer.peer_disconnected.connect(removePlayer)
	label.updateValue(0)
	
func _on_join_button_pressed():
	buttonCommon()
	
	peer.create_client(address_entry.text, PORT)
	multiplayer.multiplayer_peer = peer
	
func addPlayer(peer_id: int):
	print(peer_id)
	pass

func removePlayer(peer_id: int):
	print("ded")
	pass
