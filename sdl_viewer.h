#ifndef SDL_VIEWER_H_
#define SDL_VIEWER_H_

#include <SDL2/SDL.h>

#include "common.h"
#include "sdl_timer.h"

// RAII SDL Window.

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
    int window_width_;
    int window_height_;

    // Surface for the frames uploaded. This is blitted to window_surf_
    // on Update().
    SDL_Surface* frame_surf_ = nullptr;
    SDL_Surface* window_surf_ = nullptr;
    SDL_Window* window_ = nullptr;

    // FPS counting.
    uint32_t num_updates_ = 0;
    SDLTimer timer_;
};

#endif