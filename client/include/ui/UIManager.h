#pragma once


#include <string>
#include <memory>
#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "SetupParser.h"
#include "sge/ShittyGraphicsEngine.h"

namespace ui {


	class UIManager {
	public:
		UIManager();
		~UIManager();
		void lobby();



	private:
		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


	};

	extern std::unique_ptr<UIManager> uiManager;

	// whether we are in lobby screen or not
	extern bool isInLobby;
	// whether we are transition from the lobby to the game
	extern bool isTransitioningToGame;

	void initUIManager();
}