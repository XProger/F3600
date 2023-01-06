#if defined(__WIN32__)
    #include <windows.h>
    #include <gl/GL.h>
    #include <gl/glu.h>
    #include <gl/glext.h>
    #include <gl/wglext.h>

    extern HWND hWnd;
    HDC hDC;
    HGLRC hRC;
#elif defined(__LINUX__)
    #include <GL/gl.h>
    #include <GL/glext.h>
    #include <GL/glx.h>

    extern Display* dpy;
    extern Window wnd;
#elif defined(__MAC__)
    #include <OpenGL/gl3.h>
    #include <OpenGL/gl3ext.h>
    #include <OpenGL/glext.h>
#endif

#include "game.h"

//#define DEBUG_RENDER

#define i255    (1.0f / 255.0f)
#define i4096   (1.0f / 4096.0f)

#define PI      3.14159265358979323846f
#define DEG2RAD (PI / 180.0f)
#define RAD2DEG (180.0f / PI)

#if defined(__WIN32__) || defined(__LINUX__)
    // Textures
    PFNGLGENERATEMIPMAPPROC             glGenerateMipmap;

    // Shader
    PFNGLCREATEPROGRAMPROC              glCreateProgram;
    PFNGLDELETEPROGRAMPROC              glDeleteProgram;
    PFNGLLINKPROGRAMPROC                glLinkProgram;
    PFNGLUSEPROGRAMPROC                 glUseProgram;
    PFNGLGETPROGRAMINFOLOGPROC          glGetProgramInfoLog;
    PFNGLCREATESHADERPROC               glCreateShader;
    PFNGLDELETESHADERPROC               glDeleteShader;
    PFNGLSHADERSOURCEPROC               glShaderSource;
    PFNGLATTACHSHADERPROC               glAttachShader;
    PFNGLCOMPILESHADERPROC              glCompileShader;
    PFNGLGETSHADERINFOLOGPROC           glGetShaderInfoLog;
    PFNGLGETUNIFORMLOCATIONPROC         glGetUniformLocation;
    PFNGLUNIFORM1IVPROC                 glUniform1iv;
    PFNGLUNIFORM1FVPROC                 glUniform1fv;
    PFNGLUNIFORM2FVPROC                 glUniform2fv;
    PFNGLUNIFORM3FVPROC                 glUniform3fv;
    PFNGLUNIFORM4FVPROC                 glUniform4fv;
    PFNGLUNIFORMMATRIX4FVPROC           glUniformMatrix4fv;
    PFNGLBINDATTRIBLOCATIONPROC         glBindAttribLocation;
    PFNGLENABLEVERTEXATTRIBARRAYPROC    glEnableVertexAttribArray;
    PFNGLDISABLEVERTEXATTRIBARRAYPROC   glDisableVertexAttribArray;
    PFNGLVERTEXATTRIBPOINTERPROC        glVertexAttribPointer;
    PFNGLGETPROGRAMIVPROC               glGetProgramiv;

    // Mesh
    PFNGLGENBUFFERSARBPROC      glGenBuffers;
    PFNGLDELETEBUFFERSARBPROC   glDeleteBuffers;
    PFNGLBINDBUFFERARBPROC      glBindBuffer;
    PFNGLBUFFERDATAARBPROC      glBufferData;
    PFNGLBUFFERSUBDATAARBPROC   glBufferSubData;

    // Vertex Arrays
    PFNGLGENVERTEXARRAYSPROC    glGenVertexArrays;
    PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
    PFNGLBINDVERTEXARRAYPROC    glBindVertexArray;
#endif

// VSync control
#if defined(__WIN32__)
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
#elif defined(__LINUX__)
    PFNGLXSWAPINTERVALSGIPROC glXSwapIntervalSGI;
#endif

#if defined(__MAC__)
    #define glGenVertexArrays    glGenVertexArraysAPPLE
    #define glDeleteVertexArrays glDeleteVertexArraysAPPLE
    #define glBindVertexArray    glBindVertexArrayAPPLE
#endif

typedef struct
{
    int32 x;
    int32 y;
    int32 z;
    float s, t;
    uint32 color;
} VertexGL;

typedef uint16 IndexGL;

typedef struct
{
    float e[4][4];
} mat4;

typedef struct Mesh
{
    int32 iStart;
    int32 iCount;
    struct Mesh* next;
} Mesh;

typedef enum
{
    aCoord,
    aTexCoord,
    aColor,
    aMAX
} AttribType;

typedef enum
{
    uProjMatrix,
    uViewMatrix,
    uColor,
    uResolution,
    uMAX
} UniformType;

typedef struct
{
    GLuint id;
    GLint uid[uMAX];
} Shader;

typedef struct
{
    Mesh* mesh;
    const Shader* shader;
    const Texture* texture;
    int32 projMatrix;
    int32 viewMatrix;
    uint32 color;
    int32 layer;
    int32 blend;
    uint8 depthWrite;
    uint8 depthTest;
    int32 param;
} RenderState;

#define MAX_RENDER_VERTICES     (1024 * 16)
#define MAX_RENDER_INDICES      (1024 * 16)
#define MAX_RENDER_MATRICES     (128)
#define MAX_RENDER_MESHES       (1024)
#define MAX_RENDER_STATES       (512)

VertexGL gVertices[MAX_RENDER_VERTICES];
IndexGL gIndices[MAX_RENDER_INDICES];
IndexGL gIndicesMerged[MAX_RENDER_INDICES];
mat4 gMatrices[MAX_RENDER_MATRICES];
Mesh gMeshes[MAX_RENDER_MESHES];
RenderState gCurState;
RenderState gStates[MAX_RENDER_STATES];
RenderState* gStatePtr;

int32 gVerticesCount;
int32 gIndicesCount;
int32 gMatricesCount;
int32 gMeshesCount;
int32 gStatesCount;

int32 gMaxAniso;

uint16 tempData[320 * 240];

GLuint whiteTex;

int gRenderMode;

int32 xOffset;
int32 yOffset;

int gWidth, gHeight, gBorder;

int32 gCylX, gCylZ;

GLuint VAO;
GLuint VBO[2];
Shader shaderTrack;
Shader shaderObject;
Shader shaderShip;
Shader shaderShield;
Shader shaderSky;
Shader shaderBlob;
Shader shaderTarget;
Shader shaderGUI;

#define MAX_TRACK_TEX   22
Texture gTrackTex[MAX_TRACK_TEX];
uint16 gTrackTexData[MAX_TRACK_TEX][128 * 128];

extern Texture* gEffectTextures;

const uint32 colorGrey = 0xFF808080;
const uint32 colorWhite = 0xFFFFFFFF;

const char* GLSL_HEADER_VERT_GL3 =
    "#version 140\n"
    "#define VERTEX\n"
    "#define varying out\n"
    "#define attribute in\n"
    "#define texture2D texture\n";

const char* GLSL_HEADER_FRAG_GL3 =
    "#version 140\n"
    "#define FRAGMENT\n"
    "#define varying in\n"
    "#define texture2D texture\n"
    "out vec4 fragColor;\n";

const char* GLSL_HEADER_VERT_GL2 =
    "#version 110\n"
    "#define VERTEX\n";

const char* GLSL_HEADER_FRAG_GL2 =
    "#version 110\n"
    "#define FRAGMENT\n"
    "#define fragColor gl_FragColor\n";

