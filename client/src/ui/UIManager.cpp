#include "ui/UIManager.h"


std::unique_ptr<ui::UIManager> ui::uiManager;
bool ui::isInLobby;
bool ui::isTransitioningToGame;


void lobbyKeyMapping(GLFWwindow* window, int key, int scancode, int action, int mods) {
	ImGuiIO& io = ImGui::GetIO();
	if (key == GLFW_KEY_UP)
		io.AddKeyEvent(ImGuiKey_UpArrow, action == GLFW_PRESS);
	if (key == GLFW_KEY_DOWN)
		io.AddKeyEvent(ImGuiKey_DownArrow, action == GLFW_PRESS);
	if (key == GLFW_KEY_ENTER)
		io.AddKeyEvent(ImGuiKey_Enter, action == GLFW_PRESS);
	if (key == GLFW_KEY_KP_ENTER)
		io.AddKeyEvent(ImGuiKey_KeypadEnter, action == GLFW_PRESS);
	if (key == GLFW_KEY_SPACE)
		io.AddKeyEvent(ImGuiKey_Space, action == GLFW_PRESS);

}

void ui::initUIManager() {
	ui::uiManager = std::make_unique<ui::UIManager>();
	ui::isInLobby = true;
	ui::isTransitioningToGame = false;
}

ui::UIManager::UIManager() {
	glfwSwapInterval(1);
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();


	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls




	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(sge::window, true);

	// GL 3.0 + GLSL 330
	// const char* glsl_version = "#version 130";
	const char* glsl_version = "#version 330";

#ifdef __EMSCRIPTEN__
	ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
#endif
	ImGui_ImplOpenGL3_Init(glsl_version);

	LoadLobbyImages();


	// start the key callback for the lobby
	glfwSetKeyCallback(sge::window, lobbyKeyMapping);

}

ui::UIManager::~UIManager() {
	std::cout << "UIManager is destroyed" << std::endl;
}




bool ui::UIManager::isDebounced() {
	// Set debounce time to 400 milliseconds
	std::chrono::milliseconds debounceTime(400);
	auto now = std::chrono::high_resolution_clock::now();
	if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastClickTime) >= debounceTime)
	{
		lastClickTime = now;
		return true;
	}
	return false;
}

GLuint ui::UIManager::LoadTextureFromFile(std::string filename) {
	int width, height;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, NULL, 4);
	if (data == nullptr) {
		return 0;
	}

	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

	stbi_image_free(data);

	return image_texture;
}

void ui::UIManager::LoadLobbyImages() {
	// load characters image
	for (int i = 0; i < characters.size(); i++) {
		characters[i].textureID = LoadTextureFromFile((std::string)(PROJECT_PATH)+characters[i].imagePath);
		textures.push_back(characters[i].textureID);
	}

	// load start background
	startBackgroundImageTextureID = LoadTextureFromFile((std::string)(PROJECT_PATH)+SetupParser::getValue("start-background"));
	// load start title
	startTitleTextureID = LoadTextureFromFile((std::string)(PROJECT_PATH)+SetupParser::getValue("start-text"));
	// load start button
	startButtonImageTextureID = LoadTextureFromFile((std::string)(PROJECT_PATH)+SetupParser::getValue("start-button"));



	// load lobby background image
	lobbyBackgroundImageTextureID = LoadTextureFromFile((std::string)(PROJECT_PATH)+SetupParser::getValue("lobby-background"));
	// load secret character
	secretCharacterTextureID = LoadTextureFromFile((std::string)(PROJECT_PATH)+SetupParser::getValue("secret-character"));

	// load indicators
	redDownTriTextureID = LoadTextureFromFile((std::string)(PROJECT_PATH)+SetupParser::getValue("arrow-image"));
	greenMarkTextureID = LoadTextureFromFile((std::string)(PROJECT_PATH)+SetupParser::getValue("greenmark-image"));

}

void ui::UIManager::LoadLobbyTextFonts() {
	ImGuiIO& io = ImGui::GetIO();
	// load lobby text font
	lobbyFont = io.Fonts->AddFontFromFileTTF(((std::string)(PROJECT_PATH)+SetupParser::getValue("font-path")).c_str(), 16.0f);
}

