#version 330 core

in vec2 uv;

out vec4 color;

uniform sampler2D u_color_text;
uniform sampler2D u_depth_text;

float linearize_depth(float d, float zNear, float zFar) {
    float z_n = 2.0 * d - 1.0;
    return 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
}

void main() {
     float depth = linearize_depth(texture(u_depth_text, uv).r, 0.0001, 1000.0);
     //depth = 1.0 - depth;
     //depth = depth * depth;

     color = vec4(texture(u_color_text, uv).rgb, 1.0);
     //color = vec4(depth, depth, depth, 1.0);
}

// 		{ GL_DEPTH_COMPONENT32,GL_ZERO,      GL_DEPTH_COMPONENT,   GL_UNSIGNED_INT,     false }, // D32
