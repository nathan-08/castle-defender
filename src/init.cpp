#include "init.hpp"
#include "const.hpp"
#include <SDL_mixer.h>

bool sdl_init(SDL_Window **window_p, SDL_Renderer **renderer_p) {
  bool success = true;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    printf("SDL could not init: %s\n", SDL_GetError());
    success = false;
  }

  *window_p = SDL_CreateWindow("rpg_1.0", SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (!*window_p) {
    printf("Window could not be created! %s\n", SDL_GetError());
    success = false;
  }

  *renderer_p = SDL_CreateRenderer(*window_p, -1,
      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!*renderer_p) {
    printf("Renderer could not be created! %s\n", SDL_GetError());
    success = false;
  }

  if (TTF_Init() == -1) {
    printf("SDL_ttf could not init: %s\n", TTF_GetError());
    success = false;
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048)>0) {
    printf("SDL_mixer failed to init: %s\n", Mix_GetError());
    success = false;
  }

  SDL_RenderSetLogicalSize(*renderer_p, L_WIDTH, L_HEIGHT);

  return success;
}

bool load_font(TTF_Font **font_p) {
  //*font_p = TTF_OpenFont("fonts/press_start.ttf", 8);
  //*font_p = TTF_OpenFont("fonts/gb.ttf", 8);
  *font_p = TTF_OpenFont("fonts/04b_03.ttf", 8);
  if (!*font_p) {
    printf("Failed to load font! %s\n", TTF_GetError());
    return false;
  }
  return true;
}
