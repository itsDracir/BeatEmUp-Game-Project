#include "ai.h"

int currentLevelWidth = 2000;
SDL_Rect exitDoor = { 1900, 380, 60, 100 };
int totalEnemies = 0;
int totalObstacles = 0;

int CheckCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

void SaveScore(const char* name, int score, double time) {
    FILE* f = fopen("scores.txt", "a");
    if (f != NULL) {
        fprintf(f, "%s %d %.1lf\n", name, score, time);
        fclose(f);
    }
}

int CompareScores(const void* a, const void* b) {
    HighScore* h1 = (HighScore*)a;
    HighScore* h2 = (HighScore*)b;
    return h2->score - h1->score;
}

void FreeStageMemory(Enemy** enemies, Obstacle** obstacles) {
    if (*enemies) {
        free(*enemies);
        *enemies = NULL;
    }
    if (*obstacles) {
        free(*obstacles);
        *obstacles = NULL;
    }
    totalEnemies = 0;
    totalObstacles = 0;
}

int LoadStageFromFile(int levelNum, Enemy** enemies, Obstacle** obstacles) {
    FreeStageMemory(enemies, obstacles);

    char filename[32];
    sprintf(filename, "stage%d.txt", levelNum);
    FILE* f = fopen(filename, "r");
    
    if (!f) {
        printf("Error: No se pudo abrir %s\n", filename);
        return 0; 
    }

    if (fscanf(f, "%d", &currentLevelWidth) != 1) goto error;
    if (fscanf(f, "%d %d %d %d", &exitDoor.x, &exitDoor.y, &exitDoor.w, &exitDoor.h) != 4) goto error;

    if (fscanf(f, "%d", &totalObstacles) != 1) goto error;
    
    if (totalObstacles > 0) {
        *obstacles = (Obstacle*)malloc(sizeof(Obstacle) * totalObstacles);
        for(int i=0; i<totalObstacles; i++) {
            if (fscanf(f, "%d %d %d %d", 
                &(*obstacles)[i].x, &(*obstacles)[i].y, 
                &(*obstacles)[i].w, &(*obstacles)[i].h) != 4) goto error;
        }
    }

    if (fscanf(f, "%d", &totalEnemies) != 1) goto error;

    if (totalEnemies > 0) {
        *enemies = (Enemy*)malloc(sizeof(Enemy) * totalEnemies);
        for(int i=0; i<totalEnemies; i++) {
            int type;
            double ex, ey;
            if (fscanf(f, "%d %lf %lf", &type, &ex, &ey) != 3) goto error;

            Enemy* e = &(*enemies)[i];
            e->active = 1;
            e->type = (EnemyType)type;
            e->x = ex; e->y = ey;
            e->vx = 0; e->vy = 0;
            e->state = E_STATE_IDLE; e->lastState = E_STATE_IDLE;
            e->stunTimer = 0; e->actionTimer = 0; e->animTimer = 0;
            e->isDead = 0; e->deathTimer = 0;

            if (e->type == ENEMY_TYPE_FOLLOWER) { e->w = 40; e->h = 80; e->hp = 100; }
            else { e->w = 50; e->h = 80; e->hp = 150; }
        }
    }

    fclose(f);
    printf("Stage %d loaded. Width: %d, Enemies: %d, Obstacles: %d\n", levelNum, currentLevelWidth, totalEnemies, totalObstacles);
    return 1; 

error:
    printf("Error: Formato de archivo invalido en %s\n", filename);
    fclose(f);
    FreeStageMemory(enemies, obstacles); 
    return 0;
}

void UpdateEnemyAI(Enemy* e, Player* p, double delta) {
    if (!e->active) return;
    
    if (e->isDead) {
        e->deathTimer += delta;
        if (e->deathTimer > 0.6) {
            e->active = 0; 
            p->score += 500; 
        }
        return; 
    }

    if (e->stunTimer > 0) { 
        e->stunTimer -= delta; 
        e->state = E_STATE_STUNNED; 
        return; 
    } else if (e->state == E_STATE_STUNNED) {
        e->state = E_STATE_IDLE;
    }

    if (e->state != e->lastState) {
        e->animTimer = 0;
        e->lastState = e->state;
    }
    e->animTimer += delta;

    if (e->state == E_STATE_ATTACK) {
        e->vx = 0; e->vy = 0; 
        e->actionTimer -= delta;
        if (e->actionTimer <= 0) e->state = E_STATE_IDLE;
        return; 
    }

    double dx = p->x - e->x;
    double dy = p->y - e->y;
    double dist = sqrt(dx*dx + dy*dy);
    
    if (dx > 0) e->dir = 1; else e->dir = -1;

    if (e->type == ENEMY_TYPE_FOLLOWER) {
        if (dist <= 45) {
            e->state = E_STATE_ATTACK;
            e->actionTimer = 0.5; 
        }
        else { 
            e->state = E_STATE_MOVE;
            e->vx = (dx / dist) * ENEMY_SPEED_FOLLOW;
            e->vy = (dy / dist) * ENEMY_SPEED_FOLLOW;
        }
    }
    else if (e->type == ENEMY_TYPE_CHARGER) {
        switch (e->state) {
            case E_STATE_IDLE: case E_STATE_MOVE: {
                double targetX = p->x + ((p->x > e->x) ? -300 : 300);
                if (targetX < 50) targetX = 50; 
                if (targetX > currentLevelWidth-50) targetX = currentLevelWidth-50;

                double diffX = targetX - e->x;
                e->vy = (fabs(dy) > 10) ? ((dy > 0 ? 1 : -1) * ENEMY_SPEED_ALIGN) : 0;
                e->vx = (fabs(diffX) > 10) ? ((diffX > 0 ? 1 : -1) * ENEMY_SPEED_ALIGN * 0.5) : 0;
                
                if (fabs(dy) < 20 && fabs(diffX) < 50) {
                    e->actionTimer += delta;
                    if (e->actionTimer > 1.5) { e->state = E_STATE_PREP_CHARGE; e->actionTimer = 0.5; }
                } else e->actionTimer = 0;
            } break;
            case E_STATE_PREP_CHARGE:
                e->vx = 0; e->vy = 0; e->actionTimer -= delta;
                if (e->actionTimer <= 0) { e->state = E_STATE_CHARGING; e->vx = (p->x > e->x ? 1 : -1) * ENEMY_SPEED_CHARGE; e->actionTimer = 1.0; }
                break;
            case E_STATE_CHARGING:
                e->vy = 0; 
                if (e->x <= 60 || e->x >= currentLevelWidth - 60) {
                    if (e->x <= 60) e->x = 120; else e->x = currentLevelWidth - 120;
                    e->state = E_STATE_STUNNED; e->stunTimer = 2.0; e->vx = 0; break;
                }
                e->actionTimer -= delta;
                if (e->actionTimer <= 0) { e->state = E_STATE_IDLE; e->vx = 0; }
                break;
            default: break;
        }
    }
    
    e->x += e->vx * delta; 
    e->y += e->vy * delta;
    
    if (e->y < FLOOR_TOP_LIMIT) e->y = FLOOR_TOP_LIMIT; 
    if (e->y > FLOOR_BOTTOM_LIMIT) e->y = FLOOR_BOTTOM_LIMIT;
    if (e->x < 32) e->x = 32; 
    if (e->x > currentLevelWidth - 32) e->x = currentLevelWidth - 32;
}