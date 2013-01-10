//--------------------------------------------------------------------------------------
// MVS Project
// kinvis module
//
//--------------------------------------------------------------------------------------
#include "common/dxstdafx.h"
#include "resource.h"
#include "log.h"
#include "Manipulator.h"
#include "Camera.h"
#include <vector>
using namespace std;


//globals
HANDLE hCom;

CCamera* camera = new CCamera;
CManipulator* manipulator = new CManipulator;

//-----------------------------------------------
// CONSOLE BLOCK
//-----------------------------------------------
#define IDC_OUTPUT 10
#define IDC_INPUT 9
#define IDC_STATS 8
#define IDC_SLIDER 50
#define CS_MAXNUM 30
#define WM_SOCKET WM_USER+1

//-------------
// some stuff 4 socket
WSADATA							WSAData;
SOCKET							sock = INVALID_SOCKET;
SOCKET							client;
SOCKADDR_IN						SA;
int								SOCKPORT;


//----------------------
// for dxgui - stuff from dxutils
D3DRECT							clearRect;
CDXUTDialogResourceManager		g_DialogResourceManager; // manager for shared resources of dialogs
CDXUTDialog						g_SampleUI;             // dialog for sample specific controls


//----------------------
// additional angle stuff - for catchers

#define IDC_FICSLIDER1 100
#define IDC_FICSLIDER2 101

float							ficangle1;
float							ficangle2;
float							ficangle1_dir;
float							ficangle2_dir;
float							ficangle1_offset;
float							ficangle2_offset;
float							catchAngle;

BOOL							bCatch = false;
BOOL							bRelease = false;


//------------------------
// console
int								consoleMsgCount = 0;
wchar_t							consoleMsg[CS_MAXNUM][256];



//------------------------
// command stack
vector<wchar_t*>				commandStack;



//-----------------------------------------------
// F U N C T I O N S

//function declarations

HRESULT ProcessConsoleMessage(wchar_t *string);
HRESULT ProcessReportMessage(wchar_t *string);
HRESULT ProcessSocketMessage(char *string);
void console_init();
void console_append(wchar_t *str);
void console_submit();
void UpdateManipulator(int index=-1);
void TestManipulator();



//function definitions

void console_init()
{
	int i,j;
	for(i=0;i<CS_MAXNUM;i++)
		for(j=0;j<256;j++)
			consoleMsg[i][j] = L'\0';
	return;
}

void console_append(wchar_t* str)
{
	
	int i;
	wchar_t string[CS_MAXNUM*256];
	wsprintf(consoleMsg[consoleMsgCount%CS_MAXNUM], L"%s", str);
	consoleMsgCount++;
	wsprintf(string,L"");

	//end
	if( consoleMsgCount >= CS_MAXNUM )
		for( i = consoleMsgCount%CS_MAXNUM; i < CS_MAXNUM; i++)
		{
			wsprintf(string, L"%s\n%s", string, consoleMsg[i]);
		}
	//beginninng
	for( i=0; i < consoleMsgCount%CS_MAXNUM; i++)
	{
		wsprintf(string, L"%s\n%s", string, consoleMsg[i]);
	}

	g_SampleUI.GetStatic(IDC_OUTPUT)->SetText(string);

	return;
}

void console_submit()
{
	LPWSTR string2 = (LPWSTR)g_SampleUI.GetStatic(IDC_INPUT)->GetText();

	console_append(string2);
	ProcessConsoleMessage(string2);

	g_SampleUI.GetStatic(IDC_INPUT)->SetText(L"");
	return;
}




