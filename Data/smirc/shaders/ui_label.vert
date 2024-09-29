#version 400

in vec2 in_VertexPosition;

uniform vec2 u_Position;
uniform vec2 u_Size;
uniform vec2 u_Offset;
uniform mat4 u_MVP;

out vec2 vert_UV;

void main()
{
	vec2 pos = vec2(u_Position.x + u_Offset.x, u_Position.y) + in_VertexPosition * u_Size;
	vert_UV = vec2(in_VertexPosition.x, 1.0 - in_VertexPosition.y);
	gl_Position = u_MVP * vec4(pos.x, pos.y, 0.0, 1.0);
}
