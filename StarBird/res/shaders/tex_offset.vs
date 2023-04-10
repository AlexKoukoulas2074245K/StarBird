#version 300 core

precision mediump float;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec2 normal;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

out vec2 uv_frag;
out vec3 frag_unprojected_pos;

void main()
{
    uv_frag = uv;
    gl_Position = proj * view * world * vec4(position, 1.0);
    frag_unprojected_pos = (world * vec4(position, 1.0f)).rgb;
}
