#include <imgui/imgui.h>
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include "gl3w/gl3w.h"
#include "glfw/glfw3.h"
#include "escapi/escapi.h"
#include <iostream>
#include <basetsd.h>
#include "gl3w/glcorearb.h"

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(1280, 720, "ViCon", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gl3wInit();

    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(window, true);

    // Setup ESCAPI
	auto devices = setupESCAPI();
	if (devices == 0)
	{
		std::cout << "\nNo devices found!" << std::endl;
		return-1;
	}

	auto devicenr = 1;

	struct SimpleCapParams capture;
	capture.mWidth = 320;
	capture.mHeight = 240;
	capture.mTargetBuf = new INT32[320 * 240 * sizeof(INT32)];

	if (initCapture(devicenr, &capture) == 0)
	{
		std::cout << "\nThe device may already be in use!" << std::endl;
		return-1;
	}
	
	doCapture(devicenr);

	while (isCaptureDone(devicenr) == 0) { /* Wait for first frame */ }

	auto show_test_window = true;
	auto show_another_window = false;
    ImVec4 clear_color = ImColor(114, 144, 154);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();

        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        {
            static auto f = 0.0f;
            ImGui::Text("Hello, world!");
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", reinterpret_cast<float*>(&clear_color));
            if (ImGui::Button("Test Window")) show_test_window ^= 1;
            if (ImGui::Button("Another Window")) show_another_window ^= 1;
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			if (isCaptureDone(devicenr) == 1) doCapture(devicenr);
			
			GLuint id;
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, capture.mWidth, capture.mHeight, 0,
				                           GL_RGBA, GL_UNSIGNED_INT, capture.mTargetBuf);
			glBindTexture(GL_TEXTURE_2D, 0);

			ImGui::Image(reinterpret_cast<GLuint*>(id), ImVec2(capture.mWidth, capture.mHeight));	        
        }

        // 2. Show another simple window, this time using an explicit Begin/End pair
        if (show_another_window)
        {
            ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello");
            ImGui::End();
        }

        // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
        if (show_test_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
            ImGui::ShowTestWindow(&show_test_window);
        }

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();

    return 0;
}
