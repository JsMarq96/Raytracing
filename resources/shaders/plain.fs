#version 330 core
out vec4 FragColor;

varying vec2 v_uv;

uniform sampler2D u_albedo_map;

void main()
{
    vec4 color = texture(u_albedo_map, v_uv);
    if (color.a < 0.01) {
       discard;
    }
    FragColor = vec4(color.rgb, 1.0);
}
