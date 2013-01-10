//--------------------------------------------------------------------------------------
// File: SimpleSample.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma comment(lib, "Foundation.lib")
#pragma comment(lib, "PhysX3_x86.lib")
#pragma comment(lib, "PhysX3Extensions.lib")
#pragma comment(lib, "GeomUtils.lib")
#pragma comment(lib, "PxTask.lib")
#pragma comment(lib, "PvdRuntime.lib")

//physx
#include <PxPhysicsAPI.h>
#include <PxDefaultErrorCallback.h>
#include <PxDefaultAllocator.h> 
#include <PxExtensionsAPI.h>
#include <PxTask.h>


#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
#include "DXUTCamera.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "resource.h"

#include "LH_strings_VM.h"
#include "Manipulator.h"
#include "Camera.h"
#include <vector>
using namespace std;



typedef struct{
	enum Enum
	{
		Environment,
		Manipulator
	};
}FilterGroup;



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
CCamera*					    camera;

physx::PxRigidDynamic* manipulator_physx; 
D3DXMATRIX					matManipulator;

D3DRECT						clearRect;
D3DXQUATERNION		    quatManipulator;
D3DXVECTOR3				manipulator_toMove = D3DXVECTOR3(0,0,0);

bool						bMouseRotateX = false;
bool						bMouseRotateY = false;


//physx
physx::PxPhysics*	 mSDK;
bool recordMemoryAllocations = true;
float QUANT  = 0.1f;

static physx::PxDefaultErrorCallback gDefaultErrorCallback;
static physx::PxDefaultAllocator gDefaultAllocatorCallback;

physx::PxSimulationFilterShader pDefaultFilterShader = physx::PxDefaultSimulationFilterShader;
physx::PxDefaultCpuDispatcher * mCpuDispatcher;
physx::PxScene * mScene;

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


void InitPhysX();
void ReleasePhysX();
void AddBoxPhysX(D3DXVECTOR3 vDim, D3DXMATRIX matTrans);
physx::PxFilterFlags TestFilterShader(
        physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
        physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
        physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize);



class MyShapeData
{
public:
	int man_id;
	int chain_id;

	MyShapeData(int vman_id, int vchain_id)
	{
		man_id = vman_id;
		chain_id = vchain_id;
	};
};



class MyContactModification : public physx::PxContactModifyCallback
{
        void onContactModify(physx::PxContactModifyPair *const pairs, physx::PxU32 count);
};
void MyContactModification::onContactModify(physx::PxContactModifyPair *const pairs, physx::PxU32 count)
{
        for(int i=0; i<count; i++)
        {
			physx::PxContactSet contset = pairs[i].contacts;
			for(int j=0;j<contset.size();j++)
			{
				physx::PxVec3 contpoint = contset.getPoint(j);
				wchar_t str[512];
				MyShapeData* data1 = (MyShapeData*)pairs[i].shape[0]->userData;
				MyShapeData* data2 = (MyShapeData*)pairs[i].shape[1]->userData;
				MyShapeData* actual = NULL;

				if(data1->man_id == 0) actual = data1;
				else if(data2->man_id == 0) actual = data2;

				if(actual != NULL)
				{
					for(int k=0;k<manipulator->numOfLimbs;k++)
					{
						if(actual->chain_id == manipulator->limbs[k]->lastchain)
						{
							cprimitive* prim = manipulator->cube[actual->chain_id];
							D3DXVECTOR3 myvec = D3DXVECTOR3(-prim->vecMove.x*0.15f, 0, -prim->vecMove.z*0.15f);
							D3DXVECTOR3 myvec2;
							D3DXMATRIX mymat;

							D3DXMatrixRotationQuaternion(&mymat,&quatManipulator);
							D3DXVec3TransformCoord(&myvec2, &myvec, &mymat);
							manipulator_toMove = manipulator_toMove + D3DXVECTOR3(myvec2.x, 0, myvec2.z);
							manipulator_physx->addForce(physx::PxVec3(myvec2.x, 0, myvec2.z));

							break;
						}
					}
					
				}
			}
        }
}
MyContactModification ContactModification;




//-------------------------------------------------------------------------------------------
//--       F U N C T I O N S
//-------------------------------------------------------------------------------------------



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
}



//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
//       P H Y S X      part
//
//---------------------------------------------------------------------------


