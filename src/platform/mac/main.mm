#include <Cocoa/Cocoa.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <IOKit/hid/IOHidLib.h>
#include <AudioToolbox/AudioQueue.h>

#include "game.h"

BOOL enableRetina = YES;

bool isQuit;

// timing
mach_timebase_info_data_t gTimerBaseInfo;
uint64_t gTimerStart;

void timerInit()
{
    mach_timebase_info(&gTimerBaseInfo);
    gTimerStart = mach_absolute_time();
}

extern "C" int32 osGetSystemTimeMS(void)
{
    uint64_t absolute = mach_absolute_time();
    uint64_t milliseconds = (absolute - gTimerStart) * gTimerBaseInfo.numer / (gTimerBaseInfo.denom * 1000000ULL);
    return (int)milliseconds;
}

extern "C" void osQuit()
{
    isQuit = true;
}

// sound
static AudioQueueRef audioQueue;

void soundFill(void* inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer) {
    void* frames = inBuffer->mAudioData;
    UInt32 count = inBuffer->mAudioDataBytesCapacity / 4;
    FillNotes((int16*)frames, count);
    inBuffer->mAudioDataByteSize = count * 4;
    AudioQueueEnqueueBuffer(audioQueue, inBuffer, 0, NULL);
}

extern "C" void soundInit()
{
    AudioStreamBasicDescription deviceFormat;
    deviceFormat.mSampleRate        = 44100;
    deviceFormat.mFormatID          = kAudioFormatLinearPCM;
    deviceFormat.mFormatFlags       = kLinearPCMFormatFlagIsSignedInteger;
    deviceFormat.mBytesPerPacket    = 4;
    deviceFormat.mFramesPerPacket   = 1;
    deviceFormat.mBytesPerFrame     = 4;
    deviceFormat.mChannelsPerFrame  = 2;
    deviceFormat.mBitsPerChannel    = 16;
    deviceFormat.mReserved          = 0;
    
    AudioQueueNewOutput(&deviceFormat, soundFill, NULL, NULL, NULL, 0, &audioQueue);
    
    for (int i = 0; i < 2; i++) {
        AudioQueueBufferRef mBuffer;
        AudioQueueAllocateBuffer(audioQueue, SND_SAMPLES / 2, &mBuffer);
        soundFill(NULL, audioQueue, mBuffer);
    }
    AudioQueueStart(audioQueue, NULL);
}

// input
#define INPUT_JOY_COUNT 4
struct JoyDevice {
    IOHIDDeviceRef device;
    int32 index;
    float Lx, Ly;
    float Rx, Ry;
    float Lt, Rt;
    
    float oldLx, oldLy;
};

JoyDevice joyDevice[INPUT_JOY_COUNT];

IOHIDManagerRef hidManager;

extern "C" bool osJoyReady(int index)
{
    if (index < 0 || index >= INPUT_JOY_COUNT)
        return false;
    return joyDevice[index].device != NULL;
}

extern "C" void osJoyVibrate(int index, int L, int R)
{
    // TODO
}

float joyAxisValue(IOHIDValueRef value)
{
    IOHIDElementRef element = IOHIDValueGetElement(value);
    CFIndex val = IOHIDValueGetIntegerValue(value);
    CFIndex min = IOHIDElementGetLogicalMin(element);
    CFIndex max = IOHIDElementGetLogicalMax(element);
    
    float v = float(val - min) / float(max - min) * 2.0f - 1.0f;
    if (v > -0.2f && v < 0.2f) v = 0.0f; // check for deadzone
    return v;
}

static const struct { uint32 vendorId, productId; } JOY_VENDORS[] = {
    { 0x045E, 0x02FD }, // Microsoft Xbox One
    { 0x045E, 0x0000 }, // Microsoft x360
    { 0x2DC8, 0x0000 }, // 8Bitdo
    { 0x054C, 0x05C4 }, // Sony DS4 CUH-ZCT1x
    { 0x054C, 0x09CC }, // Sony DS4 CUH-ZCT2x
    { 0x054C, 0x0000 }, // Sony DS3
    { 0x2717, 0x0000 }, // Xiaomi
    { 0x18D1, 0x9400 }, // Google Stadia Controller
};