void ui::UIManager::characterDisplay(int columnIndex, int displayedPlayerID) {
	// offset - this is needed to make the image vertically center
	float yOffset = (windowSize.y - imageSize.y) * 0.5f;

	ImGui::Spacing();

	int actualColumnIndex = columnIndex;
	// actual usable index - because we skip out one column
	if (columnIndex > 2) {
		actualColumnIndex = columnIndex - 1;
	}

	// set the red cursor on top of character to indicate that this is the player
	if (actualColumnIndex == clientGame->client_id) {
		ImGui::SetCursorPos(ImVec2(columnIndex * columnSize + (columnSize - indicatorSize.x) / 2, yOffset - indicatorSize.y));
		ImGui::Image((void*)(intptr_t)redDownTriTextureID, indicatorSize);
	}
	// the character display
	ImGui::SetCursorPos(ImVec2(columnIndex * columnSize, yOffset));




	// the player
	if (actualColumnIndex == clientGame->client_id) {
		ImGui::Image((void*)(intptr_t)textures[selectedIndex], imageSize);
	}
	// their teammate
	else if (actualColumnIndex == clientGame->teams[clientGame->client_id]) {
		ImGui::Image((void*)(intptr_t)getBrowsingCharacter(clientGame->teams[clientGame->client_id]).textureID, imageSize);
	}
	// the other team
	else {
		ImGui::Image((void*)(intptr_t)secretCharacterTextureID, imageSize);
	}


	ImGui::Spacing();



	// display the character ID
	std::string name = "Player " + std::to_string(displayedPlayerID+1); // 1,2,3,4
	const char* playerName = (name).c_str();

	ImGui::PushFont(lobbyFont);
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
	// set the scale
	ImGui::SetWindowFontScale(4.0f);

	// get the text size
	ImVec2 textSize = ImGui::CalcTextSize(playerName);

	// set the text position
	ImGui::SetCursorPos(ImVec2(columnIndex * columnSize + (columnSize - textSize.x) / 2, yOffset + imageSize.y));

	ImGui::Text(playerName);

	// reset all
	ImGui::SetWindowFontScale(1.0f);
	ImGui::PopStyleColor();
	ImGui::PopFont();


	// if this displayedPlayer already made their selection, display the green mark underneath them
	int distanceToTag = 30; // change this distance to the tagname
	ImGui::SetCursorPos(ImVec2(columnIndex * columnSize + (columnSize - indicatorSize.x) / 2, yOffset + imageSize.y + textSize.y + distanceToTag));
	if (clientGame->characterUID[displayedPlayerID] != NO_CHARACTER) {
		ImGui::Image((void*)(intptr_t)greenMarkTextureID, indicatorSize);
	}
	ImGui::Spacing();


}

void ui::UIManager::displayLobbyTitle() {
	std::string title = "Choose your character";
	const char* displayTitle = title.c_str();


	ImGui::PushFont(lobbyFont);
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
	// set the scale
	ImGui::SetWindowFontScale(7.0f);

	// get the text size
	ImVec2 textSize = ImGui::CalcTextSize(displayTitle);

	// set the text position
	ImGui::SetCursorPos(ImVec2((windowSize.x - textSize.x) / 2, 30));

	ImGui::Text(displayTitle);

	// reset all
	ImGui::SetWindowFontScale(1.0f);
	ImGui::PopStyleColor();
	ImGui::PopFont();

}



ui::Character ui::UIManager::getBrowsingCharacter(int playerId) {
	ui::Character result;
	for (ui::Character character : ui::uiManager->characters) {
		if (character.characterUID == clientGame->browsingCharacterUID[playerId]) {
			result = character;
		}
	}
	return result;
}

bool ui::UIManager::canSelectCharacter() {
	int teammate = clientGame->teams[clientGame->client_id];
	// get teammate selected character
	int teammateSelectedCharacter = clientGame->characterUID[teammate];

	return browsingCharacterUID != teammateSelectedCharacter;
}

bool ui::UIManager::areAllPlayersReady() {
	bool areReady = true;
	for (int character : clientGame->characterUID) {
		if (character == NO_CHARACTER) {
			areReady = false;
		}
	}

	return areReady;
}


void ui::UIManager::displayStartScreen() {
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(windowSize);
	ImGui::Begin("Start Game", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);


	// draw the background image
	ImGui::GetWindowDrawList()->AddImage((void*)(intptr_t)startBackgroundImageTextureID,
		ImGui::GetWindowPos(), ImVec2(ImGui::GetWindowPos().x + windowSize.x, ImGui::GetWindowPos().y + windowSize.y));


	// put the title down
	ImVec2 titleSize = ImVec2(windowSize.x, windowSize.y);

	ImGui::SetCursorPos(ImVec2(0, 0));
	ImGui::Image((void*)(intptr_t)startTitleTextureID, titleSize);


	// put the image for button down
	ImVec2 buttonImageSize = ImVec2(windowSize.x, windowSize.y);
	ImGui::SetCursorPos(ImVec2(0, 100));
	ImGui::Image((void*)(intptr_t)startButtonImageTextureID, titleSize);

	// an invisible button that can capture click
	ImVec2 invisibleButtonSize = ImVec2(windowSize.x / 3 * 2, windowSize.y / 3 * 2);
	ImGui::SetCursorPos(ImVec2(windowSize.x / 3, windowSize.y / 6));
	if (ImGui::InvisibleButton("my_invisible_button", invisibleButtonSize)) {
		// Code to execute when the button is clicked
		isInStartScreen = false;
		isTransitionToLobby = true;
	}


}



