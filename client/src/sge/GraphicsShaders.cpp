//
// Created by benjx on 4/10/2024.
//
#include "sge/GraphicsShaders.h"
#include "SetupParser.h"

sge::ScreenShader sge::screenProgram;

sge::ToonShader sge::defaultProgram;
sge::Postprocesser sge::postprocessor;

sge::EntityShader sge::shadowProgram;
sge::ShadowMap sge::shadowprocessor;

/**
 * Initialize GLSL shaders
 */
void sge::initShaders()
{
    defaultProgram.initShaderProgram(
		(std::string)(PROJECT_PATH)+SetupParser::getValue("default-vertex-shader"),
		(std::string)(PROJECT_PATH)+SetupParser::getValue("default-fragment-shader")
	);
	screenProgram.initShaderProgram(
		(std::string)(PROJECT_PATH)+SetupParser::getValue("screen-vertex-shader"),
		(std::string)(PROJECT_PATH)+SetupParser::getValue("screen-fragment-shader")
	);

    postprocessor.initPostprocessor();

    shadowProgram.initShaderProgram((std::string)(PROJECT_PATH) + SetupParser::getValue("shadowmap-vertex-shader"),
                                    (std::string)(PROJECT_PATH) + SetupParser::getValue("nop-fragment-shader"));

    shadowprocessor.initShadowmap();
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
 * Set material uniforms for easy dereferencing later on
 * (so we can refer to shaders as GL_TEXTURE0 + TEXTURE_TYPE in glActiveShader)
 */
void sge::ToonShader::setMaterialUniforms() {
    hasDiffuseMap = glGetUniformLocation(program, "hasDiffuseMap");
    diffuseTexturePos = glGetUniformLocation(program, "diffuseTexture");
    glUniform1i(diffuseTexturePos, DIFFUSE_TEXTURE);
    diffuseColor = glGetUniformLocation(program, "diffuseColor");

    hasSpecularMap = glGetUniformLocation(program, "hasSpecularMap");
    specularTexturePos = glGetUniformLocation(program, "specularTexture");
    glUniform1i(specularTexturePos, SPECULAR_TEXTURE);
    specularColor = glGetUniformLocation(program, "specularColor");

    emissiveColor = glGetUniformLocation(program, "emissiveColor");
    ambientColor = glGetUniformLocation(program, "ambientColor");

    hasBumpMap = glGetUniformLocation(program, "hasBumpMap");
    bumpTexturePos = glGetUniformLocation(program, "bumpTexture");
    glUniform1i(bumpTexturePos, BUMP_MAP);

    hasDisplacementMap = glGetUniformLocation(program, "hasDisplacementMap");
    displacementTexturePos = glGetUniformLocation(program, "displacementTexture");
    glUniform1i(displacementTexturePos, DISPLACEMENT_MAP);

    hasRoughMap = glGetUniformLocation(program, "hasRoughMap");
    roughTexturePos = glGetUniformLocation(program, "roughTexture");
    roughColor = glGetUniformLocation(program, "roughColor");
    glUniform1i(roughTexturePos, SHININESS_TEXTURE);

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
    GLint depthTexturePos = glGetUniformLocation(program, "depthTexture");
    glUniform1i(depthTexturePos, 2);
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
    GLuint attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

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
    glDeleteFramebuffers(1, &FBO.gBuffer);
    glDeleteTextures(1, &FBO.gStencilDepth);
    glDeleteTextures(1, &FBO.gNormal);
    glDeleteTextures(1, &FBO.gColor);
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

    glBindTexture(GL_TEXTURE_2D, FBO.gStencilDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, sge::windowWidth, sge::windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
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