extends ASAbility

func _on_activate_ability(owner: Object, _spec: RefCounted) -> void:
	var asc = owner as ASComponent
	var agent = asc.get_parent()
	
	# We use ASTask to wait for the animation/timing
	var wait_task = ASTask.wait_delay(asc, 0.5)
	wait_task.completed.connect(func():
		var bt_player = agent.get_node_or_null("BTPlayer")
		if bt_player:
			var pos = bt_player.blackboard.get_var(&"minion_pos", agent.global_position)
			if agent.has_method(&"summon_minion"):
				agent.summon_minion(pos)
		end_ability(asc, _spec)
	)
	wait_task.activate()