#define JOY_MAX_VENDORS COUNT(JOY_VENDORS)
#define JOY_MAX_BUTTONS 17

static const uint32 joyAxisTable[][6] = {
    { kHIDUsage_GD_X, kHIDUsage_GD_Y, kHIDUsage_GD_Z,  kHIDUsage_GD_Rz, kHIDUsage_GD_Z, kHIDUsage_GD_Rz },
    { kHIDUsage_GD_X, kHIDUsage_GD_Y, kHIDUsage_GD_Rx, kHIDUsage_GD_Ry, kHIDUsage_GD_Z, kHIDUsage_GD_Rz },
    { kHIDUsage_GD_X, kHIDUsage_GD_Y, kHIDUsage_GD_Z,  kHIDUsage_GD_Rz, 0, 0 },
    { kHIDUsage_GD_X, kHIDUsage_GD_Y, kHIDUsage_GD_Z,  kHIDUsage_GD_Rz, 0, 0 },
    { kHIDUsage_GD_X, kHIDUsage_GD_Y, kHIDUsage_GD_Z,  kHIDUsage_GD_Rz, kHIDUsage_GD_Rx, kHIDUsage_GD_Ry },
    { kHIDUsage_GD_X, kHIDUsage_GD_Y, kHIDUsage_GD_Z,  kHIDUsage_GD_Rz, 0, 0 },
    { kHIDUsage_GD_X, kHIDUsage_GD_Y, kHIDUsage_GD_Z,  kHIDUsage_GD_Rz, 0, 0 },
    { kHIDUsage_GD_X, kHIDUsage_GD_Y, kHIDUsage_GD_Z,  kHIDUsage_GD_Rz, kHIDUsage_GD_Z, kHIDUsage_GD_Rz },
    { kHIDUsage_GD_X, kHIDUsage_GD_Y, kHIDUsage_GD_Z,  kHIDUsage_GD_Rz, kHIDUsage_GD_Z, kHIDUsage_GD_Rz }
};

static const InputKey joyButtonsTable[][JOY_MAX_BUTTONS] = {
    { IN_A, IN_B, IN_NONE, IN_X, IN_Y, IN_NONE, IN_LB, IN_RB, IN_NONE, IN_NONE, IN_NONE, IN_START, IN_NONE, IN_L, IN_R, IN_NONE, IN_NONE },
    { IN_A, IN_B, IN_X, IN_Y, IN_LB, IN_RB, IN_LT, IN_RT, IN_START, IN_SELECT, IN_NONE, IN_UP, IN_DOWN, IN_LEFT, IN_RIGHT, IN_NONE, IN_NONE },
    { IN_B, IN_A, IN_NONE, IN_Y, IN_X, IN_NONE, IN_LB, IN_RB, IN_LT, IN_RT, IN_SELECT, IN_START, IN_NONE, IN_L, IN_R, IN_NONE, IN_NONE },
    { IN_X, IN_A, IN_B, IN_Y, IN_LB, IN_RB, IN_LT, IN_RT, IN_SELECT, IN_START, IN_L, IN_R, IN_NONE, IN_NONE, IN_NONE, IN_NONE, IN_NONE },
    { IN_X, IN_A, IN_B, IN_Y, IN_LB, IN_RB, IN_LT, IN_RT, IN_SELECT, IN_START, IN_L, IN_R, IN_NONE, IN_NONE, IN_NONE, IN_NONE, IN_NONE },
    { IN_SELECT, IN_L, IN_R, IN_START, IN_UP, IN_RIGHT, IN_DOWN, IN_LEFT, IN_LT, IN_RT, IN_LB, IN_RB, IN_Y, IN_B, IN_A, IN_X, IN_NONE },
    { IN_A, IN_B, IN_NONE, IN_X, IN_Y, IN_NONE, IN_LB, IN_RB, IN_LT, IN_RT, IN_SELECT, IN_START, IN_NONE, IN_L, IN_R, IN_NONE, IN_NONE },
    { IN_A, IN_B, IN_NONE, IN_X, IN_Y, IN_NONE, IN_LB, IN_RB, IN_NONE, IN_NONE, IN_SELECT, IN_START, IN_NONE, IN_L, IN_R, IN_NONE, IN_NONE }
};