//--------------------------------------------------------------------------------------
// Rejects any devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    // Typically want to skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3DObject(); 
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
                    D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// Before a device is created, modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext )
{
    return true;
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    WCHAR wszOutput[1024];

	if( nControlID >= IDC_SLIDER && nControlID < IDC_SLIDER + manipulator->numOfChains )
	{
		float value = D3DX_PI/180.f*float(((CDXUTSlider*)pControl)->GetValue());
		if(nControlID != IDC_SLIDER) manipulator->SetAngles(nControlID - IDC_SLIDER, D3DXVECTOR2( value, 0 ),true );
		else manipulator->SetAngles(nControlID - IDC_SLIDER, D3DXVECTOR2( 0, value ),true );
	}
	else if( nControlID == IDC_FICSLIDER1 || nControlID == IDC_FICSLIDER2 )
	{
		float value = D3DX_PI/180.f*float(((CDXUTSlider*)pControl)->GetValue());
		if(nControlID == IDC_FICSLIDER1) ficangle1 = value;
		else if(nControlID == IDC_FICSLIDER2) ficangle2 = value; 
	}
	else
	{
		switch( nControlID )
		{
		}
	}
}

//--------------------------------------------------------------------------------------
// Create any D3DPOOL_MANAGED resources here 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	int i;
	HRESULT hr;

	//objects
	
	manipulator->Create(pd3dDevice);
	//additional stuff
	ficangle1_dir = _wtof(IniRead(L"./config.ini", L"other", L"ficangle1_dir"));
	ficangle1_offset = _wtof(IniRead(L"./config.ini", L"other", L"ficangle1_offset"));
	ficangle2_dir = _wtof(IniRead(L"./config.ini", L"other", L"ficangle2_dir"));
	ficangle2_offset = _wtof(IniRead(L"./config.ini", L"other", L"ficangle2_offset"));

	//ui
	V_RETURN( g_DialogResourceManager.OnCreateDevice( pd3dDevice ) );
	g_SampleUI.Init( &g_DialogResourceManager );
    g_SampleUI.SetCallback( OnGUIEvent );
    
	g_SampleUI.SetFont( 1, L"Courier New", 16, FW_NORMAL );

	//create and initialize controls
	//text
	g_SampleUI.AddStatic( IDC_OUTPUT, L"...", 20, 50, 620, 300 );
    g_SampleUI.GetControl( IDC_OUTPUT )->GetElement( 0 )->iFont = 1;
	g_SampleUI.GetControl( IDC_OUTPUT )->GetElement( 0 )->dwTextFormat = DT_LEFT|DT_BOTTOM|DT_WORDBREAK;
	g_SampleUI.AddStatic( IDC_INPUT, L"", 20, 50, 620, 300 );
    g_SampleUI.GetControl(  IDC_INPUT )->GetElement( 0 )->iFont = 1;
	g_SampleUI.GetControl(  IDC_INPUT )->GetElement( 0 )->dwTextFormat = DT_LEFT|DT_BOTTOM|DT_WORDBREAK;
	g_SampleUI.AddStatic( IDC_STATS, L"", 20, 50, 620, 300 );
    g_SampleUI.GetControl(  IDC_STATS )->GetElement( 0 )->iFont = 1;
	g_SampleUI.GetControl(  IDC_STATS )->GetElement( 0 )->dwTextFormat = DT_LEFT|DT_TOP|DT_WORDBREAK;
	//slider
	for(i = 0; i < manipulator->numOfChains; i++)
	{
		float val_x = 180.f/D3DX_PI*manipulator->cube[i]->restrictAngle.x;
		float val_y = 180.f/D3DX_PI*manipulator->cube[i]->restrictAngle.y;
		if(i==0) {val_x = -180; val_y = 180;}
		g_SampleUI.AddSlider( IDC_SLIDER + i, 200, 450, 200, 24, val_x, val_y, 0, false );
	}
	g_SampleUI.AddSlider( IDC_FICSLIDER1, 200, 450, 200, 24, -90, 90, 0, false );
	g_SampleUI.AddSlider( IDC_FICSLIDER2, 200, 450, 200, 24, -90, 90, 0, false );

	//camera
	
	camera->Setup(D3DXVECTOR3(-602,638,396), D3DXVECTOR3(-528,590,349), D3DXVECTOR2(-1,0.5));



	//console
	console_append(L"Console created\n-----------------------------------------");

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3DPOOL_DEFAULT resources here 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, 
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	int i;
	HRESULT hr;

	//ui
	V_RETURN( g_DialogResourceManager.OnResetDevice() );
	g_SampleUI.SetLocation( 0, 0 );
    g_SampleUI.SetSize( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

	g_SampleUI.GetControl(IDC_OUTPUT)->SetSize( 350, pBackBufferSurfaceDesc->Height - 20);
	g_SampleUI.GetControl(IDC_OUTPUT)->SetLocation( pBackBufferSurfaceDesc->Width - 350, 0);
	g_SampleUI.GetControl(IDC_INPUT)->SetSize( 350, 20);
	g_SampleUI.GetControl(IDC_INPUT)->SetLocation( pBackBufferSurfaceDesc->Width - 350, pBackBufferSurfaceDesc->Height - 20);
	g_SampleUI.GetControl(IDC_STATS)->SetSize( 300, pBackBufferSurfaceDesc->Height);
	g_SampleUI.GetControl(IDC_STATS)->SetLocation( pBackBufferSurfaceDesc->Width - 340 , 5 );
	//slider
	for(i = 0; i < manipulator->numOfChains; i++)
	{
		g_SampleUI.GetControl(IDC_SLIDER+i)->SetLocation( 10, pBackBufferSurfaceDesc->Height - (i+1)*50 );
	}
	g_SampleUI.GetControl(IDC_FICSLIDER1)->SetLocation( 10, pBackBufferSurfaceDesc->Height - (i+1)*50 );
	g_SampleUI.GetControl(IDC_FICSLIDER2)->SetLocation( 10, pBackBufferSurfaceDesc->Height - (i+2)*50 );


	//clear black part of screen
	clearRect.x1 = pBackBufferSurfaceDesc->Width - 350 - 10;
	clearRect.y1 = 0;
	clearRect.x2 = pBackBufferSurfaceDesc->Width;
	clearRect.y2 = pBackBufferSurfaceDesc->Height;

	//camera
	camera->OnResetDevice(pBackBufferSurfaceDesc);

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Handle updates to the scene
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
	int i;

    //chains movement
	manipulator->FrameMove(fElapsedTime, fTime);
	wchar_t* myreport = manipulator->GetReport();
	if(myreport != NULL)
	{
		ProcessReportMessage(myreport);
		//console_append(myreport);
	}

	//angles
	D3DXVECTOR2* angles = manipulator->GetAngles(); //get current angles
	wchar_t stats[512] = L"Angles:\n";

	for(i = 0; i < manipulator->numOfChains; i++)
	{
		float value_x = 180.f/D3DX_PI*angles[i].x;
		float value_y = 180.f/D3DX_PI*angles[i].y;
		float curval;

		if(i != 0) curval = value_x;
		else curval = value_y;
		
		((CDXUTSlider*)(g_SampleUI.GetControl(IDC_SLIDER+i)))->SetValue(curval);		
		swprintf(stats, 512, L"%s %d:%f\n", stats, i, D3DX_PI/180.f*curval);
	}

	//additional angles
	ficangle1 = angles[0].y;
	if(bCatch)
	{
		if(ficangle2 < 0.6f) ficangle2 += fElapsedTime*ANGSPEED;
		else
		{
			manipulator->AppendReport(L"0 Catched");
			bCatch = false;
		}
	}
	else if(bRelease)
	{
		if(ficangle2 > 0.f) ficangle2 -= fElapsedTime*ANGSPEED;
		else
		{
			manipulator->AppendReport(L"0 Released");
			bRelease = false;
		}
	}

	((CDXUTSlider*)(g_SampleUI.GetControl(IDC_FICSLIDER1)))->SetValue(ficangle1);	
	swprintf(stats, 512, L"%s %d:%f\n", stats, i, ficangle1);
	swprintf(stats, 512, L"%s %d:%f\n", stats, i+1, ficangle2);


	g_SampleUI.GetStatic(IDC_STATS)->SetText(stats);

}


