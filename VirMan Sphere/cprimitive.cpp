#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
#include "DXUTCamera.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "resource.h"

#include "cprimitive.h"
#include "log.h"

//-------------------------------------------------
// Name: constructor & destructor
// Desc:
//-------------------------------------------------
cprimitive::cprimitive(void)
{
	pMesh = NULL;
	tex = NULL;

	pFirstChild = NULL;
	pSiblings = NULL;
}

cprimitive::~cprimitive(void)
{
}



//-------------------------------------------------
// Name: Create
// Desc:
//-------------------------------------------------
HRESULT cprimitive::Create(int iID, 
						   LPDIRECT3DDEVICE9 pd3dDevice, 
						   float length, 
						   D3DXVECTOR2 angle, 
						   float width, 
						   D3DXVECTOR2 restrAngX, 
						   D3DXVECTOR2 restrAngY,
						   D3DXVECTOR3 ivOffset, 
						   float displace, 
						   float direction, 
						   float coefficient,
						   float mass,
						   BOOL ibTilt,
						   BOOL ibPressure,
						   BOOL ibLaser
						   )
{
	HRESULT hr;
	int i;

	//store
	id = iID;
	vAngle = angle;
	rAngle = angle;
	fLength = length;
	fWidth = width;
	restrictAngleX = restrAngX;
	restrictAngleY = restrAngY;
	fDisplace = displace;
	fDirection = direction;
	fCoefficient = coefficient;
	vOffset = ivOffset;
	fMass = mass;
	bTilt = ibTilt;
	bPressure = ibPressure;
	bLaser = ibLaser;


	// VERTEX BUFFER

	// Initialize three vertices for rendering a triangle
    CUSTOMVERTEX vertices[] =
    {
		{ -width, 0.0f, -width, 0.f, 0.f,},
        { width, 0.0f, -width, 1.f, 0.f,},
        { width, length, -width, 1.f, 1.f,},
		{ -width, length, -width, 0.f, 1.f, },
		{ -width, 0.0f, width, 0.f, 0.f,},
        { width, 0.0f, width, 1.f, 0.f,},
        { width, length, width, 1.f, 1.f,},
		{ -width, length, width, 0.f, 1.f,},
		
    };
	WORD indices[] =
	{
		0,3,2,
		0,2,1,

		4,7,6,
		4,6,5,

		3,7,4,
		3,4,0,

		2,6,5,
		2,5,1,

		0,4,5,
		0,5,1,

		3,6,7,
		3,2,6
	};


	m_numVertices = sizeof(vertices)/sizeof(CUSTOMVERTEX);
	m_numIndices = sizeof(indices)/sizeof(WORD);


	// CREATE MESH
	hr = D3DXCreateMeshFVF( m_numIndices / 3, m_numVertices,
					D3DXMESH_MANAGED, D3DFVF_CUSTOMVERTEX,
					pd3dDevice, &pMesh );
	if(FAILED(hr)) return NULL;

    // Create the vertex buffer	
	CUSTOMVERTEX* pVertices;
	pMesh->LockVertexBuffer(D3DLOCK_DISCARD, (void**)&pVertices);

    memcpy( pVertices, vertices, sizeof(vertices) );

	pMesh->UnlockVertexBuffer();


	// INDEX BUFFER
	WORD* pBI;
	pMesh->LockIndexBuffer(D3DLOCK_DISCARD,(void**)&pBI);

    memcpy( pBI, indices, m_numIndices*sizeof(WORD) );
    
	for(i=0;i<m_numIndices;i++)
	{
		HTMLLog("index %d ", (int)pBI[i]);
	}

	pMesh->UnlockIndexBuffer();
	

	// TEXTURE
	if( FAILED( D3DXCreateTextureFromFile( pd3dDevice, L"./data/tex000.jpg", &tex ) ) )
        return E_FAIL;

	return S_OK;
}