InputKey joyButtonToKey(const InputKey *btns, uint32 button)
{
    if (button >= 0 && button < JOY_MAX_BUTTONS)
        return btns[button];
    return IN_NONE;
}

void setKey(int code, bool down)
{
    int p = 0;

    gLastKey = down ? code : -1;

    switch (code)
    {
        case 0: // A
            p = IN_LT;
            break;
        case 2: // D
            p = IN_RT;
            break;
        case 12: // Q
            p = IN_LB;
            break;
        case 14: // E
            p = IN_RB;
            break;
        case 36: // Enter
        case 13: // W
            p = IN_A;
            break;
        case 49: // Space
            p = IN_B;
            break;
        case 48: // Tab
            p = IN_Y;
            break;
        case 53: // Escape
            p = IN_START;
            break;
        case 123: // Left
            p = IN_LEFT;
            break;
        case 124: // Right
            p = IN_RIGHT;
            break;
        case 126: // Up
            p = IN_UP;
            break;
        case 125: // Down
            p = IN_DOWN;
            break;
        default:
            return;
    }

    if (down)
    {
        gPad |= p;
    }
    else
    {
        gPad &= ~p;
    }
}

void joySetPos(int index, InputKey key, float x, float y)
{
    JoyDevice* joy = joyDevice + index;
    
    if (key == IN_L)
    {
        joy->Lx = x;
        joy->Ly = y;
    }
    else
    {
        joy->Rx = x;
        joy->Ry = y;
    }
}

void setDown(int index, InputKey key, bool down)
{
    if (down)
    {
        gPad |= key;
    }
    else
    {
        gPad &= ~key;
    }
}

void joySetTrigger(int index, InputKey key, float value)
{
    setDown(index, key, value > 0.5f);
}

int32 joyGetVendorIndex(IOHIDDeviceRef device, int32 vendorId, int32 productId)
{
    for (int i = 0; i < JOY_MAX_VENDORS; i++)
    {
        if (JOY_VENDORS[i].vendorId == vendorId && (!JOY_VENDORS[i].productId || JOY_VENDORS[i].productId == productId))
            return i;
    }
    return 0; // MS Xbox 360 as default
}

