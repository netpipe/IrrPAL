/*
This tutorial will briefly show how to use the terrain renderer of Irrlicht. It will also
show the terrain renderer triangle selector to be able to do collision detection with
terrain.

Note that the Terrain Renderer in Irrlicht is based on Spintz' GeoMipMapSceneNode, lots
of thanks go to him.
DeusXL provided a new elegant simple solution for building larger area on small heightmaps
-> terrain smoothing.
In the beginning there is nothing special. We include the needed header files and create
an event listener to listen if the user presses the 'W' key so we can switch to wireframe
mode and if he presses 'D' we toggle to material between solid and detail mapped.
*/
#include <irrlicht.h>
#include <iostream>
#include "pal/palFactory.h"
#pragma comment(lib, "libpal.lib")


using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


class MyEventReceiver : public IEventReceiver
{
public:

	MyEventReceiver(scene::ISceneNode* terrain)
	{
		// store pointer to terrain so we can change its drawing mode
		Terrain = terrain;
	}

	bool OnEvent(const SEvent& event)
	{
		// check if user presses the key 'W' or 'D'
		if (event.EventType == irr::EET_KEY_INPUT_EVENT && !event.KeyInput.PressedDown)
		{
				switch (event.KeyInput.Key)
				{
				case irr::KEY_KEY_W: // switch wire frame mode
				Terrain->setMaterialFlag(video::EMF_WIREFRAME, !Terrain->getMaterial(0).Wireframe);
				Terrain->setMaterialFlag(video::EMF_POINTCLOUD, false);
				return true;
			case irr::KEY_KEY_P: // switch wire frame mode
				Terrain->setMaterialFlag(video::EMF_POINTCLOUD, !Terrain->getMaterial(0).PointCloud);
				Terrain->setMaterialFlag(video::EMF_WIREFRAME, false);
				return true;
			case irr::KEY_KEY_D: // toggle detail map
				Terrain->setMaterialType(
					Terrain->getMaterial(0).MaterialType == video::EMT_SOLID ?
					video::EMT_DETAIL_MAP : video::EMT_SOLID);
				return true;
			}
		}

		return false;
	}

private:
	scene::ISceneNode* Terrain;
};


class BindObject {
public:
	//the Irrlicht Node
	irr::scene::ISceneNode *node;
	//the PAL "Node"
	palBodyBase *pb;
	//update Irrlicht position from physics
	void Update() {
		//get the location matrix from the physics system
		palMatrix4x4 matrix = pb->GetLocationMatrix();
		//copy it to a Irrlicht matrix
		core::matrix4 mat;
		memcpy(&mat[0], matrix._mat, sizeof(f32)*4*4);
		//set the node position from our Irrlicht matrix
		node->setPosition(mat.getTranslation());
		node->setRotation(mat.getRotationDegrees());
	}
};


//global declarations
scene::ISceneManager* g_smgr = 0;
scene::ITerrainSceneNode* g_terrain = 0;

void LoadTerrain(float x, float y, float z, float sx, float sy, float sz) {
	//call the irrlicht add terrain scene node
		g_terrain = g_smgr->addTerrainSceneNode(
		"../../media/terrain-heightmap.bmp",
		0,										// parent node
		-1,										// node id
		core::vector3df(x, y, z),			// position
		core::vector3df(0.f, 0.f, 0.f),			// rotation
		core::vector3df(sx, sy, sz),		// scale
		video::SColor ( 255, 255, 255, 255 ),	// vertexColor,
		5,										// maxLOD
		scene::ETPS_17,							// patchSize
		4										// smoothFactor
		);

	//get the vertex buffer
	//scene::SMeshBufferLightMap smb;
	//g_terrain->getMeshBufferForLOD(smb,0);
	scene::CDynamicMeshBuffer smb(video::EVT_2TCOORDS,video::EIT_32BIT);

    g_terrain->getMeshBufferForLOD(
      smb,      0 //level of detail
      );


	//get the number of vertices and indicies from Irrlicht
	int nv = smb.getVertexCount ();
	int ni = smb.getIndexCount 	();

	//create the physics terrain mesh, and some space to store our data
	palTerrainMesh *ptm = PF->CreateTerrainMesh();
	float *pVerts = new float[3*nv];
	int *pInds = new int[ni];

	//get a pointer to the verticies and indicies from Irrlicht
	irr::video::S3DVertex2TCoords* pv = (irr::video::S3DVertex2TCoords*) smb.getVertices();
	irr::u16* pi = (irr::u16*)smb.getIndices();
	//copy the vertex and index data
	int i;
	for (i=0;i<nv;i++) {
		pVerts[i*3+0]=sx*pv[i].Pos.X + x; //scale each vertex by sX and set its position with x
		pVerts[i*3+1]=sy*pv[i].Pos.Y + y;
		pVerts[i*3+2]=sz*pv[i].Pos.Z + z;
	}
	for (i=0;i<ni;i++) {
		pInds[i]=pi[i];
	}
	//initialize the physics terrain
	ptm->Init(0,0,0,pVerts,nv,pInds,ni);
	delete [] pVerts ;
	delete [] pInds ;
}