void setupFiltering(physx::PxRigidActor* actor, physx::PxU32 filterGroup)
{
        physx::PxFilterData filterData;
        filterData.word0 = filterGroup; // word0 = own ID
        const physx::PxU32 numShapes = actor->getNbShapes();
        physx::PxShape** shapes = new physx::PxShape*[numShapes];
        actor->getShapes(shapes, numShapes);
        for(physx::PxU32 i = 0; i < numShapes; i++)
        {
                physx::PxShape* shape = shapes[i];
                shape->setSimulationFilterData(filterData);
        }
        delete [] shapes;
}


void InitPhysX()
{
	mSDK = PxCreatePhysics(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback, physx::PxTolerancesScale(), recordMemoryAllocations );
	if(!mSDK) {
		MessageBox(0,L"Error initializing PhysX",L"PhysX error",0);
	}

	if (! PxInitExtensions(*mSDK))
    {
		MessageBox(0,L"Error initializing PhysX ext",L"PhysX error",0);
	}

	physx::PxSceneDesc sceneDesc(mSDK->getTolerancesScale());
    sceneDesc.gravity = physx::PxVec3(0.0f, -90.81f, 0.0f);
	sceneDesc.filterShader = TestFilterShader;
	sceneDesc.contactModifyCallback = &ContactModification;

    mCpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
    if(!mCpuDispatcher)
		MessageBox(0,L"PxDefaultCpuDispatcherCreate failed!",L"PhysX error",0);
    sceneDesc.cpuDispatcher = mCpuDispatcher;

    mScene = mSDK->createScene(sceneDesc);
    if (!mScene)
		MessageBox(0,L"createScene failed!",L"PhysX error",0);

	//--------------
	physx::PxMaterial*	mMaterial;

	mMaterial = mSDK->createMaterial(5.5f, 5.5f, 0.0f);     //static friction, dynamic friction, restitution
	if(!mMaterial)
	{
		MessageBox(0,L"material creation failed!",L"PhysX error",0);
	}

	physx::PxReal d = 0.0f;
	physx::PxU32 axis = 1;
	physx::PxTransform pose;

	if(axis == 0)
			pose = physx::PxTransform(physx::PxVec3(d, 0.0f, 0.0f));
	else if(axis == 1)
			pose = physx::PxTransform(physx::PxVec3(0.0f, d, 0.0f),physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0.0f, 0.0f, 1.0f)));
	else if(axis == 2)
			pose = physx::PxTransform(physx::PxVec3(0.0f, 0.0f, d), physx::PxQuat(-physx::PxHalfPi, physx::PxVec3(0.0f, 1.0f, 0.0f)));

	physx::PxRigidStatic* plane = mSDK->createRigidStatic(pose);
	if (!plane)
		MessageBox(0,L"create plane failed!",L"PhysX error",0);

	physx::PxShape* shape = plane->createShape(physx::PxPlaneGeometry(), *mMaterial);
	if (!shape)
		MessageBox(0,L"create shape failed!",L"PhysX error",0);

	setupFiltering(plane, FilterGroup::Environment);

	mScene->addActor(*plane);

	physx::PxShape** shapes = new physx::PxShape*[plane->getNbShapes()];
	plane->getShapes(shapes,plane->getNbShapes(),0);
	for(int i=0; i< plane->getNbShapes(); i++)
	{
		MyShapeData* shpd = new MyShapeData(-1,-1);
		shapes[i]->userData = (MyShapeData*)shpd;
	}

	//PVD
	physx::PxExtensionVisualDebugger::connect(mSDK->getPvdConnectionManager(),"localhost",5425, 10000, true);
	mScene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1000.0f);
	mScene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LIMITS, 1000.0f);

}


void ReleasePhysX()
{
	PxCloseExtensions();
    mScene->release();
    mSDK->release();
}


