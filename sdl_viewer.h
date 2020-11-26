#ifndef SDL_VIEWER_H_
#define SDL_VIEWER_H_

#include <SDL2/SDL.h>

#include "common.h"
#include "sdl_timer.h"

// RAII hardware-accelerated SDL Window.

class SDLViewer {
  public:
    SDLViewer(const std::string& title, int width, int height);
    ~SDLViewer();

    // Renders the current frame, returns a list of all events.
    std::vector<SDL_Event> Update();

    // Assumes 8-bit RGB image with stride equal to width (no padding).
    void SetFrameRGB24(uint8_t* rgb24, int width, int height);

  private:
    std::string title_;

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = NULL;
    SDL_Texture* window_tex_ = nullptr;

    // FPS counting.
    uint32_t num_updates_ = 0;
    SDLTimer timer_;
};

#endif