void hidValueCallback(void *context, IOReturn result, void *sender, IOHIDValueRef value)
{
    if (result != kIOReturnSuccess)
        return;

    IOHIDElementRef element = IOHIDValueGetElement(value);
    IOHIDDeviceRef device = IOHIDElementGetDevice(element);
    
    int joyIndex = -1;
    
    for (int i = 0; i < INPUT_JOY_COUNT; i++)
    {
        if (joyDevice[i].device == device)
        {
            joyIndex = i;
            break;
        }
    }
    
    if (joyIndex == -1)
        return;
    
    uint32 page = IOHIDElementGetUsagePage(element);
    uint32 usage = IOHIDElementGetUsage(element);
    uint32 vendor = joyDevice[joyIndex].index;
    
    const uint32* axis = joyAxisTable[vendor];
    const InputKey* btns = joyButtonsTable[vendor];

    switch (page)
    {
        case kHIDPage_GenericDesktop:
        {
            if (usage == axis[0])
                joySetPos(joyIndex, IN_L, joyAxisValue(value), joyDevice[joyIndex].Ly);
            else if (usage == axis[1])
                joySetPos(joyIndex, IN_L, joyDevice[joyIndex].Lx, joyAxisValue(value));
            else if (usage == axis[2])
                joySetPos(joyIndex, IN_R, joyAxisValue(value), joyDevice[joyIndex].Ry);
            else if (usage == axis[3])
                joySetPos(joyIndex, IN_R, joyDevice[joyIndex].Rx, joyAxisValue(value));
            else if (usage == axis[4])
                joySetTrigger(joyIndex, IN_LT, joyAxisValue(value) * 0.5f + 0.5f);
            else if (usage == axis[5])
                joySetTrigger(joyIndex, IN_RT, joyAxisValue(value) * 0.5f + 0.5f);
            else if (usage == kHIDUsage_GD_Hatswitch)
            {
                CFIndex p = IOHIDValueGetIntegerValue(value);
                CFIndex min = IOHIDElementGetLogicalMin(element);
                if (min == 0)
                {
                    p++;
                }
                setDown(joyIndex, IN_UP,    p == 8 || p == 1 || p == 2);
                setDown(joyIndex, IN_RIGHT, p == 2 || p == 3 || p == 4);
                setDown(joyIndex, IN_DOWN,  p == 4 || p == 5 || p == 6);
                setDown(joyIndex, IN_LEFT,  p == 6 || p == 7 || p == 8);
            }
            //if (usage != axis[0] && usage != axis[1] && usage != axis[2] && usage != axis[3])
            //    LOG("! joy: axis 0x%x (%d)\n", usage, (int)IOHIDValueGetIntegerValue(value));
            break;
        }
        case kHIDPage_Simulation:
        {
            if (usage == kHIDUsage_Sim_Accelerator)
                joySetTrigger(joyIndex, IN_RT, joyAxisValue(value) * 0.5f + 0.5f);
            else if (usage == kHIDUsage_Sim_Brake)
                joySetTrigger(joyIndex, IN_LT, joyAxisValue(value) * 0.5f + 0.5f);
            break;
        }
        case kHIDPage_Button:
        {
            uint32_t button = IOHIDElementGetUsage(IOHIDValueGetElement(value)) - kHIDUsage_Button_1;
            bool down = IOHIDValueGetIntegerValue(value) != 0;
            InputKey key = joyButtonToKey(btns, button);
            setDown(joyIndex, key, down);
            //LOG("! joy: button %d\n", button);
            break;
        }
            
        default : ;
    }
}

void joyAdd(void* context, IOReturn, void*, IOHIDDeviceRef device)
{
    for (int i = 0; i < INPUT_JOY_COUNT; i++)
    {
        if (joyDevice[i].device == device)
            return;
    }

    for (int i = 0; i < INPUT_JOY_COUNT; i++)
    {
        JoyDevice* joy = joyDevice + i;
        
        if (!joy->device)
        {
            int32 vendorId = [(NSNumber*)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey)) intValue];
            int32 productId = [(NSNumber*)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey)) intValue];
            
            joy->device = device;
            joy->index = joyGetVendorIndex(device, vendorId, productId);

            LOG("! joy: add index:%d vendor:0x%04X product:0x%04X\n", i, vendorId, productId);
            break;
        }
    }
}

void joyRemove(void* context, IOReturn, void*, IOHIDDeviceRef device)
{
    for (int i = 0; i < INPUT_JOY_COUNT; i++)
    {
        JoyDevice* joy = joyDevice + i;
        
        if (joy->device == device)
        {
            LOG("! joy: remove index:%d\n", i);
            memset(joy, 0, sizeof(*joy));
            break;
        }
    }
}

void inputInit()
{
    hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDManagerOptionNone);
    
    NSDictionary *matchingGamepad = @{
                                      @(kIOHIDDeviceUsagePageKey): @(kHIDPage_GenericDesktop),
                                      @(kIOHIDDeviceUsageKey): @(kHIDUsage_GD_GamePad)
                                      };
    NSDictionary *matchingJoystick = @{
                                       @(kIOHIDDeviceUsagePageKey): @(kHIDPage_GenericDesktop),
                                       @(kIOHIDDeviceUsageKey): @(kHIDUsage_GD_Joystick)
                                       };
    NSArray *matchDicts = @[ matchingGamepad, matchingJoystick ];
    
    IOHIDManagerSetDeviceMatchingMultiple(hidManager, (__bridge CFArrayRef) matchDicts);
    IOHIDManagerRegisterDeviceMatchingCallback(hidManager, joyAdd, NULL);
    IOHIDManagerRegisterDeviceRemovalCallback(hidManager, joyRemove, NULL);
    IOHIDManagerScheduleWithRunLoop(hidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    IOHIDManagerOpen(hidManager, kIOHIDOptionsTypeNone);
    IOHIDManagerRegisterInputValueCallback(hidManager, hidValueCallback, NULL);
    
    CFSetRef devices = IOHIDManagerCopyDevices(hidManager);
    if (devices)
    {
        CFIndex count = CFSetGetCount(devices);
        CFTypeRef devicesArray[count]; // array of IOHIDDeviceRef
        CFSetGetValues(devices, devicesArray);
        
        for (int i = 0; i < count; i++)
        {
            joyAdd(NULL, 0, NULL, (IOHIDDeviceRef)devicesArray[i]);
        }
        
        CFRelease(devices);
    }
}

