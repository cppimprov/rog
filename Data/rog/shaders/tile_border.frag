#version 400

in vec3 vert_BorderColor;

layout(location = 0) out vec4 out_Color;

void main()
{
	out_Color = vec4(vert_BorderColor, 1.0);
}
