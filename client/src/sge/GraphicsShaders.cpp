//
// Created by benjx on 4/10/2024.
//
#include "sge/GraphicsShaders.h"

sge::ScreenShader sge::screenProgram;

sge::ToonShader sge::defaultProgram;
sge::Postprocesser sge::postprocessor;

sge::ParticleShader sge::particleProgram;

sge::EntityShader sge::shadowProgram;
sge::ShadowMap sge::shadowprocessor;

sge::LineShaderProgram sge::lineShaderProgram;
sge::CrosshairShaderProgram sge::crosshairShaderProgram;
sge::UIShaderProgram sge::uiShaderProgram;
sge::TextShaderProgram sge::textShaderProgram;
sge::BillboardProgram sge::billboardProgram;

/**
 * Initialize GLSL shaders
 */
void sge::initShaders()
{
    defaultProgram.initShaderProgram(
		(std::string)(PROJECT_PATH)+SetupParser::getValue("default-vertex-shader"),
		(std::string)(PROJECT_PATH)+SetupParser::getValue("default-fragment-shader")
	);
    lineShaderProgram.initShaderProgram(    // it's a lightweight shader
        (std::string)(PROJECT_PATH)+SetupParser::getValue("bulletTrail-vertex-shader"),
        (std::string)(PROJECT_PATH)+SetupParser::getValue("bulletTrail-fragment-shader")
    );
	screenProgram.initShaderProgram(
		(std::string)(PROJECT_PATH)+SetupParser::getValue("screen-vertex-shader"),
		(std::string)(PROJECT_PATH)+SetupParser::getValue("screen-fragment-shader")
	);

    particleProgram.initShaderProgram((std::string)(PROJECT_PATH)+SetupParser::getValue("particles-vertex-shader"),
                                      (std::string)(PROJECT_PATH)+SetupParser::getValue("particles-fragment-shader"),
                                      (std::string)(PROJECT_PATH)+SetupParser::getValue("particles-geometry-shader"));

    crosshairShaderProgram.initShaderProgram(
        (std::string)(PROJECT_PATH)+SetupParser::getValue("crosshair-vertex-shader"),
        (std::string)(PROJECT_PATH)+SetupParser::getValue("crosshair-fragment-shader")
    );
    uiShaderProgram.initShaderProgram(
        (std::string)(PROJECT_PATH)+"/client/shaders/ui.vert.glsl",
        (std::string)(PROJECT_PATH)+"/client/shaders/ui.frag.glsl"
    );
    textShaderProgram.initShaderProgram(
        (std::string)(PROJECT_PATH)+"/client/shaders/text.vert.glsl",
        (std::string)(PROJECT_PATH)+"/client/shaders/text.frag.glsl"
    );
    billboardProgram.initShaderProgram(
        (std::string)(PROJECT_PATH) + "/client/shaders/billboard.vert.glsl",
        (std::string)(PROJECT_PATH) + "/client/shaders/billboard.frag.glsl"
    );

    postprocessor.initPostprocessor();

    shadowProgram.initShaderProgram((std::string)(PROJECT_PATH) + SetupParser::getValue("shadowmap-vertex-shader"),
                                    (std::string)(PROJECT_PATH) + SetupParser::getValue("nop-fragment-shader"));

    shadowprocessor.initShadowmap();
}

/**
 * Destructor for shader program, deletes program from OpenGL context because it's unneeded anymore
 */
sge::ShaderProgram::~ShaderProgram() {
    glDeleteProgram(program);
}

/**
 * Create a new shader program
 * @param vertexShaderPath Path to vertex shader GLSL file
 * @param fragmentShaderPath Path to fragment shader GLSL file
 */
void sge::ShaderProgram::initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) {
    vertexShader = initShader(vertexShaderPath, GL_VERTEX_SHADER);
    fragmentShader = initShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

    program = glCreateProgram();
    GLint linked;
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linked);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (!linked) {
        std::cout << "Failed to link shaders\n";
        exit(EXIT_FAILURE);
    }
}

