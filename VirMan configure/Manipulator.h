#pragma once

#include "log.h"
#include "cprimitive.h"
#include <vector>
using namespace std;


class LIMB
{
public:
		LPWSTR name;
		int firstchain;
		int lastchain;
};



//-------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------
class CManipulator
{

public:

	int							numPredefinedPos;
	vector<D3DXVECTOR2*>		predefinedPositions;
	vector<LPWSTR*>				predefinedPositionNames;

	int							numOfChains;
	vector<cprimitive*>			cube;
	D3DXVECTOR3					vDest;

	vector<LIMB*>			limbs;
	int							numOfLimbs;

	//logic
	int							selectedChain;
	

	CManipulator(void);
	~CManipulator(void);

	HRESULT Create(LPDIRECT3DDEVICE9 pd3dDevice, wchar_t* file = L"./config.ini");

	HRESULT SaveConfig(wchar_t* file);

	HRESULT AddChain(LPDIRECT3DDEVICE9 pd3dDevice);
	HRESULT RemoveChain();
	HRESULT RenewIndices();

	HRESULT AddLimb(LPWSTR name, int iFirst, int iLast);
	HRESULT RemoveLimb(LPWSTR name);
	BOOL GetKinematicsAngles(LPWSTR limbName,D3DXVECTOR3 vecDest,float** fRes);
	int GetLimbIndexByName(LPWSTR name);
	int GetLimbLength(LPWSTR nam, cprimitive* nextChain = NULL, LIMB* limb = NULL);
	int GetChainIndexByID(int ID);
	int GetLimbGradient(LPWSTR limbName, char* filename, int step, int iteration);

	HRESULT Release();
	HRESULT FrameMove(float fElapsedTime, float fTime);
	HRESULT Render(LPDIRECT3DDEVICE9 pd3dDevice);
	void DestroyChain(cprimitive* current);

	//setup functions
	HRESULT SetAngles(int index, D3DXVECTOR2 angles, bool immidiately=false);
	D3DXVECTOR2* GetAngles();
	D3DXVECTOR2* GetRAngles();

	//controller functions
	HRESULT PredefinedPosition(int num);

	//logic functions
	void StartProcess();
	BOOL IsProcessing();
	wchar_t* GetReport();
	HRESULT AppendReport(wchar_t* string);

	D3DXMATRIX* GetChainMatrix(int iID, cprimitive* pCube, D3DXMATRIX mTransform);
	D3DXMATRIX* GetChainMatrix(int iID);
	D3DXVECTOR3* GetLimbCoord(LPWSTR limbname);
	D3DXVECTOR3* GetLimbCoord(int iID, cprimitive* pCube, D3DXMATRIX mTransform);

	HRESULT InversedKinematics(D3DXVECTOR3 vecDesc, LPWSTR limbName);

protected:

	//logic
	BOOL						bProcessing;
	vector<wchar_t*>			report;

	HRESULT InversedKinematics(D3DXVECTOR3 vecDest); //deprecated

	HRESULT DrawChain(cprimitive* pCube, LPDIRECT3DDEVICE9 pd3dDevice);
	HRESULT DrawChain(cprimitive* pCube, LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX mTransform);

	cprimitive* GetParent(cprimitive* search, cprimitive* current);

	HRESULT findpoint(D3DXVECTOR3 vecDest, D3DXVECTOR3 *vecRes);
	BOOL findpoint_recursive(int i, int fini, D3DXVECTOR3 vDiff, float* fRes, float vex, float vey);
	BOOL findpoint_recursive(int i, int fini, D3DXVECTOR3 vDiff, D3DXVECTOR2* fRes, D3DXMATRIX mTransform);

	HRESULT InitPredefinedPositions(wchar_t* file);
	HRESULT InitLimbs(wchar_t* file);
};