void AddBoxPhysX(D3DXVECTOR3 vDim, D3DXMATRIX imatTrans)
{
	//-box
	D3DXVECTOR3 pOutScale;
	D3DXVECTOR3 pOutTranslation;
	D3DXQUATERNION pOutRotation;

	D3DXMatrixDecompose( &pOutScale, &pOutRotation, &pOutTranslation, &imatTrans );

	physx::PxMat44 transmat;
	transmat.column0 = physx::PxVec4(imatTrans._11, imatTrans._12, imatTrans._13, imatTrans._14);
	transmat.column1 = physx::PxVec4(imatTrans._21, imatTrans._22, imatTrans._23, imatTrans._24);
	transmat.column2 = physx::PxVec4(imatTrans._31, imatTrans._32, imatTrans._33, imatTrans._34);
	transmat.column3 = physx::PxVec4(imatTrans._41, imatTrans._42, imatTrans._43, imatTrans._44);

	physx::PxMaterial*	mMaterial;
	mMaterial = mSDK->createMaterial(0.5f, 0.5f, 0.1f);     //static friction, dynamic friction, restitution

	physx::PxReal density = 100.0f;
	physx::PxTransform transform = physx::PxTransform(transmat);
	physx::PxVec3 dimensions(vDim.x, vDim.y, vDim.z);
	physx::PxBoxGeometry geometry(dimensions);

	physx::PxRigidDynamic *actor = PxCreateDynamic(*mSDK, transform, geometry, *mMaterial, density);
	if (!actor)
		MessageBox(0,L"create actor failed!",L"PhysX error",0);

	
	setupFiltering(actor, FilterGroup::Manipulator); 

	mScene->addActor(*actor);
	
	return;
}