/**
 * Returns a shader file's source code as a string
 * @param filename Path to shader glsl source file
 * @return Shader source code as a string
 */
std::string sge::ShaderProgram::readShaderSource(const std::string &filename) {
    std::ifstream in;
    in.open(filename);
    if (!in.is_open()) {
        std::cout << "Failed to read shader file: " << filename << "\n";
        exit(EXIT_FAILURE);
    }
    std::string currLine;
    std::string out;
    while (in) {
        std::getline(in, currLine);
        out += currLine + "\n";
    }
    return out;
}

/**
 * Tells OpenGL context to use this shader program for renderng stuff
 */
void sge::ShaderProgram::useShader() const {
    glUseProgram(program);
}

/**
 * Print GLSL compiler error for given shader
 * @param shaderID Shader Identifier
 */
void sge::ShaderProgram::printCompileError(GLint shaderID) {
    // Print error message when compiling vertex shader
    GLint maxLength = 0;
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);
    // The maxLength includes the NULL terminator
    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(shaderID, maxLength, &maxLength, &errorLog[0]);
    for (GLchar c : errorLog) {
        std::cout << c;
    }
}

/**
 * Load/initializer shader from file
 * @param shaderPath Shader file path
 * @param shaderType Shader type (e.g. GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, etc.)
 * @return Identifier for compiled shader
 */
GLint sge::ShaderProgram::initShader(const std::string &shaderPath, const GLint &shaderType) {
    // Load shader source files
    std::string vertexShaderSource = readShaderSource(shaderPath);
    GLint shader;
    shader = glCreateShader(shaderType);
    const char *shaderSourceC = vertexShaderSource.c_str();
    glShaderSource(shader, 1, &shaderSourceC, nullptr);
    glCompileShader(shader);
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        std::cout << "Failed to compile shader\n";
        printCompileError(shader);
        exit(EXIT_FAILURE);
    }
    return shader;
}

/**
 * PRECONDITION: OpenGL context already created/initialized
 * Create a new shader program and initialilze uniform variables
 * @param vertexShaderPath Path to vertex shader GLSL file
 * @param fragmentShaderPath Path to fragment shader GLSL file
 */
void sge::EntityShader::initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) {
    // Call derived function to compile/link shaders and stuff
    ShaderProgram::initShaderProgram(vertexShaderPath, fragmentShaderPath);
    useShader();
    // Initialize uniform variables
    perspectivePos = glGetUniformLocation(program, "perspective");
    viewPos = glGetUniformLocation(program, "view");
    modelPos = glGetUniformLocation(program, "model");

    isAnimated = glGetUniformLocation(program, "isAnimated");
    boneTransformPos = glGetUniformLocation(program, "boneTransform");
}

/**
 * Give shader updated viewing matrix (Convert from world coord to canonical coords)
 * @param mat
 */
void sge::EntityShader::updateViewMat(const glm::mat4 &mat) const {
    useShader();
    glUniformMatrix4fv(viewPos, 1, GL_FALSE, &mat[0][0]);
}

/**
 * Give shader updated model matrix (For transforming objects and stuff)
 * @param mat
 */
void sge::EntityShader::updateModelMat(const glm::mat4 &mat) const {
    useShader();
    glUniformMatrix4fv(modelPos, 1, GL_FALSE, &mat[0][0]);
}


/**
 * Give shader updated perspective projection matrix
 * @param mat
 */
void sge::EntityShader::updatePerspectiveMat(const glm::mat4 &mat) const {
    useShader();
    glUniformMatrix4fv(perspectivePos, 1, GL_FALSE, &mat[0][0]);
}

/**
 * Update bone transformation matrices for current model's pose
 * @param transforms
 */
void sge::EntityShader::updateBoneTransforms(std::vector<glm::mat4> &transforms) {
    assert(transforms.size() == MAX_BONES);
    useShader();
    glUniformMatrix4fv(boneTransformPos, MAX_BONES, GL_FALSE, &transforms[0][0][0]);
}

/**
 * Whether to use bones when rendering current model
 * @param animated
 */
void sge::EntityShader::setAnimated(bool animated) const {
    useShader();
    glUniform1i(isAnimated, animated);
}

