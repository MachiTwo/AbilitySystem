extends ASComponent

func _on_calculate_conditional_tag(tag: StringName) -> bool:
	if tag == &"state.mock_low_health":
		return get_attribute_value_by_tag(&"attribute.health") < 50.0
	if tag == &"state.mock_full_health":
		return get_attribute_value_by_tag(&"attribute.health") >= 100.0
	return false
