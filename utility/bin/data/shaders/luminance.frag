#pragma include <header.glsl>

uniform sampler2D tex;

in block {
	vec2 texCoord;
} In;

layout(location = 0) out vec4 fragColor;

void main(void)
{
	vec3 c = texture(tex, In.texCoord).rgb;
	float luminance = dot(c, vec3(0.299, 0.587, 0.114));
    fragColor = vec4(vec3(luminance), 1.0);
}