/**
 * Give shader updated camera postition
 * @param pos
 */
void sge::ToonShader::updateCamPos(const glm::vec3 &pos) const {
    useShader();
    glUniform3fv(cameraPositionPos, 1, &pos[0]);
}

/**
 * Update light perspective matrix, this is the perspective matrix used when creating
 * the shadow map. Use orthographic projection for directional light sources and perspective
 * projection for point light sources
 * @param mat
 */
void sge::ToonShader::updateLightPerspectiveMat(const glm::mat4 &mat) const {
    useShader();
    glUniformMatrix4fv(lightPerspectivePos, 1, GL_FALSE, &mat[0][0]);
}

/**
 * Update light viewing matrix for shadow map. Should transform vertices from world space
 * to the current light source's space
 *
 * NOTE: If we want multiple shadow-casting light sources we'll need to scale this lighting perspective/viewing matrices
 * and the number of shadowmap objects up for the number of shadow-casting light sourcse
 * @param mat
 */
void sge::ToonShader::updateLightViewMat(const glm::mat4 &mat) const {
    useShader();
    glUniformMatrix4fv(lightViewPos, 1, GL_FALSE, &mat[0][0]);
}

/**
 * Update light direction/position
 * Set homogenous coordinate to 0 for directional light sources
 * @param dir
 */
void sge::ToonShader::updateLightDir(const glm::vec4 &dir) const {
    useShader();
    glUniform4fv(lightDirPos, 1, &dir[0]);
}

/**
 * Set whether to draw outline for current object
 * @param outline
 */
void sge::ToonShader::updateOutline(bool outline) const {
    useShader();
    glUniform1i(drawOutline, outline);
}

/**
 * Set the current season
 * @param season Current season (integer between 0 and 3 inclusive). 0 = spring, 1 = summer, 2 = fall, 3 = winter
 * @param blend Current blend factor with the next season. Should be a float between 0 and 1 inclusive
 */
void sge::ToonShader::updateSeason(Season _season, float blend) {
    useShader();
    glUniform1i(curSeason, _season);
    glUniform1f(seasonBlend, blend);
}

/**
 * Set material uniforms for easy dereferencing later on
 * (so we can refer to shaders as GL_TEXTURE0 + TEXTURE_TYPE in glActiveShader)
 */
void sge::ToonShader::setMaterialUniforms() {

    alternating = glGetUniformLocation(program, "multipleTextures");
    textureIdx = glGetUniformLocation(program, "textureIdx");
    seasons = glGetUniformLocation(program, "seasons");
    curSeason = glGetUniformLocation(program, "curSeason");
    seasonBlend = glGetUniformLocation(program, "seasonBlend");
    entityAlternateTextures = glGetUniformLocation(program, "entityAlternateTexture");
    entitySeasons = glGetUniformLocation(program, "entitySeasons");

    hasDiffuseMap = glGetUniformLocation(program, "hasDiffuseMap");
    diffuseColor = glGetUniformLocation(program, "diffuseColor"); // array of diffuse colors

    diffuseTexturePos[0] = glGetUniformLocation(program, "diffuseTexture0");
    glUniform1i(diffuseTexturePos[0], DIFFUSE_TEXTURE0);
    diffuseTexturePos[1] = glGetUniformLocation(program, "diffuseTexture1");
    glUniform1i(diffuseTexturePos[1], DIFFUSE_TEXTURE1);
    diffuseTexturePos[2] = glGetUniformLocation(program, "diffuseTexture2");
    glUniform1i(diffuseTexturePos[2], DIFFUSE_TEXTURE2);
    diffuseTexturePos[3] = glGetUniformLocation(program, "diffuseTexture3");
    glUniform1i(diffuseTexturePos[3], DIFFUSE_TEXTURE3);


    hasSpecularMap = glGetUniformLocation(program, "hasSpecularMap");
    specularTexturePos = glGetUniformLocation(program, "specularTexture");
    glUniform1i(specularTexturePos, SPECULAR_TEXTURE);
    specularColor = glGetUniformLocation(program, "specularColor");

    hasShinyMap = glGetUniformLocation(program, "hasShinyMap");
    shinyColor = glGetUniformLocation(program, "shinyColor");
    shinyTexturePos = glGetUniformLocation(program, "shinyTexture");
    glUniform1i(shinyTexturePos, SHININESS_TEXTURE);

    glActiveTexture(GL_TEXTURE0 + SHADOWMAP_TEXTURE);
    shadowMapTexturePos = glGetUniformLocation(program, "shadowMap");
    glUniform1i(shadowMapTexturePos, SHADOWMAP_TEXTURE);
}

