//--------------------------------------------------------------------------------------
// File: SimpleSample.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
#include "DXUTCamera.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "resource.h"

#include "LH_strings_VM.h"
#include "log.h"
#include "Manipulator.h"
#include "Camera.h"
#include <vector>
using namespace std;


//#define DEBUG_VS   // Uncomment this line to debug D3D9 vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug D3D9 pixel shaders 


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg             g_SettingsDlg;          // Device settings dialog
CDXUTDialog                 g_HUD;                  // dialog for standard controls
CDXUTDialog                 g_SampleUI;             // dialog for sample specific controls
IDirect3DDevice9*			g_pd3dDevice;
HINSTANCE*					g_pHInstance;


//-------------
// APP variables
//-------------

CManipulator*				manipulator;
CCamera*					camera;

D3DRECT						clearRect;

int							currentMenu = 0;
bool						bMouseRotateX = false;
bool						bMouseRotateY = false;

//for gradient calculus
int							iGradientStep = -1;
int							iGradientUserStep = 1;
wchar_t					wcGradientLimbName[512];
char						cGradientFilename[512];

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------


#define IDC_TOGGLEFULLSCREEN    1
#define IDC_LOADFILE 25
#define IDC_LOADFILE_BTN 26
#define IDC_SAVEFILE 27
#define IDC_SAVEFILE_BTN 28
#define IDC_NEWFILE 29
#define IDC_NEWFILE_BTN 30

#define IDC_CMD 45

//--------------------
// M E N U   1

//header
#define IDC_OBJNAME 1
#define IDC_DESCRIPTION 2

//tab1
#define IDC_TAB1NAME 3
#define IDC_STATIC1 6
#define IDC_STATIC2 7
#define IDC_STATIC3 8
#define IDC_STATIC4 9
#define IDC_STATIC5 10
#define IDC_STATIC6 11
#define IDC_STATIC7 12
#define IDC_STATIC8 13
#define IDC_STATIC9 14

#define IDC_WIDTH 15
#define IDC_LENGTH 16
#define IDC_ANGLE_RESTRICT_XL 17
#define IDC_ANGLE_RESTRICT_XU 18
#define IDC_ANGLE_RESTRICT_YL 19
#define IDC_ANGLE_RESTRICT_YU 20
#define IDC_WIDTH_SLIDER 21
#define IDC_LENGTH_SLIDER 22

#define IDC_ADDCHAIN 23
#define IDC_REMOVECHAIN 24

#define IDC_OFFSET_X 31
#define IDC_OFFSET_Y 32
#define IDC_OFFSET_Z 33

#define IDC_ANGLE_RESTRICT_XL_BTN 34
#define IDC_ANGLE_RESTRICT_XU_BTN 35
#define IDC_ANGLE_RESTRICT_YL_BTN 36
#define IDC_ANGLE_RESTRICT_YU_BTN 37

#define IDC_DISPLACE 38
#define IDC_DIRECTION 39
#define IDC_COEFFICIENT 40
#define IDC_MASS 41
#define IDC_TILT 42
#define IDC_PRESSURE 43
#define IDC_LASER 44

//tab2
#define IDC_TAB2NAME 4

//tab3
#define IDC_TAB3NAME 5

//---
//--------------------



//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK MouseProc(  bool bLeftButtonDown ,bool bRightButtonDown, bool bMiddleButtonDown, bool bSideButton1Down, 
						  bool bSideButton2Down, INT nMouseWheelDelta, INT xPos, INT yPos, void* pUserContext );
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );

bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
                                      bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext );
HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                    void* pUserContext );
void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
void CALLBACK OnD3D9LostDevice( void* pUserContext );
void CALLBACK OnD3D9DestroyDevice( void* pUserContext );
void InitApp();
HRESULT Pick();
HRESULT OnChainSelect(int iID);
HRESULT CMDProcess(LPCWSTR cmd);







//-------------------------------------------------------------------------------------------
//--       F U N C T I O N S
//-------------------------------------------------------------------------------------------


HRESULT CMDProcess(LPCWSTR cmd)
{
	char command[512];
	WideCharToMultiByte(CP_ACP,0,cmd, 512 ,command, 512, NULL, NULL);

	vector<char*>* mas = explode(" ",command);

	//limb functions
	if(!strcmp(mas->at(0),"addlimb") && mas->size()==4)
	{
		wchar_t str[512];
		MultiByteToWideChar(CP_ACP,0,mas->at(1),512,str,512);
		manipulator->AddLimb(str,atoi(mas->at(2)),atoi(mas->at(3)));
	}
	else if(!strcmp(mas->at(0),"removelimb") && mas->size()==2)
	{
		wchar_t str[512];
		MultiByteToWideChar(CP_ACP,0,mas->at(1),512,str,512);
		manipulator->RemoveLimb(str);
	}
	else if(!strcmp(mas->at(0),"listlimb") && mas->size()==1)
	{
		
		if( manipulator->limbs.size() == 0)
		{
			MessageBox(0,L"there are currently no limbs defined",L"Limbs List", 0);
		}
		else
		{
			wchar_t str[5000] = L"\0";
			memset(str,0,sizeof(str));

			for(int i=0; i < manipulator->limbs.size(); i++)
			{
				D3DXVECTOR3* limbcoord = manipulator->GetLimbCoord(manipulator->limbs[i]->name);
				if(limbcoord != NULL)
					wsprintf(str,L"%s\n%s: (%d; %d) (%d;%d;%d)",str,manipulator->limbs[i]->name,manipulator->limbs[i]->firstchain,manipulator->limbs[i]->lastchain, (int)limbcoord->x, (int)limbcoord->y, (int)limbcoord->z);
				else
				{
					wsprintf(str,L"%s\n%s: (%d; %d)",str,manipulator->limbs[i]->name,manipulator->limbs[i]->firstchain,manipulator->limbs[i]->lastchain);
					delete limbcoord;
				}
				
			}
			MessageBox(0,str,L"Limbs List",0);
		}
	}

	//kinematics
	else if(!strcmp(mas->at(0),"kin") && mas->size()==3)
	{
		float** fRes = NULL;
		int i,j;

		strcpy(cGradientFilename, mas->at(2));
		MultiByteToWideChar(CP_ACP,0,mas->at(2),512,wcGradientLimbName,512);
		
		int dimension = (int)ceil(2.f*manipulator->GetLimbLength(wcGradientLimbName)/iGradientUserStep);
		LIMB* lmb = manipulator->limbs[manipulator->GetLimbIndexByName(wcGradientLimbName)];
		int numOfChains = lmb->lastchain - lmb->firstchain;

		sprintf(cGradientFilename, "%s_%d_%d.txt",cGradientFilename,dimension,numOfChains);

		FILE* file = fopen(cGradientFilename,"w+");
		fclose(file);

		iGradientUserStep = atoi(mas->at(1));
		iGradientStep = 0;
	}
	else if(!strcmp(mas->at(0),"mt") && mas->size()==5)
	{
		wchar_t str[512];
		MultiByteToWideChar(CP_ACP,0,mas->at(1),512,str,512);
		D3DXVECTOR3 toVec = D3DXVECTOR3(atof(mas->at(2)),atof(mas->at(3)),atof(mas->at(4)));

		manipulator->InversedKinematics(toVec, str);
	}

	//default
	else
	{
		MessageBox(0,L"unknown command",cmd,0);
	}

	for(int i=0;i<mas->size();i++) free(mas->at(i));
	mas->clear();
	delete mas;
	
	return S_OK;
}