const char* sh_track =
    "varying vec2 vTexCoord;\n"
    "varying vec4 vColor;\n"

    "#ifdef VERTEX\n"
        "uniform mat4 uProjMatrix;\n"
        "uniform mat4 uViewMatrix;\n"

        "attribute vec3 aCoord;\n"
        "attribute vec2 aTexCoord;\n"
        "attribute vec4 aColor;\n"

        "void main() {\n"
            "vTexCoord = aTexCoord;\n"
            "vColor = aColor * vec4(2.0, 2.0, 2.0, 1.0);\n"
            "gl_Position = uProjMatrix * (uViewMatrix * vec4(aCoord, 1.0));\n"
        "}\n"

    "#else\n"

        "uniform sampler2D sDiffuse;\n"

        "void main() {\n"
            "fragColor = texture2D(sDiffuse, vTexCoord) * vColor;\n"
            "if (fragColor.a <= 0.0) discard;\n"
        "}\n"

    "#endif\n";

const char* sh_object =
    "varying vec2 vTexCoord;\n"
    "varying vec4 vColor;\n"

    "#ifdef VERTEX\n"
        "uniform mat4 uProjMatrix;\n"
        "uniform mat4 uViewMatrix;\n"

        "attribute vec3 aCoord;\n"
        "attribute vec2 aTexCoord;\n"
        "attribute vec4 aColor;\n"

        "void main() {\n"
            "vTexCoord = aTexCoord;\n"
            "vColor = aColor * vec4(2.0, 2.0, 2.0, 1.0);\n"
            "gl_Position = uProjMatrix * (uViewMatrix * vec4(aCoord, 1.0));\n"
        "}\n"

    "#else\n"

        "uniform sampler2D sDiffuse;\n"

        "void main() {\n"
            "fragColor = texture2D(sDiffuse, vTexCoord) * vColor;\n"
            "if (fragColor.a <= 0.0) discard;\n"
        "}\n"

    "#endif\n";

const char* sh_ship =
    "varying vec2 vTexCoord;\n"
    "varying vec4 vColor;\n"

    "#ifdef VERTEX\n"
        "uniform mat4 uProjMatrix;\n"
        "uniform mat4 uViewMatrix;\n"
        "uniform vec4 uColor;\n"

        "attribute vec3 aCoord;\n"
        "attribute vec2 aTexCoord;\n"
        "attribute vec4 aColor;\n"

        "void main() {\n"
            "vTexCoord = aTexCoord;\n"
            "vec4 ambient = min(vec4(1.0), uColor * 2.0 + 0.1);\n"
            "vColor = aColor * ambient * vec4(2.0, 2.0, 2.0, 1.0);\n"
            "gl_Position = uProjMatrix * (uViewMatrix * vec4(aCoord, 1.0));\n"
        "}\n"

    "#else\n"

        "uniform sampler2D sDiffuse;\n"

        "void main() {\n"
            "fragColor = texture2D(sDiffuse, vTexCoord) * vColor;\n"
        "}\n"

    "#endif\n";

const char* sh_shield =
    "varying vec4 vColor;\n"

    "#ifdef VERTEX\n"
        "uniform mat4 uProjMatrix;\n"
        "uniform mat4 uViewMatrix;\n"
        "uniform vec4 uColor;\n"

        "attribute vec3 aCoord;\n"

        "void main() {\n"
            "float v = aCoord.x + aCoord.y + aCoord.z;\n"
            "float a = sin(v + uColor.r * 64.0) * 0.5 + 0.5;\n"
            "float b = sin(aCoord.z + uColor.r * 128.0) * 0.5 + 0.5;\n"
            "vec3 pos = aCoord;\n"
            "pos.x += sign(pos.x) * a * 8.0;\n"
            "pos.y += sign(pos.y) * b * 8.0;\n"
            "b *= 0.25;\n"
            "vColor = vec4(a * 0.5 + b, 0.25 + b, 0.5 + b, 1.0) * 0.75;\n"
            "gl_Position = uProjMatrix * (uViewMatrix * vec4(pos, 1.0));\n"
        "}\n"

    "#else\n"

        "void main() {\n"
            "fragColor = vec4(vColor.xyz, 1.0);\n"
        "}\n"

    "#endif\n";

/*TODO aurora*/
const char* sh_sky =
    "varying vec2 vTexCoord;\n"
    "varying vec4 vColor;\n"

    "#ifdef VERTEX\n"
        "uniform mat4 uProjMatrix;\n"
        "uniform mat4 uViewMatrix;\n"

        "attribute vec3 aCoord;\n"
        "attribute vec2 aTexCoord;\n"
        "attribute vec4 aColor;\n"

        "void main() {\n"
            "vTexCoord = aTexCoord;\n"
            "vColor = aColor * vec4(2.0, 2.0, 2.0, 1.0);\n"
            "gl_Position = uProjMatrix * (uViewMatrix * vec4(aCoord, 1.0));\n"
        "}\n"

    "#else\n"

        "uniform sampler2D sDiffuse;\n"

        "void main() {\n"
            "fragColor = texture2D(sDiffuse, vTexCoord) * vColor;\n"
        "}\n"

    "#endif\n";

const char* sh_blob =
    "varying vec2 vTexCoord;\n"
    "varying float vIntensity;\n"

    "#ifdef VERTEX\n"
        "uniform mat4 uProjMatrix;\n"
        "uniform mat4 uViewMatrix;\n"

        "attribute vec3 aCoord;\n"
        "attribute vec2 aTexCoord;\n"
        "attribute vec4 aColor;\n"

        "void main() {\n"
            "vTexCoord = aTexCoord;\n"
            "vIntensity = aColor.a * 4.0;\n"
            "gl_Position = uProjMatrix * (uViewMatrix * vec4(aCoord, 1.0));\n"
        "}\n"

    "#else\n"

        "uniform sampler2D sDiffuse;\n"

        "void main() {\n"
            "fragColor = vec4(0.0, 0.0, 0.0, texture2D(sDiffuse, vTexCoord).r * vIntensity);\n"
        "}\n"

    "#endif\n";

const char* sh_target =
    "varying vec2 vTexCoord;\n"

    "#ifdef VERTEX\n"
        "uniform mat4 uProjMatrix;\n"
        "uniform mat4 uViewMatrix;\n"
        "uniform vec4 uResolution;\n"

        "attribute vec3 aCoord;\n"
        "attribute vec2 aTexCoord;\n"

        "void main() {\n"
            "vTexCoord = aTexCoord;\n"
            "vec4 p = uProjMatrix * (uViewMatrix * vec4(aCoord, 1.0));\n"
            "vec2 d = (aTexCoord.xy * 2.0 - 1.0) * (16.0 * p.w);\n"
            "p.xy += d * uResolution.xy;\n"
            "gl_Position = p;\n"
        "}\n"

    "#else\n"

        "uniform sampler2D sDiffuse;\n"

        "void main() {\n"
            "fragColor = texture2D(sDiffuse, vTexCoord) * 0.5;\n"
        "}\n"

    "#endif\n";

const char* sh_gui =
    "varying vec2 vTexCoord;\n"
    "varying vec4 vColor;\n"

    "#ifdef VERTEX\n"
        "uniform mat4 uProjMatrix;\n"

        "attribute vec2 aCoord;\n"
        "attribute vec2 aTexCoord;\n"
        "attribute vec4 aColor;\n"

        "void main() {\n"
            "vTexCoord = aTexCoord;\n"
            "vColor = vec4(aColor.xyz + aColor.xyz, 1.0);\n"
            "gl_Position = uProjMatrix * vec4(aCoord, 0.0, 1.0);\n"
        "}\n"

    "#else\n"

        "uniform sampler2D sDiffuse;\n"

        "void main() {\n"
            "fragColor = texture2D(sDiffuse, vTexCoord) * vColor;\n"
        "}\n"

    "#endif\n";

