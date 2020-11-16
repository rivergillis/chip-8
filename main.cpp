#include <iostream>
#include <vector>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "image.h"
#include "cpu_chip8.h"

//The window we'll be rendering to
SDL_Window* gWindow = NULL;
//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;
//The image we will load and show on the screen
SDL_Surface* gHelloWorld = NULL;

// Size of the window. Input images get stretched to this size.
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Starts up SDL and creates window
bool init() {
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        //Create window
        gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( gWindow == NULL )
        {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            //Get window surface
            gScreenSurface = SDL_GetWindowSurface( gWindow );
        }
    }

    return success;
}
bool loadMedia()
{
    //Loading success flag
    bool success = true;

    //Load splash image
    // TODO: Use https://wiki.libsdl.org/SDL_CreateRGBSurfaceWithFormatFrom 
    // with pixel_format == SDL_PIXELFORMAT_RGB24 and depth=24
    gHelloWorld = SDL_LoadBMP( "/Users/river/code/chip8/hello_world.bmp" );
    if( gHelloWorld == NULL )
    {
        printf( "Unable to load image %s! SDL Error: %s\n", "02_getting_an_image_on_the_screen/hello_world.bmp", SDL_GetError() );
        success = false;
    }

    return success;
}

//Frees media and shuts down SDL
void close()
{
    //Deallocate surface
    SDL_FreeSurface( gHelloWorld );
    gHelloWorld = NULL;

    //Destroy window
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;

    //Quit SDL subsystems
    SDL_Quit();
}



// TODO: Emulate at 60hz. (60 cycles per second).
void Run() {
     //Start up SDL and create window
    if( !init() )
    {
        printf( "Failed to initialize!\n" );
    }
    else
    {
        //Load media
        if( !loadMedia() )
        {
            printf( "Failed to load media!\n" );
        }
        else
        {
            // Poll till we quit.
            // Needed in order to display an image.
            SDL_Event e;
            bool quit = false;
            while (!quit){
                while (SDL_PollEvent(&e)){
                    if (e.type == SDL_QUIT){
                        quit = true;
                    }
                }
                //Apply the image
                				//Apply the image, stretch it to the size of the screen.
                SDL_Rect stretchRect;
                stretchRect.x = 0;
                stretchRect.y = 0;
                stretchRect.w = SCREEN_WIDTH;
                stretchRect.h = SCREEN_HEIGHT;
                SDL_BlitScaled(gHelloWorld, NULL, gScreenSurface, &stretchRect );
                // SDL_BlitSurface( gHelloWorld, NULL, gScreenSurface, NULL );
                //Update the surface
                SDL_UpdateWindowSurface( gWindow );
            }
        }
    }
    //Free resources and close SDL
    close();

  // Goal: SDLViewer viewer
  //        cpu.RunCycle();
  //        if cpu.framechanged()
  //          uint8_t* rgb24_img = cpu.frame->CopyToRGB24(red_multiplier=255.0)
  //          viewer.displayImage(rgba_image)
  // 
  // CpuChip8 cpu;
  // cpu.Initialize();
  // cpu.LoadROM("/Users/river/code/chip8/roms/PONG");
  // for (int i = 0; i < 500; ++i) {
  //   cpu.RunCycle();
  // }
  std::cout << "Exit main() success";
}

int main() {
  try {
    Run();
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what();
    return 1;
  }
}