int GradientCalculus()
{
	int res = manipulator->GetLimbGradient(wcGradientLimbName,cGradientFilename,iGradientUserStep,iGradientStep);

	if(res == -1)
	{
		iGradientStep == -1;
		g_HUD.GetStatic(IDC_STATIC7)->SetText(L"Gradients processed");
	}
	else
	{
		iGradientStep++;

		int dimension = (int)ceil(2.f*manipulator->GetLimbLength(wcGradientLimbName)/iGradientUserStep);
		dimension = dimension*dimension*dimension;

		wchar_t str[128];
		swprintf(str,L"Gradient processed: %d of %d", iGradientStep, dimension);
		g_HUD.GetStatic(IDC_STATIC7)->SetText(str);
	}
	

	return 0;
}




//------------------------------------------------------------------------
// Name: OnChainSelect
// Desc: event handler for select one of chains
//------------------------------------------------------------------------
HRESULT OnChainSelect(int iID)
{
	if(iID >= 0)
	{
		wchar_t str[256];
		cprimitive* chain = manipulator->cube[iID];
		
		g_SampleUI.SetVisible(true);

		swprintf(str,L"You picked chain # %d",iID);
		g_SampleUI.GetStatic( IDC_OBJNAME )->SetText(str);

		g_SampleUI.GetSlider(IDC_WIDTH_SLIDER)->SetValue(chain->fWidth);

		swprintf(str,L"%.2f",chain->fWidth);
		g_SampleUI.GetEditBox( IDC_WIDTH )->SetText(str);

		g_SampleUI.GetSlider(IDC_LENGTH_SLIDER)->SetValue(chain->fLength);

		swprintf(str,L"%.2f",chain->fLength);
		g_SampleUI.GetEditBox( IDC_LENGTH )->SetText(str);

		swprintf(str,L"%.2f",chain->restrictAngleX.x);
		g_SampleUI.GetEditBox( IDC_ANGLE_RESTRICT_XL )->SetText(str);
		swprintf(str,L"%.2f",chain->restrictAngleX.y);
		g_SampleUI.GetEditBox( IDC_ANGLE_RESTRICT_XU )->SetText(str);
		swprintf(str,L"%.2f",chain->restrictAngleY.x);
		g_SampleUI.GetEditBox( IDC_ANGLE_RESTRICT_YL )->SetText(str);
		swprintf(str,L"%.2f",chain->restrictAngleY.y);
		g_SampleUI.GetEditBox( IDC_ANGLE_RESTRICT_YU )->SetText(str);

		swprintf(str,L"%.2f",chain->vOffset.x);
		g_SampleUI.GetEditBox( IDC_OFFSET_X )->SetText(str);
		swprintf(str,L"%.2f",chain->vOffset.y);
		g_SampleUI.GetEditBox( IDC_OFFSET_Y )->SetText(str);
		swprintf(str,L"%.2f",chain->vOffset.z);
		g_SampleUI.GetEditBox( IDC_OFFSET_Z )->SetText(str);

		swprintf(str,L"%.2f",chain->fDisplace);
		g_SampleUI.GetEditBox( IDC_DISPLACE )->SetText(str);
		swprintf(str,L"%.2f",chain->fDirection);
		g_SampleUI.GetEditBox( IDC_DIRECTION )->SetText(str);
		swprintf(str,L"%.2f",chain->fCoefficient);
		g_SampleUI.GetEditBox( IDC_COEFFICIENT )->SetText(str);
		swprintf(str,L"%.2f",chain->fMass);
		g_SampleUI.GetEditBox( IDC_MASS )->SetText(str);

		g_SampleUI.GetCheckBox( IDC_TILT )->SetChecked(chain->bTilt);
		g_SampleUI.GetCheckBox( IDC_PRESSURE )->SetChecked(chain->bPressure);
		g_SampleUI.GetCheckBox( IDC_LASER )->SetChecked(chain->bLaser);


		swprintf(str,L"X: %.2f",chain->vAngle.x);
		g_SampleUI.GetStatic( IDC_STATIC7 )->SetText(str);
		swprintf(str,L"Y: %.2f",chain->vAngle.y);
		g_SampleUI.GetStatic( IDC_STATIC8 )->SetText(str);
	}
	else
	{
		g_SampleUI.SetVisible(false);
	}

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	camera = new CCamera;
	manipulator = new CManipulator;


    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );



	//g_HUD
    g_HUD.SetCallback( OnGUIEvent );

    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 5, 10, 125, 22, VK_F12 );

	//fonts
	g_HUD.SetFont( 1, L"Arial", 20, FW_BOLD );
	g_HUD.SetFont( 2, L"Arial", 16, FW_NORMAL );

	g_HUD.AddEditBox( IDC_NEWFILE, L"testconfig.ini", 150, 10, 120, 30 );
	g_HUD.AddButton( IDC_NEWFILE_BTN, L"New", 280, 15, 60, 20);
	g_HUD.AddEditBox( IDC_LOADFILE, L"testconfig.ini", 150, 40, 120, 30 );
	g_HUD.AddButton( IDC_LOADFILE_BTN, L"Load", 280, 45, 60, 20);
	g_HUD.AddEditBox( IDC_SAVEFILE, L"testconfig.ini", 150, 70, 120, 30 );
	g_HUD.AddButton( IDC_SAVEFILE_BTN, L"Save", 280, 75, 60, 20);

	g_HUD.AddEditBox( IDC_CMD, L"", 5, 5, 300, 30 );
	g_HUD.AddStatic( IDC_STATIC7, L"status", 5, 5, 300, 30 );
	g_HUD.GetStatic( IDC_STATIC7 )->GetElement( 0 )->dwTextFormat = DT_LEFT;
	g_HUD.GetStatic( IDC_STATIC7 )->SetTextColor( D3DCOLOR_ARGB( 255, 220, 220, 220 ) );
	g_HUD.GetStatic( IDC_STATIC7 )->GetElement(0)->iFont = 2;

	//g_SampleUI
    g_SampleUI.SetCallback( OnGUIEvent ); int iY = 10;
	g_SampleUI.SetVisible(false);
	
	//fonts
	g_SampleUI.SetFont( 1, L"Arial", 20, FW_BOLD );
	g_SampleUI.SetFont( 2, L"Arial", 16, FW_NORMAL );
	
	g_SampleUI.AddStatic( IDC_OBJNAME, L"Pick object", 0, 0, 200, 30 );
	g_SampleUI.GetStatic( IDC_OBJNAME )->GetElement( 0 )->dwTextFormat = DT_LEFT;
	g_SampleUI.GetStatic( IDC_OBJNAME )->SetTextColor( D3DCOLOR_ARGB( 255, 220, 220, 220 ) );
	g_SampleUI.GetStatic( IDC_OBJNAME )->GetElement(0)->iFont = 1;

	g_SampleUI.AddStatic( IDC_STATIC7, L"angleX:",210,0,50,30 );
	g_SampleUI.GetStatic( IDC_STATIC7 )->GetElement( 0 )->dwTextFormat = DT_LEFT;
	g_SampleUI.GetStatic( IDC_STATIC7 )->SetTextColor( D3DCOLOR_ARGB( 255, 220, 220, 220 ) );
	g_SampleUI.GetStatic( IDC_STATIC7 )->GetElement(0)->iFont = 2;

	g_SampleUI.AddStatic( IDC_STATIC8, L"angleY:",260,0,50,30 );
	g_SampleUI.GetStatic( IDC_STATIC8 )->GetElement( 0 )->dwTextFormat = DT_LEFT;
	g_SampleUI.GetStatic( IDC_STATIC8 )->SetTextColor( D3DCOLOR_ARGB( 255, 220, 220, 220 ) );
	g_SampleUI.GetStatic( IDC_STATIC8 )->GetElement(0)->iFont = 2;

	//width edit box
	g_SampleUI.AddStatic( IDC_STATIC1, L"Width", 0, 50, 200, 30 );
	g_SampleUI.GetStatic( IDC_STATIC1 )->GetElement( 0 )->dwTextFormat = DT_LEFT;
	g_SampleUI.GetStatic( IDC_STATIC1 )->SetTextColor( D3DCOLOR_ARGB( 255, 220, 220, 220 ) );
	g_SampleUI.GetStatic( IDC_STATIC1 )->GetElement(0)->iFont = 2;
    g_SampleUI.AddEditBox( IDC_WIDTH, L"", 70, 45, 120, 30 );
	g_SampleUI.AddSlider( IDC_WIDTH_SLIDER, 200, 50, 150, 24, 1, 75, 20, false );

	//length edit box
	g_SampleUI.AddStatic( IDC_STATIC2, L"Length", 0, 80, 200, 30 );
	g_SampleUI.GetStatic( IDC_STATIC2 )->GetElement( 0 )->dwTextFormat = DT_LEFT;
	g_SampleUI.GetStatic( IDC_STATIC2 )->SetTextColor( D3DCOLOR_ARGB( 255, 220, 220, 220 ) );
	g_SampleUI.GetStatic( IDC_STATIC2 )->GetElement(0)->iFont = 2;
    g_SampleUI.AddEditBox( IDC_LENGTH, L"", 70, 75, 120, 30 );
	g_SampleUI.AddSlider( IDC_LENGTH_SLIDER, 200, 80, 150, 24, 1, 400, 100, false );

	//angle restrictions
	g_SampleUI.AddStatic( IDC_STATIC3, L"Angles restrictions", 0, 110, 200, 30 );
	g_SampleUI.GetStatic( IDC_STATIC3 )->GetElement( 0 )->dwTextFormat = DT_LEFT | DT_TOP;
	g_SampleUI.GetStatic( IDC_STATIC3 )->SetTextColor( D3DCOLOR_ARGB( 255, 220, 220, 220 ) );
	g_SampleUI.GetStatic( IDC_STATIC3 )->GetElement(0)->iFont = 2;
	g_SampleUI.AddStatic( IDC_STATIC4, L"X", 20, 130, 200, 30 );
	g_SampleUI.GetStatic( IDC_STATIC4 )->GetElement( 0 )->dwTextFormat = DT_LEFT | DT_TOP;
	g_SampleUI.GetStatic( IDC_STATIC4 )->SetTextColor( D3DCOLOR_ARGB( 255, 220, 220, 220 ) );
	g_SampleUI.GetStatic( IDC_STATIC4 )->GetElement(0)->iFont = 2;
	g_SampleUI.AddStatic( IDC_STATIC5, L"Y", 20, 160, 200, 30 );
	g_SampleUI.GetStatic( IDC_STATIC5 )->GetElement( 0 )->dwTextFormat = DT_LEFT | DT_TOP;
	g_SampleUI.GetStatic( IDC_STATIC5 )->SetTextColor( D3DCOLOR_ARGB( 255, 220, 220, 220 ) );
	g_SampleUI.GetStatic( IDC_STATIC5 )->GetElement(0)->iFont = 2;
	g_SampleUI.AddEditBox( IDC_ANGLE_RESTRICT_XL, L"", 70, 125, 100, 30 );
	g_SampleUI.AddButton( IDC_ANGLE_RESTRICT_XL_BTN, L"A", 170, 130, 22, 22 );
	g_SampleUI.AddEditBox( IDC_ANGLE_RESTRICT_XU, L"", 220, 125, 100, 30 );
	g_SampleUI.AddButton( IDC_ANGLE_RESTRICT_XU_BTN, L"A", 320, 130, 22, 22 );
	g_SampleUI.AddEditBox( IDC_ANGLE_RESTRICT_YL, L"", 70, 155, 100, 30 );
	g_SampleUI.AddButton( IDC_ANGLE_RESTRICT_YL_BTN, L"A", 170, 160, 22, 22 );
	g_SampleUI.AddEditBox( IDC_ANGLE_RESTRICT_YU, L"", 220, 155, 100, 30 );
	g_SampleUI.AddButton( IDC_ANGLE_RESTRICT_YU_BTN, L"A", 320, 160, 22, 22 );


	//add button
	g_SampleUI.AddButton( IDC_ADDCHAIN, L"Add", 15, 200, 100, 22 );
	g_SampleUI.AddButton( IDC_REMOVECHAIN, L"Remove", 115, 200, 100, 22 );

	//offset
	g_SampleUI.AddStatic( IDC_STATIC6, L"Offset", 0, 230, 70, 30 );
	g_SampleUI.GetStatic( IDC_STATIC6 )->GetElement( 0 )->dwTextFormat = DT_LEFT;
	g_SampleUI.GetStatic( IDC_STATIC6 )->SetTextColor( D3DCOLOR_ARGB( 255, 220, 220, 220 ) );
	g_SampleUI.GetStatic( IDC_STATIC6 )->GetElement(0)->iFont = 2;
	g_SampleUI.AddEditBox( IDC_OFFSET_X, L"", 70, 225, 80, 30 );
	g_SampleUI.AddEditBox( IDC_OFFSET_Y, L"", 160, 225, 80, 30 );
	g_SampleUI.AddEditBox( IDC_OFFSET_Z, L"", 250, 225, 80, 30 );

	//physical stuff
	g_SampleUI.AddStatic( IDC_STATIC6, L"Displace", 0, 265, 70, 30 );
	g_SampleUI.GetStatic( IDC_STATIC6 )->GetElement( 0 )->dwTextFormat = DT_LEFT;
	g_SampleUI.GetStatic( IDC_STATIC6 )->SetTextColor( D3DCOLOR_ARGB( 255, 220, 220, 220 ) );
	g_SampleUI.GetStatic( IDC_STATIC6 )->GetElement(0)->iFont = 2;
	g_SampleUI.AddEditBox(IDC_DISPLACE, L"", 70, 265, 80, 30);

	g_SampleUI.AddStatic( IDC_STATIC6, L"Direction", 0, 305, 70, 30 );
	g_SampleUI.GetStatic( IDC_STATIC6 )->GetElement( 0 )->dwTextFormat = DT_LEFT;
	g_SampleUI.GetStatic( IDC_STATIC6 )->SetTextColor( D3DCOLOR_ARGB( 255, 220, 220, 220 ) );
	g_SampleUI.GetStatic( IDC_STATIC6 )->GetElement(0)->iFont = 2;
	g_SampleUI.AddEditBox(IDC_DIRECTION, L"", 70, 305, 80, 30);

	g_SampleUI.AddStatic( IDC_STATIC6, L"Coefficient", 0, 345, 70, 30 );
	g_SampleUI.GetStatic( IDC_STATIC6 )->GetElement( 0 )->dwTextFormat = DT_LEFT;
	g_SampleUI.GetStatic( IDC_STATIC6 )->SetTextColor( D3DCOLOR_ARGB( 255, 220, 220, 220 ) );
	g_SampleUI.GetStatic( IDC_STATIC6 )->GetElement(0)->iFont = 2;
	g_SampleUI.AddEditBox(IDC_COEFFICIENT, L"", 70, 345, 80, 30);

	g_SampleUI.AddStatic( IDC_STATIC6, L"Mass", 0, 385, 70, 30 );
	g_SampleUI.GetStatic( IDC_STATIC6 )->GetElement( 0 )->dwTextFormat = DT_LEFT;
	g_SampleUI.GetStatic( IDC_STATIC6 )->SetTextColor( D3DCOLOR_ARGB( 255, 220, 220, 220 ) );
	g_SampleUI.GetStatic( IDC_STATIC6 )->GetElement(0)->iFont = 2;
	g_SampleUI.AddEditBox(IDC_MASS, L"", 70, 385, 80, 30);

	g_SampleUI.AddStatic( IDC_STATIC6, L"Tilt", 0, 425, 70, 30 );
	g_SampleUI.GetStatic( IDC_STATIC6 )->GetElement( 0 )->dwTextFormat = DT_LEFT;
	g_SampleUI.GetStatic( IDC_STATIC6 )->SetTextColor( D3DCOLOR_ARGB( 255, 220, 220, 220 ) );
	g_SampleUI.GetStatic( IDC_STATIC6 )->GetElement(0)->iFont = 2;
	g_SampleUI.AddCheckBox( IDC_TILT, L"", 60, 425, 30, 30 );

	g_SampleUI.AddStatic( IDC_STATIC6, L"Pressure", 100, 425, 70, 30 );
	g_SampleUI.GetStatic( IDC_STATIC6 )->GetElement( 0 )->dwTextFormat = DT_LEFT;
	g_SampleUI.GetStatic( IDC_STATIC6 )->SetTextColor( D3DCOLOR_ARGB( 255, 220, 220, 220 ) );
	g_SampleUI.GetStatic( IDC_STATIC6 )->GetElement(0)->iFont = 2;
	g_SampleUI.AddCheckBox( IDC_PRESSURE, L"", 160, 425, 30, 30 );

	g_SampleUI.AddStatic( IDC_STATIC6, L"Laser", 200, 425, 70, 30 );
	g_SampleUI.GetStatic( IDC_STATIC6 )->GetElement( 0 )->dwTextFormat = DT_LEFT;
	g_SampleUI.GetStatic( IDC_STATIC6 )->SetTextColor( D3DCOLOR_ARGB( 255, 220, 220, 220 ) );
	g_SampleUI.GetStatic( IDC_STATIC6 )->GetElement(0)->iFont = 2;
	g_SampleUI.AddCheckBox( IDC_LASER, L"", 260, 425, 30, 30 );
}