//--------------------------------------------------------------------------------------
// Render the scene 
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
	int i;
    HRESULT hr;

    // Clear the render target and the zbuffer 
    V( pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 44, 44, 44), 1.0f, 0) );

    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {

		pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		pd3dDevice->SetTransform( D3DTS_PROJECTION, &camera->getProjMatrix() );
		pd3dDevice->SetTransform( D3DTS_VIEW, &camera->getViewMatrix() );

		// objects
		manipulator->Render(pd3dDevice);

		// ui
		V( pd3dDevice->Clear(1, &clearRect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0) );
		V( g_SampleUI.OnRender( fElapsedTime ) );

        V( pd3dDevice->EndScene() );
    }
}


//--------------------------------------------------------------------------------------
// Handle messages to the application 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, 
                          bool* pbNoFurtherProcessing, void* pUserContext )
{
	//gui handle
	*pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );

    if( *pbNoFurtherProcessing )
        return 0;
	
	switch( uMsg )
    {
		case WM_SOCKET:

			switch(lParam)
            {
				case FD_ACCEPT:
					client = accept(sock,NULL,NULL);
                    return 1;
					break;

				case FD_READ:
					 //обрабатываем команды клиента
					char bufferstring[512];
					sprintf(bufferstring,"");
                    recv(client, bufferstring, 512, 0);

                    if(wParam == SOCKET_ERROR)
                    {
						console_append(L"Error reading socket");
						return 1;
					}

					ProcessSocketMessage( bufferstring );
					manipulator->StartProcess();
					break;
			}

			break;

		case WM_CHAR:

			wchar_t cKey = (wchar_t)wParam;
			if(cKey == 8) break;

			LPWSTR string;
			string = (LPWSTR)g_SampleUI.GetStatic(IDC_INPUT)->GetText();
			wsprintf(string, L"%s%c", string, cKey);
			g_SampleUI.GetStatic(IDC_INPUT)->SetText(string);
			
			break;

	}

    return 0;
}

