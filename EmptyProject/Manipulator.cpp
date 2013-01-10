//-------------------------------------------------
// Manipulator.cpp
//
//-------------------------------------------------


#include "common/dxstdafx.h"
#include "Manipulator.h"
#include "cmath"



//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
CManipulator::CManipulator(void)
{
	bProcessing = false;
}

//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
CManipulator::~CManipulator(void)
{
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
wchar_t* CManipulator::GetReport()
{
	if(report.size() == 0)  return NULL;
	else
	{
		wchar_t* myreport = report.back();
		wchar_t* newreport = new wchar_t[256];
		swprintf(newreport,L"%s",myreport);

		report.pop_back();
		return newreport;
	}
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::AppendReport(wchar_t* string)
{
	//allocating memory
	wchar_t* newstring = new wchar_t[256];
	wsprintf(newstring,L"%s",string);

	report.push_back(newstring);
	return S_OK;
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
void CManipulator::StartProcess() { bProcessing = true; }
BOOL CManipulator::IsProcessing() { return bProcessing; }


//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::Create(LPDIRECT3DDEVICE9 pd3dDevice)
{
	int i;
	cprimitive* primitive;
	wchar_t *outdata = new wchar_t[512];

	//-----
	// read field parameters from cfg

	D3DXVECTOR2 dummy;

	outdata = IniRead(L"./config.ini", L"field", L"width");
	dummy.x = _wtoi(outdata);
	outdata = IniRead(L"./config.ini", L"field", L"height");
	dummy.y = _wtoi(outdata);

	fieldDimensions = dummy;

	D3DXVECTOR3 dummy2;

	outdata = IniRead(L"./config.ini", L"field", L"quad_width");
	dummy2.x = _wtof(outdata);
	outdata = IniRead(L"./config.ini", L"field", L"quad_height");
	dummy2.y = _wtoi(outdata);
	outdata = IniRead(L"./config.ini", L"field", L"quad_z");
	dummy2.z = _wtoi(outdata);

	quadDimensions = dummy2;

	field = new cfield;
	field->Create(pd3dDevice, dummy, dummy2);

	//-----
	// create hierarchy from CFG
	outdata = IniRead(L"./config.ini", L"manipulator", L"chainsize");
	numOfChains = _wtoi(outdata);

	for( i = 0; i < numOfChains; i++ )
	{
		primitive = new cprimitive;

		wchar_t string[512];
		wsprintf(string, L"chain_%d", (i));

		float length = _wtof(IniRead(L"./config.ini", string, L"length"));
		D3DXVECTOR2 initangle;
		swscanf( IniRead(L"./config.ini", string, L"initangle"), L"%f %f", &(initangle.x), &(initangle.y));
		float width = _wtof(IniRead(L"./config.ini", string, L"width"));
		D3DXVECTOR2 restrAng;
		swscanf( IniRead(L"./config.ini", string, L"angle_restrict_phi"), L"%f %f", &(restrAng.x), &(restrAng.y));
		float displace = _wtof(IniRead(L"./config.ini", string, L"displace"));
		float direction = _wtof(IniRead(L"./config.ini", string, L"direction"));
		float coefficient = _wtof(IniRead(L"./config.ini", string, L"coefficient"));

		cube.push_back(primitive);
		if(FAILED( cube[i]->Create(pd3dDevice, length, initangle, width, restrAng, displace, direction, coefficient) )) HTMLLog("cube[0] failed");
	}

	//build hierarchy
	for( i = 0; i < numOfChains; i++ )
	{
		wchar_t string[512];
		wsprintf(string, L"chain_%d", (i));

		int index = _wtoi(IniRead(L"./config.ini", string, L"parent_index"));
		if(index == -1) continue;

		//looking for a child
		cprimitive* ptr;
		if(cube[index]->pFirstChild == NULL)
		{
			cube[index]->pFirstChild = cube[i];
		}
		else
		{
			for(ptr = cube[index]->pFirstChild; ptr->pSiblings != NULL; ptr = ptr->pSiblings);
			ptr->pSiblings = cube[index];
		}

	}

	//init predefined positions
	InitPredefinedPositions();


	return S_OK;
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
D3DXVECTOR2* CManipulator::GetRAngles()
{
	int i;
	D3DXVECTOR2* angles = new D3DXVECTOR2[numOfChains];
	for(i=0; i < numOfChains; i++)
	{
		angles[i] = cube[i]->rAngle;
	}
	return angles;
}



//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
D3DXVECTOR2* CManipulator::GetAngles()
{
	int i;
	D3DXVECTOR2* angles = new D3DXVECTOR2[numOfChains];
	for(i=0; i < numOfChains; i++)
	{
		angles[i] = cube[i]->vAngle;
	}
	return angles;
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::SetAngles(int index, D3DXVECTOR2 angles, bool immidiately)
{
	if(immidiately) cube[index]->vAngle = angles;
	cube[index]->rAngle = angles;

	return S_OK;
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
D3DXVECTOR3 CManipulator::MoveTo(int quadrant, int level)
{
	//calculate absolute position of point
	float x = quadrant%(int)fieldDimensions.x * quadDimensions.x + quadDimensions.x*0.5f - fieldDimensions.x*0.5*quadDimensions.x;
	float y = (int)(quadrant/(int)fieldDimensions.x) * quadDimensions.y + quadDimensions.y*0.5f;
	float z = (float)level*quadDimensions.z;

	vDest = D3DXVECTOR3(x,y,z);
	InversedKinematics(vDest);

	return vDest;
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::findpoint(D3DXVECTOR3 vecDest, D3DXVECTOR3* vecRes)
{
	// method: iterative
	
	D3DXVECTOR3 vDest = D3DXVECTOR3(vecDest.x, vecDest.y, vecDest.z + cube[4]->fLength);
	D3DXVECTOR3 vInit = D3DXVECTOR3(0,0,cube[0]->fLength);
	D3DXVECTOR3 vDiff = vDest-vInit;

	float iter = 0.02f;
	float eps = 0.8f;
	float q1,q2,q3;
	float x = D3DXVec3Length(&vDiff);

		
	for( q1 = cube[1]->restrictAngle.x; q1 <= cube[1]->restrictAngle.y; q1 += iter )
	{
		for( q2 = cube[2]->restrictAngle.x; q2 <= cube[2]->restrictAngle.y; q2 += iter )
		{
			for( q3 = cube[3]->restrictAngle.x; q3 <= cube[3]->restrictAngle.y; q3 += iter )
			{
				float vex = cube[1]->fLength*sin(q1) + cube[2]->fLength*sin(q1+q2) + cube[3]->fLength*sin(q1+q2+q3);
				float vey = cube[1]->fLength*cos(q1) + cube[2]->fLength*cos(q1+q2) + cube[3]->fLength*cos(q1+q2+q3);
				
				if( fabs( vDiff.z - vey ) <= eps && fabs( sqrt(vDiff.x*vDiff.x + vDiff.y*vDiff.y) - vex ) <= eps )
				{
					*vecRes = D3DXVECTOR3(q1,q2,q3);
					return S_OK;
				}
			}
		}
	}
	return E_FAIL;
	
}



//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
BOOL CManipulator::findpoint_recursive(int i, int fini, D3DXVECTOR3 vDiff, float* fRes, float vex, float vey)
{
	//method - iterative

	int j;
	float iter = 0.02f;
	float eps = 0.8f;
	float q;
	float x = D3DXVec3Length(&vDiff);
	float start = cube[i]->restrictAngle.x + (cube[i]->restrictAngle.y-cube[i]->restrictAngle.x)*0.5f;

	//two searches - to optimize angles
	for( q = start; q >= cube[i]->restrictAngle.x; q-= iter)
	{
		fRes[i] = q;

		float totalangle = 0;
		for(j=0;j<=i;j++)  totalangle += fRes[j];

		float nVex = vex + cube[i]->fLength*sin(totalangle);
		float nVey = vey + cube[i]->fLength*cos(totalangle);

		if(i == fini)
		{
			if( fabs( vDiff.z - nVey ) <= eps && fabs( sqrt(vDiff.x*vDiff.x + vDiff.y*vDiff.y) - nVex ) <= eps )
				return true;
		}
		else
		{
			BOOL hr = findpoint_recursive(i+1, fini, vDiff, fRes, nVex, nVey);
			if(hr == true) return hr;
		}
	}
	for( q = start; q <= cube[i]->restrictAngle.y; q+= iter)
	{
		fRes[i] = q;

		float totalangle = 0;
		for(j=0;j<=i;j++)  totalangle += fRes[j];

		float nVex = vex + cube[i]->fLength*sin(totalangle);
		float nVey = vey + cube[i]->fLength*cos(totalangle);

		if(i == fini)
		{
			if( fabs( vDiff.z - nVey ) <= eps && fabs( sqrt(vDiff.x*vDiff.x + vDiff.y*vDiff.y) - nVex ) <= eps )
				return true;
		}
		else
		{
			BOOL hr = findpoint_recursive(i+1, fini, vDiff, fRes, nVex, nVey);
			if(hr == true) return hr;
		}
	}

	return false;
}



//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::InversedKinematics(D3DXVECTOR3 vecDest)
{		
	//recursive
	int i = 0;
	D3DXVECTOR3 vDest = D3DXVECTOR3(vecDest.x, vecDest.y, vecDest.z + cube[4]->fLength);
	D3DXVECTOR3 vInit = D3DXVECTOR3(0,0,cube[0]->fLength);
	D3DXVECTOR3 vDiff = vDest-vInit;
	
	float* fRes = new float[numOfChains];
	for( i=0; i < numOfChains; i++) fRes[i] = 0;

	if( findpoint_recursive(1,numOfChains-2,vDiff,fRes,0,0) )
	{
		float totalangle = 0;
		for(int j=1; j < numOfChains-1; j++)
		{
			SetAngles(j,D3DXVECTOR2( fRes[j], 0 ));
			totalangle += fRes[j];
		}

		//rotating first cube
		SetAngles(0, D3DXVECTOR2(0, atan2(vecDest.x,vecDest.y)));

		//rotationg last cube
		SetAngles(4, D3DXVECTOR2(D3DX_PI-(totalangle), 0));

		return S_OK;
	}
	else
	{
		AppendReport(L"Error 1: Destination unreachable");
		return E_FAIL;
	}
	

	/*
	// non recursive - deprecated!!!
	D3DXVECTOR3 myvec3;
	if(SUCCEEDED( findpoint(vecDest,&myvec3) ))
	{
		SetAngles(1, D3DXVECTOR2( myvec3.x, 0));
		SetAngles(2, D3DXVECTOR2( myvec3.y, 0));
		SetAngles(3, D3DXVECTOR2( myvec3.z, 0));

		//rotating first cube
		SetAngles(0, D3DXVECTOR2(0, atan2(vecDest.x,vecDest.y)));

		//rotationg last cube
		SetAngles(4, D3DXVECTOR2(D3DX_PI-(myvec3.x+myvec3.y+myvec3.z), 0));
	}
	*/

	return S_OK;
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::FrameMove(float fElapsedTime, float fTime)
{
	int i;
	BOOL bReached = true;

	if(!bProcessing) return S_OK;

	for(i=0; i < numOfChains; i++)
	{
		float delta = fElapsedTime * ANGSPEED;
		
		//check x
		if(fabs(cube[i]->vAngle.x-cube[i]->rAngle.x) > 2.f*delta)
		{
			if(cube[i]->vAngle.x < cube[i]->rAngle.x) cube[i]->vAngle.x += delta;
			else cube[i]->vAngle.x -= delta;
			bReached = false;
		}
		
		//check y
		if(fabs(cube[i]->vAngle.y-cube[i]->rAngle.y) > 2.f*delta)
		{
			if(cube[i]->vAngle.y < cube[i]->rAngle.y) cube[i]->vAngle.y += delta;
			else cube[i]->vAngle.y -= delta;
			bReached = false;
		}
	}

	//event
	if(bReached && bProcessing)
	{
		AppendReport(L"0 Destination reached");
		bProcessing = false;
	}
	
	return S_OK;
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::Render(LPDIRECT3DDEVICE9 pd3dDevice)
{
	int i;

	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);
	pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
	
	field->Render(pd3dDevice);

	for(i=0; i<cube.size(); i++)
	{
		DrawChain(cube[0], pd3dDevice);
	}

	return S_OK;
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::DrawChain(cprimitive* pCube, LPDIRECT3DDEVICE9 pd3dDevice)
{
	D3DXMATRIX matRotRes;
	D3DXMatrixIdentity(&matRotRes);

	DrawChain(pCube, pd3dDevice, matRotRes, matRotRes, D3DXVECTOR3(0,0,0));
	return S_OK;
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::DrawChain(cprimitive* pCube, LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX matRot, D3DXMATRIX matRotY, D3DXVECTOR3 vecTranslate)
{
	D3DXMATRIX matRotRes, matRotYRes;
	D3DXVECTOR3 vecTransRes;
	matRotRes = matRot * pCube->GetRotationMatrix();
	matRotYRes = matRotY * pCube->GetRotationYMatrix();
	vecTransRes = vecTranslate;

	//HTMLLog("%d %d %d\n", (int)vecTransRes.x, (int)vecTransRes.y, (int)vecTransRes.z);
	//rendering
	D3DXMATRIX matTrans;
	D3DXMatrixTranslation(&matTrans, vecTransRes.x, vecTransRes.y, vecTransRes.z);
	D3DXMATRIX worldMatrix = matRotRes*matTrans*matRotYRes;
	pd3dDevice->SetTransform(D3DTS_WORLD, &worldMatrix);

	pCube->Render(pd3dDevice);
	
	if(pCube->pSiblings != NULL) DrawChain(pCube->pSiblings, pd3dDevice, matRot, matRotY, vecTranslate);
	if(pCube->pFirstChild != NULL) DrawChain(pCube->pFirstChild, pd3dDevice, matRotRes, matRotYRes, vecTransRes + pCube->GetTranslationVector(matRotRes));


	return S_OK;
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::Release()
{
	int i;

	for(i=0; i < cube.size(); i++) 
		cube[i]->Release();

	field->Release();

	return S_OK;
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::InitPredefinedPositions()
{
	int i,j;
	cprimitive* primitive;
	wchar_t *outdata = new wchar_t[512];

	// setup predefined positions
	numPredefinedPos = _wtoi( IniRead(L"./config.ini", L"predefined_positions", L"quantity") );

	//1st - straight stick
	for(i=0; i < numPredefinedPos; i++)
	{
		wchar_t string[512];
		wsprintf(string, L"predefined_%d", (i));

		//name
		outdata = new wchar_t[512];
		wsprintf(outdata, L"%s", IniRead(L"./config.ini", string, L"name"));
		predefinedPositionNames.push_back( new LPWSTR(outdata) );

		//angles
		D3DXVECTOR2* angles;
		angles = new D3DXVECTOR2[numOfChains];
		
		for(j=0; j < numOfChains; j++)
		{
			D3DXVECTOR2 vec2;
			wchar_t index[256];
			_itow(j,index,10);

			swscanf( IniRead(L"./config.ini", string, index), L"%f %f", &(vec2.x), &(vec2.y));
			angles[j] = vec2;
		}


		predefinedPositions.push_back(angles);
	}

	return S_OK;
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::PredefinedPosition(int num)
{
	int i;
	if(num >= numPredefinedPos) return E_FAIL;

	for( i = 0; i < 5; i++ )
	{
		SetAngles(i, predefinedPositions[num][i]);
	}
	return S_OK;
}