void inputFree()
{
    IOHIDManagerUnscheduleFromRunLoop(hidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    IOHIDManagerRegisterDeviceMatchingCallback(hidManager, NULL, 0);
    IOHIDManagerRegisterDeviceRemovalCallback(hidManager, NULL, 0);
    IOHIDManagerClose(hidManager, kIOHIDOptionsTypeNone);
}

void inputUpdate()
{
    gPadStickX = gPadStickY = 256;
    
    int32 joyIndex = 0; // TODO gamepad selection
    
    JoyDevice* joy = joyDevice + joyIndex;
    if (!joy->device)
        return;
    
    if (joy->Lx != joy->oldLx)
    {
        setDown(joyIndex, IN_LEFT,  joy->Lx < 0);
        setDown(joyIndex, IN_RIGHT, joy->Lx > 0);
    }

    if (joy->Ly != joy->oldLy)
    {
        setDown(joyIndex, IN_UP,   joy->Ly < 0);
        setDown(joyIndex, IN_DOWN, joy->Ly > 0);
    }

    joy->oldLx = joy->Lx;
    joy->oldLy = joy->Ly;
    
    if (joy->Lx != 0.0f)
    {
        gPadStickX = int32((fabsf(joy->Lx)) * 256.0f);
    }
    
    if (joy->Ly != 0.0f)
    {
        gPadStickY = int32((fabsf(joy->Ly)) * 512.0f);
    }

    if (gPadStickX > 256)
        gPadStickX = 256;
    if (gPadStickY > 256)
        gPadStickY = 256;
}

// filesystem
char contentDir[255];

extern "C" int32 osLoadFile(const char* name, void* buffer)
{
    FILE* f;
    int32 size;
    char path[255];

    strcpy(path, contentDir);
    strcat(path, name);
    
    f = fopen(path, "rb");
    if (!f)
    {
        //ASSERT(0);
        LOG("can't open file!\n");
        return 0;
    }

    fseek(f, 0, SEEK_END);
    size = (int32)ftell(f);

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

NSWindow *window;

NSEventModifierFlags keySpec;

@interface GLView : NSOpenGLView

@end

@implementation GLView

- (void)keyDown:(NSEvent *)event
{
    setKey(event.keyCode, true);
}

- (void)keyUp:(NSEvent *)event
{
    setKey(event.keyCode, false);
    
    // Alt + Enter
    if ((event.keyCode == 36) && (keySpec & NSEventModifierFlagOption))
    {
        [window toggleFullScreen:nil];
    }
}

- (void)flagsChanged:(NSEvent *)event
{
    keySpec = event.modifierFlags;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)reshape
{
    [super reshape];
    
    CGSize drawableSize = self.bounds.size;
    
    if (enableRetina)
    {
        NSScreen* screen = self.window.screen;
        if (!screen)
        {
            screen = [NSScreen mainScreen];
        }
        drawableSize.width *= screen.backingScaleFactor;
        drawableSize.height *= screen.backingScaleFactor;
    }
    
    if (drawableSize.width < 1)
        drawableSize.width = 1;
    
    if (drawableSize.height < 1)
        drawableSize.height = 1;
    
    renderResize(drawableSize.width, drawableSize.height);
}

- (void)prepareOpenGL
{
    [super prepareOpenGL];
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
}

- (void)drawRect:(NSRect)dirtyRect
{
    inputUpdate();
    
    int32 time = osGetSystemTimeMS();
    gLastFrameIndex = gFrameIndex;
    gFrameIndex = time * 30 / 1000;
    gFrameDelta = (time - gFrameIndex * 1000 / 30) * 256 * 30 / 1000;

    gameCtrl();
    
    NSOpenGLContext *context = [self openGLContext];
    [context flushBuffer];

    BOOL arg = YES;
    NSInvocation *inv = [NSInvocation invocationWithMethodSignature:[self methodSignatureForSelector:@selector(setNeedsDisplay:)]];
    [inv setSelector:@selector(setNeedsDisplay:)];
    [inv setTarget:self];
    [inv setArgument:&arg atIndex:2];
    [inv performSelector:@selector(invoke) withObject:self afterDelay:(1.0f / 240.0f)];
    
    if (isQuit)
    {
        NSApplication *application = [NSApplication sharedApplication];
        [application stop:nil];
        [application abortModal]; // generate UI event
    }
}

@end

@interface WindowDelegate : NSObject<NSWindowDelegate>
@end

@implementation WindowDelegate

- (void)windowWillClose:(NSNotification *)notification
{
    [[NSApplication sharedApplication] terminate:self];
}

@end

#define WND_WIDTH   854
#define WND_HEIGHT  480

int main()
{
    gDRAM = pDRAM = (uint8*)malloc(DRAM_SIZE);
    gVRAM = pVRAM = (uint8*)malloc(VRAM_SIZE);

    renderResize(WND_WIDTH, WND_HEIGHT);

    //get and create support path
    NSString *appName, *supportPath = nil;
    NSArray *paths = NSSearchPathForDirectoriesInDomains( NSApplicationSupportDirectory, NSUserDomainMask, YES );

    if ([paths count] > 0)
    {
        appName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleExecutable"];
        supportPath = [[paths objectAtIndex:0] stringByAppendingPathComponent:appName];
        
        if (![[NSFileManager defaultManager] fileExistsAtPath:supportPath])
        {
            if (![[NSFileManager defaultManager] createDirectoryAtPath:supportPath attributes:nil])
            {
                supportPath = nil;
            }
        }
    }

    NSApplication *application = [NSApplication sharedApplication];
    
    // init window
    NSRect rect = NSMakeRect(0, 0, WND_WIDTH, WND_HEIGHT);
    window = [[NSWindow alloc] initWithContentRect:rect styleMask:NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask backing:NSBackingStoreBuffered defer:YES];
    window.title = @"wipEout xport";
    window.acceptsMouseMovedEvents = YES;
    window.delegate = [[WindowDelegate alloc] init];
    
    // init OpenGL context
    NSOpenGLPixelFormatAttribute attribs[] = {
        NSOpenGLPFANoRecovery,
        NSOpenGLPFAMultisample,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize, 32,
        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFAStencilSize, 8,
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
        0
    };
    NSOpenGLPixelFormat *format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
    
    GLView *view = [[GLView alloc] initWithFrame:window.contentLayoutRect pixelFormat:format];
    view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    window.contentView = view;
    [view.openGLContext makeCurrentContext];
    [view setWantsBestResolutionOpenGLSurface:enableRetina];
    
    // get path to game content
    NSBundle *bundle   = [NSBundle mainBundle];
    NSURL *resourceURL = bundle.resourceURL;
    [resourceURL getFileSystemRepresentation:contentDir maxLength:sizeof(contentDir)];
    strcat(contentDir, "/");
    
    /*
    [supportPath getCString:saveDir maxLength:sizeof(saveDir) encoding:NSUTF8StringEncoding];
    strcat(saveDir, "/");
    [supportPath getCString:cacheDir maxLength:sizeof(cacheDir) encoding:NSUTF8StringEncoding];
    strcat(cacheDir, "/");
    */

    [window center];
    [window makeKeyAndOrderFront:nil];

    timerInit();
    inputInit();
    
    srand((int)gTimerStart);
    
    gameInit();

    [application run];

    inputFree();
    gameFree();

    return 0;
}
