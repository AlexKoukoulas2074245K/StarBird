#version 300 core

precision mediump float;

in vec2 uv_frag;
in vec3 frag_unprojected_pos;

uniform sampler2D noise_tex;
uniform vec4 ambient_light_color;
uniform vec4 point_light_colors[32];
uniform vec3 point_light_positions[32];
uniform float point_light_powers[32];
uniform float tex_offset_x;
uniform float tex_offset_y;
uniform bool affected_by_light;
uniform int active_light_count;
out vec4 frag_color;

void main()
{
    float final_uv_x = uv_frag.x;
    float final_uv_y = 1.0 - uv_frag.y;
    
    vec4 noise_color = texture(noise_tex, vec2(final_uv_x + tex_offset_x, final_uv_y + tex_offset_y));
    
    float distanceFromCenter = distance(vec2(0.5f, 0.5f), vec2(final_uv_x, final_uv_y));
    
    frag_color = vec4(0.2f * noise_color.r, 0.2f * noise_color.r, noise_color.r, noise_color.r * pow(1.0f - distanceFromCenter, 5.0f));
        
    if (frag_color.a < 0.05f) discard;
    
    frag_color *= 5.0f;
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
