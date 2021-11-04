#version 400

in vec2 in_VertexPosition;
in vec2 in_TilePosition;

uniform vec2 u_TileSize;
uniform mat4 u_MVP;

out vec2 vert_UV;

void main()
{
	vert_UV = in_VertexPosition;
	gl_Position = u_MVP * vec4(in_TilePosition + in_VertexPosition * u_TileSize, 0.0, 1.0);
}