//-------------------------------------------------
// Name: MODIFIERS
// Desc:
//-------------------------------------------------
HRESULT cprimitive::SetLength(float g_fLength)
{
	HRESULT hr;
	LPDIRECT3DDEVICE9 pd3dDevice = DXUTGetD3D9Device();
	fLength = g_fLength;

	CUSTOMVERTEX vertices[] =
    {
		{ -fWidth, 0.0f, -fWidth, 0.f, 0.f,},
        { fWidth, 0.0f, -fWidth, 1.f, 0.f,},
        { fWidth, fLength, -fWidth, 1.f, 1.f,},
		{ -fWidth, fLength, -fWidth, 0.f, 1.f, },
		{ -fWidth, 0.0f, fWidth, 0.f, 0.f,},
        { fWidth, 0.0f, fWidth, 1.f, 0.f,},
        { fWidth, fLength, fWidth, 1.f, 1.f,},
		{ -fWidth, fLength, fWidth, 0.f, 1.f,},
		
    };
	m_numVertices = sizeof(vertices)/sizeof(CUSTOMVERTEX);


    // Create the vertex buffer	
	CUSTOMVERTEX* pVertices;
	pMesh->LockVertexBuffer(D3DLOCK_DISCARD, (void**)&pVertices);

    memcpy( pVertices, vertices, sizeof(vertices) );

	pMesh->UnlockVertexBuffer();

	return S_OK;

}
HRESULT cprimitive::SetWidth(float g_fWidth)
{
	fWidth = g_fWidth;

	CUSTOMVERTEX vertices[] =
    {
		{ -fWidth, 0.0f, -fWidth, 0.f, 0.f,},
        { fWidth, 0.0f, -fWidth, 1.f, 0.f,},
        { fWidth, fLength, -fWidth, 1.f, 1.f,},
		{ -fWidth, fLength, -fWidth, 0.f, 1.f, },
		{ -fWidth, 0.0f, fWidth, 0.f, 0.f,},
        { fWidth, 0.0f, fWidth, 1.f, 0.f,},
        { fWidth, fLength, fWidth, 1.f, 1.f,},
		{ -fWidth, fLength, fWidth, 0.f, 1.f,},
		
    };
	m_numVertices = sizeof(vertices)/sizeof(CUSTOMVERTEX);


    // Create the vertex buffer	
	CUSTOMVERTEX* pVertices;
	pMesh->LockVertexBuffer(D3DLOCK_DISCARD, (void**)&pVertices);

    memcpy( pVertices, vertices, sizeof(vertices) );

	pMesh->UnlockVertexBuffer();

	return S_OK;
}



//-------------------------------------------------
// Name: GetTransformationMatrix
// Desc:
//-------------------------------------------------
D3DXMATRIX cprimitive::GetRotationXMatrix()
{
	D3DXMATRIX matRotX;
	D3DXMatrixRotationX(&matRotX, vAngle.x);
	return matRotX;
}
D3DXMATRIX cprimitive::GetRotationYMatrix()
{
	D3DXMATRIX matRotY;
	D3DXMatrixRotationY(&matRotY, vAngle.y);
	return matRotY;
}
D3DXVECTOR3 cprimitive::GetTranslationVector(D3DXMATRIX matRot)
{
	D3DXMATRIX matTrans;

	D3DXVECTOR4 vec4res;
	D3DXVECTOR3 vec3init = D3DXVECTOR3(0.f, fLength, 0.f);
	D3DXVec3Transform(&vec4res, &vec3init, &matRot); 
	//HTMLLog("%d > %d %d %d > %d\n", (int)fLength, (int)vec4res.x, (int)vec4res.y, (int)vec4res.z, (int)(sqrt(vec4res.x*vec4res.x + vec4res.y*vec4res.y + vec4res.z*vec4res.z))); 

	return D3DXVECTOR3(vec4res.x, vec4res.y, vec4res.z);
}


LPD3DXMESH cprimitive::GetMesh()
{
	return pMesh;
}


//-------------------------------------------------
// Name: Render
// Desc:
//-------------------------------------------------
HRESULT cprimitive::Render(LPDIRECT3DDEVICE9 pd3dDevice, BOOL bHighlight)
{
	HRESULT hr;

	W( pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE ) );

	if(bHighlight) pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR,  0xFFFFFF);
	else pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR,  0x888888);

	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE); 
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);

	W( pd3dDevice->SetTexture(0, tex) );

	W( pMesh->DrawSubset(0) );

	//W( pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_numVertices, 0, m_numIndices/3) );

	return S_OK;
}


//-------------------------------------------------
// Name: Release
// Desc:
//-------------------------------------------------
HRESULT cprimitive::Release()
{	

	SAFE_RELEASE( tex );

	SAFE_RELEASE( pMesh );

	return S_OK;
}
