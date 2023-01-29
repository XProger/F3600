TARGET = F3600

SRC_C   = $(wildcard *.c)
SRC_C  += $(wildcard ../../*.c)
SRC_C  += ../win/sound.c

OBJS = $(SRC_C:%.c=%.o)

INCDIR   = ../../
CFLAGS   = -G0 -O3 -D__PSP__=1
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -Wno-invalid-offsetof
ASFLAGS  = $(CFLAGS)

BUILD_PRX = 1

LIBDIR  =
LDFLAGS =
LIBS    = -lpspgum -lpspgu -lpspaudiolib -lpspaudio -lpsprtc -lpsppower -lpspmath -lm

EXTRA_TARGETS   = EBOOT.PBP
PSP_EBOOT_TITLE = F3600
PSP_EBOOT_ICON  = ICON0.PNG
PSP_EBOOT_PIC1  = PIC1.PNG

deploy:
	tools/pspsh.exe

run: clean all deploy

PSPSDK = $(shell psp-config --pspsdk-path)
#USE_PSPSDK_LIBC=1
include $(PSPSDK)/lib/build.mak