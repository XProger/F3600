BOOL EnumTextureFormats (void);

typedef struct {
   DDSURFACEDESC2 surface565;
   DDSURFACEDESC2 surface1555;
   DDSURFACEDESC2 surface4444;
   DDSURFACEDESC2 surface555;
   DWORD flags;
} TextureFormats;

#define f565 1
#define f1555 2
#define f4444 4
#define f555 8
