#include "defs.h"
#include "structs.h"
#include "draw.h"
#include "ai.h"

extern "C" {
#include "./sdl/include/SDL.h"
#include "./sdl/include/SDL_main.h"
}

InputEvent inputBuffer[BUFFER_SIZE];
GameState globalState = STATE_MENU;
char playerName[21] = "";
int menuOption = 0;
int levelCount = 1;
double messageTimer = 0;
int scoreSaved = 0;

Enemy* enemies = NULL;
Obstacle* obstacles = NULL;

int showDevMode = 0;
int resultsScrollOffset = 0;
int totalScoresLoaded = 0;

double doorTimer = 0.0;

void AddInput(SDL_Keycode key, double time) {
    for (int i = BUFFER_SIZE - 1; i > 0; i--) {
        inputBuffer[i] = inputBuffer[i - 1];
    }
    inputBuffer[0].key = key;
    inputBuffer[0].timestamp = time;
}

const char* GetStateName(PlayerAnimState s) {
    switch (s) {
        case P_STATE_IDLE: return "IDLE";
        case P_STATE_WALK: return "WALK";
        case P_STATE_JUMP: return "JUMP";
        case P_STATE_DASH: return "DASH";
        case P_STATE_ATTACK_LIGHT: return "ATK_L";
        case P_STATE_ATTACK_HEAVY: return "ATK_H";
        case P_STATE_COMBO_TRIPLE: return "CMB_3";
        case P_STATE_COMBO_MIXED: return "CMB_MX";
        case P_STATE_DEATH: return "DEATH";
        default: return "UNK";
    }
}

void ApplyColorKey(SDL_Surface* surface) {
    if (surface) {
        Uint32 colKey = SDL_MapRGB(surface->format, 255, 0, 255);
        SDL_SetColorKey(surface, true, colKey);
    }
}