/**
 * Initialize toon shader and uniforms
 * @param vertexShaderPath
 * @param fragmentShaderPath
 */
void sge::ToonShader::initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) {
    EntityShader::initShaderProgram(vertexShaderPath, fragmentShaderPath);
    cameraPositionPos = glGetUniformLocation(program, "cameraPosition");
    lightPerspectivePos = glGetUniformLocation(program, "lightPerspective");
    lightViewPos = glGetUniformLocation(program, "lightView");
    lightDirPos = glGetUniformLocation(program, "lightDir");
    drawOutline = glGetUniformLocation(program, "drawOutline");
    setMaterialUniforms();
}

/**
 * Initialize screen/postprocessing shader program
 * @param vertexShaderPath
 * @param fragmentShaderPath
 */
void sge::ScreenShader::initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) {
    ShaderProgram::initShaderProgram(vertexShaderPath, fragmentShaderPath);
    useShader();
    // Activate texture units and bind textures
    glActiveTexture(GL_TEXTURE0);
    GLint colorTexturePos = glGetUniformLocation(program, "colorTexture");
    glUniform1i(colorTexturePos, 0);

    glActiveTexture(GL_TEXTURE0 + 1);
    GLint normalTexturePos = glGetUniformLocation(program, "normalTexture");
    glUniform1i(normalTexturePos, 1);

    glActiveTexture(GL_TEXTURE0 + 2);
    GLint maskTexturePos = glGetUniformLocation(program, "maskTexture");
    glUniform1i(maskTexturePos, 2);

    glActiveTexture(GL_TEXTURE0 + 3);
    GLint depthTexturePos = glGetUniformLocation(program, "depthTexture");
    glUniform1i(depthTexturePos, 3);
}

/**
 * PRECONDITION: screen shader program already initialized
 * Initialize postprocessor
 * Postprocessor will do additional stuff on rendered stuff like drawing contours or bloom (bloom isn't planned but just to give you an idea on what it does)
 */
void sge::Postprocesser::initPostprocessor() {
    screenProgram.useShader();
    // Generate g-buffer/framebuffers for postprocessing (e.g. drawing cartoon outlines and bloom)
    glGenFramebuffers(1, &FBO.gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO.gBuffer);

    // Color/specular color buffer
    glGenTextures(1, &FBO.gColor);
    glBindTexture(GL_TEXTURE_2D, FBO.gColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sge::windowWidth, sge::windowHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBO.gColor, 0);

    // Normal color buffer
    glGenTextures(1, &FBO.gNormal);
    glBindTexture(GL_TEXTURE_2D, FBO.gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sge::windowWidth, sge::windowHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, FBO.gNormal, 0);

    // Mask to be used for various stuff
    glGenTextures(1, &FBO.gMask);
    glBindTexture(GL_TEXTURE_2D, FBO.gMask);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, sge::windowWidth, sge::windowHeight, 0, GL_RED_INTEGER, GL_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, FBO.gMask, 0);


    // Depth buffer
    glGenTextures(1, &FBO.gStencilDepth);
    glBindTexture(GL_TEXTURE_2D, FBO.gStencilDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, sge::windowWidth, sge::windowHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, FBO.gStencilDepth, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    // Tell OpenGL which color attachments we're using this framebuffer for rendering
    GLuint attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(sizeof(attachments) / sizeof(attachments[0]), attachments);

    // Postprocessor works by drawing to a framebuffer (storing results as a texture) then drawing
    // that texture to the screen using a quad (rectangle) in front of the camera.
    // If you think that's scuffed... blame OpenGL that's a pretty standard technique
    // Initialize the quad
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    GLfloat vertices[] = {
            1.0, -1.0, 1.0, 0.0,
            -1.0, -1.0, 0.0, 0.0,
            -1.0, 1.0, 0.0, 1.0,

            1.0, 1.0, 1.0, 1.0,
            1.0, -1.0, 1.0, 0.0,
            -1.0, 1.0, 0.0, 1.0
    };
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}

