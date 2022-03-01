#version 330 core

layout (location = 0) in vec3 a_vertex;
layout (location = 1) in vec2 a_uv;
layout (location = 2) in vec3 a_normal;

varying vec2 v_uv;

uniform mat4 u_model_mat;
uniform mat4 u_viewproj_mat;

void main() {
     v_uv = a_uv;
	//calcule the position of the vertex using the matrices
	gl_Position = u_viewproj_mat * u_model_mat * vec4(a_vertex, 1.0);
}
