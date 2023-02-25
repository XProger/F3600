#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/input.h>
#include <X11/Xutil.h>
#include <linux/joystick.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <pthread.h>
#include <dirent.h>

#include <pulse/pulseaudio.h>
#include <pulse/simple.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>

#include "game.h"

int32 isQuit;

Display* dpy;
Window wnd;

// timing
unsigned int gTimerStart;

int32 osGetSystemTimeMS()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (int)(t.tv_sec * 1000 + t.tv_usec / 1000 - gTimerStart);
}

void osQuit()
{
    isQuit = 1;
}

#define WND_TITLE   "F3600"
#define WND_WIDTH   854
#define WND_HEIGHT  480

// input
#define JOY_DEAD_ZONE_STICK     8192
#define JOY_DEAD_ZONE_TRIGGER   8192
#define JOY_MIN_UPDATE_FX_TIME  50
#define JOY_RUMBLE_FADE         16

struct JoyDevice
{
    int32 fd;     // device file descriptor
    int32 fe;     // event file descriptor
    int32 Lx, Ly; // left stick axes values
    int32 Rx, Ry; // right stick axes values
    int32 vL, vR; // current value for left/right motor vibration
    int32 oL, oR; // last applied value
    int32 time;   // time when we can send effect update
    struct ff_effect fx; // effect structure
    uint8_t axismap[ABS_CNT]; // axis mapping
} joyDevice[MAX_PLAYERS];

int32 osJoyReady(int index)
{
    return joyDevice[index].fd != -1;
}

void osJoyVibrate(int32 index, int32 L, int32 R)
{
    if (!osJoyReady(index))
        return;
    joyDevice[index].vL = L;
    joyDevice[index].vR = R;
}

int32 joyAxis(int32 value)
{
    if (abs(value) < JOY_DEAD_ZONE_STICK)
        return 0;
    return value;
}

void joyRumble(struct JoyDevice* joy)
{
    struct input_event event;

    if (joy->fe == -1)
        return;

    if ((joy->oL | joy->vL | joy->oR | joy->vR) == 0)
        return;

    if (osGetSystemTimeMS() <= joy->time)
        return;

    event.type = EV_FF;

    if (joy->vL != 0 || joy->vR != 0)
    {
        // update effect
        joy->fx.u.rumble.strong_magnitude = joy->vL << 8;
        joy->fx.u.rumble.weak_magnitude = joy->vR << 8;
        joy->fx.replay.length = JOY_MIN_UPDATE_FX_TIME;

        if (ioctl(joy->fe, EVIOCSFF, &joy->fx) == -1)
        {
            LOG("! joy update fx\n");
        }

        // play effect
        event.value = 1;
        event.code = joy->fx.id;
        if (write(joy->fe, &event, sizeof(event)) == -1)
        {
            LOG("! joy play fx\n");
        }
    }
    else if (joy->oL != 0 || joy->oR != 0)
    {
        // stop effect
        event.value = 0;
        event.code = joy->fx.id;
        if (write(joy->fe, &event, sizeof(event)) == -1)
        {
            LOG("! joy stop fx\n");
        }
    }

    joy->oL = joy->vL;
    joy->oR = joy->vR;
    joy->vL -= JOY_RUMBLE_FADE;
    joy->vR -= JOY_RUMBLE_FADE;
    if (joy->vL < 0)
        joy->vL = 0;
    if (joy->vR < 0)
        joy->vR = 0;

    joy->time = osGetSystemTimeMS() + joy->fx.replay.length;
}

#define TEST_BIT(arr, bit) ((arr[bit / 32] >> (bit % 32)) & 1)

