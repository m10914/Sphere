#include "common/dxstdafx.h"
#include "cfield.h"
#include "log.h"


//-------------------------------------------------
// Name: constructor & destructor
// Desc:
//-------------------------------------------------
cfield::cfield(void)
{
	g_pVB = NULL;
	g_pIB = NULL;
}

cfield::~cfield(void)
{
}

HRESULT cfield::Create(LPDIRECT3DDEVICE9 pd3dDevice, D3DXVECTOR2 dim, D3DXVECTOR3 size)
{
	int i,j;
	m_numVertices = (dim.x+1)*(dim.y+1);
	m_numIndices = 6 * dim.x*dim.y;

	// VERTEX BUFFER

	double dx = size.x;
    double dy = size.y;
    
    double ypos = 0;

    CUSTOMVERTEX2* mesh_vert = new CUSTOMVERTEX2[m_numVertices];
    for(i = 0; i < dim.y+1; i++, ypos += dy)
    {
		double xpos = -dim.x * size.x * 0.5f;
		for(j = 0; j < dim.x+1; j++, xpos += dx)
		{
			//Vertex initialization
			mesh_vert[j + i*((int)dim.x+1)].x = (float)xpos;
			mesh_vert[j + i*((int)dim.x+1)].y = 0.f;
			mesh_vert[j + i*((int)dim.x+1)].z = (float)ypos;

			//normals
			mesh_vert[j + i*((int)dim.x+1)].tu = (float)j;
			mesh_vert[j + i*((int)dim.x+1)].tv = (float)i;

		}
	}


	// Creating Vertex Buffer
    if( FAILED( pd3dDevice->CreateVertexBuffer( m_numVertices * sizeof(CUSTOMVERTEX2),
													0, 
													D3DFVF_CUSTOMVERTEX2,
													D3DPOOL_MANAGED, 
													&g_pVB, 
													NULL ) ) )

        return E_FAIL;

    VOID* pBV;
    if( FAILED( g_pVB->Lock( 0, 
							sizeof(mesh_vert), 
							(void**)&pBV, 
							0 ) ) ) 
        return E_FAIL;
    memcpy( pBV, mesh_vert, m_numVertices*sizeof(CUSTOMVERTEX2) );
    g_pVB->Unlock(); 


	// INDEX BUFFER

	U16* m_ind = new U16[m_numIndices];
    U16* ind = m_ind;
    for(i = 0; i < dim.y; i++)
    {
      for(j = 0; j < dim.x; j++)
      {
          // first triangle
          *ind++ = (U16)(i*(dim.x+1) + j);
          *ind++ = (U16)((i+1)*(dim.x+1)  + j);
          *ind++ = (U16)((i+1)*(dim.x+1) +j+1);

          // second triangle      
          *ind++ = (U16)(i*(dim.x+1)  + j);
          *ind++ = (U16)((i+1)*(dim.x+1)  + j+1);
          *ind++ = (U16)(i*(dim.x+1)  + j+1);
      }
    }
	
	for(i=0;i<m_numIndices; i++)
	{
		HTMLLog("%d %d %d\n", (int)m_ind[i], (int)m_ind[i], (int)m_ind[i]);
	}

   	// Creating Index Buffer
    if(FAILED(pd3dDevice->CreateIndexBuffer( m_numIndices* sizeof(U16), 
												0, 
												D3DFMT_INDEX16, 
												D3DPOOL_MANAGED,
												&g_pIB, 
												NULL))) 
		return E_FAIL;
    VOID* pBI;
    g_pIB->Lock( 0, sizeof(m_ind) , (void**)&pBI, 0 );
    memcpy( pBI, m_ind, m_numIndices*sizeof(U16) );
    g_pIB->Unlock();		


	// TEXTURE
	if( FAILED( D3DXCreateTextureFromFile( pd3dDevice, L"./data/tex000.jpg", &tex ) ) )
        return E_FAIL;

	return S_OK;
}


HRESULT cfield::Render(LPDIRECT3DDEVICE9 pd3dDevice)
{
	HRESULT hr;

	W( pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE ) );

	W( pd3dDevice->SetIndices(g_pIB) );
	W( pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(CUSTOMVERTEX2) ) );
	W( pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX2 ) );
	pd3dDevice->SetTexture(0, tex);

	if(FAILED( pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_numVertices, 0, m_numIndices/3) ))
		return E_FAIL;

	return S_OK;
}


HRESULT cfield::Release()
{
	if( g_pVB != NULL )        
		g_pVB->Release();

    if( g_pIB != NULL )        
		g_pIB->Release();

	tex->Release();

	return S_OK;
}