/**
 * Gracefully deallocate postprocessor's OpenGL stuff like
 * framebuffers
 */
void sge::Postprocesser::deletePostprocessor() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteTextures(1, &FBO.gStencilDepth);
    glDeleteTextures(1, &FBO.gNormal);
    glDeleteTextures(1, &FBO.gMask);
    glDeleteTextures(1, &FBO.gColor);
    glDeleteFramebuffers(1, &FBO.gBuffer);
}

/**
 * Make future draws draw to the postprocessor's framebuffer
 */
void sge::Postprocesser::drawToFramebuffer() const {
    glViewport(0, 0, sge::windowWidth, sge::windowHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO.gBuffer);
    glClearColor(0.678f, 0.847f, 0.902f, 1.0f);  // light blue good sky :)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

/**
 * Perform postprocessing on postprocessor's framebuffer and draw to screen
 */
void sge::Postprocesser::drawToScreen() const {
    // Unbind to switch to OpenGL's default framebuffer (the default framebuffer is what's actually rendered)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1, 0, 0, 1.0f);  // light blue good sky :)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    // Set color normal and depth textures for postprocessor
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, FBO.gColor);
//    glBindTexture(GL_TEXTURE_2D, shadowprocessor.FBO.gDepth);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, FBO.gNormal);

    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, FBO.gMask);

    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, FBO.gStencilDepth);
    // Draw quad to screen - shaders will perform postprocessing
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
}

/**
 * Resize the frame buffer object to fit the new screen size
 * WARNING: THIS WILL CHANGE THE ACTIVE SHADER PROGRAM
 */
void sge::Postprocesser::resizeFBO() const {
    screenProgram.useShader();
    glBindTexture(GL_TEXTURE_2D, FBO.gColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sge::windowWidth, sge::windowHeight, 0, GL_RGBA, GL_FLOAT, nullptr);

    glBindTexture(GL_TEXTURE_2D, FBO.gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sge::windowWidth, sge::windowHeight, 0, GL_RGBA, GL_FLOAT, nullptr);

    glBindTexture(GL_TEXTURE_2D, FBO.gMask);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, sge::windowWidth, sge::windowHeight, 0, GL_RED_INTEGER, GL_INT, nullptr);

    glBindTexture(GL_TEXTURE_2D, FBO.gStencilDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, sge::windowWidth, sge::windowHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
}

/**
 * PRECONDITION: shadow shader program has already been initialized
 * Initialize shadow map framebuffers
 */