void inputInit()
{
    LOG("init gamepads:\n");

    char name[128];
    for (int32 i = 0; i < MAX_PLAYERS; i++)
    {
        struct js_event event;
        struct JoyDevice* joy = joyDevice + i;

        // open device
        sprintf(name, "/dev/input/js%d", i);
        joy->fd = open(name, O_RDONLY | O_NONBLOCK);
        if (joy->fd == -1)
            continue;

        // skip init messages
        while (read(joy->fd, &event, sizeof(event)) != -1 && (event.type & JS_EVENT_INIT)) {};

        // get gamepad info
        int8 axes, buttons;
        ioctl(joy->fd, JSIOCGAXES, &axes);
        ioctl(joy->fd, JSIOCGBUTTONS, &buttons);
        ioctl(joy->fd, JSIOCGAXMAP, joy->axismap);

        if (axes < 4 || buttons < 11) // is it really a gamepad?
        {
            close(joy->fd);
            joy->fd = -1;
            continue;
        }

        if (ioctl(joy->fd, JSIOCGNAME(sizeof(name)), name) < 0)
        {
            strcpy(name, "Unknown");
        }

        LOG("gamepad %d\n", i + 1);
        LOG(" name : %s\n", name);
        LOG(" btns : %d\n", (int)buttons);
        LOG(" axes : %d\n", (int)axes);

        joy->fe = -1;
        for (int j = 0; j < 99; j++)
        {
            sprintf(name, "/sys/class/input/js%d/device/event%d", i, j);
            DIR* dir = opendir(name);
            if (!dir)
                continue;

            closedir(dir);
            sprintf(name, "/dev/input/event%d", j);
            joy->fe = open(name, O_RDWR);
            break;
        }

        uint32 features[4];
        if (joy->fe > -1 && (ioctl(joy->fe, EVIOCGBIT(EV_FF, sizeof(features)), features) == -1 || !TEST_BIT(features, FF_RUMBLE)))
        {
            close(joy->fe);
            joy->fe = -1;
        }

        if (joy->fe > -1)
        {
            int n_effects;
            if (ioctl(joy->fe, EVIOCGEFFECTS, &n_effects) != -1)
            {
                LOG(" vibration feature %d\n", n_effects);
                joy->fx.id = -1;
                joy->fx.type = FF_RUMBLE;
                joy->fx.replay.delay = 0;
                joy->vL = joy->oL = joy->vR = joy->oR = 0.0f;
                joy->time = osGetSystemTimeMS();
            }
        }
    }
}

void inputFree()
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        struct JoyDevice* joy = joyDevice + i;
        if (joy->fd == -1) continue;
        close(joy->fd);
        if (joy->fe == -1) continue;
    }
}

void inputReset()
{
    int32 i;

    for (i = 0; i < MAX_PLAYERS; i++)
    {
        padReset(i);
    }
}

void inputUpdate()
{
    static const int32 buttons[] = {
        IN_A, IN_B, IN_X, IN_Y, IN_LB, IN_RB, IN_SELECT, IN_START, IN_NONE, IN_L, IN_R
    };

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        struct js_event event;
        struct JoyDevice* joy = joyDevice + i;

        if (joy->fd == -1)
            continue;

        joyRumble(joy);

        while (read(joy->fd, &event, sizeof(event)) != -1)
        {
        // buttons
            if (event.type & JS_EVENT_BUTTON)
            {
                if (event.number >= 0 && event.number < COUNT(buttons))
                {
                    padSetKey(i, buttons[event.number], event.value == 1);
                }
            }

        // axes
            if (event.type & JS_EVENT_AXIS)
            {
                switch (joy->axismap[event.number])
                {
                // Left stick
                    case ABS_X:
                        joy->Lx = joyAxis(event.value);
                        break;
                    case ABS_Y:
                        joy->Ly = joyAxis(event.value);
                        break;
                // Right stick
                    case ABS_RX:
                        joy->Rx = joyAxis(event.value);
                        break;
                    case ABS_RY:
                        joy->Ry = joyAxis(event.value);
                        break;
                // Left trigger
                    case ABS_Z:
                        padSetKey(i, IN_LT, event.value > 0);
                        break;
                // Right trigger
                    case ABS_RZ:
                        padSetKey(i, IN_RT, event.value > 0);
                        break;
                // D-PAD
                    case ABS_HAT0X:
                    case ABS_THROTTLE:
                        padSetKey(i, IN_LEFT,  event.value < -0x4000);
                        padSetKey(i, IN_RIGHT, event.value >  0x4000);
                        break;
                    case ABS_HAT0Y:
                    case ABS_RUDDER:
                        padSetKey(i, IN_UP,    event.value < -0x4000);
                        padSetKey(i, IN_DOWN,  event.value >  0x4000);
                        break;
                }
            }
        }

        padSetStick(i, IN_L, joy->Lx, joy->Ly);
        padSetStick(i, IN_R, joy->Rx, joy->Ry);
    }
}

