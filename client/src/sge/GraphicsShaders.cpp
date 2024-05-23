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

sge::LineShaderProgram sge::lineShaderProgram;
sge::CrosshairShaderProgram sge::crosshairShaderProgram;
sge::UIShaderProgram sge::uiShaderProgram;

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
    crosshairShaderProgram.initShaderProgram(
        (std::string)(PROJECT_PATH)+SetupParser::getValue("crosshair-vertex-shader"),
        (std::string)(PROJECT_PATH)+SetupParser::getValue("crosshair-fragment-shader")
    );
    uiShaderProgram.initShaderProgram(
        "./shaders/ui.vert.glsl",
        "./shaders/ui.frag.glsl"
    );
    // uiShaderProgram.loadImage("./assets/container.jpg");
    uiShaderProgram.loadImage("./assets/rickroll.jpg");

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
    setMaterialUniforms();
}


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
    glGenTextures(1, &FBO.gDepth);
    glBindTexture(GL_TEXTURE_2D, FBO.gDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, sge::windowWidth, sge::windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, FBO.gDepth, 0);

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
    glDeleteTextures(1, &FBO.gDepth);
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
    glBindTexture(GL_TEXTURE_2D, FBO.gDepth);
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

    glBindTexture(GL_TEXTURE_2D, FBO.gDepth);
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
    glGenTextures(1, &FBO.gDepth);
    glBindTexture(GL_TEXTURE_2D, FBO.gDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, FBO.gDepth, 0);
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
    glDeleteTextures(1, &FBO.gDepth);
}

/**
 * Give toon shader the updated shadow map
 */
void sge::ShadowMap::updateShadowmap() const {
    glActiveTexture(GL_TEXTURE0 + SHADOWMAP_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, FBO.gDepth);
}


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

void sge::CrosshairShaderProgram::drawCrossHair(float emo) {
    useShader();

    glUniform1f(scalePos, emo/2.0+0.5);

    // Bind VAO, VBO, and EBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // Buffer vertices and indices
    glBufferData(GL_ARRAY_BUFFER, sizeof(emotiveVertices), emotiveVertices, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);


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

void sge::UIShaderProgram::drawBox(float width, float height, float xOffset, float yOffset, float scale) {
    useShader();
    // scale and translate (todo: compute it once and store in class set)
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

    // todo: this width and height should be from the loaded image (texture)
    GLfloat boxVertices[] = {
        0.0, 0.0,       0.0, 0.0,
        width, 0.0,     1.0, 0.0,
        0.0, height,    0.0, 1.0,
        width, height,  1.0, 1.0
    };
    // maybe: draw a square first? 

    GLuint indices[] = {
        0,1,2,
        1,2,3
    };

    // Buffer vertices and indices
    glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(GLuint), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

}

void sge::UIShaderProgram::drawBox(float width, float height) {
    drawBox(width, height, 0, 0, 1);
}

void sge::UIShaderProgram::loadImage(const char* path) {
    useShader();
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // set the texture wrapping/filtering options 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load and generate the texture
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (!data) {
        std::printf("Failed to load UI texture from %s\n", path);
        stbi_image_free(data);
        return;
    }
    // Handle different number of channels in texture
    int format = GL_RGB;
    if (nrChannels == 1) {
        format = GL_RED;
    } else if (nrChannels == 4) {
        format = GL_RGBA;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);

}