/*
The start of the main function starts like in most other example. We ask the user
for the desired renderer and start it up.
*/
int main()
{

	//load physics
	PF->LoadPALfromDLL();

	//let user select physics driver.
	printf("Please select the physics driver you want for this example:\n"\
		" (a) Bullet\n (b) JigLib\n (c) Newton\n"\
		" (d) ODE\n (e) Simple Physics Engine\n"\
		" (f) Tokamak\n (g) TrueAxis\n (otherKey) exit\n\n");

	char i;
	std::cin >> i;
	switch(i)
	{
		case 'a': PF->SelectEngine("Bullet");break;
		case 'b': PF->SelectEngine("Jiggle");break;
		case 'c': PF->SelectEngine("Newton");break;
		case 'd': PF->SelectEngine("ODE");break;
		case 'e': PF->SelectEngine("SPE");break;
		case 'f': PF->SelectEngine("Tokamak");break;
		case 'g': PF->SelectEngine("TrueAxis");break;
		default: return 1;
	}

	// PF->SelectEngine("ODE");
	//set up physics
	palPhysics *pp = PF->CreatePhysics();
	if (!pp) {
		printf("Error : Could not load physics engine! Is \"libpal_<physicsengine>.dll\" missing?\n");
		return 0;
	}
	if (pp){
		palPhysicsDesc desc;
		pp->Init(desc);
//		pp->Init(0,-9.8,0);
	}
	// let user select driver type

	video::E_DRIVER_TYPE driverType = video::EDT_OPENGL;

//	printf("Please select the driver you want for this example:\n"\
//		" (a) Direct3D 9.0c\n (b) Direct3D 8.1\n (c) OpenGL 1.5\n"\
//		" (d) Software Renderer\n (e) Burning's Software Renderer\n"\
//		" (f) NullDevice\n (otherKey) exit\n\n");
//
//	std::cin >> i;

//	switch(i)
//	{
//		case 'a': driverType = video::EDT_DIRECT3D9;break;
////		case 'b': driverType = video::EDT_DIRECT3D8;break;
//		case 'c': driverType = video::EDT_OPENGL;   break;
//		case 'd': driverType = video::EDT_SOFTWARE; break;
//		case 'e': driverType = video::EDT_BURNINGSVIDEO;break;
//		case 'f': driverType = video::EDT_NULL;     break;
//		default: return 1;
//	}

	// create device

	IrrlichtDevice* device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(640, 480));

	if (device == 0)
		return 1; // could not create selected driver.


	/*
	First, we add standard stuff to the scene: A nice irrlicht engine
	logo, a small help text, a user controlled camera, and we disable
	the mouse cursor.
	*/

	video::IVideoDriver* driver = device->getVideoDriver();
	g_smgr = device->getSceneManager();
	gui::IGUIEnvironment* env = device->getGUIEnvironment();

	driver->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, true);

	// add irrlicht logo
	env->addImage(driver->getTexture("../../media/irrlichtlogo2.png"),
		core::position2d<s32>(10,10));

	//set other font
	env->getSkin()->setFont(env->getFont("../../media/fontlucida.png"));

	// add some help text
	gui::IGUIStaticText* text = env->addStaticText(
		L"Press 'W' to change wireframe mode\nPress 'D' to toggle detail map",
		core::rect<s32>(10,440,250,475), true, true, 0, -1, true);

	// add camera
	scene::ICameraSceneNode* camera =
		g_smgr->addCameraSceneNodeFPS(0,100.0f,0.50f);

	camera->setPosition(core::vector3df(0,15,10));
	camera->setTarget(core::vector3df(10,15,10));
	camera->setFarValue(120.0f);

	// disable mouse cursor
	device->getCursorControl()->setVisible(false);

	/*
	Here comes the terrain renderer scene node: We add it just like any
	other scene node to the scene using ISceneManager::addTerrainSceneNode().
	The only parameter we use is a file name to the heightmap we use. A heightmap
	is simply a gray scale texture. The terrain renderer loads it and creates
	the 3D terrain from it.
	To make the terrain look more big, we change the scale factor of it to (40, 4.4, 40).
	Because we don't have any dynamic lights in the scene, we switch off the lighting,
	and we set the file terrain-texture.jpg as texture for the terrain and
	detailmap3.jpg as second texture, called detail map. At last, we set
	the scale values for the texture: The first texture will be repeated only one time over
	the whole terrain, and the second one (detail map) 20 times.
	*/

	// add terrain scene node
	LoadTerrain(-100,0,-100,
				0.8,0.04,0.8);


	g_terrain->setMaterialFlag(video::EMF_LIGHTING, false);

	g_terrain->setMaterialTexture(0, driver->getTexture("../../media/terrain-texture.jpg"));
	g_terrain->setMaterialTexture(1, driver->getTexture("../../media/detailmap3.jpg"));

	g_terrain->setMaterialType(video::EMT_DETAIL_MAP);

	g_terrain->scaleTexture(1.0f, 20.0f);
	//terrain->setDebugDataVisible ( true );



	/*
	To make the user be able to switch between normal and wireframe mode, we create
	an instance of the event reciever from above and let Irrlicht know about it. In
	addition, we add the skybox which we already used in lots of Irrlicht examples.
	*/

	// create event receiver
	MyEventReceiver receiver(g_terrain);
	device->setEventReceiver(&receiver);

   	// create skybox
	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);

	g_smgr->addSkyBoxSceneNode(
		driver->getTexture("../../media/irrlicht2_up.jpg"),
		driver->getTexture("../../media/irrlicht2_dn.jpg"),
		driver->getTexture("../../media/irrlicht2_lf.jpg"),
		driver->getTexture("../../media/irrlicht2_rt.jpg"),
		driver->getTexture("../../media/irrlicht2_ft.jpg"),
		driver->getTexture("../../media/irrlicht2_bk.jpg"));

	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);


	//our vector of all physics objects
	std::vector<BindObject *> vbo;

	///lets make 7 cubes
