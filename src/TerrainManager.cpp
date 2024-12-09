#include "TerrainManager.h"
#include <iostream>

TerrainManager::TerrainManager()
    : m_shaderProgram(nullptr)
    , m_textureTilesPath(nullptr)
    , m_detailMapPath(nullptr)
    , m_repeatDetailMap(1.0f)
{
}

TerrainManager::~TerrainManager() {
    m_chunks.clear();
}

bool TerrainManager::Initialize(Shader* shader, const char* textureTilesPath, const char* detailMapPath) {
    if (!shader || !textureTilesPath || !detailMapPath) {
        std::cerr << "Invalid initialization parameters for TerrainManager" << std::endl;
        return false;
    }

    m_shaderProgram = shader;
    m_textureTilesPath = textureTilesPath;
    m_detailMapPath = detailMapPath;

    return true;
}

void TerrainManager::GenerateChunks(float heightScale, int chunkSize, float noiseScale) {
    // Clear existing chunks
    m_chunks.clear();

    // Calculate center offset to place chunks around origin
    int centerOffsetX = (CHUNK_COLS - 1) / 2;
    int centerOffsetZ = (CHUNK_ROWS - 1) / 2;

    // Reserve space for all chunks
    m_chunks.reserve(CHUNK_ROWS * CHUNK_COLS);

    // Generate grid of chunks
    for (int row = 0; row < CHUNK_ROWS; ++row) {
        for (int col = 0; col < CHUNK_COLS; ++col) {
            // Create and add new terrain chunk
            m_chunks.emplace_back();
            BruteForceTerrain& chunk = m_chunks.back();

            // Initialize chunk
            chunk.shaderProgramme = m_shaderProgram;
            chunk.LoadtextureTiles(m_textureTilesPath);
            chunk.LoadDetailMap(m_detailMapPath);
            chunk.repeatDetailMap = m_repeatDetailMap;

            // Generate terrain with offset position
            int gridX = col - centerOffsetX;
            int gridZ = row - centerOffsetZ;
            chunk.GenerateTerrainContinuous(heightScale, chunkSize, gridX, gridZ, noiseScale);

            // Set default lighting
            chunk.SetLightingType(SLOPE_LIGHT);
            chunk.CustomizeSlopeLighting(1.0f, 0.0f, 0.1f, 1.0f, 10.0f);
            chunk.CalculateLighting();
        }
    }
}

void TerrainManager::Render() {
    for (auto& chunk : m_chunks) {
        chunk.Render();
    }
}

void TerrainManager::SetLightingProperties(float ambient, float diffuse, float minSlope, float maxSlope, float slopePower) {
    for (auto& chunk : m_chunks) {
        chunk.SetLightingType(SLOPE_LIGHT);
        chunk.CustomizeSlopeLighting(ambient, diffuse, minSlope, maxSlope, slopePower);
        chunk.CalculateLighting();
    }
}