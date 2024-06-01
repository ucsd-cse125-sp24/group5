#include "ui/UIManager.h"


std::unique_ptr<ui::UIManager> ui::uiManager;
bool ui::isInLobby;
bool ui::isTransitioningToGame;

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

}

ui::UIManager::~UIManager() {
	std::cout << "UIManager is destroyed" << std::endl;
}

void ui::UIManager::lobbyKeyMapping(GLFWwindow* window, int key, int scancode, int action, int mods) {
	ImGuiIO& io = ImGui::GetIO();
	if (action == GLFW_PRESS)
		io.KeysDown[key] = true;
	if (action == GLFW_RELEASE)
		io.KeysDown[key] = false;

	// Update key modifiers
	io.KeyCtrl = (mods & GLFW_MOD_CONTROL) != 0;
	io.KeyShift = (mods & GLFW_MOD_SHIFT) != 0;
	io.KeyAlt = (mods & GLFW_MOD_ALT) != 0;
	io.KeySuper = (mods & GLFW_MOD_SUPER) != 0;
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

	// load background image
	backgroundImageTextureID = LoadTextureFromFile((std::string)(PROJECT_PATH)+SetupParser::getValue("background-image"));
	// load secret character
	secretCharacterTextureID = LoadTextureFromFile((std::string)(PROJECT_PATH)+SetupParser::getValue("secret-character"));

	// load indicators
	redDownTriTextureID = LoadTextureFromFile((std::string)(PROJECT_PATH)+SetupParser::getValue("buttonup-image"));
	greenMarkTextureID = LoadTextureFromFile((std::string)(PROJECT_PATH)+SetupParser::getValue("buttonup-image"));

}

