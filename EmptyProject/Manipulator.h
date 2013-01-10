#pragma once

#include "log.h"
#include "cprimitive.h"
#include "cfield.h"
#include <vector>
using namespace std;



//-------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------
class CManipulator
{

public:

	//field configuration
	D3DXVECTOR3					quadDimensions;
	D3DXVECTOR2					fieldDimensions;
	cfield*						field;

	int							numPredefinedPos;
	vector<D3DXVECTOR2*>		predefinedPositions;
	vector<LPWSTR*>				predefinedPositionNames;

	int							numOfChains;
	vector<cprimitive*>			cube;
	D3DXVECTOR3					vDest;

	
	CManipulator(void);
	~CManipulator(void);

	HRESULT Create(LPDIRECT3DDEVICE9 pd3dDevice);
	HRESULT Release();
	HRESULT FrameMove(float fElapsedTime, float fTime);
	HRESULT Render(LPDIRECT3DDEVICE9 pd3dDevice);

	//setup functions
	HRESULT SetAngles(int index, D3DXVECTOR2 angles, bool immidiately=false);
	D3DXVECTOR2* GetAngles();
	D3DXVECTOR2* GetRAngles();

	//controller functions
	D3DXVECTOR3 MoveTo(int quadrant, int level);
	HRESULT PredefinedPosition(int num);

	//logic functions
	void StartProcess();
	BOOL IsProcessing();
	wchar_t* GetReport();
	HRESULT AppendReport(wchar_t* string);


protected:

	//logic
	BOOL						bProcessing;
	vector<wchar_t*>			report;


	HRESULT DrawChain(cprimitive* pCube, LPDIRECT3DDEVICE9 pd3dDevice);
	HRESULT DrawChain(cprimitive* pCube, LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX matRot, D3DXMATRIX matRotY, D3DXVECTOR3 vecTranslate);
	HRESULT InversedKinematics(D3DXVECTOR3 vecDest);

	HRESULT findpoint(D3DXVECTOR3 vecDest, D3DXVECTOR3 *vecRes);
	BOOL findpoint_recursive(int i, int fini, D3DXVECTOR3 vDiff, float* fRes, float vex, float vey);
	
	HRESULT InitPredefinedPositions();
};
