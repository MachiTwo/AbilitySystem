class_name Hotbar
extends Resource

signal selection_changed(new_item)

@export var items: Array[ItemData] = []
@export var selected_index: int = 0

func select_slot(index: int) -> void:
	if items.is_empty(): return
	
	# Wrap around logic
	if index >= items.size():
		index = 0
	elif index < 0:
		index = items.size() - 1
		
	selected_index = index
	emit_signal("selection_changed", get_current_item())

func next_item() -> void:
	select_slot(selected_index + 1)

func prev_item() -> void:
	select_slot(selected_index - 1)

func get_current_item() -> ItemData:
	if items.is_empty(): return null
	if selected_index >= 0 and selected_index < items.size():
		return items[selected_index]
	return null