void sge::ShadowMap::initShadowmap() {
    shadowProgram.useShader();
    glGenFramebuffers(1, &FBO.gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO.gBuffer);
    glGenTextures(1, &FBO.gStencilDepth);
    glBindTexture(GL_TEXTURE_2D, FBO.gStencilDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, FBO.gStencilDepth, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
}

/**
 * Future draws will now draw to the shadow map
 */
void sge::ShadowMap::drawToShadowmap() const {
    glViewport(0, 0, shadowMapWidth, shadowMapHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO.gBuffer);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

/**
 * Delete shadow map framebuffers and textures
 */
void sge::ShadowMap::deleteShadowmap() {
    glDeleteFramebuffers(1, &FBO.gBuffer);
    glDeleteTextures(1, &FBO.gStencilDepth);
}

/**
 * Give toon shader the updated shadow map
 */
void sge::ShadowMap::updateShadowmap() const {
    glActiveTexture(GL_TEXTURE0 + SHADOWMAP_TEXTURE);
    // We're using stencilDepth as the shadowmap's depth buffer
    glBindTexture(GL_TEXTURE_2D, FBO.gStencilDepth);
}

/**
 * Initialize particle shader program
 * @param vertexShaderPath Path to vertex shader glsl file
 * @param fragmentShaderPath Path to fragment shader glsl file
 * @param geometryShaderPath Path to geometry shader glsl file
 */
void sge::ParticleShader::initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath,
                                            const std::string &geometryShaderPath) {
    // Not using parent implementation due to inclusion of geometry shader
    vertexShader = initShader(vertexShaderPath, GL_VERTEX_SHADER);
    geometryShader = initShader(geometryShaderPath, GL_GEOMETRY_SHADER);
    fragmentShader = initShader(fragmentShaderPath, GL_FRAGMENT_SHADER);
    program = glCreateProgram();
    GLint linked;
    glAttachShader(program, vertexShader);
    glAttachShader(program, geometryShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &linked);

    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);

    if (!linked) {
        std::cout << "Failed to link shaders\n";
        exit(EXIT_FAILURE);
    }

    perspectivePos = glGetUniformLocation(program, "perspective");
    viewPos = glGetUniformLocation(program, "view");
    sizePos = glGetUniformLocation(program, "particleSize");
}


/**
 * Update perspective matrix for particle shader, needed to render particles at the correct position on the screen
 * @param mat
 */
void sge::ParticleShader::updatePerspectiveMat(const glm::mat4 &mat) const {
    useShader();
    glUniformMatrix4fv(perspectivePos, 1, GL_FALSE, &mat[0][0]);
}

/**
 * Update camera view matrix for particle shader, needed to render particles at the correct position on the screen
 * @param mat
 */
void sge::ParticleShader::updateViewMat(const glm::mat4 &mat) const {
    useShader();
    glUniformMatrix4fv(viewPos, 1, GL_FALSE, &mat[0][0]);
}

/**
 * Update base particle size before any form of transformation
 * @param size
 */
void sge::ParticleShader::updateParticleSize(const float size) const {
    useShader();
    glUniform1f(sizePos, size);
}

// Ben's stuff above, that's why it's all documented :)
// ---------------------------------------------------------------------------------------------------------------------------

