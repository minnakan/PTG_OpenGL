#include "BruteForceTerrain.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include <algorithm>



BruteForceTerrain::BruteForceTerrain(): VAO(0), VBO(0), EBO(0)
{
    
}

BruteForceTerrain::~BruteForceTerrain()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void BruteForceTerrain::Render(void)
{
    if (!m_heightData.m_pucData) {
        return;
    }
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    shaderProgramme->setInt("terrainTexture", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, detailMapID);
    shaderProgramme->setInt("detailTexture", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, lightMapID);
    shaderProgramme->setInt("lightMapTexture", 2);

    shaderProgramme->setFloat("repeatDetailMap", repeatDetailMap);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
    
    
    /*glBindTexture(GL_TEXTURE_2D, textureID);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);*/
}

void BruteForceTerrain::GenerateTerrainFractal_FF(float iMinDelta, float iMaxDelta, int iterations, int size, bool applyFilter, float filterStrength, float heightScale)
{
    Terrain::SetHeightScale(heightScale);
    //TODO Allow generating rectangles; need width and height instead of just size;
    if (m_heightData.m_iSize >= 0) {
        delete[] m_heightData.m_pucData;
        m_heightData.m_iSize = 0;
    }
    m_heightData.m_pucData = new unsigned char[size * size];
    if (!m_heightData.m_pucData) {
        std::cerr << "Failed to allocate memory for height data." << std::endl;
        return;
    }

    m_iSize = size;
    std::fill(m_heightData.m_pucData, m_heightData.m_pucData + m_iSize * m_iSize, 0);

    // Floating-point buffer for accumulation
    std::vector<float> heightBuffer(m_iSize * m_iSize, 128.0f);  // Start at middle height

    // Fault-formation algorithm
    for (int iCurrentIteration = 0; iCurrentIteration < iterations; ++iCurrentIteration) {
        // Generate random line endpoints
        int iRandX1 = rand() % m_iSize;
        int iRandZ1 = rand() % m_iSize;
        int iRandX2 = rand() % m_iSize;
        int iRandZ2 = rand() % m_iSize;

        // Ensure we don't have a degenerate line
        if (iRandX1 == iRandX2 && iRandZ1 == iRandZ2) {
            continue;
        }

        // Line direction vector
        float iDirX1 = static_cast<float>(iRandX2 - iRandX1);
        float iDirZ1 = static_cast<float>(iRandZ2 - iRandZ1);

        // Height adjustment based on the current iteration (with decay)
        float iHeight = iMaxDelta * pow(0.5f, static_cast<float>(iCurrentIteration) / (iterations / 4.0f));

        // Loop through the grid
        for (int x = 0; x < m_iSize; x++) {
            for (int z = 0; z < m_iSize; z++) {
                // Vector from the line's start point to the current point
                float iDirX2 = static_cast<float>(x - iRandX1);
                float iDirZ2 = static_cast<float>(z - iRandZ1);

                // Cross product Z-component
                float crossZ = iDirX2 * iDirZ1 - iDirX1 * iDirZ2;

                // Elevate the point if it's on the "positive" side of the line
                int currentIndex = z * m_iSize + x;
                heightBuffer[currentIndex] += (crossZ > 0) ? iHeight : -iHeight;
            }
        }

        if (applyFilter) {
            // Filter rows (left to right)
            for (int z = 0; z < m_iSize; z++) {
                FilterHeightBand(heightBuffer.data() + z * m_iSize, 1, m_iSize, filterStrength);
            }

            // Filter rows (right to left)
            for (int z = 0; z < m_iSize; z++) {
                FilterHeightBand(heightBuffer.data() + z * m_iSize + (m_iSize - 1), -1, m_iSize, filterStrength);
            }

            // Filter columns (top to bottom)
            for (int x = 0; x < m_iSize; x++) {
                FilterHeightBand(heightBuffer.data() + x, m_iSize, m_iSize, filterStrength);
            }

            // Filter columns (bottom to top)
            for (int x = 0; x < m_iSize; x++) {
                FilterHeightBand(heightBuffer.data() + x + (m_iSize * (m_iSize - 1)), -m_iSize, m_iSize, filterStrength);
            }
        }
    }

    // Find min and max heights for normalization
    float minHeight = *std::min_element(heightBuffer.begin(), heightBuffer.end());
    float maxHeight = *std::max_element(heightBuffer.begin(), heightBuffer.end());
    float heightRange = maxHeight - minHeight;

    // Normalize and convert to unsigned char
    if (heightRange > 0) {  // Prevent division by zero
        for (int i = 0; i < m_iSize * m_iSize; ++i) {
            float normalizedHeight = (heightBuffer[i] - minHeight) / heightRange;
            m_heightData.m_pucData[i] = static_cast<unsigned char>(normalizedHeight * 255.0f);
        }
    }

    //Terrain::SaveHeightMap("heightmap_debug_FF.png");

    // Update VBO and EBO for rendering
    m_vertices.clear();
    m_indices.clear(); 

    // Generate vertex data with normals and texture coordinates
    for (int z = 0; z < m_iSize; z++) {
        for (int x = 0; x < m_iSize; x++) {

            float height = GetScaledHeightAtPoint(x, z);
            m_vertices.push_back(static_cast<float>(x)); 
            m_vertices.push_back(height);               
            m_vertices.push_back(static_cast<float>(z));

            // Placeholder normal
            m_vertices.push_back(0.0f); 
            m_vertices.push_back(1.0f); 
            m_vertices.push_back(0.0f);

            // Texture coordinates
            m_vertices.push_back(static_cast<float>(x) / m_iSize); 
            m_vertices.push_back(static_cast<float>(z) / m_iSize); 
        }
    }

    // Generate indices for triangle strips
    for (int z = 0; z < m_iSize - 1; z++) {
        for (int x = 0; x < m_iSize - 1; x++) {
            unsigned int topLeft = z * m_iSize + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (z + 1) * m_iSize + x;
            unsigned int bottomRight = bottomLeft + 1;

            m_indices.push_back(topLeft);
            m_indices.push_back(bottomLeft);
            m_indices.push_back(topRight);

            m_indices.push_back(topRight);
            m_indices.push_back(bottomLeft);
            m_indices.push_back(bottomRight);
        }
    }

    // Calculate normals
    CalculateNormals(m_vertices,m_indices);

    if (textureID) {
        glDeleteTextures(1, &textureID);
    }
    textureID = m_textureGenerator.GenerateTexture(m_heightData.m_pucData, m_iSize, 256);

    SetupMesh();
}

