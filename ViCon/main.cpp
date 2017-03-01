#include <imgui/imgui.h>
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include "gl3w/gl3w.h"
#include "glfw/glfw3.h"
#include "escapi/escapi.h"
#include <iostream>
#include "gl3w/glcorearb.h"
#include <chrono>
#include "NetworkReceiver.h"
#include "NetworkSender.h"
#include "ImageContainer.h"

using namespace std::chrono;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

bool SetupEscapi(SimpleCapParams&, GLuint&, int);
void CreateDebugdataWindow(int, ImVec4&, int, float*, int);
void CreateWebcamWindow(GLuint, int, int);

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
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
	struct SimpleCapParams capture;
	capture.mHeight = 640;
	capture.mWidth  = 800;
	auto devicenr = 1;
	ImageContainer rawWebcamContainer(capture.mWidth, capture.mHeight);
    SetupEscapi(capture, rawWebcamContainer.GetTexture(), devicenr);
    
	// Setup GUI variables
	ImVec4 clear_color = ImColor(114, 144, 154);

	// Setup frametiming
	int time = system_clock::now().time_since_epoch().count() / 1000;

	// Setup frameratecounter
	const auto frametimesCount = 20;
	float frametimes[frametimesCount];
	auto framesum = 0;
	auto framesumcount = 0;
	auto lastFrameNote = time;

	// Remove console
	FreeConsole();

	// Setup networking
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "WSAStartup failed." << std::endl;
		return 1;
	}

	char* message = "Hello, world!";
	NetworkReceiver receiver("666");
	NetworkSender sender("666", "localhost");
	sender.Send(message, strlen(message));
	auto package = receiver.Receive();
	delete[] package;
	
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

		// Get next webcam frame if ready
		if (isCaptureDone(devicenr) == 1) {
			// Swap red and blue to convert from BGRA to RGBA
			for (auto i = 0; i < capture.mWidth * capture.mHeight; i++)
				capture.mTargetBuf[i] = capture.mTargetBuf[i] & 0xff00ff00 |
				(capture.mTargetBuf[i] & 0xff) << 16 |
				(capture.mTargetBuf[i] & 0xff0000) >> 16 |
				0xff000000; // And set alpha to 255 to make it opaque

			glBindTexture(GL_TEXTURE_2D, rawWebcamTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, capture.mWidth, capture.mHeight,
				0, GL_RGBA, GL_UNSIGNED_BYTE, static_cast<GLvoid*>(capture.mTargetBuf));

			doCapture(devicenr);
		}

		// Get frame size
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);

        // Debug data window
		CreateDebugdataWindow(display_h, clear_color, framerate, frametimes, frametimesCount);

        // Webcam window
		CreateWebcamWindow(rawWebcamTexture, capture.mWidth, capture.mHeight);

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

bool SetupEscapi(SimpleCapParams &capture, GLuint &texture, int devicenr)
{
	capture.mWidth = 500;
	capture.mHeight = 500;
	capture.mTargetBuf = new int[capture.mWidth * capture.mHeight];

	auto devices = setupESCAPI();
	if (devices == 0)
	{
		std::cout << "ESCAPI initialization failure or no devices found." << std::endl;
		return false;
	}
	
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Filtering
	
	if (initCapture(devicenr, &capture) == 0) {
		std::cout << "ESCAPI: The device may already be in use!" << std::endl;
		return false;
	}

	doCapture(devicenr);
	while (isCaptureDone(devicenr) == 0) { /* Wait for first frame */ }

	return true;
}

void CreateDebugdataWindow(int windowHeight, ImVec4 &clear_color, int framerate, float* frametimes, int frametimesCount)
{
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(400, windowHeight), ImGuiSetCond_Always);
	ImGui::Begin("Debug data", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	static auto f = 0.0f;
	ImGui::Text("Hello, world!");
	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
	ImGui::ColorEdit3("clear color", reinterpret_cast<float*>(&clear_color));
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / framerate, framerate);
	ImGui::PlotHistogram("Frametimes", frametimes, frametimesCount, 0,
		nullptr, FLT_MAX, FLT_MAX, ImVec2(300, 50));
	ImGui::End();
}

void CreateWebcamWindow(GLuint texture, int width, int height)
{
	ImGui::SetNextWindowSize(ImVec2(500, 530), ImGuiSetCond_Always);
	ImGui::Begin("Webcam live feed");
	ImGui::Image(reinterpret_cast<ImTextureID>(texture), ImVec2(width, height));
	ImGui::End();
}