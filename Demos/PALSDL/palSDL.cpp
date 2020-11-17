#include <iostream>
#include "pal/palFactory.h"
#include "sdlgl.h"
//#include <SDL2/SDL.h>

int main(int argc, char *argv[]) {
    //(step 1) PF is macro defined in palFactory.h, which can get the global palFactory instance. LoadPALfromDLL will automatically find and register engine dlls.
    PF->LoadPALfromDLL();

	//(step 2) Select Bullet, which needs the existance of libpal_bullet.dll
    PF->SelectEngine("ODE");

	//(step3) create the physics object
    palPhysics *pp = PF->CreatePhysics();
    if (pp == NULL) {
        printf("Failed to create the physics engine. Check to see if you spelt the engine name correctly, and that the engine DLL is in the right location");
        return 1;
    }
    else {
        printf("%s\n%s\n", pp->GetPALVersion(), pp->GetVersion());
    }
    palPhysicsDesc desc;
    desc.m_vGravity.x = 0;
    desc.m_vGravity.y = -9.8f;
    desc.m_vGravity.z = 0;
	//(step 3) Initial the physics engine with gravity (negative y axis)
    pp->Init(desc);

    float angle = M_PIf*0.4f;
    float distance = 150;
    float height = 50;
    bool mouse_down = false;
    bool g_quit = false;
    bool wireframe = false;
    bool transparent = false;

    //(step 4) create and initial the SDL enviroment
    SDLGLEngine *g_eng = new SDLGLEngine;
    g_eng->Init(640, 480);

    //define vertices and indices for an opened box
    float ver[3 * 8] = { -10, 0, -10,
        10, 0, -10,
        10, 0, 10,
        -10, 0, 10,
        -10, 5, -10,
        10, 5, -10,
        10, 5, 10,
        -10, 5, 10 };
    int ind[3 * 10] = { 0, 1, 2, 2, 3, 0,
        6, 2, 3, 3, 7, 6,
        0, 3, 7, 7, 4, 0,
        0, 4, 5, 5, 1, 0,
        5, 1, 2, 2, 6, 5 };

	//(step 5) create a mesh terrain object
    palTerrainMesh *ptm = PF->CreateTerrainMesh();
    if (ptm != NULL) {
        ptm->Init(0, 0, 0, ver, 8, ind, 30);  //initial the terrain object
        SDL_Mesh  *graphics_mesh = new SDL_Mesh; //create a SDL mesh object for the display
        graphics_mesh->Init(3 * 8, 3 * 10, ver, ind);  //initial the sdl object
        terrain_graphics = graphics_mesh;
    }
    else {
        printf("Error: Could not create a terrain plane\n");
    }

    //use SDL to handle the input
    SDL_Event E;
    while (!g_quit) {
        if (SDL_PollEvent(&E)) {
            switch (E.type) {
            case SDL_QUIT:
                g_quit = true;
                break;
            case SDL_MOUSEBUTTONDOWN:
                mouse_down = true;
                break;
            case SDL_MOUSEBUTTONUP:
                mouse_down = false;
                break;
            case SDL_MOUSEMOTION:
                if (mouse_down)
                    angle += E.motion.xrel*0.01f;
                break;
            case SDL_KEYDOWN:
                switch (E.key.keysym.sym) {
                case SDLK_PLUS:
                case SDLK_KP_PLUS:
                    distance -= 0.5f;
                    break;
                case SDLK_MINUS:
                case SDLK_KP_MINUS:
                    distance += 0.5f;
                    break;
                case SDLK_KP_DIVIDE:
                    height -= 0.5f;
                    break;
                case SDLK_KP_MULTIPLY:
                    height += 0.5f;
                    break;
                case SDLK_KP1:
                case SDLK_TAB:
                    wireframe = !wireframe;
                    break;
                case SDLK_KP0:
                case SDLK_0:
                    transparent = !transparent;
                    break;
                case SDLK_a:
				    //(step 6) create a box inside PAL
                    palBox *pb = PF->CreateBox();
                    pb->Init(0, 60, 0, 2, 2, 2, 8);
                    BuildGraphics(pb);   //Build a SDL box at the same time, defined in example/graphics.cpp. Objects will stored as a copy in g_Graphics
                    break;
                }
            }
        }
        else {
            pp->Update(0.02f);  //step in for the physic engine
            palVector3 pos;

            g_eng->Clear();     //clear the SDL engine

            //setup SDL according to the parameters for display
            g_eng->Wireframe(wireframe);
            g_eng->Transparent(transparent);
            g_eng->SetProjMatrix(M_PIf / 4.0f, 1.0f, 0.2f, 200.0f);
            g_eng->SetViewMatrix(distance*cosf(angle), height, distance*sinf(angle), 0, height, 0, 0, 1, 0);

            //(step 7) display the terrain
            terrain_graphics->Render();
            for (int i = 0; i < g_Graphics.size(); i++) {
                g_Graphics[i]->m_pBody->GetPosition(pos);   //we can get position of each object inside PAL/physics engine
                g_Graphics[i]->Display();   //Use display routine define in example/graphics.cpp
            }

            g_eng->Flip();  //show the content to user
        }
    }

    delete terrain_graphics;
    delete g_eng;

    PF->Cleanup();
    return 0;
}
