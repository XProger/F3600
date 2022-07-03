#ifndef _GAME_H_INCLUDED_
#define _GAME_H_INCLUDED_

#include "standard.h"
#include "input.h"
#include "tim.h"
#include "object.h"
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
#include "fileops.h"
#include "sound.h"
#include "control.h"
#include "menus.h"
#include "hiscores.h"
#include "hud.h"
#include "global.h"
#include "decl.h"
#include "sfx.h"
#include "card.h"
#include "utils.h"
#include "error.h"
#include "render.h"
#include "race.h"

void gameInit();
void gameCtrl();

#endif