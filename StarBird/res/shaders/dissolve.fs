#version 300 core

precision mediump float;

in vec2 uv_frag;

uniform sampler2D tex;
uniform sampler2D dissolveTex;

uniform float dissolve_y_offset;

out vec4 frag_color;

void main()
{
    float finalUvX = uv_frag.x;
    float finalUvY = 1.0 - uv_frag.y;
    frag_color = texture(tex, vec2(finalUvX, finalUvY));
    
    if (frag_color.a < 0.2) discard;
    
    vec4 dissolve_color = texture(dissolveTex, vec2(finalUvX, finalUvY + dissolve_y_offset));
    
    if (1.0f - dissolve_y_offset > finalUvY)
    {
        frag_color = dissolve_color;
    }
    
    if (dissolve_color.a < 0.2) discard;
    
    frag_color = clamp(frag_color, 0.0f, 1.0f);
}