void BruteForceTerrain::GenerateTerrainFractal_MPD(float initialHeight, float roughness, int size, float heightScale)
{   
     Terrain::SetHeightScale(heightScale);
    //Giving sizes in powers of 2 is more intuitive

    size = size + 1;
    // Verify size is 2^n + 1
    if (((size - 1) & (size - 2)) != 0) {
        std::cerr << "Size must be 2^n + 1 for midpoint displacement algorithm." << std::endl;
        return;
    }

    // Allocate memory for height data
    if (m_heightData.m_iSize >= 0) {
        delete[] m_heightData.m_pucData;
        m_heightData.m_iSize = 0;
    }
    m_heightData.m_pucData = new unsigned char[size * size];
    if (!m_heightData.m_pucData) {
        std::cerr << "Failed to allocate memory for height data." << std::endl;
        return;
    }

    m_iSize = size;
    std::vector<float> heightBuffer(m_iSize * m_iSize, 0.0f);

    // Initialize corners with base height
    float currentSpread = initialHeight;
    float baseHeight = initialHeight * 0.5f;

    auto GetOffset = [&currentSpread]() {
        return ((static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f) * currentSpread;
        };

    auto GetAverageOf2 = [](float a, float b) {
        return (a + b) * 0.5f;
        };

    auto GetAverageOf4 = [](float a, float b, float c, float d) {
        return (a + b + c + d) * 0.25f;
        };

    // Set initial corner values
    heightBuffer[0] = baseHeight;                          
    heightBuffer[size - 1] = baseHeight;                   
    heightBuffer[size * (size - 1)] = baseHeight;          
    heightBuffer[size * size - 1] = baseHeight;            

    // Calculate number of iterations needed
    int n = static_cast<int>(log2(size - 1));

    // Main displacement loop
    for (int i = 0; i < n; i++) {
        int numberOfQuads = static_cast<int>(pow(4, i));
        int quadsPerRow = static_cast<int>(sqrt(numberOfQuads));
        int quadLength = (size - 1) / quadsPerRow;

        // Process each quad
        for (int y = 0; y < quadsPerRow; y++) {
            for (int x = 0; x < quadsPerRow; x++) {
                int x0 = quadLength * x;
                int x1 = quadLength * (x + 1);
                int y0 = quadLength * y;
                int y1 = quadLength * (y + 1);

                // Calculate midpoints
                int mx = (x0 + x1) / 2;
                int my = (y0 + y1) / 2;

                // Calculate edge midpoints first
                float bottom = GetAverageOf2(heightBuffer[x0 + y0 * size], heightBuffer[x1 + y0 * size]) + GetOffset();
                float top = GetAverageOf2(heightBuffer[x0 + y1 * size], heightBuffer[x1 + y1 * size]) + GetOffset();
                float left = GetAverageOf2(heightBuffer[x0 + y0 * size], heightBuffer[x0 + y1 * size]) + GetOffset();
                float right = GetAverageOf2(heightBuffer[x1 + y0 * size], heightBuffer[x1 + y1 * size]) + GetOffset();

                // Store edge midpoints
                heightBuffer[mx + y0 * size] = bottom;
                heightBuffer[mx + y1 * size] = top;
                heightBuffer[x0 + my * size] = left;
                heightBuffer[x1 + my * size] = right;

                // Calculate and store center point
                heightBuffer[mx + my * size] = GetAverageOf4(bottom, top, left, right) + GetOffset();
            }
        }

        // Adjust spread based on roughness
        currentSpread *= pow(0.5f, roughness);
    }

    // Find min and max heights for normalization
    float minHeight = *std::min_element(heightBuffer.begin(), heightBuffer.end());
    float maxHeight = *std::max_element(heightBuffer.begin(), heightBuffer.end());
    float heightRange = maxHeight - minHeight;

    // Normalize and convert to unsigned char
    if (heightRange > 0) {
        for (int i = 0; i < size * size; ++i) {
            float normalizedHeight = (heightBuffer[i] - minHeight) / heightRange;
            m_heightData.m_pucData[i] = static_cast<unsigned char>(normalizedHeight * 255.0f);
        }
    }

    //Terrain::SaveHeightMap("heightmap_debug_MPD.png");

    m_vertices.clear();
    m_indices.clear(); 

    // Generate vertex data with normals and texture coordinates
    for (int z = 0; z < m_iSize; z++) {
        for (int x = 0; x < m_iSize; x++) {
            float height = GetScaledHeightAtPoint(x, z);
            m_vertices.push_back(static_cast<float>(x)); 
            m_vertices.push_back(height);               
            m_vertices.push_back(static_cast<float>(z)); 

            // Placeholder normal
            m_vertices.push_back(0.0f); 
            m_vertices.push_back(1.0f); 
            m_vertices.push_back(0.0f);

            // Texture coordinates
            m_vertices.push_back(static_cast<float>(x) / m_iSize); 
            m_vertices.push_back(static_cast<float>(z) / m_iSize);
        }
    }

    // Generate indices for triangle strips
    for (int z = 0; z < m_iSize - 1; z++) {
        for (int x = 0; x < m_iSize - 1; x++) {
            unsigned int topLeft = z * m_iSize + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (z + 1) * m_iSize + x;
            unsigned int bottomRight = bottomLeft + 1;

            m_indices.push_back(topLeft);
            m_indices.push_back(bottomLeft);
            m_indices.push_back(topRight);

            m_indices.push_back(topRight);
            m_indices.push_back(bottomLeft);
            m_indices.push_back(bottomRight);
        }
    }

    // Calculate normals
    CalculateNormals(m_vertices,m_indices);

    if (textureID) {
        glDeleteTextures(1, &textureID);
    }
    std::cout << "Size = " << m_iSize<<std::endl;

    textureID = m_textureGenerator.GenerateTexture(m_heightData.m_pucData, m_iSize, 256);

    SetupMesh();
}

bool BruteForceTerrain::LoadtextureTiles(const char* TilesPath) {
    // Load base textures with overlapping ranges
    std::string basePath(TilesPath);

    if (!m_textureGenerator.AddTextureTile((basePath + "Grass.jpg").c_str(), 0, 64)) { 
        return false;
    }
    if (!m_textureGenerator.AddTextureTile((basePath + "Dirt.jpg").c_str(), 48, 128)) {      
        return false;
    }
    if (!m_textureGenerator.AddTextureTile((basePath + "BlackRock.jpg").c_str(), 112, 192)) {     
        return false;
    }
    if (!m_textureGenerator.AddTextureTile((basePath + "Snow.jpg").c_str(), 176, 255)) { 
        return false;
    }

    return true;
}

void BruteForceTerrain::FilterHeightBand(float* fpBand, int iStride, int iCount, float fFilter)
{
    float v = fpBand[0];  // Start with first value in band
    int j = iStride;      // Start at first stride position

    // Go through the height band and apply the erosion filter
    for (int i = 0; i < iCount - 1; i++) {
        //new_value = filter * previous_value + (1-filter) * current_value
        fpBand[j] = fFilter * v + (1.0f - fFilter) * fpBand[j];
        v = fpBand[j];
        j += iStride;
    }
}

void BruteForceTerrain::CalculateNormals(std::vector<float>& vertices, const std::vector<unsigned int>& indices)
{
    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int idx0 = indices[i];
        unsigned int idx1 = indices[i + 1];
        unsigned int idx2 = indices[i + 2];

        glm::vec3 v0(vertices[idx0 * 8], vertices[idx0 * 8 + 1], vertices[idx0 * 8 + 2]);
        glm::vec3 v1(vertices[idx1 * 8], vertices[idx1 * 8 + 1], vertices[idx1 * 8 + 2]);
        glm::vec3 v2(vertices[idx2 * 8], vertices[idx2 * 8 + 1], vertices[idx2 * 8 + 2]);

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        for (int j = 0; j < 3; ++j) {
            unsigned int idx = indices[i + j];
            vertices[idx * 8 + 3] += normal.x;
            vertices[idx * 8 + 4] += normal.y;
            vertices[idx * 8 + 5] += normal.z;
        }
    }

    // Normalize all normals
    for (size_t i = 0; i < vertices.size(); i += 8) {
        glm::vec3 normal(vertices[i + 3], vertices[i + 4], vertices[i + 5]);
        normal = glm::normalize(normal);
        vertices[i + 3] = normal.x;
        vertices[i + 4] = normal.y;
        vertices[i + 5] = normal.z;
    }
}

void BruteForceTerrain::SetupMesh()
{

    if (m_vertices.empty() || m_indices.empty()) {
        std::cout << "No vertex or index data!" << std::endl;
        return;
    }

    // Create buffers/arrays if they don't exist
    if (VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    }

    glBindVertexArray(VAO);

    // Load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float), m_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);

    // Set the vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Check for OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "OpenGL error in SetupMesh: " << err << std::endl;
    }

    glBindVertexArray(0);
}

