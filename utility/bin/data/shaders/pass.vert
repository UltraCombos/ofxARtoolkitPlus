#pragma include <header.glsl>

layout(location = 0) in vec4 position;
layout(location = 3) in vec2 texCoord;

out gl_PerVertex {
	vec4 gl_Position;
};

out block {
	vec2 texCoord;
} Out;

void main(void)
{
	Out.texCoord = texCoord;
	gl_Position = modelViewProjectionMatrix * position;
}