//--------------------------------------------------------------------------------------
// Create any D3D9 resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext )
{
    HRESULT hr;

	g_pd3dDevice = pd3dDevice;

	//init manipulator
	//manipulator->Create(pd3dDevice, L"./testconfig.ini");
	

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
    V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );


    // Setup the camera's view parameters
	camera->Setup(D3DXVECTOR3(-602,638,396), D3DXVECTOR3(-528,590,349), D3DXVECTOR2(-1,0.5));



    return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{

	manipulator->FrameMove(fElapsedTime, fTime);

	if(iGradientStep != -1) GradientCalculus();

	wchar_t* report = manipulator->GetReport();
	if(report != NULL)
	{
		g_HUD.GetStatic(IDC_STATIC7)->SetText(report);
	}

	Pick();
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;
    D3DXMATRIXA16 mWorld;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProj;
    D3DXMATRIXA16 mWorldViewProjection;


    // Clear the render target and the zbuffer 
    V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0, 120, 120, 120 ), 1.0f, 0 ) );

    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
		
		//world transformations
		pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		pd3dDevice->SetTransform( D3DTS_PROJECTION, &camera->getProjMatrix() );
		pd3dDevice->SetTransform( D3DTS_VIEW, &camera->getViewMatrix() );



		//render manipulator
		manipulator->Render(pd3dDevice);
        

		//MENU AND HUD
		D3DRECT stripe = D3DRECT(clearRect);
		stripe.x1 = stripe.x1-2;

		V( pd3dDevice->Clear(1, &stripe, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 40, 40, 40), 1.0f, 0) );
		V( pd3dDevice->Clear(1, &clearRect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 100, 100, 100), 1.0f, 0) );
		V( g_HUD.OnRender( fElapsedTime ) );
		V( g_SampleUI.OnRender( fElapsedTime ) );

        V( pd3dDevice->EndScene() );
    }
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass messages to settings dialog if its active
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;


	// mouse handles
	switch( uMsg )
    {
		// pick
        case WM_LBUTTONDOWN:
        {
            // Capture the mouse, so if the mouse button is 
            // released outside the window, we'll get the WM_LBUTTONUP message
            
            return TRUE;
        }
		case WM_LBUTTONDBLCLK:
        {
			SetCapture( hWnd );
            break;
        }
		default:
		{
			ReleaseCapture();
			break;
		}
	}

    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	int i;

	if( bKeyDown )
    {
		//m M
		if(nChar == 77)
		{
			bMouseRotateX = true;
		}
		if(nChar == 78)
		{
			bMouseRotateY = true;
		}
		if(nChar == VK_F1)
		{
			wchar_t str[1024];
			memset(str, 0, sizeof(wchar_t)*1024);

			for(i=0; i < manipulator->numOfChains; i++)
				swprintf(str, L"%s\n%d:%d", str, i, manipulator->cube[i]->id);
			MessageBox(0,str,0,0);
		}
	}
	else
	{
		bMouseRotateX = false;
		bMouseRotateY = false;
	}
}



