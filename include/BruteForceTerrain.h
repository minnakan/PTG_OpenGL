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

    bool LoadtextureTiles(const char* TilesPath);

private:
    //FIR(Finite Impulse Response) filter for smoothing
    void FilterHeightBand(float* fpBand, int iStride, int iCount, float fFilter);

    void CalculateNormals(std::vector<float>& vertices, const std::vector<unsigned int>& indices);

    //Populate Buffers from indices and vertices
    void SetupMesh();
};

#endif