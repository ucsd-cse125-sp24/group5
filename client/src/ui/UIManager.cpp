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

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";

#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);
	
}

ui::UIManager::~UIManager() {
	std::cout << "UIManager is destroyed" << std::endl;
}


GLuint ui::UIManager::LoadTextureFromFile(std::string filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    if (data == nullptr) {
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return textureID;
}

void ui::UIManager::LoadCharacterImages() {
    for (int i = 0; i < characters.size(); i++) {
        characters[i].textureID = LoadTextureFromFile((std::string)(PROJECT_PATH) + characters[i].imagePath);
    }
}

// the content inside the screen loop
void ui::UIManager::lobby() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // setting the window to take up entire screen - right now use fix size
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(1280, 720));

    ImGui::Begin("Character Selection", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
    ImGui::Text("This is some useful text.");

    // Create three columns
    ImGui::Columns(3, NULL, false);

    // First column: Character names
    for (int i = 0; i < characters.size(); i++) {
        ImGui::PushID(i);
        if (ImGui::Selectable(characters[i].name.c_str(), selectedCharacter == i)) {
            selectedCharacter = i;
        }
        ImGui::PopID();
    }

    // Move to the second column
    ImGui::NextColumn();

    // Second column: Character images
    for (int i = 0; i < characters.size(); i++) {
        if (characters[i].textureID) {
            ImGui::Image((void*)(intptr_t)characters[i].textureID, ImVec2(64, 64)); // Display the image (size 64x64)
        }
        ImGui::NextColumn(); // Move to the third column to continue the loop properly
        ImGui::NextColumn(); // Move back to the second column for the next image
    }

    // Move to the third column
    ImGui::NextColumn();

    // Third column: Wall of text
    ImGui::Text("Wall of Text:");
    ImGui::Separator();
    ImGui::TextWrapped("This is a wall of text that can provide additional information, "
        "instructions, or lore about the characters or the game. "
        "You can put as much text here as you like. "
        "It can be multiline and will automatically wrap.");

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