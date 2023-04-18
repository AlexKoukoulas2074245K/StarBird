#version 300 core

precision mediump float;

in vec2 uv_frag;

uniform sampler2D tex;
out vec4 frag_color;

void main()
{
    float finalUvX = uv_frag.x;
    float finalUvY = 1.0 - uv_frag.y;
    
    frag_color = texture(tex, vec2(finalUvX, finalUvY));
    
    if (frag_color.a < 0.25) discard;

    frag_color = vec4((vec3(frag_color.r + frag_color.g + frag_color.b)/10.0f).rgb, frag_color.a);
}
