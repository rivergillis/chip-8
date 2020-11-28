#ifndef SDL_VIEWER_H_
#define SDL_VIEWER_H_

#include <mutex>
#include <SDL2/SDL.h>

#include "common.h"
#include "sdl_timer.h"

// RAII hardware-accelerated SDL Window.
// Optimized for RGB24 texture streaming.
// This class is thread-safe.

// TODO: Double buffer so we can SetFrame during Update ?

class SDLViewer {
  public:
    // Width and height must be equal to the size of images uploaded
    // via SetFrameRGB24.
    SDLViewer(const std::string& title, int width, int height, int window_scale = 1);
    ~SDLViewer();

    // Renders the current frame, returns a list of all events.
    std::vector<SDL_Event> Update();

    // Assumes 8-bit RGB image with stride equal to width (no padding).
    void SetFrameRGB24(uint8_t* rgb24, int height);

  private:
    std::string title_;

    std::mutex mu_; // protects the following
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* window_tex_ = nullptr;

    // FPS counting.
    uint32_t num_updates_ = 0;
    SDLTimer timer_;
};

#endif