#version 400

in vec2 in_VertexPosition;

uniform vec2 u_Position;
uniform vec2 u_Size;
uniform mat4 u_MVP;

void main()
{
	vec2 pos = u_Position + in_VertexPosition * u_Size;
	gl_Position = u_MVP * vec4(pos.x, pos.y, 0.0, 1.0);
}
