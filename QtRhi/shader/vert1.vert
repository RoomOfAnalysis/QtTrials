#version 440

layout(binding = 0, std140) uniform UniformBufferObject
{
    mat4 transform;
}ubo;
layout(binding = 2, std140) uniform UniformBufferObject1
{
    // gpu data alignment is different from c++ compiler, no float[3] or vec3 or mat3 is available for right alignment here
    // https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
    //vec2 bounds;
    //vec4 bounds;
    vec4 extents[2];
}ubo1;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 0) in vec3 inPosition;

void main()
{
    //fragTexCoord = vec2(clamp(inPosition.x / ubo1.bounds.x, 0.0, 1.0), clamp(inPosition.y / ubo1.bounds.y, 0.0, 1.0));
    //fragTexCoord = vec2(clamp((inPosition.x - ubo1.bounds.x) / (ubo1.bounds.y - ubo1.bounds.x), 0.0, 1.0), clamp((inPosition.y - ubo1.bounds.z) / (ubo1.bounds.w- ubo1.bounds.z), 0.0, 1.0));
    fragTexCoord = vec2(clamp((inPosition.x - ubo1.extents[0].x) / (ubo1.extents[1].x - ubo1.extents[0].x), 0, 1), clamp((inPosition.y - ubo1.extents[0].y) / (ubo1.extents[1].y - ubo1.extents[0].y), 0, 1));
    gl_Position = ubo.transform * vec4(inPosition, 1.0);
}