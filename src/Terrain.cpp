#include "Terrain.h"
#include <fstream>
#include <iostream>
#include "stb_image/stb_image.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"



bool Terrain::LoadHeightMap(char* szFilename, int iSize)
{
    m_iSize = iSize;
    m_heightData.m_iSize = iSize;

    int width, height, channels;
    unsigned char* data = stbi_load(szFilename, &width, &height, &channels, 1);
    if (!data) {
        std::cerr << "Failed to load heightmap: " << szFilename << std::endl;
        return false;
    }

    if (width != iSize || height != iSize) {
        std::cerr << "Heightmap size mismatch. Expected: " << iSize << "x" << iSize
            << ", Got: " << width << "x" << height << std::endl;
        stbi_image_free(data);
        return false;
    }

    // Allocate memory for the height data
    m_heightData.m_pucData = new unsigned char[m_iSize * m_iSize];
    if (!m_heightData.m_pucData) {
        std::cerr << "Failed to allocate memory for height data." << std::endl;
        stbi_image_free(data);
        return false;
    }

    // Copy the grayscale image data to the heightmap
    std::memcpy(m_heightData.m_pucData, data, m_iSize * m_iSize);

    // Free the image data loaded by stb_image
    stbi_image_free(data);

    return true;
}

bool Terrain::SaveHeightMap(char* szFilename)
{
    //Save Heightmap
    const char* filename = szFilename;
    if (stbi_write_png(filename, m_iSize, m_iSize, 1, m_heightData.m_pucData, m_iSize) != 0) {
        std::cout << "Heightmap written to " << filename << std::endl;
        return true;
    }
    else {
        std::cerr << "Failed to write heightmap to " << filename << std::endl;
    }
    return false;
}

bool Terrain::UnloadHeightMap(void)
{
    if (m_heightData.m_pucData)
    {
        delete[] m_heightData.m_pucData;
        m_heightData.m_pucData = nullptr;
        m_heightData.m_iSize = 0;
        m_iSize = 0;
    }
    return true;
}

//TODO: Move to parent at a later point
bool Terrain::LoadTexture(const char* filePath)
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cerr << "Failed to load texture: " << filePath << std::endl;
        return false;
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}


bool Terrain::LoadDetailMap(const char* filePath) {
    glGenTextures(1, &detailMapID);
    glBindTexture(GL_TEXTURE_2D, detailMapID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 1);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);
        return true;
    }
    std::cerr << "Failed to load detail map: " << filePath << std::endl;
    return false;
}

void Terrain::UnloadDetailMap() {
    if (detailMapID) {
        glDeleteTextures(1, &detailMapID);
        detailMapID = 0;
    }
}

bool Terrain::LoadLightMap(char* szFilename, int iSize) {
    m_lightmap.m_iSize = iSize;

    int width, height, nrChannels;
    unsigned char* data = stbi_load(szFilename, &width, &height, &nrChannels, 1);
    if (!data) {
        std::cerr << "Failed to load lightmap: " << szFilename << std::endl;
        return false;
    }

    if (width != iSize || height != iSize) {
        std::cerr << "Lightmap size mismatch. Expected: " << iSize << "x" << iSize
            << ", Got: " << width << "x" << height << std::endl;
        stbi_image_free(data);
        return false;
    }

    glGenTextures(1, &lightMapID);
    glBindTexture(GL_TEXTURE_2D, lightMapID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return true;
}

bool Terrain::UnloadLightMap() {
    if (lightMapID) {
        glDeleteTextures(1, &lightMapID);
        lightMapID = 0;
    }
    if (m_lightmap.m_ucpData) {
        delete[] m_lightmap.m_ucpData;
        m_lightmap.m_ucpData = nullptr;
        m_lightmap.m_iSize = 0;
    }
    return true;
}

void Terrain::CalculateLighting() {
    float fShade;
    int x, z;

    if (m_lightmap.m_iSize >=0) {
        delete[] m_lightmap.m_ucpData;
        m_lightmap.m_iSize = 0;
    }

    m_lightmap.m_ucpData = new unsigned char[(m_iSize) * (m_iSize)]();
    m_lightmap.m_iSize = m_iSize;

    // Loop through all vertices
    for (z = 0; z < m_iSize; z++) {
        for (x = 0; x < m_iSize; x++) {
            if (m_lightingType == HEIGHT_BASED) {
                SetBrightnessAtPoint(x, z, GetTrueHeightAtPoint(x, z));
            }
            else if (m_lightingType == SLOPE_LIGHT) {
                int prevX = std::max(0, x - m_iDirectionX);
                int prevZ = std::max(0, z - m_iDirectionZ);

                float heightCurrent = GetTrueHeightAtPoint(x, z);
                float heightPrevious = GetTrueHeightAtPoint(prevX, prevZ);

                fShade = 1.0f - (heightPrevious - heightCurrent) / m_fLightSoftness;

                if (fShade < m_fMinBrightness)
                    fShade = m_fMinBrightness;
                if (fShade > m_fMaxBrightness)
                    fShade = m_fMaxBrightness;

                SetBrightnessAtPoint(x, z, static_cast<unsigned char>(fShade * 255));
            }
            else {
                SetBrightnessAtPoint(x, z, 255);
            }
        }
    }

    // Bind the lightmap data to the texture
    glGenTextures(1, &lightMapID);
    glBindTexture(GL_TEXTURE_2D, lightMapID);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevent texture wrapping artifacts
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload lightmap data to the GPU
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_lightmap.m_iSize-1, m_lightmap.m_iSize-1, 0, GL_RED, GL_UNSIGNED_BYTE, m_lightmap.m_ucpData);
    glGenerateMipmap(GL_TEXTURE_2D);


    //SaveLightMap("lightmapTest.jpg");
}

bool Terrain::SaveLightMap(const char* filename) const {
    if (!m_lightmap.m_ucpData || m_lightmap.m_iSize <= 0) {
        std::cerr << "Error: Lightmap is uninitialized or empty. Cannot save." << std::endl;
        return false;
    }

    // Use stb_image_write to save as PNG
    if (stbi_write_png(filename, m_lightmap.m_iSize, m_lightmap.m_iSize, 1, m_lightmap.m_ucpData, m_lightmap.m_iSize)) {
        std::cout << "Lightmap successfully saved to " << filename << std::endl;
        return true;
    }
    else {
        std::cerr << "Error: Failed to save lightmap to " << filename << std::endl;
        return false;
    }
}