//-------------------------------------------------
// Manipulator.cpp
//
//-------------------------------------------------


#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
#include "DXUTCamera.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "resource.h"


#include "Manipulator.h"
#include "cmath"


#define ANGSPEED 0.2f

//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
CManipulator::CManipulator(void)
{
	bProcessing = false;
	selectedChain = -1;
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
HRESULT CManipulator::Create(LPDIRECT3DDEVICE9 pd3dDevice, wchar_t* file)
{
	int i;
	cprimitive* primitive;
	wchar_t *outdata = new wchar_t[512];

	//-----
	// create hierarchy from CFG
	outdata = IniRead(file, L"manipulator", L"chainsize");
	numOfChains = _wtoi(outdata);

	for( i = 0; i < numOfChains; i++ )
	{
		primitive = new cprimitive;

		wchar_t string[512];
		wsprintf(string, L"chain_%d", (i));

		float length = _wtof(IniRead(file, string, L"length"));
		D3DXVECTOR2 initangle;
		swscanf( IniRead(file, string, L"initangle"), L"%f %f", &(initangle.x), &(initangle.y));
		float width = _wtof(IniRead(file, string, L"width"));
		D3DXVECTOR2 restrAngX;
		swscanf( IniRead(file, string, L"angle_restrict_phi"), L"%f %f", &(restrAngX.x), &(restrAngX.y));
		D3DXVECTOR2 restrAngY;
		swscanf( IniRead(file, string, L"angle_restrict_theta"), L"%f %f", &(restrAngY.x), &(restrAngY.y));
		D3DXVECTOR3 lvOffset;
		swscanf( IniRead(file, string, L"offset"), L"%f %f %f", &(lvOffset.x), &(lvOffset.y), &(lvOffset.z));
		float displace = _wtof(IniRead(file, string, L"displace"));
		float direction = _wtof(IniRead(file, string, L"direction"));
		float coefficient = _wtof(IniRead(file, string, L"coefficient"));
		float mass = _wtof(IniRead(file, string, L"mass"));

		BOOL bTilt = ( _wtof(IniRead(file, string, L"tilt")) == 1 ? true : false);
		BOOL bPressure = ( _wtof(IniRead(file, string, L"pressure")) == 1 ? true : false);
		BOOL bLaser = ( _wtof(IniRead(file, string, L"laser")) == 1 ? true : false);

		cube.push_back(primitive);
		if(FAILED( cube[i]->Create(i, pd3dDevice, length, initangle, width, restrAngX, restrAngY, lvOffset, displace, direction, coefficient,mass,bTilt,bPressure,bLaser) )) HTMLLog("cube[0] failed");
	}

	//build hierarchy
	for( i = 0; i < numOfChains; i++ )
	{
		wchar_t string[512];
		wsprintf(string, L"chain_%d", (i));

		int index = _wtoi(IniRead(file, string, L"parent_index"));
		if(index == -1) continue;

		//looking for a child, parent etc
		cprimitive* ptr;
		if(cube[index]->pFirstChild == NULL)
		{
			cube[index]->pFirstChild = cube[i];
		}
		else if(cube[index]->pFirstChild->pSiblings == NULL)
		{
			cube[index]->pFirstChild->pSiblings = cube[i];
		}
		else
		{
			for(ptr = cube[index]->pFirstChild->pSiblings; ptr->pSiblings != NULL; ptr = ptr->pSiblings);
			ptr->pSiblings = cube[i];
		}


	}

	//init predefined positions
	InitPredefinedPositions(file);

	//init limbs
	InitLimbs(file);

	return S_OK;
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::SaveConfig(wchar_t* file)
{
	int i;
	wchar_t string[512];
	wchar_t name[512];
	int pid;
	cprimitive* parent;


	

	FILE* file2 = _wfopen(file,L"w+");
	if(file2 == NULL) MessageBox(0,0,0,0);
	fclose(file2);

	//chains
	swprintf(string, L"%d", numOfChains);
	IniWrite(file,L"manipulator",L"chainsize",string);


	for(i=0; i < numOfChains; i++)
	{
		swprintf(name, L"chain_%d", i);

		//coefficient
		swprintf(string, L"%.2f", cube[i]->fCoefficient);
		IniWrite(file, name, L"coefficient", string);

		//direction
		swprintf(string, L"%.2f", cube[i]->fDirection);
		IniWrite(file, name, L"direction", string);

		//displace
		swprintf(string, L"%.2f", cube[i]->fDisplace);
		IniWrite(file, name, L"displace", string);

		//length
		swprintf(string, L"%.2f", cube[i]->fLength);
		IniWrite(file, name, L"length", string);

		//width
		swprintf(string, L"%.2f", cube[i]->fWidth);
		IniWrite(file, name, L"width", string);

		//initangle
		swprintf(string, L"%.2f %.2f", (float)cube[i]->vAngle.x, (float)cube[i]->vAngle.y);
		IniWrite(file, name, L"initangle", string);

		//angle_restrict_phi
		swprintf(string, L"%.2f %.2f", cube[i]->restrictAngleX.x, cube[i]->restrictAngleX.y);
		IniWrite(file, name, L"angle_restrict_phi", string);

		//angle_restrict_theta
		swprintf(string, L"%.2f %.2f", cube[i]->restrictAngleY.x, cube[i]->restrictAngleY.y);
		IniWrite(file, name, L"angle_restrict_theta", string);

		//offset
		swprintf(string, L"%.2f %.2f %.2f", cube[i]->vOffset.x, cube[i]->vOffset.y, cube[i]->vOffset.z);
		IniWrite(file, name, L"offset", string);

		//mass
		swprintf(string, L"%.2f", cube[i]->fMass);
		IniWrite(file, name, L"mass", string);

		//booleans
		IniWrite(file, name, L"tilt", (cube[i]->bTilt ? L"1" : L"0"));
		IniWrite(file, name, L"pressure", (cube[i]->bPressure ? L"1" : L"0"));
		IniWrite(file, name, L"laser", (cube[i]->bLaser ? L"1" : L"0"));

		//parent_index
		parent = GetParent(cube[i],cube[0]);
		if(parent == NULL) pid = -1;
		else pid = parent->id;

		swprintf(string,L"%d",pid);
		IniWrite(file, name, L"parent_index", string);

		IniWrite(file, name, L"model", L"");

		
	}

	//limbs
	swprintf(string, L"%d", limbs.size());
	IniWrite(file,L"limbs",L"num",string);

	for(i=0; i<limbs.size();i++)
	{
		swprintf(name, L"limb_%d", i);
		IniWrite(file,name,L"name",limbs[i]->name);

		swprintf(string, L"%d", limbs[i]->firstchain);
		IniWrite(file,name,L"first",string);

		swprintf(string, L"%d", limbs[i]->lastchain);
		IniWrite(file,name,L"last",string);
	}


	//-------

	MessageBox(0,L"Конфигурация успешно сохранена!", L"Сохранение", 0);

	return S_OK;
}

//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::AddChain(LPDIRECT3DDEVICE9 pd3dDevice)
{
	int i;
	cprimitive* primitive;
	wchar_t *outdata = new wchar_t[512];

	primitive = new cprimitive;

	numOfChains++;

	i = cube.size();
	cube.push_back(primitive);
	cube[i]->Create(i,pd3dDevice);


	//relations
	cprimitive* ptr;

	if(cube[selectedChain]->pFirstChild == NULL)
	{
		cube[selectedChain]->pFirstChild = cube[i];	
	}
	else if(cube[selectedChain]->pFirstChild->pSiblings == NULL)
	{
		cube[selectedChain]->pFirstChild->pSiblings = cube[i];	
	}
	else
	{
		for(ptr = cube[selectedChain]->pFirstChild->pSiblings; ptr->pSiblings != NULL; ptr = ptr->pSiblings);
		ptr->pSiblings = cube[i];
	}

	return S_OK;
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc: removes selected chain and all of its' ancestors
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::RemoveChain()
{
	//vars
	cprimitive* ptr;
	cprimitive* parent;


	// LOOKING FOR A PARENT
	//------------------------

	//check hierarchy
	parent = GetParent(cube[selectedChain], cube[0]);

	if(parent == NULL) { MessageBox(0, L"Can't find parent", L"Error!", 0); return E_FAIL; };



	//cutting links
	if(parent->pFirstChild == cube[selectedChain])
	{
		parent->pFirstChild = cube[selectedChain]->pSiblings; //link destroyed
	}
	else if(parent->pFirstChild->pSiblings == cube[selectedChain])
	{
		parent->pFirstChild->pSiblings = cube[selectedChain]->pSiblings; //link destroyed
	}
	else
	{
		for(ptr=parent->pFirstChild->pSiblings; ptr->pSiblings != NULL; ptr = ptr->pSiblings)
		{
			if(ptr->pSiblings == cube[selectedChain])
			{
				ptr->pSiblings = cube[selectedChain]; //link destroyed
			}
		}
	}


	//deleting
	DestroyChain(cube[selectedChain]);

	//deselect
	selectedChain = -1;

	return S_OK;
}

void CManipulator::DestroyChain(cprimitive* current)
{
	if(current->pFirstChild != NULL) DestroyChain(current->pFirstChild);
	if(current->pSiblings != NULL) DestroyChain(current->pSiblings);

	//destroy
	numOfChains--;
	cube.erase(cube.begin()+current->id);
	current->Release();
	free(current);

	RenewIndices();
}

cprimitive* CManipulator::GetParent(cprimitive* search, cprimitive* current)
{
	cprimitive* ptr, *res;

	//looking down the hierarchy
	if(current->pFirstChild != NULL)
	{
		//looking into it's children - siblings of firstchild
		for(ptr = current->pFirstChild; ptr != NULL; ptr = ptr->pSiblings)
		{
			if(ptr == search) return current;

			res = GetParent(search, ptr);
			if(res != NULL) return res;
		}
	}
	else
	{
		//siblings
		for(ptr = current->pSiblings; ptr != NULL; ptr = ptr->pSiblings)
		{
			res = GetParent(search, ptr);
			if(res != NULL) return res;
		}
	}


	return NULL;
}

//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::RenewIndices()
{
	int i;

	for(i=0; i < numOfChains; i++) cube[i]->id = i;
	
	selectedChain = -1;
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

		
	for( q1 = cube[1]->restrictAngleX.x; q1 <= cube[1]->restrictAngleX.y; q1 += iter )
	{
		for( q2 = cube[2]->restrictAngleX.x; q2 <= cube[2]->restrictAngleX.y; q2 += iter )
		{
			for( q3 = cube[3]->restrictAngleX.x; q3 <= cube[3]->restrictAngleX.y; q3 += iter )
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



BOOL CManipulator::findpoint_recursive(int i, int fini, D3DXVECTOR3 vDiff, D3DXVECTOR2* fRes, D3DXMATRIX mTransform)
{
	D3DXMATRIX worldMatrix;
	D3DXMATRIX mTransformAncestors;
	D3DXMATRIX matTrans;
	D3DXMATRIX offsetTrans;
	D3DXMATRIX rotMatX, rotMatY;
	
	cprimitive* pCube =cube[i];

	D3DXMatrixTranslation(&offsetTrans,pCube->vOffset.x, pCube->vOffset.y, pCube->vOffset.z);
	D3DXMatrixTranslation(&matTrans, 0, pCube->fLength, 0);

	HRESULT hr;
	int j;
	float iter = 0.1f;
	float eps = 10.0f;
	float q,p;
	float x = D3DXVec3Length(&vDiff);
	float startX = cube[i]->restrictAngleX.x + (cube[i]->restrictAngleX.y-cube[i]->restrictAngleX.x)*0.5f;
	float startY = cube[i]->restrictAngleY.x + (cube[i]->restrictAngleY.y-cube[i]->restrictAngleY.x)*0.5f;

	for(q=startX; q <= cube[i]->restrictAngleX.y; q += iter)
	{
		for(p=startY; p <= cube[i]->restrictAngleY.y; p += iter)
		{

			//-------------------------------------------------------------------------
			{
				D3DXMatrixRotationX(&rotMatX,q);
				D3DXMatrixRotationY(&rotMatY,p);

				if(i==fini)
				{
					worldMatrix = matTrans * rotMatX * rotMatY * offsetTrans * mTransform;

					D3DXVECTOR3 vOut;
					D3DXVECTOR3 vIn = D3DXVECTOR3(0,0,0);
					D3DXVec3TransformCoord(&vOut,&vIn,&worldMatrix);

					if(fabs(vOut.x-vDiff.x) <= eps && fabs(vOut.y-vDiff.y) < eps && fabs(vOut.z-vDiff.z) < eps)
					{
						fRes[i] = D3DXVECTOR2(q,p);
						return true;
					}
				}
				else
				{
					mTransformAncestors =  matTrans * rotMatX * rotMatY * offsetTrans * mTransform;

					hr = findpoint_recursive(i+1,fini,vDiff,fRes,mTransformAncestors);
					if(hr == TRUE)
					{
						fRes[i] = D3DXVECTOR2(q,p);
						return hr;
					}
				}
			}
			//-------------------------------------------------------------------------

		}
		for(p=startY; p >= cube[i]->restrictAngleY.x; p -= iter)
		{
			//-------------------------------------------------------------------------
			{
				D3DXMatrixRotationX(&rotMatX,q);
				D3DXMatrixRotationY(&rotMatY,p);

				if(i==fini)
				{
					worldMatrix = matTrans * rotMatX * rotMatY * offsetTrans * mTransform;

					D3DXVECTOR3 vOut;
					D3DXVECTOR3 vIn = D3DXVECTOR3(0,0,0);
					D3DXVec3TransformCoord(&vOut,&vIn,&worldMatrix);

					if(fabs(vOut.x-vDiff.x) <= eps && fabs(vOut.y-vDiff.y) < eps && fabs(vOut.z-vDiff.z) < eps)
					{
						fRes[i] = D3DXVECTOR2(q,p);
						return true;
					}
				}
				else
				{
					mTransformAncestors =  matTrans * rotMatX * rotMatY * offsetTrans * mTransform;

					hr = findpoint_recursive(i+1,fini,vDiff,fRes,mTransformAncestors);
					if(hr == TRUE)
					{
						fRes[i] = D3DXVECTOR2(q,p);
						return hr;
					}
				}
			}
			//-------------------------------------------------------------------------
		}
	}
	for(q=startX; q >= cube[i]->restrictAngleX.x; q -= iter)
	{
		for(p=startY; p <= cube[i]->restrictAngleY.y; p += iter)
		{

			//-------------------------------------------------------------------------
			{
				D3DXMatrixRotationX(&rotMatX,q);
				D3DXMatrixRotationY(&rotMatY,p);

				if(i==fini)
				{
					worldMatrix = matTrans * rotMatX * rotMatY * offsetTrans * mTransform;

					D3DXVECTOR3 vOut;
					D3DXVECTOR3 vIn = D3DXVECTOR3(0,0,0);
					D3DXVec3TransformCoord(&vOut,&vIn,&worldMatrix);

					if(fabs(vOut.x-vDiff.x) <= eps && fabs(vOut.y-vDiff.y) < eps && fabs(vOut.z-vDiff.z) < eps)
					{
						fRes[i] = D3DXVECTOR2(q,p);
						return true;
					}
				}
				else
				{
					mTransformAncestors =  matTrans * rotMatX * rotMatY * offsetTrans * mTransform;

					hr = findpoint_recursive(i+1,fini,vDiff,fRes,mTransformAncestors);
					if(hr == TRUE)
					{
						fRes[i] = D3DXVECTOR2(q,p);
						return hr;
					}
				}
			}
			//-------------------------------------------------------------------------

		}
		for(p=startY; p >= cube[i]->restrictAngleY.x; p -= iter)
		{
			//-------------------------------------------------------------------------
			{
				D3DXMatrixRotationX(&rotMatX,q);
				D3DXMatrixRotationY(&rotMatY,p);

				if(i==fini)
				{
					worldMatrix = matTrans * rotMatX * rotMatY * offsetTrans * mTransform;

					D3DXVECTOR3 vOut;
					D3DXVECTOR3 vIn = D3DXVECTOR3(0,0,0);
					D3DXVec3TransformCoord(&vOut,&vIn,&worldMatrix);

					if(fabs(vOut.x-vDiff.x) <= eps && fabs(vOut.y-vDiff.y) < eps && fabs(vOut.z-vDiff.z) < eps)
					{
						fRes[i] = D3DXVECTOR2(q,p);
						return true;
					}
				}
				else
				{
					mTransformAncestors =  matTrans * rotMatX * rotMatY * offsetTrans * mTransform;

					hr = findpoint_recursive(i+1,fini,vDiff,fRes,mTransformAncestors);
					if(hr == TRUE)
					{
						fRes[i] = D3DXVECTOR2(q,p);
						return hr;
					}
				}
			}
			//-------------------------------------------------------------------------
		}
	}

	return FALSE;
}

//---------------------------------------------------------------------------------------------
// Name:
// Desc: deprecated - simplified 2d version
//---------------------------------------------------------------------------------------------
BOOL CManipulator::findpoint_recursive(int i, int fini, D3DXVECTOR3 vDiff, float* fRes, float vex, float vey)
{
	//method - iterative

	int j;
	float iter = 0.02f;
	float eps = 0.8f;
	float q;
	float x = D3DXVec3Length(&vDiff);
	float start = cube[i]->restrictAngleX.x + (cube[i]->restrictAngleX.y-cube[i]->restrictAngleX.x)*0.5f;

	//two searches - to optimize angles
	for( q = start; q >= cube[i]->restrictAngleX.x; q-= iter)
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
	for( q = start; q <= cube[i]->restrictAngleX.y; q+= iter)
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


//--------------------------------------------------------------------------------------------
// Name:
// Desc:
//--------------------------------------------------------------------------------------------
int CManipulator::GetLimbGradient(LPWSTR limbname, char* filename, int step, int iteration)
{
	int limbindex = GetLimbIndexByName(limbname);
	if(limbindex == -1) return NULL;

	int limbNumChains = limbs[limbindex]->lastchain - limbs[limbindex]->firstchain;
	int dimension = (int)GetLimbLength(limbname);
	float i,j,k;
	int iter, ter;
	int count;
	int steps = (int)ceil(2.f*(float)dimension/(float)step);
	float** fRes = new float*;

	if(iteration > steps*steps*steps) return -1;

	//init
	FILE* file = fopen(filename,"r+");
	fseek(file,0,SEEK_END);
	
	i = (iteration%(steps*steps))%steps * step;
	j = (int)((iteration%(steps*steps))/steps) * step;
	k = (int)(iteration/(steps*steps)) * step;

	//operation
	if( GetKinematicsAngles(limbname, D3DXVECTOR3(i,j,k), fRes) )
	{
		for(count=0;count<limbNumChains;count++)
		{	
			float pip = (*fRes)[count];
			fwrite(&pip,sizeof(float),1,file);
		}
	}
	else
	{
		float pip = 0;
		for(count=0;count<limbNumChains;count++) fwrite(&pip,sizeof(float),1,file);
	}
	free(*fRes);
	free(fRes);

	//uninit
	fclose(file);

	return 0;
}


//--------------------------------------------------------------------------------------------
// Name:
// Desc:
//--------------------------------------------------------------------------------------------
BOOL CManipulator::GetKinematicsAngles(LPWSTR limbName,D3DXVECTOR3 vecDest,float** fRes)
{
	int index = GetLimbIndexByName(limbName);
	int i;
	int numLimbChains = limbs[index]->lastchain - limbs[index]->firstchain;
	float* fResNew = new float[numLimbChains];

	*fRes = new float[numLimbChains];
	for( i=0; i < numLimbChains; i++) (*fRes)[i] = 0;

	if( findpoint_recursive(GetChainIndexByID(limbs[index]->firstchain),GetChainIndexByID(limbs[index]->lastchain),vecDest,fResNew,0,0) )
	{
		for(i=0;i<numLimbChains;i++) (*fRes)[i] = fResNew[i];
		return true;
	}
	else return false;
}

int CManipulator::GetLimbIndexByName(LPWSTR name)
{
	int i;
	
	for(i=0;i<numOfLimbs;i++)
	{
		if(!wcscmp(name,limbs[i]->name)) return i;
	}

	return -1;
}


//----------------------------------------------------------------------------
// Name:
// Desc: Warning! This function is designed only for straight manipulator, with no branches!
//----------------------------------------------------------------------------
int CManipulator::GetLimbLength(LPWSTR nam, cprimitive* nextChain, LIMB* limb)
{
	int height;
	int i;

	if(nextChain == NULL && limb == NULL)
	{
		LIMB* limb = limbs[GetLimbIndexByName(nam)];
		cprimitive* curchain = cube[GetChainIndexByID(limb->firstchain)];
		
		return GetLimbLength(nam,curchain,limb);
	}
	else
	{
		height = nextChain->fLength;
		if(nextChain->pFirstChild == NULL || nextChain->id == limb->lastchain) return height;
		else return height + GetLimbLength(nam,nextChain->pFirstChild,limb);
	}
}

int CManipulator::GetChainIndexByID(int ID)
{
	int i;
	for(i=0;i<cube.size();i++)
			if(cube[i]->id == ID) return i;
	return -1;
}


//
HRESULT CManipulator::InversedKinematics(D3DXVECTOR3 vecDesc, LPWSTR limbName)
{
	int i,j;
	LIMB* limb = limbs[GetLimbIndexByName(limbName)];
	int limbLength = limb->lastchain - limb->firstchain;
	D3DXVECTOR2* fRes = new D3DXVECTOR2[numOfChains];
	for( i=0; i < numOfChains; i++) fRes[i] = D3DXVECTOR2(0,0);
	D3DXMATRIX matEmpty;
	D3DXMatrixIdentity(&matEmpty);
	char teststr[512] = "str: \0";



	if( findpoint_recursive(GetChainIndexByID(limb->firstchain),GetChainIndexByID(limb->lastchain),vecDesc,fRes,matEmpty) )
	{
		/*for(j=0;j<numOfChains;j++)
		{
			sprintf(teststr,"%s, (%.2f; %.2f)",teststr,fRes[j].x, fRes[j].y);
		}*/

		for(j=GetChainIndexByID(limb->firstchain); j <= GetChainIndexByID(limb->lastchain); j++)
		{
			SetAngles(j,fRes[j],true);
		}
	}
	else
	{
		AppendReport(L"Error 1: Destination unreachable");
		return E_FAIL;
	}

	free(fRes);

	return S_OK;
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc: deprecated - for single test manipulator
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
	
	
	for(i=0; i<cube.size(); i++)
	{
		DrawChain(cube[0], pd3dDevice);
	}

	//alteernate version of render
	
	/*
	for(i=0; i<numOfChains; i++)
	{
		D3DXMATRIX* mat = GetChainMatrix(i);

		pd3dDevice->SetTransform(D3DTS_WORLD, mat);

		LPD3DXMESH pMesh = cube[i]->GetMesh(pd3dDevice);//Render(pd3dDevice);

		if(pMesh != NULL)
		{
			pMesh->DrawSubset(0);
		}

		free(mat);
	}
	*/
	

	return S_OK;
}




//---------------------------------------------------------------------------------------------
// Name: CookMeshes chain
// Desc:
//---------------------------------------------------------------------------------------------
D3DXVECTOR3* CManipulator::GetLimbCoord(LPWSTR limbname)
{
	D3DXMATRIX matRotRes;
	D3DXMatrixIdentity(&matRotRes);

	LIMB* curlimb = limbs[GetLimbIndexByName(limbname)];
	D3DXVECTOR3* retMat = GetLimbCoord(curlimb->lastchain, cube[curlimb->firstchain], matRotRes);

	return retMat;
}
D3DXVECTOR3* CManipulator::GetLimbCoord(int iID, cprimitive* pCube, D3DXMATRIX mTransform)
{
	D3DXVECTOR3* retVec;

	D3DXMATRIX worldMatrix;
	D3DXMATRIX mTransformAncestors;
	D3DXMATRIX matTrans;
	D3DXMATRIX offsetTrans;

	//cooking current matrix
	D3DXMatrixTranslation(&offsetTrans,pCube->vOffset.x, pCube->vOffset.y, pCube->vOffset.z);
	worldMatrix = pCube->GetRotationXMatrix() * pCube->GetRotationYMatrix() * offsetTrans;
	worldMatrix = worldMatrix * mTransform;

	//cooking ancestor matrices
	D3DXMatrixTranslation(&matTrans, 0, pCube->fLength, 0);
	mTransformAncestors =  matTrans * pCube->GetRotationXMatrix() * pCube->GetRotationYMatrix() * offsetTrans * mTransform;


	if(pCube->id == iID)
	{	
		D3DXVECTOR3 *vec3, nvec=D3DXVECTOR3(0,0,0);
		vec3 = new D3DXVECTOR3;
		D3DXVec3TransformCoord(vec3,&nvec,&mTransformAncestors);
		return vec3;
	}

	if(pCube->pSiblings != NULL)
	{
		retVec = GetLimbCoord(iID, pCube->pSiblings, mTransform);
		if(retVec != NULL) return retVec;
	}

	if(pCube->pFirstChild != NULL)
	{
		retVec = GetLimbCoord(iID, pCube->pFirstChild, mTransformAncestors);
		if(retVec != NULL) return retVec;
	}
	
	return NULL;
}



D3DXMATRIX* CManipulator::GetChainMatrix(int iID)
{
	D3DXMATRIX matRotRes;
	D3DXMatrixIdentity(&matRotRes);

	D3DXMATRIX* retMat = GetChainMatrix(iID, cube[0], matRotRes);

	return retMat;
}

D3DXMATRIX* CManipulator::GetChainMatrix(int iID, cprimitive* pCube, D3DXMATRIX mTransform)
{
	D3DXMATRIX* retMat;

	D3DXMATRIX worldMatrix;
	D3DXMATRIX mTransformAncestors;
	D3DXMATRIX matTrans;
	D3DXMATRIX offsetTrans;

	//cooking current matrix
	D3DXMatrixTranslation(&offsetTrans,pCube->vOffset.x, pCube->vOffset.y, pCube->vOffset.z);
	worldMatrix = pCube->GetRotationXMatrix() * pCube->GetRotationYMatrix() * offsetTrans;
	worldMatrix = worldMatrix * mTransform;

	//cooking ancestor matrices
	D3DXMatrixTranslation(&matTrans, 0, pCube->fLength, 0);
	mTransformAncestors =  matTrans * pCube->GetRotationXMatrix() * pCube->GetRotationYMatrix() * offsetTrans * mTransform;


	if(pCube->id == iID)
	{	
		retMat = new D3DXMATRIX(worldMatrix);
		return retMat;
	}

	if(pCube->pSiblings != NULL)
	{
		retMat = GetChainMatrix(iID, pCube->pSiblings, mTransform);
		if(retMat != NULL) return retMat;
	}

	if(pCube->pFirstChild != NULL)
	{
		retMat = GetChainMatrix(iID, pCube->pFirstChild, mTransformAncestors);
		if(retMat != NULL) return retMat;
	}
	
	return NULL;
}

//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::DrawChain(cprimitive* pCube, LPDIRECT3DDEVICE9 pd3dDevice)
{
	D3DXMATRIX matRotRes;
	D3DXMatrixIdentity(&matRotRes);

	DrawChain(pCube, pd3dDevice, matRotRes);//, matRotRes, D3DXVECTOR3(0,0,0));
	return S_OK;
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::DrawChain(cprimitive* pCube, LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX mTransform)
{

	D3DXMATRIX worldMatrix;
	D3DXMATRIX mTransformAncestors;
	D3DXMATRIX matTrans;
	D3DXMATRIX offsetTrans;

	//cooking matrix for this primitive
	D3DXMatrixTranslation(&offsetTrans,pCube->vOffset.x, pCube->vOffset.y, pCube->vOffset.z);
	worldMatrix = pCube->GetRotationXMatrix() * pCube->GetRotationYMatrix() *offsetTrans;
	worldMatrix = worldMatrix * mTransform;

	//cooking matrices for ancestors
	D3DXMatrixTranslation(&matTrans, 0, pCube->fLength, 0);
	mTransformAncestors =  matTrans * pCube->GetRotationXMatrix() * pCube->GetRotationYMatrix() * offsetTrans * mTransform;

	pd3dDevice->SetTransform(D3DTS_WORLD, &worldMatrix);

	if(pCube->id == selectedChain)
		pCube->Render(pd3dDevice, true);
	else
		pCube->Render(pd3dDevice);


	if(pCube->pSiblings != NULL) DrawChain(pCube->pSiblings, pd3dDevice, mTransform);
	if(pCube->pFirstChild != NULL) DrawChain(pCube->pFirstChild, pd3dDevice, mTransformAncestors);

	return S_OK;
}


//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::Release()
{
	int i;

	//limbs
	for(i=0; i<limbs.size(); i++)
	{
		delete [] limbs[i]->name;
		delete limbs[i];
	}
	numOfLimbs = 0;
	limbs.clear();

	//chains
	for(i=0; i < cube.size(); i++)
	{
		cube[i]->Release();
		free(cube[i]);
	}
	numOfChains = 0;
	cube.clear();

	//TODO: predifined positions

	selectedChain = -1;

	return S_OK;
}




//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::InitLimbs(wchar_t* file)
{
	int i,j;
	cprimitive* primitive;
	wchar_t *outdata;

	// setup predefined positions
	numOfLimbs = _wtoi( IniRead(file, L"limbs", L"num") );

	//1st - straight stick
	for(i=0; i < numOfLimbs; i++)
	{
		wchar_t string[512];
		wsprintf(string, L"limb_%d", (i));

		LIMB* limb = new LIMB();

		//name
		outdata = new wchar_t[512];
		wsprintf(outdata, L"%s", IniRead(file, string, L"name"));
		limb->name = outdata;
		limb->firstchain = _wtoi( IniRead(file,string,L"first"));
		limb->lastchain = _wtoi( IniRead(file,string,L"last"));

		limbs.push_back(limb);
	}

	return S_OK;
}

HRESULT CManipulator::AddLimb(LPWSTR name, int iFirst, int iLast)
{
	LIMB* limb = new LIMB;
	limb->name =  new wchar_t[256];
	wcscpy(limb->name,name);
	limb->firstchain = iFirst;
	limb->lastchain = iLast;

	numOfLimbs++;

	limbs.push_back(limb);

	return S_OK;
}

HRESULT CManipulator::RemoveLimb(LPWSTR name)
{
	int i;

	for(i=0; i<limbs.size(); i++)
	{
		if(!wcscmp(limbs[i]->name,name))
		{
			delete [] limbs[i]->name;
			delete limbs[i];
			limbs.erase(limbs.begin()+i);
			numOfLimbs--;
			break;
		}
	}

	return S_OK;
}




//---------------------------------------------------------------------------------------------
// Name:
// Desc:
//---------------------------------------------------------------------------------------------
HRESULT CManipulator::InitPredefinedPositions(wchar_t* file)
{
	int i,j;
	cprimitive* primitive;
	wchar_t *outdata = new wchar_t[512];

	// setup predefined positions
	numPredefinedPos = _wtoi( IniRead(file, L"predefined_positions", L"quantity") );

	//1st - straight stick
	for(i=0; i < numPredefinedPos; i++)
	{
		wchar_t string[512];
		wsprintf(string, L"predefined_%d", (i));

		//name
		outdata = new wchar_t[512];
		wsprintf(outdata, L"%s", IniRead(file, string, L"name"));
		predefinedPositionNames.push_back( new LPWSTR(outdata) );

		//angles
		D3DXVECTOR2* angles;
		angles = new D3DXVECTOR2[numOfChains];
		
		for(j=0; j < numOfChains; j++)
		{
			D3DXVECTOR2 vec2;
			wchar_t index[256];
			_itow(j,index,10);

			swscanf( IniRead(file, string, index), L"%f %f", &(vec2.x), &(vec2.y));
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