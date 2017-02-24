#include <imgui/imgui.h>
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include "gl3w/gl3w.h"
#include "glfw/glfw3.h"
#include "escapi/escapi.h"
#include <iostream>
#include "gl3w/glcorearb.h"
#include <chrono>

using namespace std::chrono;

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
		std::cout << "ESCAPI initialization failure or no devices found." << std::endl;
		return-1;
	}
	auto devicenr = 1;
	struct SimpleCapParams capture;
	capture.mWidth = 256;
	capture.mHeight = 256;
	capture.mTargetBuf = new int[256 * 256];
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Filtering
	if (initCapture(devicenr, &capture) == 0)
	{
		std::cout << "ESCAPI: The device may already be in use!" << std::endl;
		return-1;
	}
	doCapture(devicenr);
	while (isCaptureDone(devicenr) == 0) { /* Wait for first frame */ }

	// Setup GUI variables
	ImVec4 clear_color = ImColor(114, 144, 154);

	// Setup frametiming
	int time = system_clock::now().time_since_epoch().count() / 1000;

	// Setup frameratecounter
	auto frametimesCount = 20;
	auto frametimes = new float[frametimesCount];
	auto framesum = 0;
	auto framesumcount = 0;
	auto lastFrameNote = time;
	
    // Main loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate time data
		int newtime = system_clock::now().time_since_epoch().count()/1000;
		auto deltatime = newtime - time;
		auto framerate = ImGui::GetIO().Framerate;

		// Poll events, start new frame 
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		// Set framerate data
		framesum += framerate;
		framesumcount++;
		if (newtime - lastFrameNote > 1000 && framesumcount > 0) {
			for (auto i = 0; i < frametimesCount-1; i++)
				frametimes[i] = frametimes[i + 1];
			frametimes[frametimesCount-1] = framesum / framesumcount;
			lastFrameNote = newtime;
			framesum = 0;
			framesumcount = 0;
		}

		// Get frame size
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);

        // Debug data window
        {
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(400, display_h), ImGuiSetCond_Always);
			ImGui::Begin("Debug data", nullptr, ImGuiWindowFlags_NoMove |
											    ImGuiWindowFlags_NoResize |
				                                ImGuiWindowFlags_NoCollapse |
											    ImGuiWindowFlags_NoBringToFrontOnFocus);
			
            static auto f = 0.0f;
            ImGui::Text("Hello, world!");
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", reinterpret_cast<float*>(&clear_color));
            
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / framerate, framerate);
			
			ImGui::PlotHistogram("Frametimes", frametimes, frametimesCount, 0, 0, 
				FLT_MAX, FLT_MAX, ImVec2(300, 50));
			ImGui::End();
		}

        // Webcam window
        {
            ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiSetCond_Always);
            ImGui::Begin("Webcam live feed");
            
			if (isCaptureDone(devicenr) == 1) {
				for (auto i = 0; i < 256 * 256; i++)
					capture.mTargetBuf[i] = capture.mTargetBuf[i] & 0xff00ff00 |
					(capture.mTargetBuf[i] & 0xff) << 16 |
					(capture.mTargetBuf[i] & 0xff0000) >> 16 |
					(0xff000000);

				glBindTexture(GL_TEXTURE_2D, texture);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, capture.mWidth, capture.mHeight,
					0, GL_RGBA, GL_UNSIGNED_BYTE, static_cast<GLvoid*>(capture.mTargetBuf));

				doCapture(devicenr);
			}

			ImVec2 picsize(capture.mWidth, capture.mHeight);
			ImGui::Image(reinterpret_cast<ImTextureID>(texture), picsize);

            ImGui::End();
        }

        // Rendering
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        glfwSwapBuffers(window);

		time = newtime;
    }

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();

    return 0;
}
