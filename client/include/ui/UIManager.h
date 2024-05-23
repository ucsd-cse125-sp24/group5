#pragma once


#include <string>
#include <memory>
#include <iostream>
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