//--------------------------------------------------------------------------------------
// Processes console messages
//--------------------------------------------------------------------------------------
HRESULT ProcessReportMessage(wchar_t* string)
{
	char consoleStr[256];
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)string, 256, consoleStr, 256, NULL, NULL);
	//wctomb(consoleStr, *string);

	//parse
	int argc = 1, i;
	char** argv;
	char *c, *pc;

	for( c = consoleStr; *c != '\0'; c++ ) if( *c == ' ' ) argc++;
	
	argv = new char*[argc];
	for( i=0; i < argc; i++ ) argv[i] = new char[256];

	c = consoleStr;
	for( i=0; i < argc; i++ )
	{
		pc = argv[i];
		for(; *c != '\0' && *c != ' '; c++, pc++) 
		{
			if( (*c >= 97 && *c <= 122) || (*c >= 65 && *c <= 90) || (*c >= 48 && *c <= 57) ) //allow only chars or digits
			{
				*pc = *c;
			}
		}
		*pc = '\0';
		c++;
		if( (*argv[i] >= 97 && *argv[i] <= 122) || (*argv[i] >= 65 && *argv[i] <= 90) || (*argv[i] >= 48 && *argv[i] <= 57) );
		else argv[i] = argv[i]+1;
	}

	wchar_t constr[512];
	swprintf(constr,L"Report: %s",string);
	console_append(constr);

	//-----------------------
	// REPORTS

	if( !strcmp(argv[0],"0") 
		&& !bCatch && !bRelease) //no errors
	{
		//call stack
		if(commandStack.size() > 0)
		{
			wchar_t* curcommand = commandStack.back();
			ProcessConsoleMessage(curcommand);
			commandStack.pop_back();
		}
		else
		{
			console_append(L"Socket out: 0");
			send(client, "0", 1, 0);
		}
	}
	else //error
	{
		console_append(constr);
	}

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Processes console messages
//--------------------------------------------------------------------------------------
HRESULT ProcessSocketMessage(char* string)
{
	char consoleStr[256];
	strcpy(consoleStr, string);

	//parse
	int argc = 1, i;
	char** argv;
	char *c, *pc;

	for( c = consoleStr; *c != '\0'; c++ ) if( *c == ' ' ) argc++;
	
	argv = new char*[argc];
	for( i=0; i < argc; i++ ) argv[i] = new char[256];

	c = consoleStr;
	for( i=0; i < argc; i++ )
	{
		pc = argv[i];
		for(; *c != '\0' && *c != ' '; c++, pc++) 
		{
			if( (*c >= 97 && *c <= 122) || (*c >= 65 && *c <= 90) || (*c >= 48 && *c <= 57) ) //allow only chars or digits
			{
				*pc = *c;
			}
		}
		*pc = '\0';
		c++;
		if( (*argv[i] >= 97 && *argv[i] <= 122) || (*argv[i] >= 65 && *argv[i] <= 90) || (*argv[i] >= 48 && *argv[i] <= 57) );
		else argv[i] = argv[i]+1;
	}

	wchar_t buffer[512];
	swprintf(buffer,512,L"");
	MultiByteToWideChar(CP_ACP, 0, consoleStr, 256, (LPWSTR)buffer, 512);
	//swprintf(buffer,L"Socket in: %s",buffer);
	console_append(buffer);

	//-----------------------
	// SOCKET MESSAGES
	
	if( (!strncmp(argv[0],"j", 1) || !strncmp(argv[0],"a", 1)) && argc == 5)
	{
		wchar_t* string;

		//1st command to stack
		string = new wchar_t[256];
		swprintf(string,L"pd 0");
		commandStack.push_back(string);
		
		//1st command to stack
		string = new wchar_t[256];
		swprintf(string,L"rel");
		commandStack.push_back(string);

		//1st command to stack
		string = new wchar_t[256];
		swprintf(string,L"mt %d %d", atoi(argv[3]), atoi(argv[4]));
		commandStack.push_back(string);

		//1st command to stack
		string = new wchar_t[256];
		swprintf(string,L"pd 0");
		commandStack.push_back(string);

		//1st command to stack
		string = new wchar_t[256];
		swprintf(string,L"cat");
		commandStack.push_back(string);

		//1st command to stack
		string = new wchar_t[256];
		swprintf(string,L"mt %d %d", atoi(argv[1]), atoi(argv[2]));
		commandStack.push_back(string);

		//1st command to stack
		/*string = new wchar_t[256];
		swprintf(string,L"pd 0");
		commandStack.push_back(string);
		*/
	}

	//
	//-----------------------

	return S_OK;
}
//--------------------------------------------------------------------------------------
// Processes console messages
//--------------------------------------------------------------------------------------
HRESULT ProcessConsoleMessage(wchar_t* string)
{
	char consoleStr[256];
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)string, 256, consoleStr, 256, NULL, NULL);
	//wctomb(consoleStr, *string);

	//parse
	int argc = 1, i;
	char** argv;
	char *c, *pc;

	for( c = consoleStr; *c != '\0'; c++ ) if( *c == ' ' ) argc++;
	
	argv = new char*[argc];
	for( i=0; i < argc; i++ ) argv[i] = new char[256];

	c = consoleStr;
	for( i=0; i < argc; i++ )
	{
		pc = argv[i];
		for(; *c != '\0' && *c != ' '; c++, pc++) 
		{
			if( (*c >= 97 && *c <= 122) || (*c >= 65 && *c <= 90) || (*c >= 48 && *c <= 57) ) //allow only chars or digits
			{
				*pc = *c;
			}
		}
		*pc = '\0';
		c++;
		if( (*argv[i] >= 97 && *argv[i] <= 122) || (*argv[i] >= 65 && *argv[i] <= 90) || (*argv[i] >= 48 && *argv[i] <= 57) );
		else argv[i] = argv[i]+1;
	}


	HTMLLog("ConsoleCommand entered, argc %d, argv:", argc);
	for( i = 0; i < argc; i++ ) HTMLLog(" %s", argv[i]);
	HTMLLog("\n");

	//-----------------------
	// CONSOLE COMMANDS

	//interface commands
	if( !strcmp( argv[0], "quit\0" ) || !strncmp( argv[0], "quit", 4))
	{
		console_append( L"quitting..." );
		exit(0);
	}
	else if( !strcmp( argv[0], "help\0" ) || !strncmp( argv[0], "help", 4))
	{
		console_append( L"command list:" );
		console_append( L"quit" );
		console_append( L"help" );
		console_append( L"setang sa" );
		console_append( L"moveto mt" );
		console_append( L"predefpos pd" );
		console_append( L"camerapos cp" );
	}

	//system commands
	else if( !strcmp( argv[0], "predefpos\0" ) || !strcmp( argv[0], "pd\0" ) || !strncmp( argv[0], "pd", 2))
	{
		if(argc != 2) console_append( L"Usage: predefpos NAME" );
		else
		{
			char wcs[256];
			for(i=0; i < manipulator->numPredefinedPos; i++)
			{
				wchar_t temp[256];
				wsprintf(temp, L"%s", *manipulator->predefinedPositionNames[i]);
				WideCharToMultiByte(CP_ACP, 0, temp, 256, wcs, 256, NULL, NULL);

				if( !strcmp(wcs, argv[1]) ) break;
			}
			
			if( i != manipulator->numPredefinedPos ) manipulator->PredefinedPosition( i );
			else console_append( L"Error: no such predefined position" );

			UpdateManipulator();
		}
	}
	else if( !strcmp( argv[0], "camerapos\0" ) || !strcmp( argv[0], "cp\0" ) || !strncmp( argv[0], "cp", 2) )
	{

		D3DXVECTOR3 vDest, vDest2;
		D3DXVECTOR2 vDest3;
		vDest = camera->getvEye();
		vDest2 = camera->getvAt();
		vDest3 = camera->getAngle();
		wchar_t string[256];
		wsprintf(string, L"Eye: %d %d %d\nAt: %d %d %d\nAngle: %d %d",
			(int)vDest.x, (int)vDest.y, (int)vDest.z, (int)vDest2.x, (int)vDest2.y, (int)vDest2.z, (int)vDest3.x, (int)vDest3.y);
		console_append(string);
	}
	else if( !strcmp( argv[0], "moveto\0" ) || !strcmp( argv[0], "mt\0" ) || !strncmp( argv[0], "mt", 2 ))
	{
		if(argc != 3)
		{
			console_append( L"Usage: moveto QUADRANT LEVEL");
		}
		else
		{
			D3DXVECTOR3 vDest;
			vDest = manipulator->MoveTo( atof(argv[1]), atof(argv[2]) );
			
			/*
			wchar_t string[256];
			wsprintf(string, L"%d %d %d", (int)vDest.x, (int)vDest.y, (int)vDest.z);
			console_append(string);
			*/

			UpdateManipulator();
		}
	}
	else if( !strcmp( argv[0], "setang\0" ) ||  !strcmp( argv[0], "sa\0" ) || !strncmp( argv[0], "sa", 2))
	{
		if(argc != 4)
		{
			console_append( L"Usage: setang INDEX ANGLE_X ANGLE_Y");
		}
		else
		{
			manipulator->SetAngles(atoi(argv[1]), D3DXVECTOR2( atof(argv[2]), atof(argv[3]) ));

			UpdateManipulator(atoi(argv[1]));
		}
	}

	else if( !strncmp( argv[0], "cat", 3) )
	{
		bCatch = true;
		catchAngle = 0.6f;
		UpdateManipulator();
	}
	else if( !strncmp( argv[0], "rel", 3) )
	{
		bRelease = true;
		catchAngle = 0.f;
		UpdateManipulator();
	}

	else
	{
		console_append(L"Error: unknown command");
	}

	return S_OK;
}

