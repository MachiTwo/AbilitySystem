
extends SceneTree

func _init():
	var ability = ASAbility.new()
	if "activation_cancel_tags" in ability:
		print("SUCCESS: activation_cancel_tags exists!")
		ability.activation_cancel_tags = ["test"]
		print("Tags: ", ability.activation_cancel_tags)
	else:
		print("FAILURE: activation_cancel_tags MISSING!")
	quit()