int main(int argc, char** argv) {
    int t1, t2, quit, frames;
    double delta, worldTime;
    SDL_Event event;
    SDL_Surface *screen, *charset, *playerSprite;
    SDL_Surface *bgSurf, *enemy1Surf, *enemy2Surf, *doorSurf;
    SDL_Texture* scrtex;
    SDL_Window* window;
    SDL_Renderer* renderer;

    Player p;
    double camera_x = 0;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return 1;
    if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer) != 0) return 1;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_SetWindowTitle(window, "Beat'em Up - Final Version");

    screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_ShowCursor(SDL_DISABLE);

    charset = SDL_LoadBMP("./sprites/cs8x8.bmp");
    playerSprite = SDL_LoadBMP("./sprites/player.bmp");
    bgSurf = SDL_LoadBMP("./sprites/background.bmp");
    enemy1Surf = SDL_LoadBMP("./sprites/enemy1.bmp");
    enemy2Surf = SDL_LoadBMP("./sprites/enemy2.bmp");
    doorSurf = SDL_LoadBMP("./sprites/door.bmp");

    if (!charset || !playerSprite || !bgSurf || !enemy1Surf || !enemy2Surf || !doorSurf) {
        printf("Warning: Check bitmaps in ./sprites/ folder.\n");
    }

    SDL_SetColorKey(charset, true, 0x000000);
    ApplyColorKey(playerSprite);
    ApplyColorKey(enemy1Surf);
    ApplyColorKey(enemy2Surf);
    ApplyColorKey(doorSurf);

    int black = SDL_MapRGB(screen->format, 0, 0, 0);
    int bg_blue = SDL_MapRGB(screen->format, 0x30, 0x30, 0x50);
    int white = SDL_MapRGB(screen->format, 255, 255, 255);
    int blue = SDL_MapRGB(screen->format, 50, 50, 255);
    int red = SDL_MapRGB(screen->format, 255, 0, 0);
    int green = SDL_MapRGB(screen->format, 0, 255, 0);
    int purple = SDL_MapRGB(screen->format, 128, 0, 128);
    int orange = SDL_MapRGB(screen->format, 255, 165, 0);
    int grey = SDL_MapRGB(screen->format, 100, 100, 100);

    t1 = SDL_GetTicks();
    frames = 0;
    quit = 0;
    worldTime = 0;

    p.x = 100;
    p.y = 400;
    p.z = 0;
    p.vz = 0;
    p.dir = 1;
    p.state = P_STATE_IDLE;
    p.stateTimer = 0;
    p.animTimer = 0;
    p.score = 0;
    p.multiplier = 1;
    p.multiplierTimer = 0;
    p.maxHp = MAX_HP;
    p.currentHp = MAX_HP;
    p.hitTimer = 0;
    p.hitDone = 0;

    if (!LoadStageFromFile(1, &enemies, &obstacles)) {
        
    }

    for (int i = 0; i < BUFFER_SIZE; i++) {
        inputBuffer[i].key = 0;
        inputBuffer[i].timestamp = -999;
    }

    while (!quit) {
        t2 = SDL_GetTicks();
        delta = (t2 - t1) * 0.001;
        t1 = t2;
        worldTime += delta;

        if (messageTimer > 0) messageTimer -= delta;

        SDL_FillRect(screen, NULL, black);

        if (globalState == STATE_MENU) {
            DrawString(screen, SCREEN_WIDTH / 2 - 40, 100, "BEAT'EM UP", charset);
            DrawString(screen, SCREEN_WIDTH / 2 - 60, 200, (menuOption == 0) ? "> START GAME" : "  Start Game", charset);
            DrawString(screen, SCREEN_WIDTH / 2 - 60, 220, (menuOption == 1) ? "> CHECK RESULTS" : "  Check Results", charset);
            DrawString(screen, SCREEN_WIDTH / 2 - 60, 240, (menuOption == 2) ? "> EXIT" : "  Exit", charset);
            
            if (messageTimer > 0) DrawString(screen, SCREEN_WIDTH / 2 - 80, 300, "[UNAVAILABLE OPTION]", charset);
        }
        else if (globalState == STATE_NAME_INPUT) {
            DrawString(screen, SCREEN_WIDTH / 2 - 50, 150, "ENTER NICKNAME:", charset);
            DrawRectangle(screen, SCREEN_WIDTH / 2 - 60, 170, 130, 12, white, black);
            DrawString(screen, SCREEN_WIDTH / 2 - 55, 172, playerName, charset);
            DrawString(screen, SCREEN_WIDTH / 2 - 80, 300, "Press ENTER to continue", charset);
        }
        else if (globalState == STATE_RESULTS) {
            DrawString(screen, SCREEN_WIDTH / 2 - 40, 50, "HALL OF FAME", charset);
            DrawString(screen, SCREEN_WIDTH / 2 - 80, 70, "Name        Score   Time", charset);
            DrawRectangle(screen, SCREEN_WIDTH / 2 - 90, 80, 180, 1, white, white);

            DrawString(screen, SCREEN_WIDTH / 2 + 110, 100, "^", charset);
            DrawString(screen, SCREEN_WIDTH / 2 + 110, 200, "v", charset);

            FILE* f = fopen("scores.txt", "r");
            if (f) {
                HighScore scores[100];
                int count = 0;
                while (count < 100 && fscanf(f, "%s %d %lf", scores[count].name, &scores[count].score, &scores[count].time) == 3) {
                    count++;
                }
                fclose(f);
                totalScoresLoaded = count;
                qsort(scores, count, sizeof(HighScore), CompareScores);

                for (int i = resultsScrollOffset; i < resultsScrollOffset + 5 && i < count; i++) {
                    char line[128];
                    snprintf(line, sizeof(line), "%-10.20s  %5d   %3.0lf", scores[i].name, scores[i].score, scores[i].time);
                    DrawString(screen, SCREEN_WIDTH / 2 - 80, 100 + ((i - resultsScrollOffset) * 20), line, charset);
                }
            } else {
                DrawString(screen, SCREEN_WIDTH / 2 - 60, 100, "No records yet.", charset);
            }

            DrawString(screen, SCREEN_WIDTH / 2 - 60, 400, "Press ESC to Menu", charset);
            DrawString(screen, SCREEN_WIDTH / 2 - 80, 420, "Use ARROWS to Scroll", charset);
        }
        else if (globalState == STATE_GAME) {
            if (p.multiplier > 1) {
                p.multiplierTimer -= delta;
                if (p.multiplierTimer <= 0) p.multiplier = 1;
            }
            p.animTimer += delta;
            if (p.hitTimer > 0) p.hitTimer -= delta;

            if (enemies) {
                for (int i = 0; i < totalEnemies; i++) {
                    UpdateEnemyAI(&enemies[i], &p, delta);
                }
            }

            if (p.state != P_STATE_DEATH && p.state < P_STATE_ATTACK_LIGHT && p.z == 0) {
                if (worldTime - inputBuffer[0].timestamp < 0.1) {
                    if (inputBuffer[0].key == SDLK_RIGHT && inputBuffer[1].key == SDLK_RIGHT && worldTime - inputBuffer[1].timestamp < COMBO_WINDOW) {
                        p.state = P_STATE_DASH;
                        p.stateTimer = 0.3;
                        p.dir = 1;
                        inputBuffer[0].key = 0;
                    }
                    else if (inputBuffer[0].key == SDLK_LEFT && inputBuffer[1].key == SDLK_LEFT && worldTime - inputBuffer[1].timestamp < COMBO_WINDOW) {
                        p.state = P_STATE_DASH;
                        p.stateTimer = 0.3;
                        p.dir = -1;
                        inputBuffer[0].key = 0;
                    }
                }
            }

            if (p.stateTimer > 0) {
                p.stateTimer -= delta;
                if (p.stateTimer <= 0 && p.state != P_STATE_DEATH) {
                    p.state = P_STATE_IDLE;
                    p.hitDone = 0;
                }
            }

            const Uint8* state = SDL_GetKeyboardState(NULL);
            if (p.state != P_STATE_DEATH) {
                double prevX = p.x;
                double prevY = p.y;

                if (p.state == P_STATE_DASH) {
                    p.x += (DASH_SPEED * p.dir) * delta;
                }
                else if (p.state < P_STATE_ATTACK_LIGHT) {
                    int isMoving = 0;
                    if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) { p.x += PLAYER_SPEED * delta; p.dir = 1; isMoving = 1; }
                    if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]) { p.x -= PLAYER_SPEED * delta; p.dir = -1; isMoving = 1; }
                    if (p.z <= 0 && p.state != P_STATE_JUMP) {
                        if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_W]) { p.y -= PLAYER_SPEED * delta; isMoving = 1; }
                        if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S]) { p.y += PLAYER_SPEED * delta; isMoving = 1; }
                        p.state = isMoving ? P_STATE_WALK : P_STATE_IDLE;
                    }
                }

                if (obstacles) {
                    for (int i = 0; i < totalObstacles; i++) {
                        if (CheckCollision((int)p.x - 10, (int)p.y - 10, 20, 10, obstacles[i].x, obstacles[i].y, obstacles[i].w, obstacles[i].h)) {
                            p.x = prevX;
                            if (CheckCollision((int)p.x - 10, (int)p.y - 10, 20, 10, obstacles[i].x, obstacles[i].y, obstacles[i].w, obstacles[i].h)) {
                                p.y = prevY;
                            }
                        }
                    }
                }

                if (p.state == P_STATE_JUMP || p.z > 0) {
                    p.vz -= GRAVITY * delta;
                    p.z += p.vz * delta;
                    if (p.z <= 0) {
                        p.z = 0;
                        p.vz = 0;
                        p.state = P_STATE_IDLE;
                    } else {
                        p.state = P_STATE_JUMP;
                    }
                }
            }

            if (p.state >= P_STATE_ATTACK_LIGHT && p.state != P_STATE_DEATH && !p.hitDone) {
                if (p.stateTimer < 0.2) {
                    int atkW = 60, atkH = 40, atkOffX = 20;
                    int hitboxX = (p.dir == 1) ? (p.x + atkOffX) : (p.x - atkOffX - atkW);
                    int hitboxY = p.y - 40;
                    int anyHit = 0;

                    if (enemies) {
                        for (int i = 0; i < totalEnemies; i++) {
                            if (!enemies[i].active || enemies[i].isDead) continue;
                            if (CheckCollision(hitboxX, hitboxY, atkW, atkH, (int)enemies[i].x - enemies[i].w / 2, (int)enemies[i].y - enemies[i].h, enemies[i].w, enemies[i].h)) {
                                int dmg = (p.state == P_STATE_ATTACK_HEAVY) ? 25 : 10;
                                enemies[i].hp -= dmg;
                                if (enemies[i].hp <= 0) {
                                    enemies[i].isDead = 1;
                                    enemies[i].deathTimer = 0;
                                    enemies[i].state = E_STATE_STUNNED;
                                } else {
                                    enemies[i].stunTimer = 0.5;
                                    enemies[i].state = E_STATE_STUNNED;
                                }
                                p.score += 100 * p.multiplier;
                                anyHit = 1;
                            }
                        }
                    }
                    if (anyHit) {
                        p.hitDone = 1;
                        p.multiplier++;
                        p.multiplierTimer = 2.0;
                    }
                }
            }

            if (p.hitTimer <= 0 && enemies) {
                for (int i = 0; i < totalEnemies; i++) {
                    if (enemies[i].active && !enemies[i].isDead && enemies[i].state != E_STATE_STUNNED) {
                        if (fabs(p.x - enemies[i].x) < 60 && fabs(p.y - enemies[i].y) < 20) {
                            p.currentHp -= 10;
                            p.hitTimer = 1.0;
                            if (p.currentHp <= 0) {
                                p.state = P_STATE_DEATH;
                                globalState = STATE_GAMEOVER;
                                scoreSaved = 0;
                            }
                        }
                    }
                }
            }

            int allDead = 1;
            if (enemies) {
                for (int i = 0; i < totalEnemies; i++) {
                    if (enemies[i].active) {
                        allDead = 0;
                        break;
                    }
                }
            }

            if (allDead && CheckCollision((int)p.x, (int)p.y - 40, 40, 80, exitDoor.x, exitDoor.y, exitDoor.w, exitDoor.h)) {
                levelCount++;
                p.x = 50;
                doorTimer = 0;
                if (!LoadStageFromFile(levelCount, &enemies, &obstacles)) {
                    levelCount = 1;
                    LoadStageFromFile(1, &enemies, &obstacles);
                }
            }

            if (p.x < 32) p.x = 32;
            if (p.x > currentLevelWidth - 32) p.x = currentLevelWidth - 32;
            if (p.y < FLOOR_TOP_LIMIT) p.y = FLOOR_TOP_LIMIT;
            if (p.y > FLOOR_BOTTOM_LIMIT) p.y = FLOOR_BOTTOM_LIMIT;

            camera_x = p.x - SCREEN_WIDTH / 2;
            if (camera_x < 0) camera_x = 0;
            if (camera_x > currentLevelWidth - SCREEN_WIDTH) camera_x = currentLevelWidth - SCREEN_WIDTH;

            SDL_Rect bgRect = { (int)-camera_x, 0, currentLevelWidth, SCREEN_HEIGHT };
            if (bgSurf) SDL_BlitSurface(bgSurf, NULL, screen, &bgRect);
            else DrawRectangle(screen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, bg_blue, bg_blue);

            if (obstacles) {
                for (int i = 0; i < totalObstacles; i++) {
                    int obScreenX = obstacles[i].x - (int)camera_x;
                    if (obScreenX + obstacles[i].w > 0 && obScreenX < SCREEN_WIDTH) {
                        DrawRectangle(screen, obScreenX, obstacles[i].y, obstacles[i].w, obstacles[i].h, black, grey);
                        DrawRectangle(screen, obScreenX, obstacles[i].y, obstacles[i].w, 1, black, black);
                        DrawRectangle(screen, obScreenX, obstacles[i].y + obstacles[i].h - 1, obstacles[i].w, 1, black, black);
                        DrawRectangle(screen, obScreenX, obstacles[i].y, 1, obstacles[i].h, black, black);
                        DrawRectangle(screen, obScreenX + obstacles[i].w - 1, obstacles[i].y, 1, obstacles[i].h, black, black);

                        if (showDevMode) {
                            DrawRectangle(screen, obScreenX, obstacles[i].y, obstacles[i].w, 1, red, red);
                            DrawRectangle(screen, obScreenX, obstacles[i].y + obstacles[i].h, obstacles[i].w, 1, red, red);
                            DrawRectangle(screen, obScreenX, obstacles[i].y, 1, obstacles[i].h, red, red);
                            DrawRectangle(screen, obScreenX + obstacles[i].w, obstacles[i].y, 1, obstacles[i].h, red, red);
                        }
                    }
                }
            }

            int doorDrawX = exitDoor.x - (int)camera_x;
            if (doorDrawX > -100 && doorDrawX < SCREEN_WIDTH + 100) {
                int doorFrame = 0;
                if (!allDead) {
                    doorFrame = 0;
                    doorTimer = 0;
                } else {
                    doorTimer += delta;
                    if (doorTimer < 0.2) doorFrame = 0;
                    else if (doorTimer < 0.4) doorFrame = 1;
                    else doorFrame = 2;
                }

                if (doorSurf) {
                    DrawSpriteFrame(screen, doorSurf, doorDrawX + 50, exitDoor.y + 100, doorFrame, 0, 100, 100, 1);
                } else {
                    Uint32 dColor = (doorFrame == 2) ? green : red;
                    DrawRectangle(screen, doorDrawX, exitDoor.y, exitDoor.w, exitDoor.h, black, dColor);
                }
            }

            if (enemies) {
                for (int i = 0; i < totalEnemies; i++) {
                    if (!enemies[i].active) continue;
                    int ex = (int)(enemies[i].x - camera_x);
                    SDL_Surface* eSprite = (enemies[i].type == ENEMY_TYPE_FOLLOWER) ? enemy1Surf : enemy2Surf;
                    int eRow = 0, maxFrames = 7;

                    if (enemies[i].isDead) { eRow = 3; maxFrames = 5; }
                    else if (enemies[i].state == E_STATE_STUNNED) { eRow = 2; maxFrames = (enemies[i].type == ENEMY_TYPE_FOLLOWER) ? 2 : 3; }
                    else if (enemies[i].state == E_STATE_CHARGING || enemies[i].state == E_STATE_PREP_CHARGE || enemies[i].state == E_STATE_ATTACK) { eRow = 1; maxFrames = 5; }
                    else if (enemies[i].state == E_STATE_MOVE) { eRow = 0; maxFrames = 7; }
                    else { eRow = 0; maxFrames = 1; }

                    int eCol = (int)(enemies[i].animTimer * 10.0);
                    if (enemies[i].isDead || enemies[i].state == E_STATE_ATTACK) {
                        if (eCol >= maxFrames) eCol = maxFrames - 1;
                    } else {
                        eCol = eCol % maxFrames;
                    }

                    DrawSpriteFrame(screen, eSprite, ex, (int)enemies[i].y + VISUAL_OFFSET_Y, eCol, eRow, FRAME_W, FRAME_H, enemies[i].dir);
                }
            }

            int row = 0, col = 0;
            if (p.state == P_STATE_JUMP || p.z > 0) { row = 0; col = 4 - (int)(p.vz / 140.0); if (col < 0) col = 0; if (col > 8) col = 8; }
            else if (p.state == P_STATE_WALK) { row = 1; col = (int)(worldTime * 12) % 9; }
            else if (p.state == P_STATE_ATTACK_LIGHT) { row = 2; double pr = (0.3 - p.stateTimer) / 0.3; col = (int)(pr * 3); if (col > 2) col = 2; }
            else if (p.state == P_STATE_ATTACK_HEAVY) { row = 3; double pr = (0.6 - p.stateTimer) / 0.6; col = (int)(pr * 4); if (col > 3) col = 3; }
            else if (p.state == P_STATE_COMBO_TRIPLE) { row = 2; col = (int)(worldTime * 25) % 3; }
            else if (p.state == P_STATE_COMBO_MIXED) { row = 3; col = (int)(worldTime * 25) % 4; }
            else if (p.state == P_STATE_DEATH) { row = 4; double pr = (1.0 - p.stateTimer) / 1.0; col = (int)(pr * 4); if (col > 3) col = 3; }
            else { row = 1; col = 0; }


            if (p.hitTimer > 0 && ((int)(worldTime * 10) % 2 == 0)) {}
            else DrawSpriteFrame(screen, playerSprite, (int)(p.x - camera_x), (int)(p.y - p.z) + VISUAL_OFFSET_Y, col, row, FRAME_W, FRAME_H, p.dir);

            if (showDevMode) {
                int px = (int)(p.x - camera_x);
                int py = (int)p.y;
                DrawRectangle(screen, px - 10, py - 10, 20, 10, green, green);
            }

            DrawRectangle(screen, 0, 0, SCREEN_WIDTH, UI_HEIGHT, blue, blue);
            char bufferText[128];
            sprintf(bufferText, "Score: %d | Level: %d | %s", p.score, levelCount, playerName);
            DrawString(screen, 10, 4, bufferText, charset);
            DrawString(screen, 10, 20, "HP:", charset);
            DrawRectangle(screen, 40, 20, 100, 8, black, red);
            int hpWidth = (int)((float)p.currentHp / MAX_HP * 100);
            if (hpWidth < 0) hpWidth = 0;
            DrawRectangle(screen, 40, 20, hpWidth, 8, black, green);

            if (p.multiplier > 1) {
                char multiStr[32];
                sprintf(multiStr, "COMBO x%d!", p.multiplier);
                DrawString(screen, 160, 20, multiStr, charset);
            }

            if (showDevMode) {
                DrawRectangle(screen, 0, SCREEN_HEIGHT - 60, 220, 60, black, black);
                char devState[64];
                sprintf(devState, "State: %s (Time: %.2f)", GetStateName(p.state), p.stateTimer);
                DrawString(screen, 10, SCREEN_HEIGHT - 50, devState, charset);
                DrawString(screen, 10, SCREEN_HEIGHT - 35, "Buff:", charset);
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    if (inputBuffer[i].key != 0) {
                        const char* kName = SDL_GetKeyName(inputBuffer[i].key);
                        char shortN[2] = { kName[0], '\0' };
                        DrawString(screen, 50 + (i * 10), SCREEN_HEIGHT - 35, shortN, charset);
                    }
                }
            }
        }
        else if (globalState == STATE_GAMEOVER) {
            if (!scoreSaved) {
                SaveScore(playerName, p.score, worldTime);
                scoreSaved = 1;
            }
            DrawString(screen, SCREEN_WIDTH / 2 - 40, 150, "GAME OVER", charset);
            char scoreMsg[64];
            sprintf(scoreMsg, "Final Score: %d", p.score);
            DrawString(screen, SCREEN_WIDTH / 2 - 60, 180, scoreMsg, charset);
            DrawString(screen, SCREEN_WIDTH / 2 - 80, 220, "Press ENTER to Menu", charset);
            DrawString(screen, SCREEN_WIDTH / 2 - 80, 240, "Press ESC to Quit", charset);
        }

        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) quit = 1;
            else if (event.type == SDL_KEYDOWN) {

                if (event.key.keysym.sym == SDLK_i) {
                    showDevMode = !showDevMode;
                }

                if (globalState == STATE_MENU) {
                    if (event.key.keysym.sym == SDLK_DOWN) {
                        menuOption++;
                        if (menuOption > 2) menuOption = 0;
                    }
                    if (event.key.keysym.sym == SDLK_UP) {
                        menuOption--;
                        if (menuOption < 0) menuOption = 2;
                    }
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        if (menuOption == 0) globalState = STATE_NAME_INPUT;
                        else if (menuOption == 1) {
                            globalState = STATE_RESULTS;
                            resultsScrollOffset = 0;
                        }
                        else if (menuOption == 2) quit = 1;
                    }
                }
                else if (globalState == STATE_NAME_INPUT) {
                    SDL_Keycode k = event.key.keysym.sym;
                    int len = strlen(playerName);
                    if (k == SDLK_BACKSPACE && len > 0) playerName[len - 1] = '\0';
                    else if (k == SDLK_RETURN && len > 0) {
                        globalState = STATE_GAME;
                        p.currentHp = MAX_HP;
                        p.x = 100;
                        p.score = 0;
                        levelCount = 1;
                        p.state = P_STATE_IDLE;
                        LoadStageFromFile(1, &enemies, &obstacles);
                    }
                    else if (len < 20) {
                        if (k >= SDLK_a && k <= SDLK_z) { playerName[len] = (char)k; playerName[len + 1] = '\0'; }
                        if (k >= SDLK_0 && k <= SDLK_9) { playerName[len] = (char)k; playerName[len + 1] = '\0'; }
                        if (k == SDLK_SPACE) { playerName[len] = ' '; playerName[len + 1] = '\0'; }
                    }
                }
                else if (globalState == STATE_RESULTS) {
                    if (event.key.keysym.sym == SDLK_ESCAPE) globalState = STATE_MENU;
                    if (event.key.keysym.sym == SDLK_DOWN) {
                        if (resultsScrollOffset + 5 < totalScoresLoaded) resultsScrollOffset++;
                    }
                    if (event.key.keysym.sym == SDLK_UP) {
                        if (resultsScrollOffset > 0) resultsScrollOffset--;
                    }
                }
                else if (globalState == STATE_GAME) {
                    if (event.key.keysym.sym == SDLK_ESCAPE) globalState = STATE_MENU;
                    AddInput(event.key.keysym.sym, worldTime);
                    if (p.state == P_STATE_IDLE || p.state == P_STATE_WALK) {
                        if (event.key.keysym.sym == SDLK_SPACE && p.z == 0) {
                            p.state = P_STATE_JUMP;
                            p.vz = JUMP_FORCE;
                        }
                        else if (event.key.keysym.sym == SDLK_z) {
                            if (p.state == P_STATE_ATTACK_LIGHT && p.stateTimer < 0.1) {
                                p.state = P_STATE_COMBO_TRIPLE;
                                p.stateTimer = 0.4;
                                p.hitDone = 0;
                            } else if (p.state < P_STATE_ATTACK_LIGHT) {
                                p.state = P_STATE_ATTACK_LIGHT;
                                p.stateTimer = 0.3;
                                p.hitDone = 0;
                            }
                        }
                        else if (event.key.keysym.sym == SDLK_x) {
                            if (p.state == P_STATE_ATTACK_LIGHT && p.stateTimer < 0.2) {
                                p.state = P_STATE_COMBO_MIXED;
                                p.stateTimer = 0.5;
                                p.hitDone = 0;
                            } else if (p.state < P_STATE_ATTACK_LIGHT) {
                                p.state = P_STATE_ATTACK_HEAVY;
                                p.stateTimer = 0.6;
                                p.hitDone = 0;
                            }
                        }
                    }
                }
                else if (globalState == STATE_GAMEOVER) {
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        globalState = STATE_MENU;
                        p.currentHp = MAX_HP;
                        p.x = 100;
                        p.score = 0;
                        levelCount = 1;
                        p.state = P_STATE_IDLE;
                    }
                    if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
                }
            }
        }
        frames++;
    };

    SDL_FreeSurface(charset);
    SDL_FreeSurface(playerSprite);
    SDL_FreeSurface(bgSurf);
    SDL_FreeSurface(enemy1Surf);
    SDL_FreeSurface(enemy2Surf);
    SDL_FreeSurface(doorSurf);
    FreeStageMemory(&enemies, &obstacles);
    SDL_DestroyTexture(scrtex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}