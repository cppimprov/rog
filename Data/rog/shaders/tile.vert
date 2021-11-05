#version 400

in vec2 in_VertexPosition;
in vec2 in_TilePosition;
in float in_TileLayer;

uniform vec2 u_TileSize;
uniform mat4 u_MVP;

out vec2 vert_UV;
out float vert_TileLayer;

void main()
{
	vert_UV = in_VertexPosition;
	vert_TileLayer = in_TileLayer;
	vec2 pos = in_TilePosition + in_VertexPosition * u_TileSize;
	gl_Position = u_MVP * vec4(pos.x, 0.0, -pos.y, 1.0);
}
