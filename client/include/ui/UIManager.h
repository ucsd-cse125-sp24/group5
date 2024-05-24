#pragma once


#include <string>
#include <memory>
#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "SetupParser.h"

namespace ui {


	class UIManager {
	public:
		UIManager();
		~UIManager();



	private:


	};

	extern std::unique_ptr<UIManager> uiManager;
	void initUIManager();
}