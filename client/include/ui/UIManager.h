#pragma once


#include <string>
#include <memory>
#include <iostream>

#include "imgui.h"
#include "stb_image.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "SetupParser.h"
#include "sge/ShittyGraphicsEngine.h"

namespace ui {
	struct Character {
		std::string imagePath;

		// this number is used for rendering in lobby screen - not universal ID
		GLuint textureID;

		// this number is used to identified the character in the server and in the game
		int characterUID;
	};


	class UIManager {
	public:
		UIManager();
		~UIManager();
		void lobby();

		GLuint LoadTextureFromFile(std::string filename);
		void LoadCharacterImages();


		// which character we choose - this is the textureID
		int selectedCharacterUID = NO_CHARACTER;
		bool isLobbySelectionSent = false;




	private:
		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


		// the index of the texture
		int selectedIndex = 0;
		std::vector<int> textures;
 


		// List of characters (example)
		std::vector<Character> characters = {
			{SetupParser::getValue("spring-character"), 0, SPRING_CHARACTER},
			{SetupParser::getValue("summer-character"), 0, SUMMER_CHARACTER},
			{SetupParser::getValue("fall-character"), 0, FALL_CHARACTER},
			{SetupParser::getValue("winter-character"), 0, WINTER_CHARACTER}
		};
	};

	extern std::unique_ptr<UIManager> uiManager;

	// whether we are in lobby screen or not
	extern bool isInLobby;
	// whether we are transition from the lobby to the game
	extern bool isTransitioningToGame;

	void initUIManager();
}