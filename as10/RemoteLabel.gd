extends Label

var value = 0
@onready var playerDisplayText = $"../PlayersDisplay"

func updateValue(newValue, senderID = 1):    
	# senderID 1 = host, 0 = non-host client
	# host may not always be mult. authority
	if !is_multiplayer_authority() and senderID != 0:
		updateValueRPC.rpc_id(1, newValue, multiplayer.get_unique_id()) # only calls rpc for this specific id (1)
		return
		
	value = newValue
	text = "Value: " + String.num(value)
	
	# Make sure the value change gets propigated to all of the clients! 
	if is_multiplayer_authority() and senderID != 0:
		print(senderID, " has set the value to: ", value)
		updateValueRPC.rpc(newValue, 0)
		
func updatePlayersText(string):
	playerDisplayText.text = str(playerDisplayText.text, "\n", string)

@rpc("any_peer")
func updateValueRPC(newValue, senderID):
	if is_multiplayer_authority() or senderID == 0:
		updateValue(newValue, senderID)


func _on_increment_pressed():
	updateValue(value + 1)
	
func _on_decrement_pressed():
	updateValue(value - 1)
