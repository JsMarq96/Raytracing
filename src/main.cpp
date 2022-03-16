#include <iostream>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "color.h"
#include "quaternion.h"
#include "render_quad.h"
#include "rt_primitives.h"
#include "rt_scene.h"

#include "input_layer.h"

// Dear IMGUI
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"
#include "texture_gpu.h"

#define WIN_WIDTH	640
#define WIN_HEIGHT	480
#define WIN_NAME	"Test"

sInputLayer input_state = {};

void temp_error_callback(int error_code, const char* descr) {
	std::cout << "GLFW Error: " << error_code << " " << descr << std::endl;
}

// INPUT MOUSE CALLBACk
void key_callback(GLFWwindow *wind, int key, int scancode, int action, int mods) {
	// ESC to close the game
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(wind, GL_TRUE);
	}

	eKeyMaps pressed_key;
	switch(key) {
		case GLFW_KEY_W:
			pressed_key = W_KEY;
			break;
		case GLFW_KEY_A:
			pressed_key = A_KEY;
			break;
		case GLFW_KEY_S:
			pressed_key = S_KEY;
			break;
		case GLFW_KEY_D:
			pressed_key = D_KEY;
			break;
		case GLFW_KEY_UP:
			pressed_key = UP_KEY;
			break;
		case GLFW_KEY_DOWN:
			pressed_key = DOWN_KEY;
			break;
		case GLFW_KEY_RIGHT:
			pressed_key = RIGHT_KEY;
			break;
		case GLFW_KEY_LEFT:
			pressed_key = LEFT_KEY;
			break;
	};

	input_state.keyboard[pressed_key] = (action == GLFW_PRESS) ? KEY_PRESSED : KEY_RELEASED;
}

void mouse_button_callback(GLFWwindow *wind, int button, int action, int mods) {
	char index;

	switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT:
		index = LEFT_CLICK;
		break;

	case GLFW_MOUSE_BUTTON_RIGHT:
		index = RIGHT_CLICK;
		break;

	case GLFW_MOUSE_BUTTON_MIDDLE:
		index = MIDDLE_CLICK;
		break;
	}

	input_state.mouse_clicks[index] = (action == GLFW_PRESS) ? KEY_PRESSED : KEY_RELEASED;
}

void cursor_enter_callback(GLFWwindow *window, int entered) {
	sInputLayer *input = get_game_input_instance();
	input->is_mouse_on_screen = entered;
}

void draw_loop(GLFWwindow *window) {
	glfwMakeContextCurrent(window);

	// Config scene
	sCamera camera;
	sVector3 camera_original_position = sVector3{0.0f, 0.0f, -1.0f};
	camera.position = camera_original_position;
	camera.look_at(sVector3{0.0f, 0.0f, 2.0f});

	double prev_frame_time = glfwGetTime();

	float camera_angle = 274.001f;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	float yaw, pitch;

	sVector3 light_position = {0.0f, 6.0f, 0.0f};

	// RT SCENE CONFIG
	sRT_Scene rt_scene = {};
	rt_scene.init();

	rt_scene.is_obj_enabled[0] = true;
	rt_scene.obj_primitive[0] = RT_SPHERE;
	rt_scene.obj_transforms[0].scale = {0.25f, 0.5f, 0.0f};
	rt_scene.obj_transforms[0].position = {0.0f, 0.0f, 2.0f};
	rt_scene.obj_material[0].albedo = {255, 0, 0, 1};

	rt_scene.is_obj_enabled[1] = true;
	rt_scene.obj_primitive[1] = RT_SPHERE;
	rt_scene.obj_transforms[1].scale = {0.35f, 0.5f, 0.0f};
	rt_scene.obj_transforms[1].position = {0.35f, 0.4f, 4.0f};
	rt_scene.obj_material[1].albedo = {0, 255, 0, 1};


	rt_scene.is_obj_enabled[2] = true;
	rt_scene.obj_primitive[2] = RT_SPHERE;
	rt_scene.obj_transforms[2].scale = {1000.0f, 0.5f, 0.0f};
	rt_scene.obj_transforms[2].position = {0.0f, -1000.7f, 0.0f};
	rt_scene.obj_material[2].albedo = {0, 255, 255, 1};


	rt_scene.light_position = {-1.0f, 5.0f, 0.0f};
	rt_scene.light_color = {255, 255, 255, 255};

	sGPU_Texture framebuffer = {};

	sQuadRenderer quad_render = {};
	quad_render.init();

	float FOV = 100.0f;
	while(!glfwWindowShouldClose(window)) {
		// Draw loop
		int width, heigth;
		double temp_mouse_x, temp_mouse_y;


		glfwGetFramebufferSize(window, &width, &heigth);
		// Set to OpenGL viewport size anc coordinates
		glViewport(0,0, width, heigth);


		camera.look_at({0.0f, 0.0f, 2.0f});

		rt_scene.render_heigth = heigth / 2;
		rt_scene.render_width = width / 2;

		float aspect_ratio = (float) width / heigth;

		// FRAME UPDATE ===============

		// FRAME RENDER ===============
		glClearColor(0.5f, 0.0f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		ImGui_ImplOpenGL3_NewFrame();
    	ImGui_ImplGlfw_NewFrame();
    	ImGui::NewFrame();

		double curr_frame_time = glfwGetTime();
		double elapsed_time = curr_frame_time - prev_frame_time;
		prev_frame_time = curr_frame_time;

		rt_scene.render_to_texture(camera,
								   FOV,
								   &framebuffer);

		quad_render.render(framebuffer.gpu_id);

		ImGui::Begin("Scene control");
		ImGui::SliderFloat3("Light position", rt_scene.light_position.raw_values, -7.0f, 7.0f);
		ImGui::SliderFloat3("Camera position", camera.position.raw_values, -5.0f, 5.0f);
		ImGui::SliderFloat("FOV", &FOV, 0.0f, 120.0f);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

int main() {
	if (!glfwInit()) {
		return EXIT_FAILURE;
	}
	
	// GLFW config
	glfwSetErrorCallback(temp_error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	
	
	GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WIN_NAME, NULL, NULL);

	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorEnterCallback(window, cursor_enter_callback);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (!window) {
		std::cout << "Error, could not create window" << std::endl; 
	} else {
		if (!gl3wInit()) {
			// IMGUI version
			//IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO &io = ImGui::GetIO();
			// Platform IMGUI
			ImGui_ImplGlfw_InitForOpenGL(window, true);
			ImGui_ImplOpenGL3_Init("#version 130");
			ImGui::StyleColorsDark();
			draw_loop(window);
		} else {
			std::cout << "Cannot init gl3w" << std::endl;
		}
		
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