void matrixIdentity(mat4* m)
{
    memset(m, 0, sizeof(Matrix));
    m->e[0][0] = m->e[1][1] = m->e[2][2] = m->e[3][3] = 1.0f;
}

void matrixOrtho(mat4* m, float l, float r, float b, float t, float znear, float zfar)
{
    memset(m, 0, sizeof(mat4));
    m->e[0][0] = 2.0f / (r - l);
    m->e[1][1] = 2.0f / (t - b);
    m->e[2][2] = 2.0f / (znear - zfar);
    m->e[3][3] = 1.0f;
    m->e[3][0] = (l + r) / (l - r);
    m->e[3][1] = (t + b) / (b - t);
    m->e[3][2] = (znear + zfar) / (znear - zfar);
}

void matrixPerspective(mat4* m, float fov, float aspect, float xoff, float yoff, float znear, float zfar)
{
    float y = tanf(fov * 0.5f * DEG2RAD) * znear;
    float x = y;

    x = y * aspect;

    memset(m, 0, sizeof(mat4));
    m->e[0][0] = znear / x;
    m->e[1][1] = znear / y;
    m->e[2][2] = (znear + zfar) / (znear - zfar);
    m->e[3][3] = 0.0f;
    m->e[2][0] = xoff;
    m->e[2][1] = yoff;
    m->e[2][3] = -1.0f;
    m->e[3][2] = 2.0f * zfar * znear / (znear - zfar);

    // wipEout specific axis flip (X, -Y, -Z)
    m->e[1][0] = -m->e[1][0];
    m->e[1][1] = -m->e[1][1];
    m->e[1][2] = -m->e[1][2];
    m->e[1][3] = -m->e[1][3];

    m->e[2][0] = -m->e[2][0];
    m->e[2][1] = -m->e[2][1];
    m->e[2][2] = -m->e[2][2];
    m->e[2][3] = -m->e[2][3];
}

int32 stateCmp(const RenderState* a, const RenderState* b)
{
    return (a->shader == b->shader) &&
           (a->texture == b->texture) &&
           (a->projMatrix == b->projMatrix) &&
           (a->viewMatrix == b->viewMatrix) &&
           (a->color == b->color) &&
           (a->layer == b->layer) &&
           (a->blend == b->blend) &&
           (a->depthWrite == b->depthWrite) &&
           (a->depthTest == b->depthTest);
}

void stateCheck()
{
    int32 i;
    Mesh* mesh;

    if (gStatePtr)
        return;

    for (i = 0; i < gStatesCount; i++)
    {
        if (stateCmp(&gCurState, gStates + i))
        {
            gStatePtr = gStates + i;
            break;
        }
    }

    if (!gStatePtr)
    {
        gStatePtr = gStates + gStatesCount++;
        *gStatePtr = gCurState;
        ASSERT(gStatePtr->mesh == NULL);
    }

    mesh = gMeshes + gMeshesCount++;

    mesh->iStart = gIndicesCount;
    mesh->iCount = 0;
    mesh->next = gStatePtr->mesh;

    gStatePtr->mesh = mesh;
}

void setTexFilter(int bilinear, int mipmap)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? (bilinear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST) : (bilinear ? GL_LINEAR : GL_NEAREST));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bilinear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (mipmap && gMaxAniso)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, gMaxAniso);
    }
}

GLuint genTex(uint32 color)
{
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);
    setTexFilter(0, 0);
    return id;
}

void compileShader(Shader* shader, const char* text)
{
    int32 i;
    GLchar info[1024];
    GLuint obj;
    const GLenum type[2] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };

#if defined(__MAC__)
    const GLchar *code[2][3] = {
        { GLSL_HEADER_VERT_GL2, "#line 0\n", text },
        { GLSL_HEADER_FRAG_GL2, "#line 0\n", text }
    };
#else
    const GLchar *code[2][3] = {
        { GLSL_HEADER_VERT_GL3, "#line 0\n", text },
        { GLSL_HEADER_FRAG_GL3, "#line 0\n", text }
    };
#endif
    
    shader->id = glCreateProgram();

    for (i = 0; i < 2; i++)
    {
        obj = glCreateShader(type[i]);
        glShaderSource(obj, 3, code[i], NULL);
        glCompileShader(obj);

        glGetShaderInfoLog(obj, sizeof(info), NULL, info);
        if (info[0] && strlen(info) > 8)
        {
            LOG("%s\n", info);
            ASSERT(0);
        }

        glAttachShader(shader->id, obj);
        glDeleteShader(obj);
    }

    glBindAttribLocation(shader->id, aCoord, "aCoord");
    glBindAttribLocation(shader->id, aTexCoord, "aTexCoord");
    glBindAttribLocation(shader->id, aColor, "aColor");

    glLinkProgram(shader->id);

    glGetProgramInfoLog(shader->id, sizeof(info), NULL, info);
    if (info[0] && strlen(info) > 8)
    {
        LOG("%s\n", info);
        ASSERT(0);
    }


    i = 0;
    glUseProgram(shader->id);
    glUniform1iv(glGetUniformLocation(shader->id, "sDiffuse"), 1, &i);

    shader->uid[uProjMatrix] = glGetUniformLocation(shader->id, "uProjMatrix");
    shader->uid[uViewMatrix] = glGetUniformLocation(shader->id, "uViewMatrix");
    shader->uid[uColor] = glGetUniformLocation(shader->id, "uColor");
    shader->uid[uResolution] = glGetUniformLocation(shader->id, "uResolution");
}

void* GetProc(const char *name)
{
#if defined(__WIN32__)
    return (void*)wglGetProcAddress(name);
#elif defined(__LINUX__)
    return (void*)glXGetProcAddress((GLubyte*)name);
#endif
    return NULL;
}

#define GetProcOGL(x) x=GetProc(#x)

void renderInit()
{
    VertexGL* v = NULL;
#if defined(__WIN32__)
    int format;
    PFNWGLCHOOSEPIXELFORMATARBPROC    wglChoosePixelFormatARB    = NULL;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
    PIXELFORMATDESCRIPTOR pfd;

    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cGreenBits = 8;
    pfd.cBlueBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 24;

    {
        HWND fWnd = CreateWindow("static", NULL, WS_POPUP, 0, 0, 0, 0, 0, 0, 0, 0);
        HDC fDC = GetDC(fWnd);
        HGLRC fRC;

        format = ChoosePixelFormat(fDC, &pfd);
        SetPixelFormat(fDC, format, &pfd);
        fRC = wglCreateContext(fDC);
        wglMakeCurrent(fDC, fRC);

        wglChoosePixelFormatARB    = GetProcOGL(wglChoosePixelFormatARB);
        wglCreateContextAttribsARB = GetProcOGL(wglCreateContextAttribsARB);

        wglMakeCurrent(0, 0);
        ReleaseDC(fWnd, fDC);
        wglDeleteContext(fRC);
        DestroyWindow(fWnd);
    }

    hDC = GetDC(hWnd);

    if (wglChoosePixelFormatARB && wglCreateContextAttribsARB)
    {
        const int pixelAttribs[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
            WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
            WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB,     32,
            WGL_ALPHA_BITS_ARB,     8,
            WGL_DEPTH_BITS_ARB,     24,
            0
        };

        const int contextAttribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 2,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };

        UINT numFormats;
        int status = wglChoosePixelFormatARB(hDC, pixelAttribs, NULL, 1, &format, &numFormats);
        ASSERT(status && numFormats > 0);

        DescribePixelFormat(hDC, format, sizeof(pfd), &pfd);
        SetPixelFormat(hDC, format, &pfd);

        hRC = wglCreateContextAttribsARB(hDC, 0, contextAttribs);
        LOG("OpenGL 3.2\n");
    } else {
        format = ChoosePixelFormat(hDC, &pfd);
        SetPixelFormat(hDC, format, &pfd);
        hRC = wglCreateContext(hDC);
        LOG("OpenGL 2.0\n");
    }

    wglMakeCurrent(hDC, hRC);
