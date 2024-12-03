#ifndef TERRAIN_H
#define TERRAIN_H

#include <glad/glad.h>

struct SHEIGHT_DATA
{
	unsigned char* m_pucData;
	int m_iSize;
};

class Terrain
{
protected:
	GLuint textureID;

public:
	SHEIGHT_DATA m_heightData;
	float m_fHeightScale;
	int m_iSize;

	

	virtual void Render(void) = 0;

	bool LoadHeightMap(char* szFilename, int iSize);
	bool SaveHeightMap(char* szFilename);
	bool UnloadHeightMap(void);

	bool LoadTexture(const char* filePath);


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

	Terrain(void):textureID(0)
	{

	}

	~Terrain(void)
	{

	}
};
#endif