int32 keyToInputKey(Display *dpy, XKeyEvent event)
{
    static const struct {
        KeySym code;
        int32 key;
    } keyMapping[] = {
        { XK_A,         IN_LT },
        { XK_D,         IN_RT },
        { XK_Q,         IN_LB },
        { XK_E,         IN_RB },
        { XK_W,         IN_A },
        { XK_Return,    IN_A },
        { XK_space,     IN_B },
        { XK_Tab,       IN_Y },
        { XK_Escape,    IN_START },
        { XK_Left,      IN_LEFT },
        { XK_Right,     IN_RIGHT },
        { XK_Up,        IN_UP },
        { XK_Down,      IN_DOWN }
    };

    for (int i = 0; i < COUNT(keyMapping); i++)
    {
        if (XKeysymToKeycode(dpy, keyMapping[i].code) == event.keycode)
        {
            return keyMapping[i].key;
        }
    }

    return IN_NONE;
}


// sound
#define SND_DATA_SIZE   (SND_SAMPLES * sizeof(int16))

pa_simple *sndOut;
pthread_t sndThread;

int16 *sndData;

void* soundFill(void* arg)
{
    while (1)
    {
        FillNotes(sndData, SND_SAMPLES / 2);
        pa_simple_write(sndOut, sndData, SND_DATA_SIZE, NULL);
    }
    return NULL;
}

void soundInit()
{
    static const pa_sample_spec spec = {
        .format   = PA_SAMPLE_S16LE,
        .rate     = 44100,
        .channels = 2
    };

    static const pa_buffer_attr attr = {
        .maxlength  = SND_DATA_SIZE * 4,
        .tlength    = 0xFFFFFFFF,
        .prebuf     = 0xFFFFFFFF,
        .minreq     = SND_DATA_SIZE,
        .fragsize   = 0xFFFFFFFF,
    };

    int error;
    if (!(sndOut = pa_simple_new(NULL, WND_TITLE, PA_STREAM_PLAYBACK, NULL, "game", &spec, NULL, &attr, &error)))
    {
        LOG("pa_simple_new() failed: %s\n", pa_strerror(error));
        sndData = NULL;
        return;
    }

    sndData = (int16*)malloc(SND_DATA_SIZE);
    pthread_create(&sndThread, NULL, soundFill, NULL);
}

void soundFree()
{
    if (sndOut) {
        pthread_cancel(sndThread);
    //    pa_simple_flush(sndOut, NULL);
    //    pa_simple_free(sndOut);
        free(sndData);
    }
}

void toggle_fullscreen(Display* dpy, Window win)
{
    const size_t _NET_WM_STATE_TOGGLE = 2;

    XEvent xev;
    Atom wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
    Atom scr_full = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);

    memset(&xev, 0, sizeof(xev));
    xev.type = ClientMessage;
    xev.xclient.window = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = _NET_WM_STATE_TOGGLE;
    xev.xclient.data.l[1] = scr_full;

    XSendEvent(dpy, DefaultRootWindow(dpy), False, SubstructureNotifyMask, &xev);
}

void WndProc(XEvent* e, Display* dpy, Window wnd)
{
    switch (e->type)
    {
        case ConfigureNotify:
        {
            renderResize(e->xconfigure.width, e->xconfigure.height);
            break;
        }
        case KeyPress:
        case KeyRelease:
        {
            if (e->type == KeyPress && (e->xkey.state & Mod1Mask) && e->xkey.keycode == 36)
            {
                toggle_fullscreen(dpy, wnd);
                break;
            }
            padSetKey(0, keyToInputKey(dpy, e->xkey), e->type == KeyPress);
            break;
        }
    }
}

// filesystem
#define MAX_FILES 4096
char* gFiles[MAX_FILES];
int32 gFilesCount;