#endif

    LOG("Vendor   : %s\n", (char*)glGetString(GL_VENDOR));
    LOG("Renderer : %s\n", (char*)glGetString(GL_RENDERER));
    LOG("Version  : %s\n", (char*)glGetString(GL_VERSION));

    glClearColor(0, 0, 0, 1);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

#if defined(__WIN32__) || defined(__LINUX__)
    GetProcOGL(glGenerateMipmap);

    GetProcOGL(glCreateProgram);
    GetProcOGL(glDeleteProgram);
    GetProcOGL(glLinkProgram);
    GetProcOGL(glUseProgram);
    GetProcOGL(glGetProgramInfoLog);
    GetProcOGL(glCreateShader);
    GetProcOGL(glDeleteShader);
    GetProcOGL(glShaderSource);
    GetProcOGL(glAttachShader);
    GetProcOGL(glCompileShader);
    GetProcOGL(glGetShaderInfoLog);
    GetProcOGL(glGetUniformLocation);
    GetProcOGL(glUniform1iv);
    GetProcOGL(glUniform1fv);
    GetProcOGL(glUniform2fv);
    GetProcOGL(glUniform3fv);
    GetProcOGL(glUniform4fv);
    GetProcOGL(glUniformMatrix4fv);
    GetProcOGL(glBindAttribLocation);
    GetProcOGL(glEnableVertexAttribArray);
    GetProcOGL(glDisableVertexAttribArray);
    GetProcOGL(glVertexAttribPointer);
    GetProcOGL(glGetProgramiv);

    GetProcOGL(glGenBuffers);
    GetProcOGL(glDeleteBuffers);
    GetProcOGL(glBindBuffer);
    GetProcOGL(glBufferData);
    GetProcOGL(glBufferSubData);

    GetProcOGL(glGenVertexArrays);
    GetProcOGL(glDeleteVertexArrays);
    GetProcOGL(glBindVertexArray);
#endif

#if defined(__WIN32__)
    GetProcOGL(wglSwapIntervalEXT);
#elif defined(__LINUX__)
    GetProcOGL(glXSwapIntervalSGI);
#endif

    compileShader(&shaderTrack, sh_track);
    compileShader(&shaderObject, sh_object);
    compileShader(&shaderShip, sh_ship);
    compileShader(&shaderShield, sh_shield);
    compileShader(&shaderSky, sh_sky);
    compileShader(&shaderBlob, sh_blob);
    compileShader(&shaderTarget, sh_target);
    compileShader(&shaderGUI, sh_gui);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(2, VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_RENDER_INDICES * sizeof(IndexGL), NULL, GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, MAX_RENDER_VERTICES * sizeof(VertexGL), NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(aCoord);
    glEnableVertexAttribArray(aTexCoord);
    glEnableVertexAttribArray(aColor);

    glVertexAttribPointer(aCoord, 3, GL_INT, GL_FALSE, sizeof(*v), &v->x);
    glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(*v), &v->s);
    glVertexAttribPointer(aColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(*v), &v->color);

    whiteTex = genTex(0xFFFFFFFF);

    glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gMaxAniso);

    renderSetVSync(1);
}

void renderFree(void)
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(2, VBO);
 #if defined(__WIN32__)
    wglMakeCurrent(0, 0);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
#endif
}

void renderSetVSync(int32 enable)
{
#if defined(__WIN32__)
    if (wglSwapIntervalEXT) wglSwapIntervalEXT(enable ? 1 : 0);
#elif defined(__LINUX__)
    if (glXSwapIntervalSGI) glXSwapIntervalSGI(enable ? 1 : 0);
#endif
}

void renderSwap(void)
{
#if defined(__WIN32__)
    SwapBuffers(hDC);
#elif defined(__LINUX__)
    glXSwapBuffers(dpy, wnd);
#endif
}

void renderResize(int width, int height)
{
    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;
    gWidth = width;
    gHeight = height;
}

void renderOffset(int x, int y)
{
    xOffset = x;
    yOffset = y;
}

void renderSetBlend(int blend)
{
    if (gCurState.blend != blend)
    {
        gCurState.blend = blend;
        gStatePtr = NULL;
    }
}

void renderDepthWrite(int enable)
{
    if (gCurState.depthWrite != enable)
    {
        gCurState.depthWrite = enable;
        gStatePtr = NULL;
    }
}

void renderDepthTest(int enable)
{
    if (gCurState.depthTest != enable)
    {
        gCurState.depthTest = enable;
        gStatePtr = NULL;
    }
}

void renderSetColor(uint32 color)
{
    if (gCurState.color != color)
    {
        gCurState.color = color;
        gStatePtr = NULL;
    }
}

void renderNextLayer()
{
    gCurState.layer++;
    gStatePtr = NULL;
}

void renderSetTexture(const Texture* texture)
{
    if (gCurState.texture != texture)
    {
        gCurState.texture = texture;
        gStatePtr = NULL;
    }
}

void renderSetShader(const Shader* shader)
{
    if (gCurState.shader != shader)
    {
        gCurState.shader = shader;
        gStatePtr = NULL;
    }
}