//--------------------------------------------------------------------------------------
// As a convenience, DXUT inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke 
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
		if(nChar == VK_RETURN)
		{
			console_submit();
		}
		else if(nChar == VK_BACK)
		{
			LPWSTR string2 = (LPWSTR)g_SampleUI.GetStatic(IDC_INPUT)->GetText();
			WCHAR string[256];
			int i=0;
			for(wchar_t* ptr = string2; *ptr != L'\0'; ptr++) i++;
			swprintf(string, i, L"%s", string2);
			g_SampleUI.GetStatic(IDC_INPUT)->SetText(string);
		}
		else if(nChar == VK_UP)
		{
			g_SampleUI.GetStatic(IDC_INPUT)->SetText( consoleMsg[ consoleMsgCount%CS_MAXNUM - 1 ] );
		}
		else if(nChar == VK_F1)
		{
			UpdateManipulator();
		}
		else if(nChar == VK_F2)
		{
			ProcessSocketMessage("a 21 0 15 0");
			manipulator->StartProcess();
		}
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
}

//--------------------------------------------------------------------------------------
// Release resources created in the OnResetDevice callback here 
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnLostDevice();
}


//--------------------------------------------------------------------------------------
// Release resources created in the OnCreateDevice callback here
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
	int i;

    g_DialogResourceManager.OnDestroyDevice();

	manipulator->Release();
}

