#ifndef TERRAIN_H
#define TERRAIN_H

#include <glad/glad.h>
#include "Shader.h"

struct SHEIGHT_DATA
{
	unsigned char* m_pucData;
	int m_iSize;
};

struct SLIGHTMAP_DATA {
	unsigned char* m_ucpData;
	int m_iSize;
};

enum LightingType {
	LIGHTMAP,
	HEIGHT_BASED,
	SLOPE_LIGHT
};

class Terrain
{
protected:
	GLuint textureID;
	GLuint detailMapID;
	GLuint lightMapID;


	//Lighting values
	int m_iDirectionX;
	int m_iDirectionZ;    
	float m_fMinBrightness; 
	float m_fMaxBrightness; 
	float m_fLightSoftness; 
	LightingType m_lightingType; 

public:
	SHEIGHT_DATA m_heightData;
	float m_fHeightScale;
	int m_iSize;

	SLIGHTMAP_DATA m_lightmap;

	Shader *shaderProgramme;

	float repeatDetailMap;

	virtual void Render(void) = 0;

	bool LoadHeightMap(char* szFilename, int iSize);
	bool SaveHeightMap(char* szFilename);
	bool UnloadHeightMap(void);

	bool LoadTexture(const char* filePath);

	bool LoadDetailMap(const char* filePath);
	void UnloadDetailMap();

	bool LoadLightMap(char* szFilename, int iSize);
	bool UnloadLightMap();

	void CalculateLighting();


	inline void SetHeightScale(float fScale)
	{
		m_fHeightScale = fScale;
	}

	inline void SetHeightAtPoint(unsigned char ucHeight,int iX, int iZ)
	{
		m_heightData.m_pucData[(iZ * m_iSize) + iX] = ucHeight;
	}

	inline unsigned char GetTrueHeightAtPoint(int iX, int iZ)
	{
		return (m_heightData.m_pucData[(iZ * m_iSize) + iX]);
	}

	inline float GetScaledHeightAtPoint(int iX, int iZ)
	{
		return ((m_heightData.m_pucData[(iZ * m_iSize) + iX]) * m_fHeightScale);
	}

	inline unsigned char GetBrightnessAtPoint(int x, int z) {
		return (m_lightmap.m_ucpData[(z * m_lightmap.m_iSize) + x]);
	}

	inline void SetLightingType(LightingType lightingType) {
		m_lightingType = lightingType;
	}

	inline void CustomizeSlopeLighting(int iDirX, int iDirZ, float fMinBrightness, float fMaxBrightness, float fSoftness) {
		m_iDirectionX = iDirX;
		m_iDirectionZ = iDirZ;
		m_fMinBrightness = fMinBrightness;
		m_fMaxBrightness = fMaxBrightness;
		m_fLightSoftness = fSoftness;
	}

	inline void SetBrightnessAtPoint(int x, int z, unsigned char brightness) {
		if (m_lightmap.m_ucpData) {
			m_lightmap.m_ucpData[(z * m_lightmap.m_iSize) + x] = brightness;
		}
	}

	bool SaveLightMap(const char* filename) const;

	Terrain(void):textureID(0)
	{

	}

	~Terrain(void)
	{

	}
};
#endif