void renderClear(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void addTriangle()
{
    ASSERT(gIndicesCount + 3 <= MAX_RENDER_INDICES);

    gIndices[gIndicesCount++] = gVerticesCount;
    gIndices[gIndicesCount++] = gVerticesCount + 1;
    gIndices[gIndicesCount++] = gVerticesCount + 2;

    gStatePtr->mesh->iCount += 3;
}

void addQuad()
{
    ASSERT(gIndicesCount + 6 <= MAX_RENDER_INDICES);

    gIndices[gIndicesCount++] = gVerticesCount;
    gIndices[gIndicesCount++] = gVerticesCount + 1;
    gIndices[gIndicesCount++] = gVerticesCount + 2;

    gIndices[gIndicesCount++] = gVerticesCount;
    gIndices[gIndicesCount++] = gVerticesCount + 2;
    gIndices[gIndicesCount++] = gVerticesCount + 3;

    gStatePtr->mesh->iCount += 6;
}

void addVertex(int32 x, int32 y, int32 z, float s, float t, uint32 color)
{
    ASSERT(gVerticesCount < MAX_RENDER_VERTICES);

    VertexGL* v = gVertices + gVerticesCount++;
    v->x = x;
    v->y = y;
    v->z = z;
    v->s = s;
    v->t = t;
    v->color = color;
}

void addVertexI(const vec3i* pos, float s, float t, uint32 color)
{
    ASSERT(gVerticesCount < MAX_RENDER_VERTICES);

    VertexGL* v = gVertices + gVerticesCount++;
    v->x = pos->x;
    v->y = pos->y;
    v->z = pos->z;
    v->s = s;
    v->t = t;
    v->color = color;
}

void addVertexS(const vec3s* pos, float s, float t, uint32 color)
{
    ASSERT(gVerticesCount < MAX_RENDER_VERTICES);

    VertexGL* v = gVertices + gVerticesCount++;
    v->x = pos->x;
    v->y = pos->y;
    v->z = pos->z;
    v->s = s;
    v->t = t;
    v->color = color;
}

void renderGlyph(const Glyph* g, int32 x, int32 y, uint32 color)
{
    const Texture* t = g->texture;
    float u0 = g->x * t->iw;
    float v0 = g->y * t->ih;
    float u1 = (g->x + g->w) * t->iw;
    float v1 = (g->y + g->h) * t->ih;
    int x0 = x;
    int y0 = y;
    int x1 = x + g->w;
    int y1 = y + g->h;

    renderSetTexture(t);
    stateCheck();

    addQuad();
    addVertex(x0, y0, 0, u0, v0, color);
    addVertex(x1, y0, 0, u1, v0, color);
    addVertex(x1, y1, 0, u1, v1, color);
    addVertex(x0, y1, 0, u0, v1, color);
}

void renderImage(const Texture* t, int32 x, int32 y)
{
    const uint32 color = 0xFF808080;

    float u0 = 0;
    float v0 = 0;
    float u1 = 1.0f;
    float v1 = 1.0f;
    int x0 = x;
    int y0 = y;
    int x1 = x + t->width;
    int y1 = y + t->height;

    renderSetTexture(t);
    stateCheck();

    addQuad();
    addVertex(x0, y0, 0, u0, v0, color);
    addVertex(x1, y0, 0, u1, v0, color);
    addVertex(x1, y1, 0, u1, v1, color);
    addVertex(x0, y1, 0, u0, v1, color);
}

void renderBackground(const Texture* t)
{
    renderImage(t, 0, 0);
}

#define SHADOW_SHIFT_N  3
#define SHADOW_SHIFT_W  4

extern Texture* gShadowTex;

void renderShadow(int32 type, int32 intensity, int32 px, int32 py, int32 pz, int32 dx, int32 dy, int32 dz, int32 rx, int32 ry, int32 rz)
{
    uint32 color = (intensity << 24) | 0xFFFFFF;

    dx >>= SHADOW_SHIFT_N;
    dy >>= SHADOW_SHIFT_N;
    dz >>= SHADOW_SHIFT_N;

    rx >>= SHADOW_SHIFT_W;
    ry >>= SHADOW_SHIFT_W;
    rz >>= SHADOW_SHIFT_W;

    renderSetShader(&shaderBlob);
    renderSetTexture(gShadowTex);
    stateCheck();

    addQuad();
    addVertex(px + dx - rx, py + dy - ry, pz + dz - rz, 0.0f, 0.0f, color);
    addVertex(px + dx + rx, py + dy + ry, pz + dz + rz, 1.0f, 0.0f, color);
    addVertex(px - dx + rx, py - dy + ry, pz - dz + rz, 1.0f, 1.0f, color);
    addVertex(px - dx - rx, py - dy - ry, pz - dz - rz, 0.0f, 1.0f, color);
}

void renderColorQuad(const ColorQuad* q)
{
    renderSetTexture(NULL);
    stateCheck();

    addQuad();
    addVertex(q->x0, q->y0, 0, 0.0f, 0.0f, q->color[0]);
    addVertex(q->x1, q->y1, 0, 0.0f, 0.0f, q->color[1]);
    addVertex(q->x2, q->y2, 0, 0.0f, 0.0f, q->color[2]);
    addVertex(q->x3, q->y3, 0, 0.0f, 0.0f, q->color[3]);
}

void renderBillboard(const Billboard* b)
{
    int rx, ry, rz, ux, uy, uz, size;
    const Texture* t = b->texture;

    size = b->size * 8;

    rx = gMatrixPtr->e00 * size >> FIXED_SHIFT;
    ry = gMatrixPtr->e01 * size >> FIXED_SHIFT;
    rz = gMatrixPtr->e02 * size >> FIXED_SHIFT;

    ux = gMatrixPtr->e10 * size >> FIXED_SHIFT;
    uy = gMatrixPtr->e11 * size >> FIXED_SHIFT;
    uz = gMatrixPtr->e12 * size >> FIXED_SHIFT;

    renderSetTexture(t);
    renderSetShader(&shaderObject);
    stateCheck();

    addQuad();
    addVertex(b->x - rx - ux, b->y - ry - uy, b->z - rz - uz, 0.0f, 0.0f, colorGrey);
    addVertex(b->x + rx - ux, b->y - ry - uy, b->z + rz - uz, 1.0f, 0.0f, colorGrey);
    addVertex(b->x + rx + ux, b->y + ry + uy, b->z + rz + uz, 1.0f, 1.0f, colorGrey);
    addVertex(b->x - rx + ux, b->y + ry + uy, b->z - rz + uz, 0.0f, 1.0f, colorGrey);
}

void renderBackFill(int x, int y, int w, int h)
{
    ColorQuad q;
    q.x0 = x;
    q.y0 = y;
    q.x1 = x + w;
    q.y1 = y;
    q.x2 = x + w;
    q.y2 = y + h;
    q.x3 = x;
    q.y3 = y + h;
    q.color[0] =
    q.color[1] =
    q.color[2] =
    q.color[3] = 0xFF404040;

    renderNextLayer();
    renderSetBlend(BLEND_MUL);
    renderColorQuad(&q);
    renderSetBlend(BLEND_ALPHA);
}

void prepareTrack(void)
{
    int i;

    ASSERT(ctrlnumLib <= MAX_TRACK_TEX);
    for (i = 0; i < ctrlnumLib; i++)
    {
        glGenTextures(1, &gTrackTex[i].id);
        glBindTexture(GL_TEXTURE_2D, gTrackTex[i].id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 128, 128, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, gTrackTexData[i]);

        setTexFilter(1, glGenerateMipmap != 0);
        if (glGenerateMipmap)
        {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
    }
}

void FillTrackTex(uint16* data)
{
    int id, i, j, k, x, y;

    id = (gTexturePtr - gTextures) - ctrllibraryTextures;

    for (i = 0; i < ctrlnumLib; i++)
    {
        for (j = 0; j < 16; j++)
        {
            if (ctrltexTemplate[i].tex16[j] != id)
                continue;

            x = (j % 4) * 32;
            y = (j / 4) * 32;

            for (k = 0; k < 32; k++)
            {
                memcpy(gTrackTexData[i] + (k + y) * 128 + x, data + k * 32, 32 * sizeof(uint16));
            }
        }
    }
}

void renderPrepare(void)
{
    prepareTrack();
}

void renderSetMatrix(const Matrix* matrix)
{
    mat4* m = gMatrices + gMatricesCount;

    gCurState.viewMatrix = gMatricesCount++;
    gStatePtr = NULL;

    m->e[0][0] = matrix->e00 * i4096;
    m->e[0][1] = matrix->e10 * i4096;
    m->e[0][2] = matrix->e20 * i4096;
    m->e[0][3] = 0.0f;

    m->e[1][0] = matrix->e01 * i4096;
    m->e[1][1] = matrix->e11 * i4096;
    m->e[1][2] = matrix->e21 * i4096;
    m->e[1][3] = 0.0f;

    m->e[2][0] = matrix->e02 * i4096;
    m->e[2][1] = matrix->e12 * i4096;
    m->e[2][2] = matrix->e22 * i4096;
    m->e[2][3] = 0.0f;

    m->e[3][0] = matrix->e03;
    m->e[3][1] = matrix->e13;
    m->e[3][2] = matrix->e23;
    m->e[3][3] = 1.0f;
}

void renderFrameBegin(void)
{
    mat4* m;

    // get fixed right vector for sprites
    rsincos(gCamera.hdg, gCylZ, gCylX);

    gBorder = ((gWidth * 240) / gHeight - 320) >> 1;

    gCurState.texture = NULL;
    gCurState.blend = BLEND_NONE;
    gCurState.projMatrix = 1;
    gCurState.viewMatrix = 0;
    gCurState.layer = 0;
    gCurState.blend = BLEND_NONE;
    gCurState.depthWrite = 1;
    gCurState.depthTest = 0;
    gStatePtr = NULL;

    gRenderMode = RENDER_MODE_3D;
    renderSetMode(RENDER_MODE_2D);
    renderSetColor(colorWhite);

    // identity matrix
    m = gMatrices + 0;
    matrixIdentity(m);

    // otho projection matrix
    m = gMatrices + 1;
    matrixOrtho(m, -gBorder, 320 + gBorder, 240, 0, 0, 1);

    gMatricesCount = 2;

    glViewport(0, 0, gWidth, gHeight);
}

void renderFrameEnd(void)
{
    int32 i, start, count;
    const RenderState* state;
    const Mesh* mesh;

    const Shader* shader = NULL;
    const Texture* texture = NULL;
    int32 projMatrix = -1;
    int32 viewMatrix = -1;
    int32 depthTest = 0;
    int32 depthWrite = 1;
    int32 blend = BLEND_NONE;
    uint32 color = 0xBAAAAAAD;
    int32 resolution = 0;

    const GLfloat resolutionValue[4] = {
        1.f / (240.0f * gWidth / gHeight), 1.0f / 240.0f, 1.0 / gWidth, 1.0 / gHeight
    };

    GLfloat colorValue[4] = {
        1.0f, 1.0f, 1.0f, 1.0f
    };


    count = 0;
    for (i = 0; i < gStatesCount; i++)
    {
        state = gStates + i;

        start = count;

        mesh = state->mesh;
        ASSERT(mesh);
        while (mesh)
        {
            memcpy(gIndicesMerged + count, gIndices + mesh->iStart, mesh->iCount * sizeof(IndexGL));
            count += mesh->iCount;
            mesh = mesh->next;
        }

        state->mesh->next = NULL;
        state->mesh->iStart = start;
        state->mesh->iCount = count - start;
    }

    glBindTexture(GL_TEXTURE_2D, whiteTex);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, gIndicesCount * sizeof(IndexGL), gIndicesMerged);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gVerticesCount * sizeof(VertexGL), gVertices);

    for (i = 0; i < gStatesCount; i++)
    {
        state = gStates + i;

        // set shader
        if (shader != state->shader)
        {
            shader = state->shader;
            // need to reset all uniforms for the shader
            projMatrix = -1;
            viewMatrix = -1;
            color = 0xBAAAAAAD;
            resolution = 0;
            glUseProgram(shader->id);
        }

        // set texture
        if (texture != state->texture)
        {
            texture = state->texture;
            glBindTexture(GL_TEXTURE_2D, texture ? texture->id : whiteTex);
        }

        // set geomProj matrix
        if (projMatrix != state->projMatrix)
        {
            projMatrix = state->projMatrix;
            ASSERT(shader);
            if (shader->uid[uProjMatrix] > -1)
            {
                glUniformMatrix4fv(shader->uid[uProjMatrix], 1, GL_FALSE, (GLfloat*)(gMatrices + projMatrix));
            }
        }

        // set view matrix
        if (viewMatrix != state->viewMatrix)
        {
            viewMatrix = state->viewMatrix;
            ASSERT(shader);
            if (shader->uid[uViewMatrix] > -1)
            {
                glUniformMatrix4fv(shader->uid[uViewMatrix], 1, GL_FALSE, (GLfloat*)(gMatrices + viewMatrix));
            }
        }

        // set color (ships only)
        if (color != state->color)
        {
            color = state->color;

            if (shader->uid[uColor] > -1)
            {
                colorValue[0] = i255 * (0xFF & (color));
                colorValue[1] = i255 * (0xFF & (color >> 8));
                colorValue[2] = i255 * (0xFF & (color >> 16));
                glUniform4fv(shader->uid[uColor], 1, colorValue);
            }
        }

        // set screen resolution (for target icon only)
        if (!resolution)
        {
            resolution = 1;
            if (shader->uid[uResolution] > -1)
            {
                glUniform4fv(shader->uid[uResolution], 1, resolutionValue);
            }
        }

        // set depth test
        if (depthTest != state->depthTest)
        {
            depthTest = state->depthTest;
            if (depthTest)
                glEnable(GL_DEPTH_TEST);
            else
                glDisable(GL_DEPTH_TEST);
        }

        // set depth write
        if (depthWrite != state->depthWrite)
        {
            depthWrite = state->depthWrite;
            glDepthMask(depthWrite ? GL_TRUE : GL_FALSE);
        }

        // set blending
        if (blend != state->blend)
        {
            if (blend == BLEND_NONE)
            {
                glEnable(GL_BLEND);
            }
            blend = state->blend;

            switch (blend)
            {
                case BLEND_NONE:
                    glDisable(GL_BLEND);
                    break;
                case BLEND_ALPHA:
                    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                    break;
                case BLEND_ADD:
                    glBlendFunc(GL_ONE, GL_ONE);
                    break;
                case BLEND_SUB:
                    glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
                    break;
                case BLEND_MUL:
                    glBlendFunc(GL_DST_COLOR, GL_ZERO);
                    break;
            }
        }

        mesh = state->mesh;
        ASSERT(mesh);
        ASSERT(mesh->next == NULL);
        glDrawElements(GL_TRIANGLES, mesh->iCount, GL_UNSIGNED_SHORT, (GLvoid*)(mesh->iStart * sizeof(IndexGL)));
    }

#ifdef DEBUG_RENDER
    {
        static int32 dbg_v, dbg_i, dbg_x, dbg_m, dbg_s;
        if (dbg_v < gVerticesCount)
            dbg_v = gVerticesCount;
        if (dbg_i < gIndicesCount)
            dbg_i = gIndicesCount;
        if (dbg_x < gMatricesCount)
            dbg_x = gMatricesCount;
        if (dbg_m < gMeshesCount)
            dbg_m = gMeshesCount;
        if (dbg_s < gStatesCount)
            dbg_s = gStatesCount;

        LOG("v:%d i:%d x:%d m:%d s:%d\n", dbg_v, dbg_i, dbg_x, dbg_m, dbg_s);
    }
#endif

    gVerticesCount = 0;
    gIndicesCount = 0;
    gMatricesCount = 0;
    gMeshesCount = 0;
    gStatesCount = 0;
}