//--------------------------------------------------------------------------------------
// Updates manipulator state 
//--------------------------------------------------------------------------------------
void UpdateManipulator(int index)
{
	int i;

	manipulator->StartProcess();

	console_append(L"Updating manipulator state...");
	//manipulator communications
	D3DXVECTOR2* angles = manipulator->GetRAngles(); //get result angles
	if (hCom != INVALID_HANDLE_VALUE)
	{
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));

		if(index == -1)
		{
			sprintf(buffer, "a"); //set angles command
			for(i = 0; i < manipulator->numOfChains; i++)
			{
				float value;
				
				if(i == 0) value = (angles[i].y*manipulator->cube[i]->fCoefficient +  manipulator->cube[i]->fDisplace)* manipulator->cube[i]->fDirection ;
				else value = (angles[i].x*manipulator->cube[i]->fCoefficient +  manipulator->cube[i]->fDisplace)* manipulator->cube[i]->fDirection ;
				//translate radians to degrees
				value = 180.f/D3DX_PI*value;

				sprintf(buffer, "%s %d", buffer, (int)value);
			}
			D3DXVECTOR2* vecs = manipulator->GetRAngles();
			ficangle1 = -1.f * vecs[0].y;
			float ficangle1_deg = 180.f / D3DX_PI*(ficangle1+ficangle1_offset)*ficangle1_dir;
			float ficangle2_deg = 180.f / D3DX_PI*(ficangle2+ficangle2_offset)*ficangle2_dir;
			float catchAngle_deg = 180.f / D3DX_PI*(catchAngle);
			sprintf(buffer, "%s %d %d \r", buffer, (int)ficangle1_deg, (int)catchAngle_deg);	//additional number
		}
		else
		{
			sprintf(buffer, "s %d", index); //set angle command
			
			float value;
			if(index == 0) value = (angles[index].y +  manipulator->cube[index]->fDisplace)* manipulator->cube[index]->fDirection ;
			else value = (angles[index].x +  manipulator->cube[index]->fDisplace)* manipulator->cube[index]->fDirection ;

			//translate radians to degrees
			value = 180.f/D3DX_PI*value;

			sprintf(buffer, "%s %d \r", buffer, (int)value);
		}

		//to console
		wchar_t buffer2[256];
		swprintf(buffer2,L"");
		MultiByteToWideChar(CP_ACP, 0, buffer, 256, (LPWSTR)buffer2, 256);
		//swprintf(buffer2,L"COM out: %s",buffer2);
		console_append(buffer2);


		DWORD nb;
		OVERLAPPED ov;

		if(!WriteFile(hCom, buffer, (DWORD)strlen(buffer), &nb, NULL))
		{
			console_append(L"Error writing into port");
		}
		FlushFileBuffers(hCom);
	}
}


