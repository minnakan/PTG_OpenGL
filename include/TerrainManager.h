#pragma once

#include "BruteForceTerrain.h"
#include <vector>

class TerrainManager {
public:
    TerrainManager();
    ~TerrainManager();

    // Initialize the terrain manager
    bool Initialize(Shader* shader, const char* textureTilesPath, const char* detailMapPath);

    // Generate terrain chunks
    void GenerateChunks(float heightScale, int chunkSize, float noiseScale = 1.0f);

    // Render all chunks
    void Render();

    // Set lighting properties for all chunks
    void SetLightingProperties(float ambient, float diffuse, float minSlope, float maxSlope, float slopePower);

private:
    std::vector<BruteForceTerrain> m_chunks;
    Shader* m_shaderProgram;
    const char* m_textureTilesPath;
    const char* m_detailMapPath;

    // Chunk configuration
    static const int CHUNK_ROWS = 3;
    static const int CHUNK_COLS = 3;

    // Common settings for all chunks
    float m_repeatDetailMap;
};