//	for (int i=1;i<2;i++) {
//		BindObject *pbo = 0;
//		float size = 2.0f;
//		//create a new bind object
//		pbo = new BindObject;
//		//lets set the bind object node to a cube scene node
//		pbo->node =  g_smgr->addCubeSceneNode(size,0,-1,core::vector3df(0,0,0),core::vector3df(0,0,0),core::vector3df(1,1,1));
//		//lets create a physics box as well
//		palBox *pb = PF->CreateBox();
//		//set the physics position, and a matching size
//		pb->Init(size*i*5,size*5,size*i*5,	size,size,size,	10);
//		//assing the bind object body to the physics box
//		pbo->pb = pb;
//		//add this to our vector of objects
//		vbo.push_back(pbo);
//	}
	/*
	That's it, draw everything. Now you know how to use terrain in Irrlicht.
	*/



	int lastFPS = -1;

	while(device->run()){
	//if (device->isWindowActive())
	//{


			//update physics
		if (pp)
			pp->Update(1/60.0f); //timestep

		//update the irrlicht graphics to be at the same location as the physics engine
		for (i=0;i<vbo.size();i++) {
			vbo[i]->Update();
		}

	//	driver->beginScene(ECBF_COLOR | ECBF_DEPTH, SColor(255,100,101,140));
		driver->beginScene(true, true, 0 );

		g_smgr->drawAll();
		env->drawAll();

		driver->endScene();

		// display frames per second in window title
		int fps = driver->getFPS();
		if (lastFPS != fps)
		{
			core::stringw str = L"Terrain Renderer - Irrlicht Engine [";
			str += driver->getName();
			str += "] FPS:";
			str += fps;
			// Also print terrain height of current camera position
			// We can use camera position because terrain is located at coordinate origin
			str += " Height: ";
			str += g_terrain->getHeight(camera->getAbsolutePosition().X, camera->getAbsolutePosition().Z);

			device->setWindowCaption(str.c_str());
			lastFPS = fps;
		}
	}

	device->drop();

	return 0;
}

