#include "common/dxstdafx.h"
#include "cprimitive.h"
#include "log.h"

//-------------------------------------------------
// Name: constructor & destructor
// Desc:
//-------------------------------------------------
cprimitive::cprimitive(void)
{
	g_pVB = NULL;
	g_pIB = NULL;

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
HRESULT cprimitive::Create(LPDIRECT3DDEVICE9 pd3dDevice, float length, D3DXVECTOR2 angle, float width, D3DXVECTOR2 restrAng, float displace, float direction, float coefficient)
{
	//store
	vAngle = angle;
	rAngle = angle;
	fLength = length;
	restrictAngle = restrAng;
	fDisplace = displace;
	fDirection = direction;
	fCoefficient = coefficient;

	// VERTEX BUFFER

	// Initialize three vertices for rendering a triangle
    CUSTOMVERTEX vertices[] =
    {
		{ -width, 0.0f, -width,  0xffff0000, },
        { width, 0.0f, -width,  0xff00ff00, },
        { width, length, -width,  0xff00ffff, },
		{ -width, length, -width,  0xffffffff, },
		{ -width, 0.0f, width,  0xffff0000, },
        { width, 0.0f, width,  0xff00ff00, },
        { width, length, width,  0xff00ffff, },
		{ -width, length, width,  0xffffffff, },
		
    };
	m_numVertices = sizeof(vertices)/sizeof(CUSTOMVERTEX);

    // Create the vertex buffer
    if( FAILED( pd3dDevice->CreateVertexBuffer( m_numVertices*sizeof(CUSTOMVERTEX),
                                                  0, D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_MANAGED, &g_pVB, NULL ) ) )
    {
        return E_FAIL;
    }
    VOID* pVertices;
    if( FAILED( g_pVB->Lock( 0, sizeof(vertices), (void**)&pVertices, 0 ) ) )
        return E_FAIL;
    memcpy( pVertices, vertices, sizeof(vertices) );
    g_pVB->Unlock();


	// INDEX BUFFER

	unsigned short indices[] =
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
	m_numIndices = sizeof(indices)/sizeof(unsigned short);

	// Creating Index Buffer
    if(FAILED(pd3dDevice->CreateIndexBuffer( m_numIndices*sizeof(unsigned short), 
												0, 
												D3DFMT_INDEX16, 
												D3DPOOL_MANAGED,
												&g_pIB, 
												NULL))) 
		return E_FAIL;

    VOID* pBI;
    g_pIB->Lock( 0, sizeof(indices) , (void**)&pBI, 0 );
    memcpy( pBI, indices, m_numIndices*sizeof(unsigned short) );
    g_pIB->Unlock();	

	return S_OK;
}

//-------------------------------------------------
// Name: GetTransformationMatrix
// Desc:
//-------------------------------------------------
D3DXMATRIX cprimitive::GetRotationMatrix()
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


//-------------------------------------------------
// Name: Render
// Desc:
//-------------------------------------------------
HRESULT cprimitive::Render(LPDIRECT3DDEVICE9 pd3dDevice)
{
	HRESULT hr;

	W( pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE ) );

	W( pd3dDevice->SetIndices(g_pIB) );
	W( pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(CUSTOMVERTEX) ) );
	W( pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX ) );

	W( pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_numVertices, 0, m_numIndices/3) );

	return S_OK;
}


//-------------------------------------------------
// Name: Release
// Desc:
//-------------------------------------------------
HRESULT cprimitive::Release()
{	
    if( g_pVB != NULL )        
		g_pVB->Release();

    if( g_pIB != NULL )        
		g_pIB->Release();
	return S_OK;
}
