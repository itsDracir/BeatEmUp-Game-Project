#ifndef DEFS_H
#define DEFS_H

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SCREEN_WIDTH     640
#define SCREEN_HEIGHT    480
#define UI_HEIGHT        40
#define FLOOR_TOP_LIMIT  (UI_HEIGHT + 50)
#define FLOOR_BOTTOM_LIMIT (SCREEN_HEIGHT - 20)
#define MAX_HP           100

#define FRAME_W          128
#define FRAME_H          128

#define PLAYER_SPEED     300.0
#define DASH_SPEED       800.0
#define GRAVITY          1500.0
#define JUMP_FORCE       600.0

#define ENEMY_SPEED_FOLLOW  150.0
#define ENEMY_SPEED_ALIGN   180.0

#define ENEMY_SPEED_CHARGE  600.0

#define BUFFER_SIZE      10
#define COMBO_WINDOW     0.4
#define VISUAL_OFFSET_Y  20

#define DOOR_FRAME_W     100
#define DOOR_FRAME_H     100

#endif