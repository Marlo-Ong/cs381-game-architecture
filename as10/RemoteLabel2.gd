extends Label

func _ready():
	text = "Players: "

func updateValue(senderID = 1):
	print("Update value: ", senderID)
	updatePlayersText(str(senderID))
	
	if is_multiplayer_authority() and senderID != 0:
		updateValueRPC.rpc(text)
	
func updatePlayersText(string):
	text = str(text, "\n", string)

@rpc("any_peer")
func updateValueRPC(senderID):
	print("RPC")
	updatePlayersText(str(senderID))
