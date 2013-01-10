//-------------------------------------------------
// camera.cpp
// 
//-------------------------------------------------



#include "common/dxstdafx.h"
#include "Camera.h"


//-------------------------------------------------
// Name: constructor & destructor
// Desc:
//-------------------------------------------------
CCamera::CCamera(void)
{
	vUp = D3DXVECTOR3(0,1,0);
	vEye = D3DXVECTOR3(300,300,300);
	vAt = D3DXVECTOR3(0,0,0);

	CRange = 700.f;

	angle = D3DXVECTOR2(0,0);
}

CCamera::~CCamera(void)
{
}



//-------------------------------------------------
// angle depending movers
// 
//-------------------------------------------------
HRESULT CCamera::SetRange(float rang)
{
	/*
	CRange -= rang;
	if(CRange < 100.f) CRange = 100.f;
	*/
	CRange = 100.f;

	vAt.x = (float)(vAt.x - sin(angle.x)*cos(angle.y)*rang) ; 
	vAt.z = (float)(vAt.z - cos(angle.x)*cos(angle.y)*rang) ; 
	vAt.y = (float)(vAt.y - sin(angle.y)*rang) ;

	vEye.x = (float)(vAt.x + sin(angle.x)*cos(angle.y)*CRange) ; 
	vEye.z = (float)(vAt.z + cos(angle.x)*cos(angle.y)*CRange) ; 
	vEye.y = (float)(vAt.y + sin(angle.y)*CRange) ;

	g_Camera.SetViewParams( &vEye, &vAt, &vUp );
	return S_OK;
}


HRESULT CCamera::Rotate(D3DXVECTOR2 vecRotate)
{
	angle.x += vecRotate.x;
	angle.y += vecRotate.y;

	//borders
	if(angle.y < -D3DX_PI) angle.y += 2.f*D3DX_PI;
	if(angle.y > D3DX_PI) angle.y -= 2.f*D3DX_PI;

	if(angle.y >= -D3DX_PI*0.5f && angle.y < D3DX_PI*0.5f) vUp = D3DXVECTOR3(0,1,0); 
	else vUp = D3DXVECTOR3(0,-1,0);


	//vEye.x = (float)(vAt.x + sin(angle.x)*cos(angle.y)*CRange) ; 
	//vEye.z = (float)(vAt.z + cos(angle.x)*cos(angle.y)*CRange) ; 
	//vEye.y = (float)(vAt.y + sin(angle.y)*CRange) ;

	vAt.x = (float)(vEye.x - sin(angle.x)*cos(angle.y)*CRange) ; 
	vAt.z = (float)(vEye.z - cos(angle.x)*cos(angle.y)*CRange) ; 
	vAt.y = (float)(vEye.y - sin(angle.y)*CRange) ;

	g_Camera.SetViewParams( &vEye, &vAt, &vUp );
	return S_OK;
}

HRESULT CCamera::Slide(D3DXVECTOR2 vecSlide)
{
	vAt.x -= vecSlide.x*cos(D3DX_PI-angle.x) + vecSlide.y*sin(D3DX_PI-angle.y)*sin(angle.x);
	vAt.y -= vecSlide.y*cos(D3DX_PI-angle.y);
	vAt.z -= vecSlide.x*sin(angle.x) + vecSlide.y*sin(D3DX_PI-angle.y)*cos(angle.x);

	vEye.x = (float)(vAt.x + sin(angle.x)*cos(angle.y)*CRange) ; 
	vEye.z = (float)(vAt.z + cos(angle.x)*cos(angle.y)*CRange) ; 
	vEye.y = (float)(vAt.y + sin(angle.y)*CRange) ;

	g_Camera.SetViewParams( &vEye, &vAt, &vUp );
	return S_OK;
}

//-------------------------------------------------
// absolute movers
// 
//-------------------------------------------------
HRESULT CCamera::Setup(D3DXVECTOR3 vecEye, D3DXVECTOR3 vecAt, D3DXVECTOR2 vecAngle)
{
	vAt = vecAt;
	vEye = vecEye;
	angle = vecAngle;

	SetRange(10.f);
    g_Camera.SetViewParams( &vecEye, &vecAt, &vUp );

	return S_OK;
}

HRESULT CCamera::Move(D3DXVECTOR3 vecEyeOffset, D3DXVECTOR3 vecAtOffset)
{
	vEye = D3DXVECTOR3(vEye.x + vecEyeOffset.x, vEye.y + vecEyeOffset.y, vEye.z + vecEyeOffset.z);
	vAt = D3DXVECTOR3(vAt.x + vecAtOffset.x, vAt.y + vecAtOffset.y, vAt.z + vecAtOffset.z);

	g_Camera.SetViewParams( &vEye, &vAt, &vUp );
	return S_OK;
}



//-------------------------------------------------
// system functions
// 
//-------------------------------------------------
HRESULT CCamera::OnResetDevice(const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	// Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI/4, fAspectRatio, 0.1f, 30000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

	return S_OK;
}


D3DXMATRIX CCamera::getProjMatrix()
{
	return *g_Camera.GetProjMatrix();
}

D3DXMATRIX CCamera::getViewMatrix()
{
	return *g_Camera.GetViewMatrix();
}