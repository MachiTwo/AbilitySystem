@tool
extends EditorPlugin
class_name ASEditorPlugin

var inspector_plugin: ASInspectorPlugin
var tags_editor: ASTagsPanel

func _enter_tree():
	# Register C++ InspectorPlugin
	inspector_plugin = ASInspectorPlugin.new()
	add_inspector_plugin(inspector_plugin)

	# Register Tags Editor in Bottom Panel
	tags_editor = ASTagsPanel.new()
	print("ASTagsPanel created")

	# Add to Bottom Panel
	add_control_to_bottom_panel(tags_editor, "Ability System Tags")
	print("Added to Bottom Panel successfully")

func _exit_tree():
	# Unregister C++ InspectorPlugin
	if inspector_plugin:
		remove_inspector_plugin(inspector_plugin)

	# Remove Tags Editor from Bottom Panel
	if tags_editor and is_instance_valid(tags_editor):
		remove_control_from_bottom_panel(tags_editor)
		tags_editor.queue_free()
