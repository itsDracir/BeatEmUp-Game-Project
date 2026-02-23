#ifndef DRAW_H
#define DRAW_H
#include "structs.h"

void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset);
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor);
void DrawSpriteFrame(SDL_Surface *screen, SDL_Surface *sprite, int x, int y, int col, int row, int frameW, int frameH, int dir);

#endif