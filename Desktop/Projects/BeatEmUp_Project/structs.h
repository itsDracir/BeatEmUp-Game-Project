#ifndef STRUCTS_H
#define STRUCTS_H

#include "defs.h"
extern "C" {
#include "./sdl/include/SDL.h"
}

// --- ENUMS ---
typedef enum {
    STATE_MENU, STATE_NAME_INPUT, STATE_GAME, STATE_GAMEOVER, STATE_RESULTS
} GameState;

typedef enum {
    P_STATE_IDLE, P_STATE_WALK, P_STATE_JUMP, P_STATE_DASH,
    P_STATE_ATTACK_LIGHT, P_STATE_ATTACK_HEAVY,
    P_STATE_COMBO_TRIPLE, P_STATE_COMBO_MIXED, P_STATE_DEATH
} PlayerAnimState;

typedef enum { ENEMY_TYPE_FOLLOWER, ENEMY_TYPE_CHARGER } EnemyType;
typedef enum { 
    E_STATE_IDLE, E_STATE_MOVE, E_STATE_PREP_CHARGE, 
    E_STATE_CHARGING, E_STATE_ATTACK, E_STATE_STUNNED 
} EnemyState;

// --- STRUCTS ---
typedef struct {
    SDL_Keycode key;
    double timestamp;
} InputEvent;

typedef struct {
    double x, y, z, vz;
    int dir;
    PlayerAnimState state;
    double stateTimer, animTimer;
    int score, multiplier;
    double multiplierTimer;
    int currentHp, maxHp;
    double hitTimer;
    int hitDone; 
} Player;

typedef struct {
    int active;
    EnemyType type;
    EnemyState state;
    EnemyState lastState;
    double x, y, vx, vy;
    int w, h, dir, hp;
    double stunTimer, actionTimer, animTimer;
    int isDead;
    double deathTimer;
} Enemy;

// [NUEVO] Estructura para obstaculos (cajas, piedras, limites)
typedef struct {
    int x, y, w, h;
} Obstacle;

typedef struct {
    char name[21];
    int score;
    double time;
} HighScore;

#endif