physx::PxFilterFlags TestFilterShader(
        physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
        physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
        physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{


	if(filterData0.word0 == filterData1.word0)
	{
		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
		return  physx::PxFilterFlag::eKILL;
	}
	else
	{
		//f(filterData0.word0 == FilterGroup::Manipulator) manipulator->cube[filterData0.word1]->bContact = true;
		//else if(filterData1.word0 == FilterGroup::Manipulator) manipulator->cube[filterData1.word1]->bContact = true;

		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT | physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS | physx::PxPairFlag::eMODIFY_CONTACTS;
        return physx::PxFilterFlag::eDEFAULT;
	}
}


//------- end of  p h y s x  part
//---------------------------------------------------------------------




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



	manipulator->Create(g_pd3dDevice, L"./spider.ini");
	manipulator->FrameMove(0.1f,0.1f);



	//--------------------------------------------------------------
	// convert manipulator to PhysX model

	physx::PxMaterial* mMaterial;
	mMaterial = mSDK->createMaterial(0.5f,0.5f, 0.0f);     //static friction, dynamic friction, restitution

	physx::PxRigidDynamic* actor = mSDK->createRigidDynamic(physx::PxTransform(physx::PxVec3(0,5,0)));
	if(!actor)
	{
		MessageBox(0,L"Cannot create actor",L"Physx Error",0);
	}

	actor->setActorFlag(physx::PxActorFlag::eVISUALIZATION, true);
	actor->setAngularDamping(0.1f);


	vector<physx::PxTransform> localPoses; 
	for(int i=0;i<manipulator->numOfChains;i++)
	{
		//matrices
		cprimitive* prim = manipulator->cube[i];
		D3DXMATRIX* imatTrans = manipulator->GetChainMatrix(i,QUANT);
		D3DXMATRIX resMat;

		D3DXVECTOR3 v3 = D3DXVECTOR3(prim->fWidth*QUANT, prim->fLength/2*QUANT, prim->fWidth*QUANT);
		D3DXVECTOR3 pOutScale;
		D3DXVECTOR3 pOutTranslation;
		D3DXQUATERNION pOutRotation;
		physx::PxTransform localPose;
		D3DXMATRIX offset;

		//first matrix - without offset
		resMat = *imatTrans;
		D3DXMatrixDecompose( &pOutScale, &pOutRotation, &pOutTranslation, &resMat );
		localPose = physx::PxTransform(physx::PxVec3(pOutTranslation.x,pOutTranslation.y,pOutTranslation.z), physx::PxQuat(pOutRotation.x,pOutRotation.y,pOutRotation.z,pOutRotation.w));
		localPoses.push_back(localPose);

		//actual matrix
		D3DXMatrixTranslation(&offset,0,prim->fLength/2*QUANT,0);
		resMat = offset*(*imatTrans);

		D3DXMatrixDecompose( &pOutScale, &pOutRotation, &pOutTranslation, &resMat );
		localPose = physx::PxTransform(physx::PxVec3(pOutTranslation.x,pOutTranslation.y,pOutTranslation.z), physx::PxQuat(pOutRotation.x,pOutRotation.y,pOutRotation.z,pOutRotation.w));

		delete imatTrans;

		physx::PxBoxGeometry geometry = physx::PxBoxGeometry(physx::PxVec3(v3.x,v3.y,v3.z));

		physx::PxShape* shape = actor->createShape(geometry, *mMaterial, localPose);//physx::PxTransform::createIdentity());

		MyShapeData* data = new MyShapeData(0,i);
		shape->userData = (void*)data;
	}

	if(actor)
	{
		//actor->setGlobalPose(localPose);
		setupFiltering(actor,FilterGroup::Manipulator);
		mScene->addActor(*actor);
		manipulator_physx = actor;

		actor->setMass(0.01f);
		actor->setSolverIterationCounts(10);

		/*if(prim->parentID >= 0)
		{
			//creating joint in global systems
			cprimitive* pParent = manipulator->cube[prim->parentID];
			physx::PxTransform pxTransformParent = localPoses[i];
			physx::PxTransform pxTransformPrim = localPoses[i];
			physx::PxFixedJoint* joint = physx::PxFixedJointCreate(*mSDK, actor, pxTransformPrim, manipulator_physx[prim->parentID], pxTransformParent);
			joint->setConstraintFlag(physx::PxConstraintFlag::eVISUALIZATION, true);
			joint->disableAutoResolve();
			joint->setProjectionLinearTolerance(0.f);
			joint->setProjectionAngularTolerance(0.f);
		}*/
			
	}

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	manipulator->FrameMove(fElapsedTime, fTime);

	wchar_t* report = manipulator->GetReport();
	if(report != NULL)
	{
	}

	//update physx
	physx::PxShape** shapes = new physx::PxShape*[manipulator->numOfChains];
	manipulator_physx->getShapes(shapes,manipulator->numOfChains);

	for(int i=0;i<manipulator->numOfChains;i++)
	{
		cprimitive* prim = manipulator->cube[i];
		D3DXMATRIX* imatTrans = manipulator->GetChainMatrix(i, QUANT);
		D3DXVECTOR3 v3 = D3DXVECTOR3(prim->fWidth*QUANT, prim->fLength/2*QUANT, prim->fWidth*QUANT);

		D3DXMATRIX offset;
		D3DXMatrixTranslation(&offset,0,prim->fLength/2*QUANT,0);

		*imatTrans = offset*(*imatTrans);

		D3DXVECTOR3 pOutScale;
		D3DXVECTOR3 pOutTranslation;
		D3DXQUATERNION pOutRotation;

		D3DXMatrixDecompose( &pOutScale, &pOutRotation, &pOutTranslation, imatTrans );

		physx::PxTransform localPose;
		localPose = physx::PxTransform(physx::PxVec3(pOutTranslation.x,pOutTranslation.y,pOutTranslation.z), physx::PxQuat(pOutRotation.x,pOutRotation.y,pOutRotation.z,pOutRotation.w));
		delete imatTrans;

		shapes[i]->setLocalPose(localPose);

	}
	delete[] shapes;
	
	//fetch physx
	mScene->simulate(min(fElapsedTime,1.f/60.f));
	mScene->fetchResults(true);
	manipulator_physx->wakeUp();



	//get results from physX
	physx::PxTransform manTransform = manipulator_physx->getGlobalPose();
	physx::PxMat44 mymat = physx::PxMat44(manTransform);

	matManipulator._11 = mymat.column0[0];
	matManipulator._12 = mymat.column0[1];
	matManipulator._13 = mymat.column0[2];
	matManipulator._14 = mymat.column0[3];
	matManipulator._21 = mymat.column1[0];
	matManipulator._22 = mymat.column1[1];
	matManipulator._23 = mymat.column1[2];
	matManipulator._24 = mymat.column1[3];
	matManipulator._31 = mymat.column2[0];
	matManipulator._32 = mymat.column2[1];
	matManipulator._33 = mymat.column2[2];
	matManipulator._34 = mymat.column2[3];
	matManipulator._41 = mymat.column3[0];
	matManipulator._42 = mymat.column3[1];
	matManipulator._43 = mymat.column3[2];
	matManipulator._44 = mymat.column3[3];

	D3DXVECTOR3 pVecTranslate;
	D3DXVECTOR3 pVecScale;
	D3DXQUATERNION pQuat;
	D3DXVECTOR3 newvec = D3DXVECTOR3(0,0,0);

	D3DXMatrixDecompose(&pVecScale, &pQuat, &pVecTranslate, &matManipulator);

	//manipulator_physx->setGlobalPose(physx::PxTransform(physx::PxVec3(pVecTranslate.x+manipulator_toMove.x,pVecTranslate.y+manipulator_toMove.y,pVecTranslate.z+manipulator_toMove.z), physx::PxQuat(pQuat.x, pQuat.y, pQuat.z, pQuat.w)));
	//manipulator_toMove = D3DXVECTOR3(0,0,0);

	quatManipulator = pQuat;
	pVecTranslate /= QUANT;
	D3DXMatrixAffineTransformation(&matManipulator,1,&newvec,&pQuat,&pVecTranslate);
	
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
		manipulator->Render(pd3dDevice,&matManipulator);
        

		//MENU AND HUD
		D3DRECT stripe = D3DRECT(clearRect);
		stripe.x1 = stripe.x1-2;

		//V( pd3dDevice->Clear(1, &stripe, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 40, 40, 40), 1.0f, 0) );
		//V( pd3dDevice->Clear(1, &clearRect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 100, 100, 100), 1.0f, 0) );
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

		/*
		1
			manipulator->InversedKinematics(D3DXVECTOR3(95,28,-153),L"1stleg");
			manipulator->InversedKinematics(D3DXVECTOR3(180,27,11),L"2ndleg");
			manipulator->InversedKinematics(D3DXVECTOR3(-132,28,-153),L"3rdleg");
			manipulator->InversedKinematics(D3DXVECTOR3(-173,28,47),L"4thleg");
		2
			manipulator->InversedKinematics(D3DXVECTOR3(114,-111,-150),L"1stleg");
			manipulator->InversedKinematics(D3DXVECTOR3(110,-108,10),L"2ndleg");
			manipulator->InversedKinematics(D3DXVECTOR3(-143,-110,-96),L"3rdleg");
			manipulator->InversedKinematics(D3DXVECTOR3(-157,-110,43),L"4thleg");
		3
			manipulator->InversedKinematics(D3DXVECTOR3(180,-110,-35),L"1stleg");
			manipulator->InversedKinematics(D3DXVECTOR3(129,-110,130),L"2ndleg");
			manipulator->InversedKinematics(D3DXVECTOR3(-174,-107,-18),L"3rdleg");
			manipulator->InversedKinematics(D3DXVECTOR3(-100,-110,131),L"4thleg");
		4
			manipulator->InversedKinematics(D3DXVECTOR3(166,29,-38),L"1stleg");
			manipulator->InversedKinematics(D3DXVECTOR3(141,30,142),L"2ndleg");
			manipulator->InversedKinematics(D3DXVECTOR3(-222,38,-24),L"3rdleg");
			manipulator->InversedKinematics(D3DXVECTOR3(-121,28,165),L"4thleg");
		*/

		//test controlling based on inversed kinematics
		if(nChar == VK_F1)
		{
			manipulator->InversedKinematics(D3DXVECTOR3(114,-111,-150),L"1stleg");
			manipulator->InversedKinematics(D3DXVECTOR3(-143,-110,-96),L"3rdleg");
			
		}
		if(nChar == VK_F2)
		{
			manipulator->InversedKinematics(D3DXVECTOR3(180,-110,-35),L"1stleg");
			manipulator->InversedKinematics(D3DXVECTOR3(-174,-107,-18),L"3rdleg");
			manipulator->InversedKinematics(D3DXVECTOR3(180,27,11),L"2ndleg");
			manipulator->InversedKinematics(D3DXVECTOR3(-173,28,47),L"4thleg");
		}
		if(nChar == VK_F3)
		{
			manipulator->InversedKinematics(D3DXVECTOR3(110,-108,10),L"2ndleg");
			manipulator->InversedKinematics(D3DXVECTOR3(-157,-110,43),L"4thleg");
		}
		if(nChar == VK_F4)
		{
			manipulator->InversedKinematics(D3DXVECTOR3(166,29,-38),L"1stleg");
			manipulator->InversedKinematics(D3DXVECTOR3(-222,38,-24),L"3rdleg");
		}
		if(nChar == VK_F5)
		{
			manipulator->InversedKinematics(D3DXVECTOR3(95,28,-153),L"1stleg");
			manipulator->InversedKinematics(D3DXVECTOR3(-132,28,-153),L"3rdleg");
			manipulator->InversedKinematics(D3DXVECTOR3(129,-110,130),L"2ndleg");
			manipulator->InversedKinematics(D3DXVECTOR3(-100,-110,131),L"4thleg");
		}


		if(nChar == VK_F6)
		{
			manipulator_physx->addForce(physx::PxVec3(10.f,10.f,10.f));
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
		
	}
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
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
	InitPhysX();
    
	DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true );
    DXUTCreateWindow( L"VirMan modeller" );
    DXUTCreateDevice( true, 1600, 1000 );
    DXUTMainLoop(); // Enter into the DXUT render loop

	ReleasePhysX();

    return DXUTGetExitCode();
}