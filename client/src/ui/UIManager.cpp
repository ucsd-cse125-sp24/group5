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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls     // Enable Gamepad Controls

 
    

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
	
}

ui::UIManager::~UIManager() {
	std::cout << "UIManager is destroyed" << std::endl;
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
        characters[i].textureID = LoadTextureFromFile((std::string)(PROJECT_PATH) + characters[i].imagePath);
        textures.push_back(characters[i].textureID);
    }

    // load background image
    backgroundImageTextureID = LoadTextureFromFile((std::string)(PROJECT_PATH) + SetupParser::getValue("background-image"));
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

    
    // setting the window to take up entire screen - right now use fix size
    ImVec2 windowSize = ImVec2(1920, 1080);
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

    // Create three columns
    ImGui::Columns(3, NULL, false);

    // Size of the image
    ImVec2 imageSize(ImGui::GetColumnWidth(-1), ImGui::GetColumnWidth(-1));
    float yOffset = (windowSize.y - imageSize.y) * 0.5f;

    // First column: we display the image of your teammate
    ImGui::Spacing();

    ImGui::SetCursorPos(ImVec2(0, yOffset));
    ImGui::Image((void*)(intptr_t)getBrowsingCharacter(clientGame->teams[clientGame->client_id]).textureID, imageSize);
    ImGui::Spacing();
    

    // Move to the second column
    ImGui::NextColumn();


    // here we select the character that we want
    ImGui::Spacing();
    // janky hack here: we offset x by the width of previous image
    // and offset y by the calculated yOffset plus the size of the button height
    ImGui::SetCursorPos(ImVec2(ImGui::GetColumnWidth(-1), yOffset));

    // "lock" these move up and down button if player already make selection
    if (isLobbySelectionSent) {
        ImGui::BeginDisabled();
    }
    
    if (ImGui::Button("Previous Character")) {
        selectedIndex = (selectedIndex + textures.size() - 1) % textures.size();
    }

    ImGui::Image((void*)(intptr_t)textures[selectedIndex], imageSize);
    browsingCharacterUID = characters[selectedIndex].characterUID;
    if (ImGui::Button("Next Character")) {
        selectedIndex = (selectedIndex + 1) % textures.size();
    }

    // exit the disabling buttons
    if (isLobbySelectionSent) {
        ImGui::EndDisabled();
    }


    ImGui::Spacing();



    // Move to the third column
    ImGui::NextColumn();

    // Third column: Wall of text
    ImGui::Text("Wall of Text:");
    ImGui::SetCursorPos(ImVec2(imageSize.x*2, yOffset - 60));
    ImGui::Separator();
    ImGui::TextWrapped("This is a wall of text that can provide additional information, "
        "instructions, or lore about the characters or the game. "
        "You can put as much text here as you like. "
        "It can be multiline and will automatically wrap.");

    // TODO: remove the button - should enter game immediately when all players have selected their characters
    if (ImGui::Button("Enter Game")) {
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