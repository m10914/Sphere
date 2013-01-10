#pragma once

#define U16 unsigned short

// A structure for our custom vertex type
struct CUSTOMVERTEX2
{
    FLOAT x, y, z; // The transformed position for the vertex
	FLOAT tu, tv; //texture coordinates
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX2 (D3DFVF_XYZ | D3DFVF_TEX2)



//-------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------
class cfield
{
protected:
	int m_numVertices;
	int m_numIndices;
	LPDIRECT3DVERTEXBUFFER9 g_pVB;
	LPDIRECT3DINDEXBUFFER9 g_pIB;
	LPDIRECT3DTEXTURE9 tex;

public:
	cfield(void);
	~cfield(void);

	HRESULT Create(LPDIRECT3DDEVICE9 pd3dDevice, D3DXVECTOR2 dim, D3DXVECTOR3 size);
	HRESULT Render(LPDIRECT3DDEVICE9 pd3dDevice);
	HRESULT Release();
};
