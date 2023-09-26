#version 400

in vec2 vert_UV;

uniform vec3 u_AccentColor;
uniform sampler2D u_Texture;
uniform sampler2D u_AccentTexture;

layout(location = 0) out vec4 out_Color;

void main()
{
	vec4 color = texture(u_Texture, vert_UV);
	float accent = texture(u_AccentTexture, vert_UV).a;

	vec3 final = mix(color.rgb, u_AccentColor, accent);

	out_Color = vec4(final, color.a);
}