void addDir(char* path)
{
    char* fileName;
    struct dirent* e;
    DIR* dir = opendir(path);

    int32 pathLen = strlen(path);
    path[pathLen] = '/';

    while ((e = readdir(dir)))
    {
        if (e->d_type == DT_DIR)
        {
            if (e->d_name[0] != '.')
            {
                strcpy(path + 1 + pathLen, e->d_name);
                addDir(path);
            }
        }
        else
        {
            ASSERT(gFilesCount < MAX_FILES);
            if (gFilesCount < MAX_FILES)
            {
                strcpy(path + 1 + pathLen, e->d_name);
                fileName = (char*)malloc(strlen(path) + 1 - 2);
                gFiles[gFilesCount++] = strcpy(fileName, path + 2);
            }
        }
    }

    path[pathLen] = '\0';
}

void streamInit()
{
    char path[1024];
    strcpy(path, ".");
    addDir(path);
    LOG("scan %d files\n", gFilesCount);
}

void streamFree()
{
    int32 i;
    for (i = 0; i < gFilesCount; i++)
    {
        free(gFiles[i]);
    }
}

const char* getFileName(const char* fileName)
{
    int32 i;
    for (i = 0; i < gFilesCount; i++)
    {
        if (!strcasecmp(fileName, gFiles[i]))
        {
            //LOG("open file %s -> %s\n", fileName, gFiles[i]);
            return gFiles[i];
        }
    }
    LOG("file not found %s\n", fileName);
    return NULL;
}

int32 osLoadFile(const char* name, void* buffer)
{
    FILE* f;
    int32 size;

    name = getFileName(name);
    ASSERT(name);
    if (!name)
    {
        return 0;
    }

    f = fopen(name, "rb");
    ASSERT(f);
    if (!f)
    {
        return 0;
    }

    fseek(f, 0, SEEK_END);
    size = ftell(f);

    if (buffer)
    {
        fseek(f, 0, SEEK_SET);
        if (fread(buffer, 1, size, f) != size)
        {
            fclose(f);
            ASSERT(0);
            return 0;
        }
    }

    fclose(f);
    return size;
}

int main(int argc, char **argv)
{
    static int XGLAttr[] = {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_DEPTH_SIZE, 24,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        0
    };

    gDRAM = pDRAM = (uint8*)malloc(DRAM_SIZE);
    gVRAM = pVRAM = (uint8*)malloc(VRAM_SIZE);

    dpy = XOpenDisplay(NULL);
    XVisualInfo* vis = glXChooseVisual(dpy, XDefaultScreen(dpy), XGLAttr);

    XSetWindowAttributes attr;
    attr.colormap = XCreateColormap(dpy, RootWindow(dpy, vis->screen), vis->visual, AllocNone);
    attr.border_pixel = 0;
    attr.event_mask = KeyPressMask | KeyReleaseMask | StructureNotifyMask;

    wnd = XCreateWindow(
        dpy, RootWindow(dpy, vis->screen),
        0, 0, WND_WIDTH, WND_HEIGHT, 0,
        vis->depth, InputOutput, vis->visual,
        CWColormap | CWBorderPixel | CWEventMask, &attr);
    XStoreName(dpy, wnd, WND_TITLE);

    GLXContext ctx = glXCreateContext(dpy, vis, NULL, 1);
    glXMakeCurrent(dpy, wnd, ctx);
    XMapWindow(dpy, wnd);

    Atom WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(dpy, wnd, &WM_DELETE_WINDOW, 1);

    renderResize(WND_WIDTH, WND_HEIGHT);

    gTimerStart = osGetSystemTimeMS();
    srand(gTimerStart);

    streamInit();
    inputInit();
    gameInit();

    while (!isQuit)
    {
        if (XPending(dpy))
        {
            XEvent event;
            XNextEvent(dpy, &event);
            if (event.type == ClientMessage && *event.xclient.data.l == WM_DELETE_WINDOW)
            {
                osQuit();
            }
            WndProc(&event, dpy, wnd);
        }
        else
        {
            int32 time = osGetSystemTimeMS();
            gLastFrameIndex = gFrameIndex;
            gFrameIndex = time * 30 / 1000;
            gFrameDelta = (time - gFrameIndex * 1000 / 30) * 256 * 30 / 1000;

            inputUpdate();

            gameCtrl();

            renderSwap();
        }
    };

    gameFree();
    inputFree();
    streamFree();

    glXMakeCurrent(dpy, 0, 0);
    XCloseDisplay(dpy);

    free(gDRAM);
    free(gVRAM);

    return 0;
}