/*
Init line shader program for drawing bullet trails
*/
void sge::LineShaderProgram::initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) {

    ShaderProgram::initShaderProgram(vertexShaderPath, fragmentShaderPath);
    useShader();

    // pointers to uniforms location
    viewPos = glGetUniformLocation(program, "view");
    perspectivePos = glGetUniformLocation(program, "perspective");
    red = glGetUniformLocation(program, "red");
    green = glGetUniformLocation(program, "green");
    blue = glGetUniformLocation(program, "blue");

    // init VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // init VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // init EBO (for indicing)
    glGenBuffers(1, &EBO);

    // 3 floats (x,y,z) to define a vertex (position)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);   // location 0

    // unbind for now (don't cause trouble for other shaders)
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void sge::LineShaderProgram::updateViewMat(const glm::mat4 &mat) {
    useShader();
    glUniformMatrix4fv(viewPos, 1, GL_FALSE, &mat[0][0]);
}

void sge::LineShaderProgram::updatePerspectiveMat(const glm::mat4 &mat) {
    useShader();
    glUniformMatrix4fv(perspectivePos, 1, GL_FALSE, &mat[0][0]);
}

void sge::LineShaderProgram::renderBulletTrail(const glm::vec3& start, const glm::vec3& end) {
    useShader();
    glUniform1f(red, 0.5 + 0.5 * std::sin(t));
    glUniform1f(green, 0.2 + 0.5 * std::sin(t+3.14f));
    glUniform1f(blue, 0.5 + 0.5 * std::cos(t+3.14f));
    t += 0.1f;

    // Calculate additional vertices for the prism
    glm::vec3 offset(0.08f, 0.2f, 0.08f);

    GLfloat vertices[] = {
        start.x + offset.x, start.y, start.z + offset.z,
        start.x - offset.x, start.y, start.z + offset.z,
        start.x - offset.x, start.y, start.z - offset.z,
        start.x + offset.x, start.y, start.z - offset.z,

        end.x + offset.x, end.y, end.z + offset.z,
        end.x - offset.x, end.y, end.z + offset.z,
        end.x - offset.x, end.y, end.z - offset.z,
        end.x + offset.x, end.y, end.z - offset.z,
};

    GLuint indices[] = {
        // Bottom face
        0, 1, 2,
        2, 3, 0,

        // Top face
        4, 5, 6,
        6, 7, 4,

        // Side faces
        0, 1, 5,
        5, 4, 0,

        1, 2, 6,
        6, 5, 1,

        2, 3, 7,
        7, 6, 2,

        3, 0, 4,
        4, 7, 3,
    };

    // Bind VAO, VBO, and EBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // Buffer vertices and indices
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    // Draw the triangular cone
    glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(GLint), GL_UNSIGNED_INT, 0);

    // Unbind VAO and VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    // // do clean up somewhere after?
    // glDeleteBuffers(1, &VBO);
    // glDeleteVertexArrays(1, &VAO);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void sge::LineShaderProgram::deleteLineShader() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

// ---------------------------------------------------------------------------------------------------------------------------
/*
Init shader program to draw lines onto screen (without texture)
*/
void sge::CrosshairShaderProgram::initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) {
    
    ShaderProgram::initShaderProgram(vertexShaderPath, fragmentShaderPath);
    useShader();

    // store uniform location
    float aspectRatio = (float)sge::windowHeight/(float)sge::windowWidth;
    aspectRatioPos = glGetUniformLocation(program, "aspectRatio");
    glUniform1f(aspectRatioPos, aspectRatio);

    scalePos = glGetUniformLocation(program, "scale");

    // init VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // init VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // init EBO (for indicing)
    glGenBuffers(1, &EBO);

    // 2 floats (x,y) to define a screen position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);   // location 0

    // unbind for now (don't cause trouble for other shaders)
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

// void sge::CrosshairShaderProgram::drawCrossHair() {
//     useShader();
//     glUniform1f(scalePos, 1.0f);


//     glBindVertexArray(VAO);
//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(crossHairVertices), crossHairVertices, GL_STATIC_DRAW);

//     glDrawArrays(GL_LINES, 0, 4);

//     glBindVertexArray(0);
// }


void sge::CrosshairShaderProgram::deleteLineUI() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void sge::CrosshairShaderProgram::drawCrossHair(float emo) {

    useShader();

    glUniform1f(scalePos, emo/2.0+0.5);

    // Bind VAO, VBO, and EBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // Buffer vertices and indices
    glBufferData(GL_ARRAY_BUFFER, sizeof(emotiveVertices), emotiveVertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    glDrawElements(GL_LINES, sizeof(indices)/sizeof(GLint), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}


void sge::UIShaderProgram::initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) {
    
    ShaderProgram::initShaderProgram(vertexShaderPath, fragmentShaderPath);
    useShader();

    // store uniform location
    float aspectRatio = (float)sge::windowHeight/(float)sge::windowWidth;
    aspectRatioPos = glGetUniformLocation(program, "aspectRatio");
    glUniform1f(aspectRatioPos, aspectRatio);

    transPos = glGetUniformLocation(program, "trans");

    // init VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // init VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // init EBO (for indicing)
    glGenBuffers(1, &EBO);

    // 2 floats (x,y) to define a screen position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);   // location 0
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);


    // unbind for now (don't cause trouble for other shaders)
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void sge::UIShaderProgram::drawUI(float width, float height, float xOffset, float yOffset, float scale, GLuint texture) {
    useShader();
    
    glm::mat3 trans (
        scale,  0,  xOffset, 
        0,  scale,  yOffset,
        0,  0,      1
    );
    glUniformMatrix3fv(transPos, 1, GL_TRUE, &trans[0][0]);

    // Bind VAO, VBO, and EBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // this width and height should be from the loaded image (texture)
    GLfloat boxVertices[] = {
        // position         // texture coords
        0.0,    0.0,        0.0, 0.0,
        width,  0.0,        1.0, 0.0,
        0.0,    height,     0.0, 1.0,
        width,  height,     1.0, 1.0
    };

    GLuint indices[] = {
        0,1,2,
        1,2,3
    };

    // Buffer vertices and indices
    glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(GLuint), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    
}


void sge::TextShaderProgram::initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) {
    
    ShaderProgram::initShaderProgram(vertexShaderPath, fragmentShaderPath);
    loadFont();
    useShader();

    // store uniform location
    projectionPos = glGetUniformLocation(program, "projection");
    // DO NOT CHANGE NUMBERS IN THIS ORTHOGRAPHICS PROJ MATRIX (EVEN IF WE USE A DIFFERENT SCREEN SIZE)
    glm::mat4 projection = glm::ortho(0.0f, 1400.0f, 0.0f, 800.0f, 0.0f, 100.0f);
    glUniformMatrix4fv(projectionPos, 1, GL_FALSE, &projection[0][0]);

    // init VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // init VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);


    // unbind for now (don't cause trouble for other shaders)
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void sge::TextShaderProgram::renderText(std::string text, float x, float y, float scale, glm::vec3 color) {
    // do not draw outside the screen as defined the ortho projection
    if (x<0 || x >= 1400.0 || y<0 || y >= 800.0) {
        std::printf("WARNING: you're rendering text outside of the ortho projection\n");
    }
    
    useShader();

    glUniform3f(glGetUniformLocation(program, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}



void sge::BillboardProgram::initShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) {

    ShaderProgram::initShaderProgram(vertexShaderPath, fragmentShaderPath);
    useShader();

    // pointers to uniforms location
    billboardCenterPos = glGetUniformLocation(program, "billboardCenter");
    billboardDimensionPos = glGetUniformLocation(program, "billboardDimension");
    CameraRightPos = glGetUniformLocation(program, "CameraRight");
    CameraUpPos = glGetUniformLocation(program, "CameraUp");
    viewPos = glGetUniformLocation(program, "view");
    projectionPos = glGetUniformLocation(program, "projection");

    glm::vec2 billboardSize = glm::vec2(0.5f, 0.5f);
    glUniform2fv(billboardDimensionPos, 1, &billboardSize[0]);

    // init VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // init VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // 3 floats (x,y,z) to define a vertex (position)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);   // location 0
    // and then 2 floats for texture coord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);  // location 1

    // unbind for now (don't cause trouble for other shaders)
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void sge::BillboardProgram::updateViewMat(const glm::mat4 &mat) {
    useShader();
    glUniformMatrix4fv(viewPos, 1, GL_FALSE, &mat[0][0]);
}

void sge::BillboardProgram::updatePerspectiveMat(const glm::mat4 &mat) {
    useShader();
    glUniformMatrix4fv(projectionPos, 1, GL_FALSE, &mat[0][0]);
}

void sge::BillboardProgram::updateCameraOrientation(const glm::vec3 &cameraRight, const glm::vec3 &cameraUp) {
    useShader();
    glUniform3fv(CameraRightPos, 1, &cameraRight[0]);
    glUniform3fv(CameraUpPos, 1, &cameraUp[0]);
}

void sge::BillboardProgram::renderPlayerTag(const glm::vec3 &playerPos, GLuint textureID) {
    renderPlayerTag(playerPos, textureID, 1.0f);
}

void sge::BillboardProgram::renderPlayerTag(const glm::vec3 &playerPos, GLuint textureID, float scale) {
    useShader();

    // Vertex data for the billboard
    const GLfloat vertices[] = {
        -scale, -scale, 0.0f,     0.0f, 0.0f,
        scale, -scale, 0.0f,      1.0f, 0.0f,
        -scale,  scale, 0.0f,     0.0f, 1.0f,
        scale,  scale, 0.0f,      1.0f, 1.0f
    };

    // pass uniforms to shader
    glm::vec3 billboardPosition = playerPos + glm::vec3(0,1.3,0); // tag distance above player
    glUniform3fv(billboardCenterPos, 1, &billboardPosition[0]);

    // Bind VAO, VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Draw the quad
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}