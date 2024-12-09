#ifndef BRUTE_FORCE_TERRAIN_H
#define BRUTE_FORCE_TERRAIN_H

#include "Terrain.h"
#include <vector>
#include "TextureGenerator.h"


class BruteForceTerrain : public Terrain
{
private:
    unsigned int VAO, VBO, EBO;
    std::vector<float> m_vertices;
    std::vector<unsigned int> m_indices;

    TextureGenerator m_textureGenerator;
    

public:
    BruteForceTerrain();
    ~BruteForceTerrain();

    void Render(void) override;

    //Fractal Terrain Generation using fault formation
    void GenerateTerrainFractal_FF(float iMinDelta, float iMaxDelta, int iterations, int size, bool applyFilter = false, float filterStrength = 0.4f, float heightScale = 0.1f);

    //Fractal Terrain Generation using Mid Point displacement
    void GenerateTerrainFractal_MPD(float initialHeight, float roughness, int size, float heightScale = 0.1f);

    void StitchTerrainMPD(const std::vector<float>& sourceVertices, const std::vector<unsigned int>& sourceIndices, int sourceSize, int direction, float heightScale, float initialHeight, float roughness);

    void GenerateIndicesWithBlendZone(int size, int direction, float blendDistance);

    glm::vec3 GetVertexPosition(unsigned int index);

    unsigned int AddVertex(const glm::vec3& position);

    bool LoadtextureTiles(const char* TilesPath);

    const std::vector<float>& GetVertices() const { return m_vertices; }

    const std::vector<unsigned int>& GetIndices() const { return m_indices; }

    int GetSize() const { return m_iSize; }

private:
    //FIR(Finite Impulse Response) filter for smoothing
    void FilterHeightBand(float* fpBand, int iStride, int iCount, float fFilter);

    void CalculateNormals(std::vector<float>& vertices, const std::vector<unsigned int>& indices);

    //Populate Buffers from indices and vertices
    void SetupMesh();

    void SetupTerrainData(int newSize, const std::vector<float>& heightBuffer);
};

#endif