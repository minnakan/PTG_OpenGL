// TextureGenerator.cpp
#include "TextureGenerator.h"
#include "stb_image/stb_image.h"
#include <iostream>

TextureGenerator::TextureGenerator() {}

TextureGenerator::~TextureGenerator() {
    for (auto& tile : m_textureTiles) {
        if (tile.data) {
            stbi_image_free(tile.data);
        }
        if (tile.textureID) {
            glDeleteTextures(1, &tile.textureID);
        }
    }
}

bool TextureGenerator::AddTextureTile(const char* filename, int lowHeight, int optimalHeight) {
    if (m_textureTiles.size() >= MAX_TILES) {
        return false;
    }

    TextureTile tile;
    int channels;
    tile.data = stbi_load(filename, &tile.width, &tile.height, &channels, 4); // Force RGBA

    if (!tile.data) {
        return false;
    }

    // Calculate region boundaries
    tile.region.lowHeight = lowHeight;
    tile.region.optimalHeight = optimalHeight;
    tile.region.highHeight = optimalHeight + (optimalHeight - lowHeight); // Symmetric falloff

    m_textureTiles.push_back(tile);
    return true;
}

float TextureGenerator::CalculateBlendFactor(int height, const TextureRegion& region) {
    if (height < region.lowHeight || height > region.highHeight) {
        return 0.0f;
    }

    if (height == region.optimalHeight) {
        return 1.0f;
    }

    if (height < region.optimalHeight) {
        float temp1 = static_cast<float>(height - region.lowHeight);
        float temp2 = static_cast<float>(region.optimalHeight - region.lowHeight);
        return temp1 / temp2;
    }
    else {
        float temp1 = static_cast<float>(region.highHeight - region.optimalHeight);
        return (temp1 - (height - region.optimalHeight)) / temp1;
    }
}

void TextureGenerator::BlendPixel(unsigned char* finalColor, const unsigned char* tileColor, float blendFactor) {
    for (int i = 0; i < 4; i++) {
        finalColor[i] += static_cast<unsigned char>(tileColor[i] * blendFactor);
    }
}

unsigned int TextureGenerator::GenerateTexture(const unsigned char* heightData, int size) {

    if (m_textureTiles.empty() || !heightData) {
        std::cerr << "Invalid input parameters for texture generation" << std::endl;
        return 0;
    }

    // Allocate memory for final texture
    std::vector<unsigned char> finalTexture(size * size * 4, 0);

    // For each pixel in the final texture
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int heightValue = heightData[y * size + x];
            unsigned char* finalPixel = &finalTexture[(y * size + x) * 4];

            // First calculate total blend factor to normalize later
            float totalBlend = 0.0f;
            std::vector<float> blendFactors;

            // Get all blend factors first
            for (const auto& tile : m_textureTiles) {
                float blend = CalculateBlendFactor(heightValue, tile.region);
                totalBlend += blend;
                blendFactors.push_back(blend);
            }

            // Clear the pixel
            std::fill(finalPixel, finalPixel + 4, 0);

            // If we have any blending to do
            if (totalBlend > 0.0f) {
                // Blend all texture tiles based on height with normalization
                for (size_t i = 0; i < m_textureTiles.size(); i++) {
                    float normalizedBlend = blendFactors[i] / totalBlend;
                    if (normalizedBlend > 0.0f) {
                        // Calculate texture coordinates
                        int texX = (x * m_textureTiles[i].width) / size;
                        int texY = (y * m_textureTiles[i].height) / size;
                        const unsigned char* tilePixel = &m_textureTiles[i].data[(texY * m_textureTiles[i].width + texX) * 4];

                        // Blend each color component
                        for (int c = 0; c < 4; c++) {
                            finalPixel[c] += static_cast<unsigned char>(tilePixel[c] * normalizedBlend);
                        }
                    }
                }
            }
            else {
                // If no blend factors, use a default color or the lowest terrain texture
                const auto& defaultTile = m_textureTiles[0];
                int texX = (x * defaultTile.width) / size;
                int texY = (y * defaultTile.height) / size;
                const unsigned char* defaultPixel = &defaultTile.data[(texY * defaultTile.width + texX) * 4];
                std::copy(defaultPixel, defaultPixel + 4, finalPixel);
            }
        }
    }

    // Create OpenGL texture
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, finalTexture.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    return textureID;
}