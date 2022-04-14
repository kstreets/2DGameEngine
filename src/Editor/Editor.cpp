#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include "imgui/imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "../Components/Components.h"
#include "../Renderer/Renderer.h"
#include "../Core/Scene.h"
#include "../Core/Input.h"
#include "../Core/Prefabs.h"
#include "Editor.h"
#include <iostream>
#include <string>

bool Editor::showGizmos = false;
static int selectedPrefabId = 0;

static void DrawPrefabWindow();
static void PlacePrefab(GLFWwindow* window);

void Editor::Start(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.FontDefault = io.Fonts->AddFontFromFileTTF("src/Editor/JetBrainsMono-Regular.ttf", 20.0f);

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init((char*)glGetString(GL_VERSION_4_6));

	SetTheme();
}

void Editor::Update(GLFWwindow* window) {
	if (OnKeyHold(GLFW_KEY_G)) {
		showGizmos = true;
	}

	if (OnKeyHold(GLFW_KEY_H)) {
		showGizmos = false;
	}

	if (showGizmos) {
		RenderAllGizmos();
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::BeginMainMenuBar();
	if (ImGui::Button("Save")) {
		SaveScene(activeScene);
	}
	ImGui::EndMainMenuBar();

	DrawPrefabWindow();

	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse && !io.WantCaptureKeyboard) {
		PlacePrefab(window);
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

//PushID is needed because ImGUI uses textureId for the button Id.
//Since we use texture atlasing, textureId can be the same for many buttons.
void DrawPrefabWindow() {
	ImGui::Begin("Prefabs");
	for (int i = 0; i < 24; i++) {
		ImGui::PushID(i); 
		IconData data = Prefabs::GetPrefabIcon(i);
		if (ImGui::ImageButton(data.textureId, ImVec2(60, 60), data.uvMin, data.uvMax)) {
			selectedPrefabId = i;
		}
		ImGui::PopID();
	}
	ImGui::End();
}

void PlacePrefab(GLFWwindow* window) {
	if (!OnMousePress(GLFW_MOUSE_BUTTON_1)) return;

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	double xCenter = 1920.0 / 2.0;
	double xDist = xpos - xCenter;
	double clipX = xDist / xCenter;

	double yCenter = 1080.0 / 2.0;
	double yDist = (1080.0 - ypos) - yCenter;
	double clipY = yDist / yCenter;

	glm::vec4 clipSpaceCursorPos(clipX, clipY, 0.0, 1.0);

	Camera* mainCam = Camera::mainCam;
	glm::vec4 worldPos = glm::inverseTranspose(mainCam->projection) * glm::inverseTranspose(mainCam->view) * clipSpaceCursorPos;

	const float gridSize = 0.32f;

	int minX = glm::floor(worldPos.x / gridSize);
	int maxX = glm::ceil(worldPos.x / gridSize);
	int selectedX = (glm::abs(minX * gridSize - worldPos.x) < glm::abs(maxX * gridSize - worldPos.x)) ? minX : maxX;

	int minY = glm::floor(worldPos.y / gridSize);
	int maxY = glm::ceil(worldPos.y / gridSize);
	int selectedY = (glm::abs(minY * gridSize - worldPos.y) < glm::abs(maxY * gridSize - worldPos.y)) ? minY : maxY;

	float snappedX = selectedX * gridSize;
	float snappedY = selectedY * gridSize;

	Prefabs::PlacePrefab(selectedPrefabId, glm::vec3(snappedX, snappedY, 0));
}

void Editor::SetTheme() {
	auto& style = ImGui::GetStyle();
	style.ChildRounding = 0;
	style.GrabRounding = 0;
	style.FrameRounding = 0;
	style.PopupRounding = 0;
	style.ScrollbarRounding = 0;
	style.TabRounding = 0;
	style.WindowRounding = 0;
	style.FramePadding = { 4, 4 };

	style.WindowTitleAlign = { 0.0, 0.5 };
	style.ColorButtonPosition = ImGuiDir_Left;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = { 1.0f, 1.0f, 1.0f, 1.00f };				
	colors[ImGuiCol_TextDisabled] = { 0.25f, 0.25f, 0.25f, 1.00f };		
	colors[ImGuiCol_WindowBg] = { 0.09f, 0.09f, 0.09f, 1.00f };			
	colors[ImGuiCol_ChildBg] = { 0.11f, 0.11f, 0.11f, 1.00f };		
	colors[ImGuiCol_PopupBg] = { 0.11f, 0.11f, 0.11f, 0.94f };			
	colors[ImGuiCol_Border] = { 0.07f, 0.08f, 0.08f, 1.00f };
	colors[ImGuiCol_BorderShadow] = { 0.00f, 0.00f, 0.00f, 0.00f };
	colors[ImGuiCol_FrameBg] = { 0.35f, 0.35f, 0.35f, 0.54f };			
	colors[ImGuiCol_FrameBgHovered] = { 0.31f, 0.29f, 0.27f, 1.00f };
	colors[ImGuiCol_FrameBgActive] = { 0.40f, 0.36f, 0.33f, 0.67f };
	colors[ImGuiCol_TitleBg] = { 0.1f, 0.1f, 0.1f, 1.00f };
	colors[ImGuiCol_TitleBgActive] = { 0.3f, 0.3f, 0.3f, 1.00f };		
	colors[ImGuiCol_TitleBgCollapsed] = { 0.0f, 0.0f, 0.0f, 0.61f };
	colors[ImGuiCol_MenuBarBg] = { 0.18f, 0.18f, 0.18f, 0.94f };		
	colors[ImGuiCol_ScrollbarBg] = { 0.00f, 0.00f, 0.00f, 0.16f };
	colors[ImGuiCol_ScrollbarGrab] = { 0.24f, 0.22f, 0.21f, 1.00f };
	colors[ImGuiCol_ScrollbarGrabHovered] = { 0.31f, 0.29f, 0.27f, 1.00f };
	colors[ImGuiCol_ScrollbarGrabActive] = { 0.40f, 0.36f, 0.33f, 1.00f };
	colors[ImGuiCol_CheckMark] = { 0.84f, 0.84f, 0.84f, 1.0f };			
	colors[ImGuiCol_SliderGrab] = { 0.8f, 0.8f, 0.8f, 1.0f };					
	colors[ImGuiCol_SliderGrabActive] = { 0.55f, 0.55f, 0.55f, 1.00f }; 
	colors[ImGuiCol_Button] = { 0.55f, 0.55f, 0.55f, 0.40f };			
	colors[ImGuiCol_ButtonHovered] = { 0.15f, 0.15f, 0.15f, 0.62f };		
	colors[ImGuiCol_ButtonActive] = { 0.60f, 0.60f, 0.60f, 1.00f };		
	colors[ImGuiCol_Header] = { 0.84f, 0.36f, 0.05f, 0.0f };			
	colors[ImGuiCol_HeaderHovered] = { 0.25f, 0.25f, 0.25f, 0.80f };	
	colors[ImGuiCol_HeaderActive] = { 0.42f, 0.42f, 0.42f, 1.00f };
	colors[ImGuiCol_Separator] = { 0.35f, 0.35f, 0.35f, 0.50f };		
	colors[ImGuiCol_SeparatorHovered] = { 0.31f, 0.29f, 0.27f, 0.78f };
	colors[ImGuiCol_SeparatorActive] = { 0.40f, 0.36f, 0.33f, 1.00f };
	colors[ImGuiCol_ResizeGrip] = { 1.0f, 1.0f, 1.0f, 0.25f };			
	colors[ImGuiCol_ResizeGripHovered] = { 1.00f, 1.0f, 1.0f, 0.4f };	
	colors[ImGuiCol_ResizeGripActive] = { 1.00f, 1.00f, 1.0f, 0.95f };	
	colors[ImGuiCol_Tab] = { 0.18f, 0.18f, 0.18f, 1.0f };				
	colors[ImGuiCol_TabHovered] = { 0.58f, 0.58f, 0.58f, 0.80f };	
	colors[ImGuiCol_TabActive] = { 0.6f, 0.60f, 0.60f, 1.00f };
	colors[ImGuiCol_TabUnfocused] = { 0.07f, 0.10f, 0.15f, 0.97f };
	colors[ImGuiCol_TabUnfocusedActive] = { 0.14f, 0.26f, 0.42f, 1.00f };
	colors[ImGuiCol_PlotLines] = { 0.66f, 0.60f, 0.52f, 1.00f };
	colors[ImGuiCol_PlotLinesHovered] = { 0.98f, 0.29f, 0.20f, 1.00f };
	colors[ImGuiCol_PlotHistogram] = { 0.60f, 0.59f, 0.10f, 1.00f };
	colors[ImGuiCol_PlotHistogramHovered] = { 0.72f, 0.73f, 0.15f, 1.00f };
	colors[ImGuiCol_TextSelectedBg] = { 0.27f, 0.52f, 0.53f, 0.35f };
	colors[ImGuiCol_DragDropTarget] = { 0.60f, 0.59f, 0.10f, 0.90f };
	colors[ImGuiCol_NavHighlight] = { 0.51f, 0.65f, 0.60f, 1.00f };
	colors[ImGuiCol_NavWindowingHighlight] = { 1.00f, 1.00f, 1.00f, 0.70f };
	colors[ImGuiCol_NavWindowingDimBg] = { 0.80f, 0.80f, 0.80f, 0.20f };
	colors[ImGuiCol_ModalWindowDimBg] = { 0.11f, 0.13f, 0.13f, 0.35f };
}

void Editor::RenderAllGizmos() {
	const auto& dynamicBoxView = activeScene->registry.view<Transform, DynamicBox>();
	for (const auto& [entity, trans, box] : dynamicBoxView.each()) {
		RendererDebugDrawRect(trans.position, box.width, box.height);
	}

	const auto& staticBoxView = activeScene->registry.view<Transform, StaticBox>();
	for (const auto& [entity, trans, box] : staticBoxView.each()) {
		RendererDebugDrawRect(trans.position, box.width, box.height);
	}

	const auto& dynamicCircleView = activeScene->registry.view<Transform, DynamicCircle>();
	for (const auto& [entity, trans, circle] : dynamicCircleView.each()) {
		RendererDebugDrawCircle(trans.position, circle.radius);
	}

	const auto& triggerCircleView = activeScene->registry.view<Transform, TriggerCircle>();
	for (const auto& [entity, trans, circle] : triggerCircleView.each()) {
		RendererDebugDrawCircle(trans.position, circle.radius);
	}
}