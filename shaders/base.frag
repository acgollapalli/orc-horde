#version 450

#define MAX_TEXTURES_LOADED 1024


layout(location=0) in vec3 fragColor;
layout(location=1) in vec2 fragTexCoord;
layout(location=2) flat in int  fragTexLayer;

layout(location=0) out vec4 outColor;

layout(binding=1) uniform sampler2D texSampler[MAX_TEXTURES_LOADED];

void main() {
  outColor = texture(texSampler[fragTexLayer], fragTexCoord);
}
