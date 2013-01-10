#pragma once

#define U16 unsigned short

// A structure for our custom vertex type
struct CUSTOMVERTEX
{
    FLOAT x, y, z; // The transformed position for the vertex
    DWORD color;        // The vertex color
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)



//-------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------
class cprimitive
{
protected:
	int m_numVertices;
	int m_numIndices;
	LPDIRECT3DVERTEXBUFFER9 g_pVB;
	LPDIRECT3DINDEXBUFFER9 g_pIB;

public:

	float fCoefficient;
	float fLength;
	float fDirection;
	float fDisplace;
	D3DXVECTOR2 vAngle;
	D3DXVECTOR2 rAngle;
	D3DXVECTOR2 restrictAngle;

	cprimitive* pFirstChild;
	cprimitive* pSiblings;

	cprimitive(void);
	~cprimitive(void);

	HRESULT Create(LPDIRECT3DDEVICE9 pd3dDevice, float length = 100.f, D3DXVECTOR2 angle = D3DXVECTOR2(0,0), float width = 40.f, D3DXVECTOR2 restrAng = D3DXVECTOR2(-D3DX_PI,D3DX_PI), float displace = 0.f, float direction = 1.f, float coefficient = 1.f);
	HRESULT Release();
	HRESULT Render(LPDIRECT3DDEVICE9 pd3dDevice);

	//hierarchy methods
	D3DXMATRIX GetRotationMatrix();
	D3DXMATRIX GetRotationYMatrix();
	D3DXVECTOR3 GetTranslationVector(D3DXMATRIX matRot);
};

