#include "ui/UIManager.h"


std::unique_ptr<ui::UIManager> ui::uiManager;

void ui::initUIManager() {
	ui::uiManager = std::make_unique<ui::UIManager>();
}

ui::UIManager::UIManager() {

	
}

ui::UIManager::~UIManager() {
	std::cout << "UIManager is destroyed" << std::endl;
}