//--------------------------------------------------------------------------------------
// MouseCallback
//--------------------------------------------------------------------------------------
void CALLBACK MouseProc(  bool bLeftButtonDown,
						  bool bRightButtonDown,
						  bool bMiddleButtonDown,
						  bool bSideButton1Down,
						  bool bSideButton2Down,
						  INT nMouseWheelDelta,
						  INT xPos,
						  INT yPos,
						  void* pUserContext )
{
	static D3DXVECTOR2 mousepos = D3DXVECTOR2(xPos, yPos);
	int dx = xPos - mousepos.x;
	int dy = yPos - mousepos.y;
	mousepos = D3DXVECTOR2(xPos, yPos);

	camera->SetRange(nMouseWheelDelta);

	//move up, down, left, right
	if(bLeftButtonDown && bRightButtonDown)
	{
		camera->Slide(D3DXVECTOR2(dx,dy));
	}
	//rotate
	else if(bRightButtonDown)
	{
		camera->Rotate(D3DXVECTOR2(dx*0.003f, dy*0.003f));
	}
	else if(bLeftButtonDown)
	{
		//rotate selected chain
		if(bMouseRotateX && manipulator->selectedChain != -1)
		{
			float MCOEFF = 0.02f;
			cprimitive* cube = manipulator->cube[manipulator->selectedChain];

			float newAngX;
			newAngX = cube->vAngle.x + (float)dy*MCOEFF;

			if(newAngX >= cube->restrictAngleX.x && newAngX <= cube->restrictAngleX.y)
				cube->vAngle.x = newAngX;

			wchar_t str[256];
			swprintf(str,L"X: %.2f",cube->vAngle.x);
			g_SampleUI.GetStatic(IDC_STATIC7)->SetText(str);
		}
		if(bMouseRotateY && manipulator->selectedChain != -1)
		{
			float MCOEFF = 0.02f;
			cprimitive* cube = manipulator->cube[manipulator->selectedChain];

			float newAngY;
			newAngY = cube->vAngle.y + (float)dx*MCOEFF;

			if(newAngY >= cube->restrictAngleY.x && newAngY <= cube->restrictAngleY.y)
				cube->vAngle.y = newAngY;

			wchar_t str[256];
			swprintf(str,L"Y: %.2f",cube->vAngle.y);
			g_SampleUI.GetStatic(IDC_STATIC8)->SetText(str);
		}
	}
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	float val;
	wchar_t str[512];
	FILE* file2;
	LPCWSTR ttext;

    switch( nControlID )
    {
		//main menu
        case IDC_TOGGLEFULLSCREEN:
            DXUTToggleFullScreen(); 
			break;
		
		case IDC_SAVEFILE_BTN:

			swprintf(str,L"./%s",g_HUD.GetEditBox( IDC_SAVEFILE )->GetText());
			manipulator->SaveConfig(str);

			break;

		case IDC_NEWFILE_BTN:

			swprintf(str,L"./%s",g_HUD.GetEditBox( IDC_NEWFILE )->GetText());

			file2 = _wfopen(str,L"w+");
			fclose(file2);

			IniWrite(str,L"manipulator",L"chainsize",L"1");

			IniWrite(str,L"chain_0",L"coefficient",L"0.8");
			IniWrite(str,L"chain_0",L"direction",L"1.00");
			IniWrite(str,L"chain_0",L"displace",L"-1.57");
			IniWrite(str,L"chain_0",L"length",L"90.00");
			IniWrite(str,L"chain_0",L"width",L"10.00");
			IniWrite(str,L"chain_0",L"initangle",L"0.00 0.00");
			IniWrite(str,L"chain_0",L"angle_restrict_phi",L"-3.14 3.14");
			IniWrite(str,L"chain_0",L"angle_restrict_theta",L"-3.14 3.14");
			IniWrite(str,L"chain_0",L"parent_index",L"-1");
			IniWrite(str,L"chain_0",L"model",L"");
			IniWrite(str,L"chain_0",L"offset",L"0 0 0");
			IniWrite(str,L"chain_0",L"mass",L"100.00");
			IniWrite(str,L"chain_0",L"tilt",L"0");
			IniWrite(str,L"chain_0",L"pressure",L"0");
			IniWrite(str,L"chain_0",L"laser",L"0");

			manipulator->Release();
			manipulator->Create(DXUTGetD3D9Device(), str);

			MessageBox(0,L"Создана новая конфигурация манипулятора",L"",0);

			OnChainSelect(-1);
			break;

		case IDC_LOADFILE_BTN:

			manipulator->Release();

			swprintf(str,L"./%s",g_HUD.GetEditBox( IDC_LOADFILE )->GetText());
			manipulator->Create(DXUTGetD3D9Device(), str);

			MessageBox(0,L"Манипулятор загружен из файла",L"",0);
			OnChainSelect(-1);
			break;

			
		//dialog menu
		case IDC_ADDCHAIN:
			manipulator->AddChain(DXUTGetD3D9Device());
			break;

		case IDC_REMOVECHAIN:
			manipulator->RemoveChain();
			break;

		case IDC_WIDTH:
			switch( nEvent )
            {
				case EVENT_EDITBOX_STRING:
                {
					float fWidth;
					LPCWSTR text = (( CDXUTEditBox* )pControl )->GetText();
					swscanf(text, L"%f", &fWidth);

					g_SampleUI.GetSlider(IDC_WIDTH_SLIDER)->SetValue(fWidth);

					manipulator->cube[manipulator->selectedChain]->SetWidth(fWidth);
					break;
				}
			}
			break;

		case IDC_WIDTH_SLIDER:

			val = (float)( ( CDXUTSlider* )pControl )->GetValue();
            manipulator->cube[manipulator->selectedChain]->SetWidth(val);

			swprintf(str,L"%.2f",val);
			g_SampleUI.GetEditBox( IDC_WIDTH )->SetText(str);

            break;

		case IDC_LENGTH:
			switch( nEvent )
            {
				case EVENT_EDITBOX_STRING:
                {
					float fLength;
					LPCWSTR text = (( CDXUTEditBox* )pControl )->GetText();
					swscanf(text, L"%f", &fLength);

					g_SampleUI.GetSlider(IDC_LENGTH_SLIDER)->SetValue(fLength);

					manipulator->cube[manipulator->selectedChain]->SetLength(fLength);
					break;
				}
			}
			break;

		case IDC_LENGTH_SLIDER:

			val = (float)( ( CDXUTSlider* )pControl )->GetValue();
            manipulator->cube[manipulator->selectedChain]->SetLength(val);

			swprintf(str,L"%.2f",val);
			g_SampleUI.GetEditBox( IDC_LENGTH )->SetText(str);

            break;

		case IDC_ANGLE_RESTRICT_XL:
		case IDC_ANGLE_RESTRICT_XU:
		case IDC_ANGLE_RESTRICT_YL:
		case IDC_ANGLE_RESTRICT_YU:
		switch( nEvent )
        {
			case EVENT_EDITBOX_STRING:
            {
				LPCWSTR text = (( CDXUTEditBox* )pControl )->GetText();
				swscanf(text, L"%f", &val);

				if(pControl->GetID() == IDC_ANGLE_RESTRICT_XL)
						manipulator->cube[manipulator->selectedChain]->restrictAngleX.x = val;
				else if(pControl->GetID() == IDC_ANGLE_RESTRICT_XU)
						manipulator->cube[manipulator->selectedChain]->restrictAngleX.y = val;
				else if(pControl->GetID() == IDC_ANGLE_RESTRICT_YL)
						manipulator->cube[manipulator->selectedChain]->restrictAngleY.x = val;
				else if(pControl->GetID() == IDC_ANGLE_RESTRICT_YU)
						manipulator->cube[manipulator->selectedChain]->restrictAngleY.y = val;
				break;
			}
		}
		break;

		case IDC_ANGLE_RESTRICT_XL_BTN:
			swprintf(str,L"%.2f", manipulator->cube[manipulator->selectedChain]->vAngle.x);	
			g_SampleUI.GetEditBox( IDC_ANGLE_RESTRICT_XL )->SetText(str);
			manipulator->cube[manipulator->selectedChain]->restrictAngleX.x = manipulator->cube[manipulator->selectedChain]->vAngle.x;
			break;
		case IDC_ANGLE_RESTRICT_XU_BTN:
			swprintf(str,L"%.2f", manipulator->cube[manipulator->selectedChain]->vAngle.x);	
			g_SampleUI.GetEditBox( IDC_ANGLE_RESTRICT_XU )->SetText(str);
			manipulator->cube[manipulator->selectedChain]->restrictAngleX.y = manipulator->cube[manipulator->selectedChain]->vAngle.x;
			break;
		case IDC_ANGLE_RESTRICT_YL_BTN:
			swprintf(str,L"%.2f", manipulator->cube[manipulator->selectedChain]->vAngle.y);	
			g_SampleUI.GetEditBox( IDC_ANGLE_RESTRICT_YL )->SetText(str);
			manipulator->cube[manipulator->selectedChain]->restrictAngleY.x = manipulator->cube[manipulator->selectedChain]->vAngle.y;
			break;
		case IDC_ANGLE_RESTRICT_YU_BTN:
			swprintf(str,L"%.2f", manipulator->cube[manipulator->selectedChain]->vAngle.y);	
			g_SampleUI.GetEditBox( IDC_ANGLE_RESTRICT_YU )->SetText(str);
			manipulator->cube[manipulator->selectedChain]->restrictAngleY.y = manipulator->cube[manipulator->selectedChain]->vAngle.y;
			break;


		case IDC_OFFSET_X:
		case IDC_OFFSET_Y:
		case IDC_OFFSET_Z:
			switch( nEvent )
            {
				case EVENT_EDITBOX_STRING:
                {
					LPCWSTR text = (( CDXUTEditBox* )pControl )->GetText();
					swscanf(text, L"%f", &val);

					if(pControl->GetID() == IDC_OFFSET_X)
						manipulator->cube[manipulator->selectedChain]->vOffset.x = val;
					else if(pControl->GetID() == IDC_OFFSET_Y)
							manipulator->cube[manipulator->selectedChain]->vOffset.y = val;
					else if(pControl->GetID() == IDC_OFFSET_Z)
							manipulator->cube[manipulator->selectedChain]->vOffset.z = val;

					break;
				}
			}
			break;


		case IDC_DISPLACE:
			ttext = (( CDXUTEditBox* )pControl )->GetText();
			swscanf(ttext, L"%f", &val);
			manipulator->cube[manipulator->selectedChain]->fDisplace = val;
			break;

		case IDC_DIRECTION:
			ttext = (( CDXUTEditBox* )pControl )->GetText();
			swscanf(ttext, L"%f", &val);
			manipulator->cube[manipulator->selectedChain]->fDirection = val;
			break;

		case IDC_COEFFICIENT:
			ttext = (( CDXUTEditBox* )pControl )->GetText();
			swscanf(ttext, L"%f", &val);
			manipulator->cube[manipulator->selectedChain]->fCoefficient = val;
			break;

		case IDC_MASS:
			ttext = (( CDXUTEditBox* )pControl )->GetText();
			swscanf(ttext, L"%f", &val);
			manipulator->cube[manipulator->selectedChain]->fMass = val;
			break;

		case IDC_TILT:
			manipulator->cube[manipulator->selectedChain]->bTilt = ((CDXUTCheckBox*)pControl )->GetChecked();
			break;

		case IDC_PRESSURE:
			manipulator->cube[manipulator->selectedChain]->bPressure = ((CDXUTCheckBox*)pControl )->GetChecked();
			break;

		case IDC_LASER:
			manipulator->cube[manipulator->selectedChain]->bLaser = ((CDXUTCheckBox*)pControl )->GetChecked();
			break;


		case IDC_CMD:
			switch( nEvent )
            {
				case EVENT_EDITBOX_STRING:
					ttext = (( CDXUTEditBox* )pControl )->GetText();
					CMDProcess(ttext);
					(( CDXUTEditBox* )pControl )->SetText(L"");
					break;

				case EVENT_EDITBOX_CHANGE:
				break;
			}
			
			break;

    }
}

