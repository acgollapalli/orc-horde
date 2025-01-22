#version 450

layout(binding = 0) uniform UniformBufferObject {
  mat4 view;
  mat4 proj;
} ubo;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 3) in vec3 instPosition;
layout(location = 4) in vec4 instRotation;
layout(location = 5) in float instScale;
layout(location = 6) in int instTexture;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

mat4 rotationFromQuaternion(vec3 position, vec4 quat) {
	 float w = quat.x;
	 float x = quat.y;
	 float y = quat.z;
	 float z = quat.w;
	 
	 vec4 v0 = vec4(1 - 2*(y*y + z*z), 2*(x*y + z*w), 2*(x*z - y*w), 0.0);
	 vec4 v1 = vec4(2*(x*y - z*w), 1 - 2*(x*x + z*z), 2*(y*z + x*w), 0.0);
	 vec4 v2 = vec4(2*(x*z + y*w), 2*(y*z - x*w), 1 - 2*(x*x + y*y), 0.0);
	 vec4 v3 = vec4(position, 1.0);

	 return mat4(v0, v1, v2, v3);
}

void main() {
  mat4 instTransform = rotationFromQuaternion(instPosition, instRotation);
  gl_Position = ubo.proj * ubo.view * instTransform * vec4(inPosition * instScale, 1.0);
  fragColor = inColor;
  fragTexCoord = inTexCoord;
}
