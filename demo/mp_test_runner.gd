extends SceneTree

# mp_test_runner.gd
# Entry point for multiplayer tests in GHA / CLI

func _init():
	print("[MP-TEST] Starting integrated AbilitySystem tests...")
	
	# Get network configuration from environment
	var player_id = OS.get_environment("MP_GHA_PLAYER_ID")
	var server_port = OS.get_environment("MP_SERVER_PORT")
	
	if player_id:
		print("[MP-TEST] Player ID: ", player_id)
		# TODO: Configure ENet/MultiplayerAPI if specific networked test-cases are added
	
	# Run the actual C++ doctest suite exposed via GDExtension
	# This will execute all tests marked with doctest macros.
	var result = AbilitySystem.run_tests()
	
	if result == 0:
		print("[MP-TEST] All tests passed!")
	else:
		print("[MP-TEST] Tests failed with status: ", result)
	
	# Quit with the doctest result as exit code
	quit(result)