//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9DestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();
    g_SettingsDlg.OnD3D9DestroyDevice();

	manipulator->Release();
}



//---------------------------------------------------------------------------------------
// Pick
//---------------------------------------------------------------------------------------
HRESULT Pick()
{
	int i;
	HRESULT hr;
    D3DXVECTOR3 vPickRayDir;
    D3DXVECTOR3 vPickRayOrig;
	D3DXVECTOR3 v;
    IDirect3DDevice9* pD3Device = DXUTGetD3D9Device();
    const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();


	// Get the pick ray from the mouse position
    if( GetCapture() )
    {
		D3DXMATRIX pmatProj = camera->getProjMatrix();

        POINT ptCursor;
        GetCursorPos( &ptCursor );
        ScreenToClient( DXUTGetHWND(), &ptCursor );

		//if menuclick
		if(ptCursor.x >= pd3dsdBackBuffer->Width-360) return S_OK;

        // Compute the vector of the pick ray in screen space
        v.x = ( ( ( 2.0f * ptCursor.x ) / pd3dsdBackBuffer->Width ) - 1 ) / pmatProj._11;
        v.y = -( ( ( 2.0f * ptCursor.y ) / pd3dsdBackBuffer->Height ) - 1 ) / pmatProj._22;
        v.z = 1.0f;

 
		//intersection!
		
		BOOL bHit;
        DWORD dwFace;
        FLOAT fBary1, fBary2, fDist;
		FLOAT fMinDist = 9999999.f;
		int iSelected = -1;

		for(i=0; i < manipulator->numOfChains; i++)
		{
			LPD3DXMESH pMesh;

			D3DXMATRIX* matrix;

			pMesh = manipulator->cube[i]->GetMesh();
			matrix = manipulator->GetChainMatrix(i);

			// transforming ray
			// Get the inverse view matrix
			D3DXMATRIX matView = camera->getViewMatrix();
			D3DXMATRIX matWorld = *matrix;
			D3DXMATRIX mWorldView = matWorld * matView;
			D3DXMATRIX m;
			D3DXMatrixInverse( &m, NULL, &mWorldView );

			// Transform the screen space pick ray into 3D space
			vPickRayDir.x = v.x * m._11 + v.y * m._21 + v.z * m._31;
			vPickRayDir.y = v.x * m._12 + v.y * m._22 + v.z * m._32;
			vPickRayDir.z = v.x * m._13 + v.y * m._23 + v.z * m._33;
			vPickRayOrig.x = m._41;
			vPickRayOrig.y = m._42;
			vPickRayOrig.z = m._43;


			D3DXIntersect( pMesh, &vPickRayOrig, &vPickRayDir, &bHit, &dwFace, &fBary1, &fBary2, &fDist,
							NULL, NULL );
			if( bHit )
			{	
				if(fDist < fMinDist )
				{
					fMinDist = fDist;
					iSelected = i;
				}
			}

			delete matrix;	
		}

		//trigger select event
		if(manipulator->selectedChain != iSelected)
		{
			manipulator->selectedChain = iSelected;
			OnChainSelect(iSelected);
		}
    
	}
	

	return S_OK;
}





