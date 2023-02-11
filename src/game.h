#ifndef _GAME_H_INCLUDED_
#define _GAME_H_INCLUDED_

#include "standard.h"
#include "lang.h"
#include "render.h"
#include "track.h"
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

#ifdef __cplusplus
extern "C" {
#endif

void gameInit(void);
void gameFree(void);
void gameCtrl(void);

#ifdef __cplusplus
}
#endif

#endif
