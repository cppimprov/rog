#version 400

uniform vec4 u_Color;

layout(location = 0) out vec4 out_Color;

void main()
{
	out_Color = u_Color;
}