//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
// STANDART FUNCTIONAL, NOT MODIFYABLE


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice,
                                    const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );
    V_RETURN( g_SettingsDlg.OnD3D9ResetDevice() );


    // Setup the camera's projection parameters
	camera->OnResetDevice(pBackBufferSurfaceDesc);


    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 350, 0 );
    g_HUD.SetSize( 360, 100 );
	g_HUD.GetEditBox(IDC_CMD)->SetLocation(5,pBackBufferSurfaceDesc->Height-30);
	g_HUD.GetStatic(IDC_STATIC7)->SetLocation(5,pBackBufferSurfaceDesc->Height-60);

    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 350, 130 );
	g_SampleUI.SetSize( 350, pBackBufferSurfaceDesc->Height - 130 );

	//clear black part of screen
	clearRect.x1 = pBackBufferSurfaceDesc->Width - 360;
	clearRect.y1 = 0;
	clearRect.x2 = pBackBufferSurfaceDesc->Width;
	clearRect.y2 = pBackBufferSurfaceDesc->Height;

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9LostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();
    g_SettingsDlg.OnD3D9LostDevice();

}

//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat,
                                      D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{

    // Skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                                         AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
                                         D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;


    // No fallback defined by this app, so reject any device that 
    // doesn't support at least ps2.0
    if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D10 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    if( pDeviceSettings->ver == DXUT_D3D9_DEVICE )
    {
        IDirect3D9* pD3D = DXUTGetD3D9Object();
        D3DCAPS9 Caps;
        pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal, pDeviceSettings->d3d9.DeviceType, &Caps );

        // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
        // then switch to SWVP.
        if( ( Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
            Caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
        {
            pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }

        // Debugging vertex shaders requires either REF or software vertex processing 
        // and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
        if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
        {
            pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
            pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
            pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }
#endif
#ifdef DEBUG_PS
        pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif
    }

    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( ( DXUT_D3D9_DEVICE == pDeviceSettings->ver && pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF ) ||
            ( DXUT_D3D10_DEVICE == pDeviceSettings->ver &&
              pDeviceSettings->d3d10.DriverType == D3D10_DRIVER_TYPE_REFERENCE ) )
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
    }

    return true;
}

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device (either D3D9 or D3D10) 
    // that is available on the system depending on which D3D callbacks are set below
	g_pHInstance = &hInstance;

	
    // Set DXUT callbacks
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackMouse( MouseProc, true );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    DXUTSetCallbackD3D9DeviceAcceptable( IsD3D9DeviceAcceptable );
    DXUTSetCallbackD3D9DeviceCreated( OnD3D9CreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnD3D9ResetDevice );
    DXUTSetCallbackD3D9DeviceLost( OnD3D9LostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnD3D9DestroyDevice );
    DXUTSetCallbackD3D9FrameRender( OnD3D9FrameRender );
	

    InitApp();
    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true );
    DXUTCreateWindow( L"VirMan modeller" );
    DXUTCreateDevice( true, 1600, 1000 );
    DXUTMainLoop(); // Enter into the DXUT render loop

    return DXUTGetExitCode();
}