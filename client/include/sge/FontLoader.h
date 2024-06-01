#pragma once

#ifndef FONTLOADER_H
#define FONTLOADER_H

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cmath>   // for sin() and other math functions
#include <ctime>   // for time()

// for freetype 
#include <ft2build.h>
#include FT_FREETYPE_H

#include "sge/GraphicsConstants.h"
#include "SetupParser.h"

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};

namespace sge {
    extern std::map<char, Character> Characters;  // Declare the variable
    void loadFont();
}

#endif // FONTLOADER_H
