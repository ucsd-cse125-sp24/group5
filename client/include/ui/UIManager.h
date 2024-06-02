#pragma once


#include <string>
#include <memory>
#include <chrono>
#include <iostream>

#include "imgui.h"
#include "stb_image.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "SetupParser.h"
#include "sge/ShittyGraphicsEngine.h"
#include "Client.h"

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
		void LoadLobbyImages();

		bool charJustChanged();
		int getCurrentCharSelection();
		int getPrevCharSelection();

		// given a playerId get the character
		Character getBrowsingCharacter(int playerId);

		// check if player can select the current browsing character
		bool canSelectCharacter();




		// character display in one column - with indicator
		// in this columnIndex, display selection of the playerID
		// we will "modify" the columnIndex as needed - see implementation for explanation
		void characterDisplay(int columnIndex, int displayedPlayerID);


		// for debouncing the keyboard i.e prevent multiple click in short period
		bool isDebounced();


		// check if all players are finished selecting character
		bool areAllPlayersReady();



		// once we select a character, selected and browsing should show the same character
		// TODO: make this true
		// which character we choose
		int selectedCharacterUID = NO_CHARACTER;
		// which character we current browsing on
		int browsingCharacterUID = SPRING_CHARACTER;


		// determine whether the user selection choice has been sent to server
		bool isLobbySelectionSent = false;

		// whether the user can select this character or not
		// user cannot select a character their teammate already select
		bool isSelectionNotAllowed = false;



	private:
		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		// control options - from user's keyboard
		bool goUp = false;
		bool goDown = false;
		bool select = false;


		// the index of the texture (current character on select)
		int selectedIndex = 0;
		int prevSelectedIndex = -1;
		std::vector<int> textures;

		// background image
		GLuint backgroundImageTextureID;
		// secret - to hide which character this is
		GLuint secretCharacterTextureID;

		// downward triangle, to indicate which character the player is controlling
		GLuint redDownTriTextureID;
		// green mark, to indicate that the character is ready
		GLuint greenMarkTextureID;


		// size of each components in the lobby
		// need to set to correct value later in the lobby()
		ImVec2 windowSize = ImVec2(0, 0);
		ImVec2 imageSize = ImVec2(0, 0);
		float columnSize = 0;
		ImVec2 buttonSize = ImVec2(0, 0);
		ImVec2 indicatorSize = ImVec2(0, 0);




		// List of characters (example)
		std::vector<Character> characters = {
			{SetupParser::getValue("spring-character"), 0, SPRING_CHARACTER},
			{SetupParser::getValue("summer-character"), 0, SUMMER_CHARACTER},
			{SetupParser::getValue("fall-character"), 0, FALL_CHARACTER},
			{SetupParser::getValue("winter-character"), 0, WINTER_CHARACTER}
		};


		// for debouncing the keyboard
		std::chrono::high_resolution_clock::time_point lastClickTime;


	};

	extern std::unique_ptr<UIManager> uiManager;

	// whether we are in lobby screen or not
	extern bool isInLobby;
	// whether we are transition from the lobby to the game
	extern bool isTransitioningToGame;




	void initUIManager();
}