// the content inside the screen loop
void ui::UIManager::lobby() {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();


	windowSize = ImVec2(1920, 1080);




	if (isInStartScreen) {
		displayStartScreen();
	}
	else {
		isTransitionToLobby = false;
		// Lobby screen
		imageSize = ImVec2(ImGui::GetColumnWidth(-1), ImGui::GetColumnWidth(-1) * 4 / 3);
		buttonSize = ImVec2(ImGui::GetColumnWidth(-1), 30);
		indicatorSize = ImVec2(ImGui::GetColumnWidth(-1) / 2, 80);
		columnSize = ImGui::GetColumnWidth(-1);



		int columnIndex = 0;



		// setting the window to take up entire screen - right now use fix size
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(windowSize);



		ImGui::Begin("Character Selection", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);

		// draw the background image
		ImGui::GetWindowDrawList()->AddImage((void*)(intptr_t)lobbyBackgroundImageTextureID,
			ImGui::GetWindowPos(), ImVec2(ImGui::GetWindowPos().x + windowSize.x, ImGui::GetWindowPos().y + windowSize.y));


		// update prev selectedIndex - used for playing music background matching character
		if (prevSelectedIndex != selectedIndex) {
			prevSelectedIndex = selectedIndex;
		}


		// lobby title
		displayLobbyTitle();


		// Create 5 columns
		// column 0,1,3,4 display players
		// column 2 in the middle show the egg
		ImGui::Columns(5, NULL, false);



		//--------------------------------------------------------------------------------------------------------------------------------
		// offset - this is needed to make the image vertically center
		columnIndex = ImGui::GetColumnIndex();
		characterDisplay(columnIndex, 0);

		ImGui::NextColumn();


		//--------------------------------------------------------------------------------------------------------------------------------


		// offset - this is needed to make the image vertically center
		columnIndex = ImGui::GetColumnIndex();
		characterDisplay(columnIndex, 1);

		ImGui::NextColumn();


		//--------------------------------------------------------------------------------------------------------------------------------

		// empty column
		ImGui::NextColumn();



		//--------------------------------------------------------------------------------------------------------------------------------
		columnIndex = ImGui::GetColumnIndex();
		characterDisplay(columnIndex, 2);

		ImGui::NextColumn();



		//--------------------------------------------------------------------------------------------------------------------------------
		columnIndex = ImGui::GetColumnIndex();
		characterDisplay(columnIndex, 3);













		// handle keyboard selection and disable selection

		//--------------------------------------------------------------------------------------------------------------------------------

		ImGuiIO& io = ImGui::GetIO();


		if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && isDebounced() && !isLobbySelectionSent) {
			// arrow up key is hit
			selectedIndex = (selectedIndex + textures.size() - 1) % textures.size();
		}
		if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && isDebounced() && !isLobbySelectionSent) {
			// arrow down key is hit
			selectedIndex = (selectedIndex + 1) % textures.size();
		}
		if ((ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) && isDebounced()) {
			// Enter key is hit
			if (canSelectCharacter() && !isLobbySelectionSent) {
				selectedCharacterUID = characters[selectedIndex].characterUID;
			}
			else {
				std::cout << "your teammate already chosen this character" << std::endl;
			}
		}
		browsingCharacterUID = characters[selectedIndex].characterUID;

		if (ImGui::IsKeyPressed(ImGuiKey_Space) && isDebounced()) {
			// Space key is hit
			// TODO: remove manually enter game - here for debugging purpose only
			isInLobby = false;
			isTransitioningToGame = true;
			//enableInput = true;
		}

		if (areAllPlayersReady()) {
			isInLobby = false;
			isTransitioningToGame = true;
			//enableInput = true;
		}
	}





	ImGui::End();


	// Rendering
	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(sge::window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(sge::window);
}

/**
 * Precondition: call after lobby() is called
*/
bool ui::UIManager::charJustChanged() {
	return selectedIndex != prevSelectedIndex;
}

int ui::UIManager::getCurrentCharSelection() {
	return selectedIndex;
}

int ui::UIManager::getPrevCharSelection() {
	return prevSelectedIndex;
}