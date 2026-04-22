#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <string>

unsigned int loadTexture(const char* path, bool mipmap = true);

#endif
