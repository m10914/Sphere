// VirMan kinematics.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	CManipulator manipulator;
	manipulator.Create(NULL,L"./test5.ini");
	HRESULT hr;
	wchar_t limbname[512];
	D3DXVECTOR3 vec = D3DXVECTOR3(0,0,0);
	int len;
	float vect_len;
	float deviation;


	D3DXVECTOR3* vResult = manipulator.GetLimbCoord(L"hand");
	printf("1 Coordinates of endpoint %f %f %f\n",vResult->x,vResult->y,vResult->z);

	for(;;)
	{
		wprintf(L"Enter limb and coordinates: ");
	//	wscanf(L"%s %f %f %f", limbname, &(vec.x), &(vec.y), &(vec.z));
		scanf("%f %f %f",&(vec.x), &(vec.y), &(vec.z));
		wcscpy(limbname,L"hand");
		int index = manipulator.GetLimbIndexByName(limbname);
		if(index == -1)
		{
			printf("No limb with this name\n");
			continue;
		}
		LIMB* limb = manipulator.limbs[index];
		printf("prepare for solving\n"); 
		hr = manipulator.InversedKinematics(vec,limbname,true);

		if(hr == S_OK)
			printf("S_OK, We get it!\n");
		if(hr == E_FAIL) 
			printf("E_FAIL, Go closer! It's really beside us!\n");
		len = manipulator.GetLimbLength(limbname);
		vect_len = sqrt( vec.x * vec.x + vec.y * vec.y + vec.z * vec.z );

//		printf("Lens: limb: %d, vect: %f.\n",len,vect_len);
		
		printf("Results are:\n");
		D3DXVECTOR2* vRes = new D3DXVECTOR2[manipulator.numOfChains];
		vRes = manipulator.GetAngles();

		D3DXVECTOR3* vResult = manipulator.GetLimbCoord(L"hand");

		D3DXVECTOR3 vDiff = vec - *vResult;
		deviation = D3DXVec3Length(&vDiff);

		printf("Coordinates of endpoint %f %f %f\ndeviation %f\n",vResult->x,vResult->y,vResult->z,deviation);
		
		for(int i=manipulator.GetChainIndexByID(limb->firstchain); i<=manipulator.GetChainIndexByID(limb->lastchain); i++)
		{
			printf("(%.2f %.2f)\n", vRes[i].x, vRes[i].y);
			
		}
	}

	/*CManipulator manipulator;
	manipulator.Create(NULL,L"./testconfig.ini");

	wchar_t limbname[512];
	D3DXVECTOR3 vec = D3DXVECTOR3(0,0,0);

	for(;;)
	{
		wprintf(L"Enter limb and coordinates: ");
		wscanf(L"%s %f %f %f", limbname, &(vec.x), &(vec.y), &(vec.z));

		int index = manipulator.GetLimbIndexByName(limbname);
		if(index == -1)
		{
			printf("No limb with this name\n");
			continue;
		}
		LIMB* limb = manipulator.limbs[index];

		if(SUCCEEDED(manipulator.InversedKinematics(vec,limbname,true)))
		{
			printf("Results are:\n");
			D3DXVECTOR2* vRes = new D3DXVECTOR2[manipulator.numOfChains];
			vRes = manipulator.GetAngles();
			for(int i=manipulator.GetChainIndexByID(limb->firstchain); i<=manipulator.GetChainIndexByID(limb->lastchain); i++)
			{
				printf("(%.2f %.2f)\n", vRes[i].x, vRes[i].y);
			}
		}
		else
		{
			printf("Destination unreachable, trying to reach\n");

			printf("Results are:\n");
			D3DXVECTOR2* vRes = new D3DXVECTOR2[manipulator.numOfChains];
			vRes = manipulator.GetAngles();
			for(int i=manipulator.GetChainIndexByID(limb->firstchain); i<=manipulator.GetChainIndexByID(limb->lastchain); i++)
			{
				printf("(%.2f %.2f)\n", vRes[i].x, vRes[i].y);
			}
		}
	}*/

	return 0;
}