void renderSection(const TrackSection* section)
{
    const Face* face = gTrack.faces + section->faceStart;
    const vec3i* vertices = gTrack.vertices;
    int i;
    float u0, u1;
    uint32 color;

    for (i = 0; i < section->faceCount; i++, face++)
    {
        ASSERT(face->texture < ctrlnumLib);

    #ifdef CHECK_UNUSED_TEX
        texUsed[face->texture] = 1;
    #endif

        renderSetTexture(gTrackTex + face->texture);
        stateCheck();

        color = face->red | (face->green << 8) | (face->blue << 16) | 0xFF000000;

        if (face->flags & FLIP_TEXTURE)
        {
            u0 = 0.0f;
            u1 = 1.0f;
        }
        else
        {
            u0 = 1.0f;
            u1 = 0.0f;
        }

        addQuad();
        addVertexI(vertices + face->vertex[3], u0, 1.0f, color);
        addVertexI(vertices + face->vertex[2], u1, 1.0f, color);
        addVertexI(vertices + face->vertex[1], u1, 0.0f, color);
        addVertexI(vertices + face->vertex[0], u0, 0.0f, color);
    }
}

void renderTrack(void)
{
    int i, j, s;
    int dir;
    short* counts;
    short** sections;
    TrackSection* section;

//    #define CHECK_UNUSED_TEX

#ifdef CHECK_UNUSED_TEX
    uint8 texUsed[MAX_TRACK_TEX];
    memset(texUsed, 0, MAX_TRACK_TEX);
#endif

    dir = TrackDirection(&gCamera);

    renderSetMatrix(gMatrixPtr);
    renderSetShader(&shaderTrack);

    switch (dir)
    {
        case VIEW_DIR_NORTH:
            counts = gCamera.section->northViewCount;
            sections = gCamera.section->northViewSection;
            break;
        case VIEW_DIR_SOUTH:
            counts = gCamera.section->southViewCount;
            sections = gCamera.section->southViewSection;
            break;
        case VIEW_DIR_WEST:
            counts = gCamera.section->westViewCount;
            sections = gCamera.section->westViewSection;
            break;
        default: // VIEW_DIR_EAST
            counts = gCamera.section->eastViewCount;
            sections = gCamera.section->eastViewSection;
    }

    section = gTrack.sections + sections[0][0];
    renderSection(section->prevSection);

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < counts[i]; j += 2)
        {
            section = gTrack.sections + sections[i][j];

            for (s = 0; s < sections[i][j + 1]; s++)
            {
                renderSection(section);
                section = section->nextSection;
            }
        }
    }

