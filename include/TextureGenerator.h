#pragma once
#include <vector>
#include <string>
#include <glad/glad.h>


struct TextureRegion {
    int lowHeight;     
    int optimalHeight; 
    int highHeight; 
};

class TextureGenerator {
public:
    TextureGenerator();
    ~TextureGenerator();
    
    bool AddTextureTile(const char* filename, int lowHeight, int optimalHeight);

    unsigned int GenerateTexture(const unsigned char* heightData, int size);

private:
    struct TextureTile {
        unsigned int textureID;
        TextureRegion region;
        int width, height;
        unsigned char* data;
    };

    float CalculateBlendFactor(int height, const TextureRegion& region);

    void BlendPixel(unsigned char* finalColor, const unsigned char* tileColor, float blendFactor);

    std::vector<TextureTile> m_textureTiles;
    const int MAX_TILES = 4;
};