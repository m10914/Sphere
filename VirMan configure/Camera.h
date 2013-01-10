
#pragma once




//-------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------
class CCamera
{
protected:
	D3DXVECTOR3 vAt, vEye, vUp;
	D3DXVECTOR2 angle;
	float CRange;
	CModelViewerCamera g_Camera;

public:
	CCamera(void);
	~CCamera(void);

	//base cycle

	//absolute pos changers
	HRESULT Setup(D3DXVECTOR3 vecEye, D3DXVECTOR3 vecAt, D3DXVECTOR2 vecAngle);
	HRESULT Move(D3DXVECTOR3 vecEyeOffset, D3DXVECTOR3 vecAtOffset);

	//angle depend changers
	HRESULT Rotate(D3DXVECTOR2 vecRotate);
	HRESULT Slide(D3DXVECTOR2 vecSlide);
	HRESULT OnResetDevice(const D3DSURFACE_DESC* pBackBufferSurfaceDesc);
	HRESULT SetRange(float rang);

	//additional methods
	D3DXMATRIX getViewMatrix();
	D3DXMATRIX getProjMatrix();
	D3DXVECTOR3 getvEye() { return vEye; }
	D3DXVECTOR3 getvAt() { return vAt; }
	D3DXVECTOR2 getAngle() { return angle; }
	
};