#ifdef CHECK_UNUSED_TEX
    for (i = 0; i < ctrlnumLib; i++)
    {
        ASSERT(texUsed[i]);
    }
#endif
}

void renderEffects(const Effect* fx)
{
    Billboard b;

    renderSetMatrix(gMatrixPtr);
    renderDepthWrite(0);
    renderSetBlend(BLEND_ADD);

    while (fx)
    {
        b.x = fx->pos.x;
        b.y = fx->pos.y;
        b.z = fx->pos.z;
        b.size = fx->size;
        b.texture = gEffectTextures + fx->type;
        renderBillboard(&b);

        fx = fx->next;
    }

    renderDepthWrite(1);
    renderSetBlend(BLEND_NONE);
}

void renderMesh(const Object* obj)
{
    int32 i;
    float iw, ih;
    const vec3s* v = obj->vertices;
    const ObjFace* f = obj->faces;
    const ObjSprite* s = obj->sprites;

    for (i = 0; i < obj->fCount; i++, f++)
    {
        uint32 texIndex = FACE_TEXTURE(f);

        // TODO precalc float uv's
        if (texIndex != NO_TEXTURE)
        {
            Texture* t = gTextures + FACE_TEXTURE(f);
            renderSetTexture(t);
            iw = t->iw;
            ih = t->ih;
        }
        else
        {
            renderSetTexture(NULL);
            iw = 0.0f;
            ih = 0.0f;
        }

        if (FACE_QUAD(f))
        {
            stateCheck();

            addQuad();
            addVertexS(v + f->index[0], f->uv[0][0] * iw, f->uv[0][1] * ih, f->color[0]);
            addVertexS(v + f->index[1], f->uv[1][0] * iw, f->uv[1][1] * ih, f->color[1]);
            addVertexS(v + f->index[2], f->uv[2][0] * iw, f->uv[2][1] * ih, f->color[2]);
            addVertexS(v + f->index[3], f->uv[3][0] * iw, f->uv[3][1] * ih, f->color[3]);
        }
        else
        {
            if (f->flags & FACE_FLAG_ENGINE) // triangles only
            {
                uint32 color = gCurState.color;

                renderSetColor(colorWhite);
                renderSetBlend(BLEND_ADD);
                stateCheck();

                addTriangle();
                addVertexS(v + f->index[0], f->uv[0][0] * iw, f->uv[0][1] * ih, colorWhite);
                addVertexS(v + f->index[1], f->uv[1][0] * iw, f->uv[1][1] * ih, colorWhite);
                addVertexS(v + f->index[2], f->uv[2][0] * iw, f->uv[2][1] * ih, colorWhite);

                renderSetBlend(BLEND_NONE);
                renderSetColor(color);
            }
            else
            {
                stateCheck();

                addTriangle();
                addVertexS(v + f->index[0], f->uv[0][0] * iw, f->uv[0][1] * ih, f->color[0]);
                addVertexS(v + f->index[1], f->uv[1][0] * iw, f->uv[1][1] * ih, f->color[1]);
                addVertexS(v + f->index[2], f->uv[2][0] * iw, f->uv[2][1] * ih, f->color[2]);
            }
        }
    }

    for (i = 0; i < obj->sCount; i++, s++)
    {
        Texture* t = gTextures + SPRITE_TEXTURE(s);
        const vec3s* p = v + s->index;

        int32 w = s->width >> 1;
        int32 wx = gCylX * w >> FIXED_SHIFT;
        int32 wz = gCylZ * w >> FIXED_SHIFT;
        int32 h0, h1;

        if (SPRITE_FLIP(s))
        {
            h0 = s->height;
            h1 = 0;
        }
        else
        {
            h0 = 0;
            h1 = s->height;
        }

        renderSetTexture(t);
        stateCheck();

        addQuad();
        addVertex(p->x - wx, p->y - h0, p->z - wz, 0.0f, 0.0f, s->color);
        addVertex(p->x + wx, p->y - h0, p->z + wz, 1.0f, 0.0f, s->color);
        addVertex(p->x + wx, p->y + h1, p->z + wz, 1.0f, 1.0f, s->color);
        addVertex(p->x - wx, p->y + h1, p->z - wz, 0.0f, 1.0f, s->color);
    }
}

void renderObject(const Object* object)
{
    renderSetMatrix(gMatrixPtr);
    renderSetShader(&shaderObject);

    renderMesh(object);
}

void renderShip(const Object* object, uint32 color)
{
    renderSetMatrix(gMatrixPtr);
    renderSetShader(&shaderShip);
    renderSetColor(color);

    renderMesh(object);

    renderSetColor(0xFFFFFFFF);
}

void renderShield(const Object* object, int32 counter)
{
    renderSetMatrix(gMatrixPtr);
    renderSetShader(&shaderShield);
    renderSetBlend(BLEND_ADD);
    renderSetColor(counter);

    renderMesh(object);

    renderSetBlend(BLEND_NONE);
}

