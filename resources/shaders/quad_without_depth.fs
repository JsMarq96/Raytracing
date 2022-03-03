#version 330 core

in vec2 uv;

out vec4 color;

uniform sampler2D u_color_text;

void main() {
     color = vec4(texture(u_color_text, uv).rgb, 1.0);
}

