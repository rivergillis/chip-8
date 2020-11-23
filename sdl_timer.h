#ifndef SDL_TIMER_H_
#define SDL_TIMER_H_

#include "common.h"

class SDLTimer {
  public:
    SDLTimer();

    void Start();

    // Returns elapsed Ms since last call to Start().
    uint32_t Ms();

  private:
    uint32_t start_ticks_;
};

#endif