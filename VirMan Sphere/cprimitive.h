#pragma once

#define U16 unsigned short


// A structure for our custom vertex type
struct CUSTOMVERTEX
{
    FLOAT x, y, z; // The transformed position for the vertex
	FLOAT tu, tv;
    //DWORD color;        // The vertex color
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_TEX1)



//-------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------
class cprimitive
{
protected:
	int m_numVertices;
	int m_numIndices;

	LPDIRECT3DTEXTURE9 tex;
	LPD3DXMESH pMesh;

public:

	int id;
	int parentID;

	bool bContact;
	D3DXVECTOR3 oldCoord;
	D3DXVECTOR3 curCoord;
	D3DXVECTOR3 vecMove;
	D3DXVECTOR3 contactPoint;


	float fCoefficient;
	float fLength;
	float fWidth;
	float fDirection;
	float fDisplace;
	D3DXVECTOR2 vAngle;
	D3DXVECTOR2 rAngle;
	D3DXVECTOR2 restrictAngleX;
	D3DXVECTOR2 restrictAngleY;

	D3DXVECTOR3 vOffset;

	float fMass;

	BOOL bTilt;
	BOOL bPressure;
	BOOL bLaser;

	cprimitive* pFirstChild;
	cprimitive* pSiblings;

	cprimitive(void);
	~cprimitive(void);

	HRESULT Create(	int iID, 
					LPDIRECT3DDEVICE9 pd3dDevice, 
					float length = 100.f, 
					D3DXVECTOR2 angle = D3DXVECTOR2(0,0), 
					float width = 10.f, 
					D3DXVECTOR2 restrAngX = D3DXVECTOR2(-D3DX_PI,D3DX_PI),
					D3DXVECTOR2 restrAngY = D3DXVECTOR2(-D3DX_PI,D3DX_PI), 
					D3DXVECTOR3 ivOffset = D3DXVECTOR3(0,0,0),
					float displace = 0.f, 
					float direction = 1.f, 
					float coefficient = 1.f,
					float mass = 100.f,
					BOOL ibTilt = false,
					BOOL ibPressure = false,
					BOOL ibLaser = false);
	HRESULT Render(LPDIRECT3DDEVICE9 pd3dDevice, BOOL bHighlight = false);
	HRESULT Release();

	//modify
	HRESULT SetWidth(float g_fWidth);
	HRESULT SetLength(float g_fLength);


	LPD3DXMESH GetMesh();

	//hierarchy methods
	D3DXMATRIX GetRotationXMatrix();
	D3DXMATRIX GetRotationYMatrix();
	D3DXVECTOR3 GetTranslationVector(D3DXMATRIX matRot);
};

