#version 300 core

precision mediump float;

in vec2 uv_frag;
in vec3 frag_unprojected_pos;

uniform sampler2D tex;
uniform sampler2D dissolveTex;
uniform float dissolve_y_offset;
uniform vec4 ambient_light_color;
uniform vec4 point_light_colors[32];
uniform vec3 point_light_positions[32];
uniform float point_light_powers[32];
uniform bool affected_by_light;
uniform int active_light_count;
out vec4 frag_color;

void main()
{
    float finalUvX = uv_frag.x;
    float finalUvY = 1.0 - uv_frag.y;
    frag_color = texture(tex, vec2(finalUvX, finalUvY));
    
    if (frag_color.a < 0.25) discard;
    
    vec4 dissolve_color = texture(dissolveTex, vec2(finalUvX, finalUvY + dissolve_y_offset));
    
    if (1.0f - dissolve_y_offset > finalUvY)
    {
        frag_color = dissolve_color;
    }
    
    if (dissolve_color.a < 0.2) discard;
    
    frag_color = clamp(frag_color, 0.0f, 1.0f);
    
    if (affected_by_light)
    {
        vec4 light_accumulator = vec4(0.0f, 0.0f, 0.0f, 0.0f);
        
        for (int i = 0; i < active_light_count; ++i)
        {
            float dst = distance(point_light_positions[i], frag_unprojected_pos);
            float attenuation = point_light_powers[i] / (dst * dst);
                    
            light_accumulator.rgb += (point_light_colors[i] * attenuation).rgb;
        }
        
        frag_color = frag_color * ambient_light_color + light_accumulator;
    }
}