//--------------------------------------------------------------------------------------
// Initialize socket
//--------------------------------------------------------------------------------------
int initSocket(HWND hwnd)
{
	SOCKPORT = _wtoi(IniRead(L"./config.ini", L"system", L"socketport"));

	WSAStartup(0x0101,&WSAData);
	sock = socket(AF_INET,SOCK_STREAM,0);
    WSAAsyncSelect(sock, hwnd, WM_SOCKET,FD_ACCEPT | FD_READ);
    
	SA.sin_family = AF_INET;
    SA.sin_addr.s_addr = htonl(INADDR_ANY);
    SA.sin_port = htons(SOCKPORT);

    bind(sock, (LPSOCKADDR)&SA, sizeof(struct sockaddr));
    listen(sock,15);

	return 0;
}

//--------------------------------------------------------------------------------------
// Initialize com port
//--------------------------------------------------------------------------------------
int initComPort()
{
	//setup comm port
	wchar_t *outdata = new wchar_t[512];
	outdata = IniRead(L"./config.ini", L"system", L"portname");
	console_append(L"Port configuration:");
	console_append(outdata);
	
	hCom = CreateFile(outdata,
					  GENERIC_READ | GENERIC_WRITE,
					  0,
					  NULL,
					  OPEN_EXISTING,
					  0,
					  NULL);
	if(hCom == INVALID_HANDLE_VALUE)
	{
		console_append(L"Cannot open port: invalid port name or parameters");
	}
	else
	{	
		int TIMEOUT	= _wtoi(IniRead(L"./config.ini", L"system", L"timeout"));
		int PORTSPEED = _wtoi(IniRead(L"./config.ini", L"system", L"portspeed"));
		//configurating port
		
		SetCommMask(hCom, EV_RXCHAR);
		SetupComm(hCom, 1500, 1500);
		
		//set timeouts
		COMMTIMEOUTS CommTimeOuts;
		CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
		CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
		CommTimeOuts.ReadTotalTimeoutConstant = TIMEOUT;
		CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
		CommTimeOuts.WriteTotalTimeoutConstant = TIMEOUT;
		if(!SetCommTimeouts(hCom, &CommTimeOuts))
		{
			console_append(L"Unable to set comm timeouts");
		}

		//set speed and other stuff
		DCB ComDCM;
		memset(&ComDCM,0,sizeof(ComDCM));
		ComDCM.DCBlength = sizeof(DCB);
		GetCommState(hCom, &ComDCM);
		
		
		ComDCM.BaudRate = PORTSPEED; //port speed
		ComDCM.ByteSize = 8;
		ComDCM.Parity = NOPARITY;
		ComDCM.StopBits = ONESTOPBIT;
		ComDCM.fAbortOnError = TRUE;
		ComDCM.fDtrControl = DTR_CONTROL_DISABLE;
		ComDCM.fRtsControl = RTS_CONTROL_DISABLE;
		ComDCM.fBinary = TRUE;
		ComDCM.fParity = FALSE;
		ComDCM.fInX = ComDCM.fOutX = FALSE;
		ComDCM.XonChar = 0;
		ComDCM.XoffChar = 0xff;
		ComDCM.fErrorChar = FALSE;
		ComDCM.fNull = FALSE;
		ComDCM.fOutxCtsFlow = FALSE;
		ComDCM.fOutxDsrFlow = FALSE;
		ComDCM.XonLim = 128;
		ComDCM.XoffLim = 128;
		if(!SetCommState(hCom, &ComDCM))
		{
			console_append(L"Unable to set com port configuration.");
		}
	}

	return 0;
}

