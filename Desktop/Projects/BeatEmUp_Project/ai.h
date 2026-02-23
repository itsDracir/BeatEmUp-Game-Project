#ifndef AI_H
#define AI_H

#include "structs.h"

extern int currentLevelWidth;
extern SDL_Rect exitDoor;
extern int totalEnemies;
extern int totalObstacles;

int CheckCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
void SaveScore(const char* name, int score, double time);
int CompareScores(const void* a, const void* b);
int LoadStageFromFile(int levelNum, Enemy** enemies, Obstacle** obstacles);
void FreeStageMemory(Enemy** enemies, Obstacle** obstacles);

void UpdateEnemyAI(Enemy* e, Player* p, double delta);

#endif