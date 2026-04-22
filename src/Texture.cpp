#include "Texture.h"
#include "../stb_image.h"
#include <iostream>
#include <algorithm>

// Create a 1x1 fallback texture so nothing ever crashes
static unsigned int createFallbackTexture(unsigned char r, unsigned char g, unsigned char b)
{
    unsigned int id;
    glGenTextures(1, &id);
    unsigned char pixels[] = { r, g, b };
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return id;
}

// Simple box-filter downscale by 2x (halves both width and height)
static unsigned char* downscale2x(unsigned char* src, int w, int h, int channels)
{
    int nw = w / 2;
    int nh = h / 2;
    unsigned char* dst = (unsigned char*)malloc(nw * nh * channels);
    if (!dst) return nullptr;
    
    for (int y = 0; y < nh; y++) {
        for (int x = 0; x < nw; x++) {
            for (int c = 0; c < channels; c++) {
                int sum = 0;
                sum += src[((y*2  ) * w + (x*2  )) * channels + c];
                sum += src[((y*2  ) * w + (x*2+1)) * channels + c];
                sum += src[((y*2+1) * w + (x*2  )) * channels + c];
                sum += src[((y*2+1) * w + (x*2+1)) * channels + c];
                dst[(y * nw + x) * channels + c] = (unsigned char)(sum / 4);
            }
        }
    }
    return dst;
}

unsigned int loadTexture(const char* path, bool mipmap)
{
    // Safety: if GLAD hasn't loaded yet, glGenTextures will be null
    if (!glGenTextures) {
        std::cout << "ERROR: OpenGL not initialized yet, cannot load: " << path << std::endl;
        return 0;
    }

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    
    if (!data) {
        std::cout << "WARNING: Texture not found: " << path << std::endl;
        return createFallbackTexture(180, 170, 160);
    }

    std::cout << "Loaded: " << path << " (" << width << "x" << height << ", " << nrComponents << "ch)";

    // Downscale if too large for the GPU (Intel iGPUs choke on large textures)
    const int MAX_SIZE = 1024;
    unsigned char* current = data;
    bool ownsData = false;
    
    while (width > MAX_SIZE || height > MAX_SIZE) {
        if (width < 2 || height < 2) break;
        unsigned char* smaller = downscale2x(current, width, height, nrComponents);
        if (!smaller) break;
        if (ownsData) free(current);
        current = smaller;
        ownsData = true;
        width /= 2;
        height /= 2;
    }
    
    if (width != height || ownsData) {
        std::cout << " -> resized to " << width << "x" << height;
    }
    std::cout << std::endl;

    GLenum format = GL_RGB;
    if (nrComponents == 1) format = GL_RED;
    else if (nrComponents == 3) format = GL_RGB;
    else if (nrComponents == 4) format = GL_RGBA;

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // Upload to GPU
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, current);
    
    // Check for GL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cout << "ERROR: glTexImage2D failed for " << path << " (GL error " << err << ")" << std::endl;
        if (ownsData) free(current);
        stbi_image_free(data);
        glDeleteTextures(1, &textureID);
        return createFallbackTexture(180, 170, 160);
    }

    if (mipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);

    if (ownsData) free(current);
    stbi_image_free(data);

    return textureID;
}
