#include "draw.h"

void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset) {
    int px, py, c;
    SDL_Rect s, d;
    s.w = 8; 
    s.h = 8; 
    d.w = 8; 
    d.h = 8;
    while(*text) {
        c = *text & 255;
        px = (c % 16) * 8; 
        py = (c / 16) * 8;
        s.x = px; 
        s.y = py; 
        d.x = x; 
        d.y = y;
        SDL_BlitSurface(charset, &s, screen, &d);
        x += 8; 
        text++;
    };
}

void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) {
    SDL_Rect r = {x, y, l, k};
    SDL_FillRect(screen, &r, fillColor);
    SDL_Rect border = {x,y,l,1}; 
    SDL_FillRect(screen, &border, outlineColor);
    border = {x,y+k-1,l,1}; 
    SDL_FillRect(screen, &border, outlineColor);
    border = {x,y,1,k}; 
    SDL_FillRect(screen, &border, outlineColor);
    border = {x+l-1,y,1,k}; 
    SDL_FillRect(screen, &border, outlineColor);
}

void DrawSpriteFrame(SDL_Surface *screen, SDL_Surface *sprite, int x, int y, int col, int row, int frameW, int frameH, int dir) {
    SDL_Rect src = {col * frameW, row * frameH, frameW, frameH};
    SDL_Rect dest = {x - frameW/2, y - frameH, frameW, frameH};
    if(sprite) SDL_BlitSurface(sprite, &src, screen, &dest);
}