void ui::UIManager::characterDisplay(int columnIndex, int displayedPlayerID) {
	// offset - this is needed to make the image vertically center
	float yOffset = (windowSize.y - imageSize.y) * 0.5f;
	// set the red cursor on top of character to indicate that this is the player
	if (columnIndex == clientGame->client_id) {
		ImGui::SetCursorPos(ImVec2(columnIndex * columnSize, yOffset - indicatorSize.y));
		ImGui::Image((void*)(intptr_t)redDownTriTextureID, indicatorSize);
	}
	// the character display
	ImGui::SetCursorPos(ImVec2(columnIndex * columnSize, yOffset));
		

	// actual usable index - because we skip out one column
	if (columnIndex > 2) {
		columnIndex = columnIndex - 1;
	}

	// the player
	if (columnIndex == clientGame->client_id) {
		ImGui::Image((void*)(intptr_t)textures[selectedIndex], imageSize);
	}
	// their teammate
	else if (columnIndex == clientGame->teams[clientGame->client_id]) {
		ImGui::Image((void*)(intptr_t)getBrowsingCharacter(clientGame->teams[clientGame->client_id]).textureID, imageSize);
	}
	// the other team
	else {
		ImGui::Image((void*)(intptr_t)secretCharacterTextureID, imageSize);
	}
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

bool ui::UIManager::checkCanSelectCharacter() {
	int teammate = clientGame->teams[clientGame->client_id];
	// get teammate selected character
	int teammateSelectedCharacter = clientGame->characterUID[teammate];

	return browsingCharacterUID != teammateSelectedCharacter;
}


// the content inside the screen loop
void ui::UIManager::lobby() {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();


	windowSize = ImVec2(1920, 1080);
	imageSize = ImVec2(ImGui::GetColumnWidth(-1), ImGui::GetColumnWidth(-1) * 4 / 3);
	buttonSize = ImVec2(ImGui::GetColumnWidth(-1), 30);
	indicatorSize = ImVec2(ImGui::GetColumnWidth(-1) / 2, 50);
	columnSize = ImGui::GetColumnWidth(-1);



	int columnIndex = 0;



	// setting the window to take up entire screen - right now use fix size
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(windowSize);

	

	ImGui::Begin("Character Selection", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);

	// draw the background image
	ImGui::GetWindowDrawList()->AddImage((void*)(intptr_t)backgroundImageTextureID,
		ImGui::GetWindowPos(), ImVec2(ImGui::GetWindowPos().x + windowSize.x, ImGui::GetWindowPos().y + windowSize.y));


	// update prev selectedIndex
	if (prevSelectedIndex != selectedIndex) {
		prevSelectedIndex = selectedIndex;
	}

	float yOffset = (windowSize.y - imageSize.y) * 0.5f;


	// Create 5 columns
	// column 0,1,3,4 display players
	// column 2 in the middle show the egg
	ImGui::Columns(5, NULL, false);



	//--------------------------------------------------------------------------------------------------------------------------------
	ImGui::Spacing();
	// offset - this is needed to make the image vertically center
	columnIndex = ImGui::GetColumnIndex();
	// set the red cursor on top of character to indicate that this is the player
	if (columnIndex == clientGame->client_id) {
		ImGui::SetCursorPos(ImVec2(columnIndex * ImGui::GetColumnWidth(-1), yOffset - indicatorSize.y));
		ImGui::Image((void*)(intptr_t)redDownTriTextureID, indicatorSize);
	}
	// the character display
	ImGui::SetCursorPos(ImVec2(columnIndex * ImGui::GetColumnWidth(-1), yOffset));
	// the player
	if (columnIndex == clientGame->client_id) {
		ImGui::Image((void*)(intptr_t)textures[selectedIndex], imageSize);
	}
	// their teammate
	else if (columnIndex == clientGame->teams[clientGame->client_id]) {
		ImGui::Image((void*)(intptr_t)getBrowsingCharacter(clientGame->teams[clientGame->client_id]).textureID, imageSize);
	}
	// the other team
	else {
		ImGui::Image((void*)(intptr_t)secretCharacterTextureID, imageSize);
	}
	ImGui::Spacing();


	// Move to the second column
	ImGui::NextColumn();







	//--------------------------------------------------------------------------------------------------------------------------------
	// here we select the character that we want


	// janky hack here: we offset x by the width of previous image
	// and offset y by the calculated yOffset plus the size of the button height

	//ImGui::SetCursorPos(ImVec2(ImGui::GetColumnWidth(-1), yOffset));

	//// "lock" these move up and down button if player already make selection
	//if (isLobbySelectionSent) {
	//	ImGui::BeginDisabled();
	//}


	//if (ImGui::ImageButton((void*)(intptr_t)redDownTriTextureID, buttonSize)) {
	//	selectedIndex = (selectedIndex + textures.size() - 1) % textures.size();
	//}

	//ImGui::PopStyleVar(2);  // We pushed two style variables, so we pop them
	//ImGui::PopStyleColor(3);

	//ImGui::Image((void*)(intptr_t)textures[selectedIndex], imageSize);
	//browsingCharacterUID = characters[selectedIndex].characterUID;
	//if (ImGui::Button("Next Character")) {
	//	selectedIndex = (selectedIndex + 1) % textures.size();
	//}

	// exit the disabling buttons
	/*if (isLobbySelectionSent) {
		ImGui::EndDisabled();
	}*/


	ImGui::Spacing();
	// offset - this is needed to make the image vertically center
	columnIndex = ImGui::GetColumnIndex();
	// set the red cursor on top of character to indicate that this is the player
	if (columnIndex == clientGame->client_id) {
		ImGui::SetCursorPos(ImVec2(columnIndex * ImGui::GetColumnWidth(-1), yOffset - indicatorSize.y));
		ImGui::Image((void*)(intptr_t)redDownTriTextureID, indicatorSize);
	}
	// the character display
	ImGui::SetCursorPos(ImVec2(columnIndex * ImGui::GetColumnWidth(-1), yOffset));
	// the player
	if (columnIndex == clientGame->client_id) {
		ImGui::Image((void*)(intptr_t)textures[selectedIndex], imageSize);
	}
	// their teammate
	else if (columnIndex == clientGame->teams[clientGame->client_id]) {
		ImGui::Image((void*)(intptr_t)getBrowsingCharacter(clientGame->teams[clientGame->client_id]).textureID, imageSize);
	}
	// the other team
	else {
		ImGui::Image((void*)(intptr_t)secretCharacterTextureID, imageSize);
	}
	ImGui::Spacing();


	// Move to the second column
	ImGui::NextColumn();


	//--------------------------------------------------------------------------------------------------------------------------------

	// Move to the third column
	if (isLobbySelectionSent) {
		ImGui::BeginDisabled();
	}


	if (ImGui::ImageButton((void*)(intptr_t)redDownTriTextureID, buttonSize)) {
		selectedIndex = (selectedIndex + textures.size() - 1) % textures.size();
	}

	ImGui::PopStyleVar(2);  // We pushed two style variables, so we pop them
	ImGui::PopStyleColor(3);

	browsingCharacterUID = characters[selectedIndex].characterUID;
	if (ImGui::Button("Next Character")) {
		selectedIndex = (selectedIndex + 1) % textures.size();
	}

	if (isLobbySelectionSent) {
		ImGui::EndDisabled();
	}
	ImGui::NextColumn();

	

	//--------------------------------------------------------------------------------------------------------------------------------

	ImGui::Spacing();
	// offset - this is needed to make the image vertically center
	columnIndex = ImGui::GetColumnIndex();
	// set the red cursor on top of character to indicate that this is the player
	if (columnIndex == clientGame->client_id) {
		ImGui::SetCursorPos(ImVec2(columnIndex * ImGui::GetColumnWidth(-1), yOffset - indicatorSize.y));
		ImGui::Image((void*)(intptr_t)redDownTriTextureID, indicatorSize);
	}
	// the character display
	ImGui::SetCursorPos(ImVec2(columnIndex * ImGui::GetColumnWidth(-1), yOffset));
	// the player
	if (columnIndex == clientGame->client_id) {
		ImGui::Image((void*)(intptr_t)textures[selectedIndex], imageSize);
	}
	// their teammate
	else if (columnIndex == clientGame->teams[clientGame->client_id]) {
		ImGui::Image((void*)(intptr_t)getBrowsingCharacter(clientGame->teams[clientGame->client_id]).textureID, imageSize);
	}
	// the other team
	else {
		ImGui::Image((void*)(intptr_t)secretCharacterTextureID, imageSize);
	}
	ImGui::Spacing();


	// Move to the second column
	ImGui::NextColumn();



	//--------------------------------------------------------------------------------------------------------------------------------
	ImGui::Spacing();
	// offset - this is needed to make the image vertically center
	columnIndex = ImGui::GetColumnIndex();
	// set the red cursor on top of character to indicate that this is the player
	if (columnIndex == clientGame->client_id) {
		ImGui::SetCursorPos(ImVec2(columnIndex * ImGui::GetColumnWidth(-1), yOffset - indicatorSize.y));
		ImGui::Image((void*)(intptr_t)redDownTriTextureID, indicatorSize);
	}
	// the character display
	ImGui::SetCursorPos(ImVec2(columnIndex * ImGui::GetColumnWidth(-1), yOffset));
	// the player
	if (columnIndex == clientGame->client_id) {
		ImGui::Image((void*)(intptr_t)textures[selectedIndex], imageSize);
	}
	// their teammate
	else if (columnIndex == clientGame->teams[clientGame->client_id]) {
		ImGui::Image((void*)(intptr_t)getBrowsingCharacter(clientGame->teams[clientGame->client_id]).textureID, imageSize);
	}
	// the other team
	else {
		ImGui::Image((void*)(intptr_t)secretCharacterTextureID, imageSize);
	}
	ImGui::Spacing();














	// handle keyboard selection and disable selection

	//--------------------------------------------------------------------------------------------------------------------------------
	

	// TODO: remove the button - should enter game immediately when all players have selected their characters
	/*if (ImGui::Button("Enter Game")) {
		isInLobby = false;
		isTransitioningToGame = true;
	}
	if (isLobbySelectionSent || !checkCanSelectCharacter()) {
		ImGui::BeginDisabled();
	}
	if (ImGui::Button("Select this Character")) {
		selectedCharacterUID = characters[selectedIndex].characterUID;
	}
	if (isLobbySelectionSent || !checkCanSelectCharacter()) {
		ImGui::EndDisabled();
	}*/


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