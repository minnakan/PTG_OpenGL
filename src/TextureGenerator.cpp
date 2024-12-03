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

unsigned int TextureGenerator::GenerateTexture(const unsigned char* heightData, int size, int targetSize) {
    if (m_textureTiles.empty() || !heightData) {
        std::cerr << "Invalid input parameters for texture generation" << std::endl;
        return 0;
    }

    // Allocate memory for final texture
    std::vector<unsigned char> finalTexture(targetSize * targetSize * 4, 0);

    // For each pixel in the final texture
    for (int y = 0; y < targetSize; y++) {
        for (int x = 0; x < targetSize; x++) {
            // Map coordinates to heightmap resolution
            int heightMapX = x * size / targetSize;
            int heightMapY = y * size / targetSize;

            int heightValue = heightData[heightMapY * size + heightMapX];
            unsigned char* finalPixel = &finalTexture[(y * targetSize + x) * 4];

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

            // Blend all texture tiles based on height with normalization
            if (totalBlend > 0.0f) {
                for (size_t i = 0; i < m_textureTiles.size(); i++) {
                    const auto& tile = m_textureTiles[i];
                    float normalizedBlend = blendFactors[i] / totalBlend;

                    if (normalizedBlend > 0.0f) {
                        // Calculate texture coordinates with upscaling
                        float texX = static_cast<float>(x) * tile.width / targetSize;
                        float texY = static_cast<float>(y) * tile.height / targetSize;

                        // Perform bilinear interpolation
                        int x0 = static_cast<int>(texX);
                        int y0 = static_cast<int>(texY);
                        int x1 = std::min(x0 + 1, tile.width - 1);
                        int y1 = std::min(y0 + 1, tile.height - 1);

                        float dx = texX - x0;
                        float dy = texY - y0;

                        const unsigned char* p00 = &tile.data[(y0 * tile.width + x0) * 4];
                        const unsigned char* p10 = &tile.data[(y0 * tile.width + x1) * 4];
                        const unsigned char* p01 = &tile.data[(y1 * tile.width + x0) * 4];
                        const unsigned char* p11 = &tile.data[(y1 * tile.width + x1) * 4];

                        unsigned char interpolatedPixel[4];
                        for (int c = 0; c < 4; c++) {
                            float v0 = p00[c] * (1 - dx) + p10[c] * dx;
                            float v1 = p01[c] * (1 - dx) + p11[c] * dx;
                            interpolatedPixel[c] = static_cast<unsigned char>(v0 * (1 - dy) + v1 * dy);
                        }

                        // Blend interpolated color into final pixel
                        for (int c = 0; c < 4; c++) {
                            finalPixel[c] += static_cast<unsigned char>(interpolatedPixel[c] * normalizedBlend);
                        }
                    }
                }
            }
            else {
                // Default color if no blend factors are valid
                const auto& defaultTile = m_textureTiles[0];
                int texX = (x * defaultTile.width) / targetSize;
                int texY = (y * defaultTile.height) / targetSize;
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

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, targetSize, targetSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, finalTexture.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    return textureID;
}