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