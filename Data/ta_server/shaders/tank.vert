#version 400

in vec2 in_VertexPosition;

uniform vec2 u_Position;
uniform vec2 u_Size;
uniform mat4 u_MVP;

out vec2 vert_UV;

void main()
{
	vert_UV = in_VertexPosition;

	vec2 pos = u_Position + in_VertexPosition * u_Size;
	gl_Position = u_MVP * vec4(pos.x, pos.y, 0.0, 1.0);
}
