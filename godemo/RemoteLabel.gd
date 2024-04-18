extends Label

var second = 1
var counter = 0

func updateValue(newValue, senderID = 1):
	if !(is_multiplayer_authority() or senderID == 0):
		updateValueRPC.rpc_id(1, newValue, multiplayer.get_unique_id())
		return
	counter = newValue
	
	if is_multiplayer_authority() and senderID != 0:
		print(senderID, " has set the value to: ", counter)
		updateValueRPC.rpc(newValue, senderID)
	
# Remote Procedure Call
@rpc("any_peer") # any peer is allowed to call this RPC
func updateValueRPC(newValue, senderID):
	if is_multiplayer_authority() or senderID == 0:
		updateValue(newValue)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	if !is_multiplayer_authority(): return
	second -= delta
	if second < 0:
		second = 1
		updateValue(counter + 1)
		text = "Counter: " + str(counter)
