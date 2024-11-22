@tool

extends Node3D

class_name ShellSphere

@export var mesh: MeshInstance3D : set = set_mesh
func set_mesh(val: MeshInstance3D) -> void:
	mesh = val
	editor_reset()

@export var shell_count: int = 8 : set = set_shell_count
func set_shell_count(val: int) -> void:
	shell_count = val
	editor_reset()

@export var shell_offset_strength: float = 0.05 : set = set_shell_offset_strength
func set_shell_offset_strength(val: float) -> void:
	shell_offset_strength = val
	editor_reset()

@export var shell_strand_constant: float = 3.0 : set = set_shell_strand_constant
func set_shell_strand_constant(val: float) -> void:
	shell_strand_constant = val
	editor_reset()

@export var shell_strand_strength: float = 1.0 : set = set_shell_strand_strength
func set_shell_strand_strength(val: float) -> void:
	shell_strand_strength = val
	editor_reset()

@export var shell_strand_add_strength: float = 1.5 : set = set_shell_strand_add_strength
func set_shell_strand_add_strength(val: float) -> void:
	shell_strand_add_strength = val
	editor_reset()

@export var shell_strand_minus_strength: float = 1.0 : set = set_shell_strand_minus_strength
func set_shell_strand_minus_strength(val: float) -> void:
	shell_strand_minus_strength = val
	editor_reset()

@export var shell_strand_divide_constant: float = 1.0 : set = set_shell_strand_divide_constant
func set_shell_strand_divide_constant(val: float) -> void:
	shell_strand_divide_constant = val
	editor_reset()

@export var shell_top_color: Color = Color(0.3, 0.1, 0.1) : set = set_shell_top_color
func set_shell_top_color(val: Color) -> void:
	shell_top_color = val
	editor_reset()

@export var shell_bottom_color: Color = Color(0.7, 0.5, 0.5) : set = set_shell_bottom_color
func set_shell_bottom_color(val: Color) -> void:
	shell_bottom_color = val
	editor_reset()

@export var uv_scale: float = 50.0 : set = set_uv_scale
func set_uv_scale(val: float) -> void:
	uv_scale = val
	editor_reset()

@export var worley_columns: float = 5.0 : set = set_worley_columns
func set_worley_columns(val: float) -> void:
	worley_columns = val
	editor_reset()

@export var worley_rows: float = 5.0 : set = set_worley_rows
func set_worley_rows(val: float) -> void:
	worley_rows = val
	editor_reset()

@export var shell_movement: bool = true : set = set_shell_movement
func set_shell_movement(val: bool) -> void:
	shell_movement = val
	editor_reset()

@export var shell_movement_speed: float = 3.0 : set = set_shell_movement_speed
func set_shell_movement_speed(val: float) -> void:
	shell_movement_speed = val
	editor_reset()

@export var respawn_shells : bool = false : set = set_respawn_shells
func set_respawn_shells(val: bool) -> void:
	if Engine.is_editor_hint():
		shells_material = null
		init_shells_material()
		remove_shells()
		shells = null
		spawn_shells()

var shells: MultiMeshInstance3D = null
var shells_material: Material = null

func _ready() -> void:
	shells_material = null
	init_shells_material()
	remove_shells()
	shells = null
	spawn_shells()

func init_shells_material() -> void:
	if shells_material != null:
		return
	shells_material = ShaderMaterial.new()
	shells_material.shader = load("res://ShellSphere/shell_sphere_shader.gdshader")
	shells_material.set_shader_parameter("shell_offset_strength", shell_offset_strength)
	shells_material.set_shader_parameter("shell_strand_constant", shell_strand_constant)
	shells_material.set_shader_parameter("shell_strand_strength", shell_strand_strength)
	shells_material.set_shader_parameter("shell_strand_add_strength", shell_strand_add_strength)
	shells_material.set_shader_parameter("shell_strand_minus_strength", shell_strand_minus_strength)
	shells_material.set_shader_parameter("shell_strand_divide_constant", shell_strand_divide_constant)
	shells_material.set_shader_parameter("shell_top_color", Vector3(shell_top_color.r, shell_top_color.g, shell_top_color.b))
	shells_material.set_shader_parameter("shell_bottom_color", Vector3(shell_bottom_color.r, shell_bottom_color.g, shell_bottom_color.b))
	shells_material.set_shader_parameter("uv_scale", uv_scale)
	shells_material.set_shader_parameter("worley_columns", worley_columns)
	shells_material.set_shader_parameter("worley_rows", worley_rows)
	shells_material.set_shader_parameter("shell_movement", shell_movement)
	shells_material.set_shader_parameter("shell_movement_speed", shell_movement_speed)

func editor_reset() -> void:
	if Engine.is_editor_hint():
		reset_shells()

func reset_shells() -> void:
	shells_material = null
	init_shells_material()
	remove_shells()
	shells = null
	spawn_shells()

func spawn_shells() -> void:
	if shells == null:
		shells = MultiMeshInstance3D.new()
		add_child(shells)
		shells.set_owner(get_tree().get_edited_scene_root())
		var new_mesh : MeshInstance3D = mesh.duplicate(true)
		if shells.multimesh == null:
			shells.multimesh = MultiMesh.new()
			shells.multimesh.transform_format = MultiMesh.TRANSFORM_3D
			shells.multimesh.use_colors = true
			shells.multimesh.use_custom_data = true
			
			shells.multimesh.mesh = new_mesh.mesh
		shells.multimesh.instance_count = shell_count;
		
		for i in range(0, shell_count):
			shells.multimesh.set_instance_transform(i, Transform3D(Basis(), Vector3()))
			var grey = float(i) / float(shell_count)
			shells.multimesh.set_instance_color(i, Color(1.0, 1.0, 1.0, grey))
			shells.multimesh.set_instance_custom_data(i, Color(1.0, 1.0, 1.0, grey))
		for surface in new_mesh.mesh.get_surface_count():
			shells.multimesh.mesh.surface_set_material(surface, shells_material)

func remove_shells():
	for node in get_children():
		if node is MultiMeshInstance3D:
			node.queue_free()
