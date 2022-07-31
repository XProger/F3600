#ifndef _GAME_H_INCLUDED_
#define _GAME_H_INCLUDED_

#include "standard.h"
#include "tim.h"
#include "object.h"
#include "render.h"
#include "file.h"
#include "skeleton.h"
#include "track.h"
#include "ttf.h"
#include "ships.h"
#include "weapon.h"
#include "droid.h"
#include "camera.h"
#include "draw.h"
#include "scene.h"
#include "sound.h"
#include "control.h"
#include "menus.h"
#include "hiscores.h"
#include "hud.h"
#include "global.h"
#include "sfx.h"
#include "utils.h"
#include "race.h"
#include "search.h"
#include "dynam.h"
#include "collide.h"
#include "cone.h"
#include "effects.h"
#include "init.h"
#include "radscn.h"
#include "lzss.h"
#include "spline.h"
#include "credits.h"
#include "options.h"

void gameInit(void);
void gameCtrl(void);

#endif