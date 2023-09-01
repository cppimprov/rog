#version 400

in vec2 vert_UV;

uniform vec3 u_Color;

layout(location = 0) out vec4 out_Color;

void main()
{
	vec3 color = u_Color;

	out_Color = vec4(color, 1.0);
}