//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
	/*
	//чтение и запись конфигурации
	wchar_t *data = L"test test test";
	wchar_t *outdata = new wchar_t[512];
	if (IniWrite(L"./file.ini", L"testsection", 0L"testkey", data))
		outdata = IniRead(L"./file.ini", L"testsection", L"testkey");
	*/

    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	


    // Set the callback functions
    DXUTSetCallbackDeviceCreated( OnCreateDevice );
    DXUTSetCallbackDeviceReset( OnResetDevice );
    DXUTSetCallbackDeviceLost( OnLostDevice );
    DXUTSetCallbackDeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackFrameRender( OnFrameRender );
    DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackMouse( MouseProc, true );
   
    // TODO: Perform any application-level initialization here
	InitLog();
	console_init();

    // Initialize DXUT and create the desired Win32 window and Direct3D device for the application
    DXUTInit( true, true, true ); // Parse the command line, handle the default hotkeys, and show msgboxes
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
	DXUTCreateWindow( L"MVS: Kinetics & Visualization" );
    DXUTCreateDevice( D3DADAPTER_DEFAULT, true, 1600, 1050, IsDeviceAcceptable, ModifyDeviceSettings );

	//setup port
	initComPort();
	initSocket( DXUTGetHWND());

    // Start the render loop
    DXUTMainLoop();

    // TODO: Perform any application-level cleanup here

    return DXUTGetExitCode();
}