void renderSky(const Object* object)
{
    renderSetMatrix(gMatrixPtr);
    renderSetShader(&shaderSky);
    renderDepthTest(0);

    renderMesh(object);

    renderDepthTest(1);
}

void renderTargetIcon(Texture* tex)
{
    renderSetBlend(BLEND_ADD);
    renderDepthTest(0);

    renderSetMatrix(gMatrixPtr);
    renderSetShader(&shaderTarget);
    renderSetTexture(tex);
    stateCheck();

    addQuad();
    addVertex(0, 0, 0, 0.0f, 1.0f, 0);
    addVertex(0, 0, 0, 1.0f, 1.0f, 0);
    addVertex(0, 0, 0, 1.0f, 0.0f, 0);
    addVertex(0, 0, 0, 0.0f, 0.0f, 0);

    renderDepthTest(1);
    renderSetBlend(BLEND_ALPHA);
}

void renderSetMode(int renderMode)
{
    if (gRenderMode == renderMode)
        return;
    gRenderMode = renderMode;

    if (renderMode == RENDER_MODE_2D)
    {
        renderDepthTest(0);
        renderSetBlend(BLEND_ALPHA);

        gCurState.viewMatrix = 0;
        gCurState.projMatrix = 1;
        gCurState.shader = &shaderGUI;
        gStatePtr = NULL;
    }
    else
    {
        renderDepthTest(1);
        renderSetBlend(BLEND_NONE);

        matrixSetViewMenu();

        matrixPerspective(gMatrices + gMatricesCount, gCameraFOV / 4096.0f, (float)gWidth / (float)gHeight, -xOffset / 160.0f, yOffset / 120.f, (float)64, (float)(64 * 1024));

        gCurState.viewMatrix = 0;
        gCurState.projMatrix = gMatricesCount++;
        gCurState.shader = NULL;
        gStatePtr = NULL;
    }
}

uint16 RGB5551(uint16 c)
{
    char r = (c) & 0x1F;
    char g = (c >> 5) & 0x1F;
    char b = (c >> 10) & 0x1F;
    c = (r << 11) | (g << 6) | (b << 1);

    if (c != 0)
    {
        c |= 1;
    }

    return c;
}

uint16 RGB555(uint16 c)
{
    char r = (c) & 0x1F;
    char g = (c >> 5) & 0x1F;
    char b = (c >> 10) & 0x1F;
    c = (r << 11) | (g << 6) | (b << 1) | 1;
    return c;
}

void dumpBitmap(const char* fileName, int32 width, int32 height, uint8* data32)
{
    struct BITMAPFILEHEADER {
        uint32  bfSize;
        uint16  bfReserved1;
        uint16  bfReserved2;
        uint32  bfOffBits;
    } fhdr;

    struct BITMAPINFOHEADER{
        uint32 biSize;
        uint32 biWidth;
        uint32 biHeight;
        uint16 biPlanes;
        uint16 biBitCount;
        uint32 biCompression;
        uint32 biSizeImage;
        uint32 biXPelsPerMeter;
        uint32 biYPelsPerMeter;
        uint32 biClrUsed;
        uint32 biClrImportant;
    } ihdr;

    memset(&fhdr, 0, sizeof(fhdr));
    memset(&ihdr, 0, sizeof(ihdr));

    ihdr.biSize      = sizeof(ihdr);
    ihdr.biWidth     = width;
    ihdr.biHeight    = height;
    ihdr.biPlanes    = 1;
    ihdr.biBitCount  = 16;
    ihdr.biSizeImage = width * height * 2;

    fhdr.bfOffBits   = 2 + sizeof(fhdr) + ihdr.biSize;
    fhdr.bfSize      = fhdr.bfOffBits + ihdr.biSizeImage;

    FILE* f = fopen(fileName, "wb");
    ASSERT(f);

    uint16 tag = 'B' + ('M' << 8);
    fwrite(&tag, sizeof(tag), 1, f);
    fwrite(&fhdr, sizeof(fhdr), 1, f);
    fwrite(&ihdr, sizeof(ihdr), 1, f);

    data32 += width * height * 2;
    for (int32 i = 0; i < height; i++)
    {
        data32 -= width * 2;
        fwrite(data32, 1, width * 2, f);
    }

    fclose(f);
}

Texture* renderCreateTexture(uint8* ptr)
{
    Tim* tim = (Tim*)ptr;
    Texture* tex;
    int32 w, h;
    int32 i, j;

    if (TimType(tim) == TIM_4)
    {
        uint16 clut[16];
        Tim4* t = &tim->tim4;
        uint8* src = (uint8*)(t + 1);
        uint16* dst = tempData;

        w = t->textureW * 4;
        h = t->textureH;

        for (i = 0; i < 16; i++)
        {
            clut[i] = RGB5551(t->clut[i]);
        }

        for (i = 0; i < h; i++)
        {
            for (j = 0; j < (w >> 1); j++)
            {
                uint8 indexAB = *src++;
                uint8 indexA = (indexAB & 0x0F);
                uint8 indexB = (indexAB & 0xF0) >> 4;

                *dst++ = clut[indexA];
                *dst++ = clut[indexB];
            }
        }
    }
    else if (TimType(tim) == TIM_8)
    {
        uint16 clut[256];
        Tim8* t = &tim->tim8;
        uint8* src = (uint8*)(t + 1);
        uint16* dst = tempData;
        w = t->textureW * 2;
        h = t->textureH;

        for (i = 0; i < 256; i++)
        {
            clut[i] = RGB5551(t->clut[i]);
        }

        for (i = 0; i < h; i++)
        {
            for (j = 0; j < w; j++)
            {
                *dst++ = clut[*src++];
            }
        }

        //dumpBitmap("test.bmp", w, h, tempData);
    }
    else if (TimType(tim) == TIM_16)
    {
        Tim16* t = &tim->tim16;
        uint16* src = (uint16*)(t + 1);
        uint16* dst = tempData;
        w = t->textureW;
        h = t->textureH;

        for (i = 0; i < h; i++)
        {
            for (j = 0; j < w; j++)
            {
                *dst++ = RGB555(*src++);
            }
        }
    }
    else
    {
        w = 1;
        h = 1;
        ASSERT(0);
    }

    tex = gTexturePtr;
    tex->width = w;
    tex->height = h;
    tex->iw = 1.0f / w;
    tex->ih = 1.0f / h;

#ifdef _DEBUG
    tex->tim = tim;
#endif

    if (HACK_LOAD_TRACK)
    {
        ASSERT(w == 32);
        ASSERT(h == 32);
        FillTrackTex(tempData);

        tex->id = 0;
    }
    else
    {
        ASSERT(w < 511);
        ASSERT(h < 511);

        glGenTextures(1, &tex->id);
        glBindTexture(GL_TEXTURE_2D, tex->id);

        if (HACK_LOAD_SHADOW)
        {
            uint16* src = tempData;
            uint8* dst = (uint8*)tempData;
            for (i = 0; i < w * h; i++)
            {
                *dst++ = ((*src++ >> 1) & 31) << 3;
            }
            // create alpha only texture for the shadow
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, tempData);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, tempData);
        }

        setTexFilter(HACK_LOAD_SHADOW || HACK_TEX_FILTER, 0);
    }

    gTexturePtr++;

    return tex;
}

void renderDeleteTexture(Texture* texture)
{
    if (texture->id == 0)
        return;
    glDeleteTextures(1, &texture->id);
    texture->id = 0;
}
