#include <windows.h>
#include <gl/GL.h>
#include <gl/glu.h>
#include <gl/glext.h>
#include <gl/wglext.h>

#include "game.h"

//#define DEBUG_RENDER

#define i255    (1.0f / 255.0f)
#define i4096   (1.0f / 4096.0f)

#define PI      3.14159265358979323846f
#define DEG2RAD (PI / 180.0f)
#define RAD2DEG (180.0f / PI)

void* GetProc(const char *name)
{
    return (void*)wglGetProcAddress(name);
}

#define GetProcOGL(x) x=GetProc(#x)

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

typedef struct
{
    uint16 width;
    uint16 height;
    GLuint id;
    float iw;
    float ih;
} TextureGL;

typedef struct
{
    int32 x;
    int32 y;
    int32 z;
    float s, t;
    uint8 r, g, b, a;
} VertexGL;

typedef uint16 IndexGL;

typedef struct
{
    float e[4][4];
} Matrix;

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
    const TextureGL* texture;
    int32 projMatrix;
    int32 viewMatrix;
    uint32 color;
    int32 layer;
    BlendMode blend;
    uint8 depthWrite;
    uint8 depthTest;
} RenderState;

#define MAX_RENDER_VERTICES     (1024 * 16)
#define MAX_RENDER_INDICES      (1024 * 16)
#define MAX_RENDER_MATRICES     (128)
#define MAX_RENDER_MESHES       (1024)
#define MAX_RENDER_STATES       (512)

VertexGL gVertices[MAX_RENDER_VERTICES];
IndexGL gIndices[MAX_RENDER_INDICES];
IndexGL gIndicesMerged[MAX_RENDER_INDICES];
Matrix gMatrices[MAX_RENDER_MATRICES];
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

const TextureGL* gTexture;
GLuint whiteTex;

extern HWND hWnd;
HDC hDC;
HGLRC hRC;

extern const TrackCamera ctrlcamera;

int gRenderMode;

int32 xOffset;
int32 yOffset;

int gWidth, gHeight, gBorder;

int gCylX, gCylZ;

GLuint VAO;
GLuint VBO[2];
Shader shaderTrack;
Shader shaderObject;
Shader shaderShip;
Shader shaderShield;
Shader shaderSky;
Shader shaderBlob;
Shader shaderGUI;

#define MAX_TRACK_TEX   22
TextureGL gTrackTex[MAX_TRACK_TEX];
uint16 gTrackTexData[MAX_TRACK_TEX][128 * 128];

extern TrackData* track;

const CVECTOR colorGrey = { 128, 128, 128, 255 };
const CVECTOR colorWhite = { 255, 255, 255, 255 };

const char* GLSL_HEADER_VERT_GL3 = 
    "#version 150\n"
    "#define VERTEX\n"
    "#define varying out\n"
    "#define attribute in\n"
    "#define texture2D texture\n";

const char* GLSL_HEADER_FRAG_GL3 = 
    "#version 150\n"
    "#define FRAGMENT\n"
    "#define varying in\n"
    "#define texture2D texture\n"
    "out vec4 fragColor;\n";

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

void matrixIdentity(Matrix* m)
{
    memset(m, 0, sizeof(Matrix));
    m->e[0][0] = m->e[1][1] = m->e[2][2] = m->e[3][3] = 1.0f;
}

void matrixOrtho(Matrix* m, float l, float r, float b, float t, float znear, float zfar)
{
    memset(m, 0, sizeof(Matrix));
    m->e[0][0] = 2.0f / (r - l);
    m->e[1][1] = 2.0f / (t - b);
    m->e[2][2] = 2.0f / (znear - zfar);
    m->e[3][3] = 1.0f;
    m->e[3][0] = (l + r) / (l - r);
    m->e[3][1] = (t + b) / (b - t);
    m->e[3][2] = (znear + zfar) / (znear - zfar);
}

void matrixPerspective(Matrix* m, float fov, float aspect, float xoff, float yoff, float znear, float zfar)
{
    float y = tanf(fov * 0.5f * DEG2RAD) * znear;
    float x = y;

    x = y * aspect;

    memset(m, 0, sizeof(Matrix));
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

int UnloadTexture(TextureGL* pTex)
{
#if 0
    if (pTex->Texture == 0)
        return FALSE;

    glDeleteTextures(1, &pTex->Texture);
    pTex->Texture = 0;
#endif
    return TRUE;
}

void compileShader(Shader* shader, const char* text)
{
    int32 i;
    GLchar info[1024];
    GLuint obj;
    const GLenum type[2] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };

    const GLchar *code[2][3] = {
        { GLSL_HEADER_VERT_GL3, "#line 0\n", text },
        { GLSL_HEADER_FRAG_GL3, "#line 0\n", text }
    };

    shader->id = glCreateProgram();

    for (i = 0; i < 2; i++)
    {
        obj = glCreateShader(type[i]);
        glShaderSource(obj, 3, code[i], NULL);
        glCompileShader(obj);

        glGetShaderInfoLog(obj, sizeof(info), NULL, info);
        if (info[0] && strlen(info) > 8)
        {
            OutputDebugString(info);
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
        OutputDebugString(info);
        ASSERT(0);
    }


    i = 0;
    glUseProgram(shader->id);
    glUniform1iv(glGetUniformLocation(shader->id, "sDiffuse"), 1, &i);

    shader->uid[uProjMatrix] = glGetUniformLocation(shader->id, "uProjMatrix");
    shader->uid[uViewMatrix] = glGetUniformLocation(shader->id, "uViewMatrix");
    shader->uid[uColor] = glGetUniformLocation(shader->id, "uColor");
}

void renderInit(void)
{
    VertexGL* v = NULL;
    int format;
    PIXELFORMATDESCRIPTOR pfd;
    PFNWGLCHOOSEPIXELFORMATARBPROC    wglChoosePixelFormatARB    = NULL;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;

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

        int format = ChoosePixelFormat(fDC, &pfd);
        SetPixelFormat(fDC, format, &pfd);
        HGLRC fRC = wglCreateContext(fDC);
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

        int format;
        UINT numFormats;
        int status = wglChoosePixelFormatARB(hDC, pixelAttribs, NULL, 1, &format, &numFormats);
        ASSERT(status && numFormats > 0);

        DescribePixelFormat(hDC, format, sizeof(pfd), &pfd);
        SetPixelFormat(hDC, format, &pfd);

        int contextAttribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 2,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };

        hRC = wglCreateContextAttribsARB(hDC, 0, contextAttribs);
    } else {
        int format = ChoosePixelFormat(hDC, &pfd);
        SetPixelFormat(hDC, format, &pfd);
        hRC = wglCreateContext(hDC);
        ASSERT(0);
    }

    wglMakeCurrent(hDC, hRC);

    glClearColor(0, 0, 0, 1);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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

    compileShader(&shaderTrack, sh_track);
    compileShader(&shaderObject, sh_object);
    compileShader(&shaderShip, sh_ship);
    compileShader(&shaderShield, sh_shield);
    compileShader(&shaderSky, sh_sky);
    compileShader(&shaderBlob, sh_blob);
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
    glVertexAttribPointer(aColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(*v), &v->r);

    whiteTex = genTex(0xFFFFFFFF);

    glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gMaxAniso);
}

void renderFree(void)
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(2, VBO);
    wglMakeCurrent(0, 0);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
}

void renderSwap(void)
{
    SwapBuffers(hDC);
}

void renderResize(int width, int height)
{
    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;
    gWidth = width;
    gHeight = height;
    glViewport(0, 0, width, height);
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

void renderSetTexture(const TextureGL* texture)
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

void addVertex(int32 x, int32 y, int32 z, float s, float t, const CVECTOR* color)
{
    ASSERT(gVerticesCount < MAX_RENDER_VERTICES);

    VertexGL* v = gVertices + gVerticesCount++;
    v->x = x;
    v->y = y;
    v->z = z;
    v->s = s;
    v->t = t;
    v->r = color->r;
    v->g = color->g;
    v->b = color->b;
    v->a = 255;
}

void addVertexI(const vec4i* pos, float s, float t, const CVECTOR* color)
{
    ASSERT(gVerticesCount < MAX_RENDER_VERTICES);

    VertexGL* v = gVertices + gVerticesCount++;
    v->x = pos->x;
    v->y = pos->y;
    v->z = pos->z;
    v->s = s;
    v->t = t;
    v->r = color->r;
    v->g = color->g;
    v->b = color->b;
    v->a = 255;
}

void addVertexS(const vec4s* pos, float s, float t, const CVECTOR* color)
{
    ASSERT(gVerticesCount < MAX_RENDER_VERTICES);

    VertexGL* v = gVertices + gVerticesCount++;
    v->x = pos->x;
    v->y = pos->y;
    v->z = pos->z;
    v->s = s;
    v->t = t;
    v->r = color->r;
    v->g = color->g;
    v->b = color->b;
    v->a = 255;
}

void renderGlyph(const Glyph* g, int32 x, int32 y, uint32 color)
{
    TextureGL* t = (TextureGL*)g->texture;
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
    addVertex(x0, y0, 0, u0, v0, (CVECTOR*)&color);
    addVertex(x1, y0, 0, u1, v0, (CVECTOR*)&color);
    addVertex(x1, y1, 0, u1, v1, (CVECTOR*)&color);
    addVertex(x0, y1, 0, u0, v1, (CVECTOR*)&color);
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

    renderSetTexture((TextureGL*)t);
    stateCheck();

    addQuad();
    addVertex(x0, y0, 0, u0, v0, (CVECTOR*)&color);
    addVertex(x1, y0, 0, u1, v0, (CVECTOR*)&color);
    addVertex(x1, y1, 0, u1, v1, (CVECTOR*)&color);
    addVertex(x0, y1, 0, u0, v1, (CVECTOR*)&color);
}

void renderBackground(const Texture* t)
{
    renderImage(t, 0, 0);
}

void renderShadow(const Shadow* s)
{
    TextureGL* t = (TextureGL*)s->texture;
    CVECTOR color = { 255, 255, 255, 255 };

    renderSetShader(&shaderBlob);
    renderSetTexture(t);
    stateCheck();

    addQuad();
    addVertex(s->x0, s->y0, s->z0, 0.0f, 0.0f, &color);
    addVertex(s->x1, s->y1, s->z1, 1.0f, 0.0f, &color);
    addVertex(s->x2, s->y2, s->z2, 1.0f, 1.0f, &color);
    addVertex(s->x3, s->y3, s->z3, 0.0f, 1.0f, &color);

    gVertices[gVerticesCount - 3].a =
    gVertices[gVerticesCount - 2].a =
    gVertices[gVerticesCount - 1].a =
    gVertices[gVerticesCount - 0].a = s->intensity;
}

void renderColorQuad(const ColorQuad* q)
{
    renderSetTexture(NULL);
    stateCheck();

    addQuad();
    addVertex(q->x0, q->y0, 0, 0.0f, 0.0f, q->color + 0);
    addVertex(q->x1, q->y1, 0, 0.0f, 0.0f, q->color + 1);
    addVertex(q->x2, q->y2, 0, 0.0f, 0.0f, q->color + 2);
    addVertex(q->x3, q->y3, 0, 0.0f, 0.0f, q->color + 3);
}

void renderBillboard(const Billboard* b)
{
    int sx, sy, sz, size;
    TextureGL* t = (TextureGL*)b->texture;

    size = b->size * 8;
    sx = size * gCylX >> 12;
    sy = size;
    sz = size * gCylZ >> 12;

    renderSetTexture(t);
    stateCheck();

    addQuad();
    addVertex(b->x - sx, b->y - sy, b->z - sz, 0.0f, 0.0f, &colorGrey);
    addVertex(b->x + sx, b->y - sy, b->z + sz, 1.0f, 0.0f, &colorGrey);
    addVertex(b->x + sx, b->y + sy, b->z + sz, 1.0f, 1.0f, &colorGrey);
    addVertex(b->x - sx, b->y + sy, b->z - sz, 0.0f, 1.0f, &colorGrey);
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
    q.color[0].r = q.color[0].g = q.color[0].b =
    q.color[1].r = q.color[1].g = q.color[1].b =
    q.color[2].r = q.color[2].g = q.color[2].b =
    q.color[3].r = q.color[3].g = q.color[3].b = 64;

    renderNextLayer();
    renderSetBlend(BLEND_MUL);
    renderColorQuad(&q);
    renderSetBlend(BLEND_ALPHA);
}

void prepareTrack(void)
{
    int i, j, x, y;

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

    id = TextureTableCount - ctrllibraryTextures;

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

void renderFrameBegin(void)
{
    Matrix* m;

    gBorder = ((gWidth * 240) / gHeight - 320) >> 1;

    gCylX = rcos(-ctrlcamera.hdg);
    gCylZ = rsin(-ctrlcamera.hdg);

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
    renderSetColor(0xFFFFFFFF);

    // identity matrix
    m = gMatrices + 0;
    matrixIdentity(m);

    // otho projection matrix
    m = gMatrices + 1;
    matrixOrtho(m, -gBorder, 320 + gBorder, 240, 0, 0, 1);

    gMatricesCount = 2;
}

void renderFrameEnd(void)
{
    int32 i, start, count;
    const RenderState* state;
    const Mesh* mesh;

    const Shader* shader = NULL;
    const TextureGL* texture = NULL;
    int32 projMatrix = -1;
    int32 viewMatrix = -1;
    int32 depthTest = 0;
    int32 depthWrite = 1;
    BlendMode blend = BLEND_NONE;
    uint32 color = 0xBAAAAAAD;

    GLfloat colorValue[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

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
            glUseProgram(shader->id);
        }

        // set texture
        if (texture != state->texture)
        {
            texture = state->texture;
            glBindTexture(GL_TEXTURE_2D, texture ? texture->id : whiteTex);
        }

        // set proj matrix
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

        char buf[256];
        sprintf(buf, "v:%d i:%d x:%d m:%d s:%d\n", dbg_v, dbg_i, dbg_x, dbg_m, dbg_s);
        OutputDebugString(buf);    
    }
#endif

    gVerticesCount = 0;
    gIndicesCount = 0;
    gMatricesCount = 0;
    gMeshesCount = 0;
    gStatesCount = 0;
}

void renderSetMatrix(const MATRIX* matrix, int x, int y, int z)
{
    Matrix* m = gMatrices + gMatricesCount;

    gCurState.viewMatrix = gMatricesCount++;
    gStatePtr = NULL;

    m->e[0][0] = matrix->m[0][0] * i4096;
    m->e[0][1] = matrix->m[1][0] * i4096;
    m->e[0][2] = matrix->m[2][0] * i4096;
    m->e[0][3] = 0.0f;

    m->e[1][0] = matrix->m[0][1] * i4096;
    m->e[1][1] = matrix->m[1][1] * i4096;
    m->e[1][2] = matrix->m[2][1] * i4096;
    m->e[1][3] = 0.0f;

    m->e[2][0] = matrix->m[0][2] * i4096;
    m->e[2][1] = matrix->m[1][2] * i4096;
    m->e[2][2] = matrix->m[2][2] * i4096;
    m->e[2][3] = 0.0f;

    m->e[3][0] = x;
    m->e[3][1] = y;
    m->e[3][2] = z;
    m->e[3][3] = 1.0f;
}

void renderSetSkeleton(Skeleton* skeleton, int x, int y, int z)
{
    renderSetMatrix(GetSkeletonMatrix(skeleton), x, y, z);
}

void renderSection(const TrackSection* section)
{
    const Face* face = track->faces + section->faceStart;
    const TrackVertex* vertices = track->vertices;
    const vec4i* v;
    int i;
    float u0, u1;
    CVECTOR color;
    
    color.cd = 255;

    for (i = 0; i < section->faceCount; i++, face++)
    {
        ASSERT(face->texture < ctrlnumLib);

    #ifdef CHECK_UNUSED_TEX
        texUsed[face->texture] = 1;
    #endif

        renderSetTexture(gTrackTex + face->texture);
        stateCheck();

        color.r = face->red;
        color.g = face->green;
        color.b = face->blue;

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
        addVertexI((vec4i*)(vertices + face->vertex[3]), u0, 1.0f, &color);
        addVertexI((vec4i*)(vertices + face->vertex[2]), u1, 1.0f, &color);
        addVertexI((vec4i*)(vertices + face->vertex[1]), u1, 0.0f, &color);
        addVertexI((vec4i*)(vertices + face->vertex[0]), u0, 0.0f, &color);
    }
}

void renderTrack(void)
{
    int i, j, s;
    int cx, cy, cz;
    int dir;
    short* counts;
    const short** sections;
    TrackSection* section;

//    #define CHECK_UNUSED_TEX

#ifdef CHECK_UNUSED_TEX
    uint8 texUsed[MAX_TRACK_TEX];
    memset(texUsed, 0, MAX_TRACK_TEX);
#endif

    dir = TrackDirection(&ctrlcamera);

    cx = ctrlcamera.camPos->absolute.t[0];
    cy = ctrlcamera.camPos->absolute.t[1];
    cz = ctrlcamera.camPos->absolute.t[2];

    renderSetSkeleton(ctrlcamera.camPos->super, cx, cy, cz);
    renderSetShader(&shaderTrack);

    switch (dir)
    {
        case VIEW_DIR_NORTH:
            counts = ctrlcamera.section->northViewCount;
            sections = ctrlcamera.section->northViewSection;
            break;
        case VIEW_DIR_SOUTH:
            counts = ctrlcamera.section->southViewCount;
            sections = ctrlcamera.section->southViewSection;
            break;
        case VIEW_DIR_WEST:
            counts = ctrlcamera.section->westViewCount;
            sections = ctrlcamera.section->westViewSection;
            break;
        default: // VIEW_DIR_EAST
            counts = ctrlcamera.section->eastViewCount;
            sections = ctrlcamera.section->eastViewSection;
    }

    section = track->sections + sections[0][0];
    renderSection(section->prevSection);

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < counts[i]; j += 2)
        {
            section = track->sections + sections[i][j];

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

void renderMesh(const vec4s* vertices, Primitive* prim, int32 count)
{
    int i;
    Prm poly;
    const vec4s* v;

    poly.primitive = prim;

    for (i = 0; i < count; i++)
    {
        switch (poly.primitive->type)
        {
            case TypeF3:
            {
                const F3* f = poly.f3++;

                renderSetTexture(NULL);
                stateCheck();

                addTriangle();
                addVertexS(vertices + f->coords[0], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[1], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[2], 0.0f, 0.0f, &f->colour);
                break;
            }

            case TypeLSF3:
            {
                const LSF3* f = poly.lsf3++;

                renderSetTexture(NULL);
                stateCheck();

                addTriangle();
                addVertexS(vertices + f->coords[0], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[1], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[2], 0.0f, 0.0f, &f->colour);
                break;
            }

            case TypeFT3:
            {
                const FT3* f = poly.ft3++;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                renderSetTexture(t);

                if (f->flag & ShipEngine)
                {
                    uint32 color = gCurState.color;

                    renderSetColor(0xFFFFFFFF);
                    renderSetBlend(BLEND_ADD);
                    stateCheck();

                    addTriangle();
                    addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, &colorWhite);
                    addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, &colorWhite);
                    addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, &colorWhite);

                    renderSetBlend(BLEND_NONE);
                    renderSetColor(color);
                }
                else
                {
                    stateCheck();

                    addTriangle();
                    addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, &f->colour);
                    addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, &f->colour);
                    addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, &f->colour);
                }
                break;
            }

            case TypeLSFT3:
            {
                const LSFT3* f = poly.lsft3++;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                renderSetTexture(t);
                stateCheck();

                addTriangle();
                addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, &f->colour);
                addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, &f->colour);
                addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, &f->colour);
                break;
            }

            case TypeF4:
            {
                const F4* f = poly.f4++;

                renderSetTexture(NULL);
                stateCheck();

                addQuad();
                addVertexS(vertices + f->coords[0], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[1], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[3], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[2], 0.0f, 0.0f, &f->colour);
                break;
            }

            case TypeLSF4:
            {
                const LSF4* f = poly.lsf4++;

                renderSetTexture(NULL);
                stateCheck();

                addQuad();
                addVertexS(vertices + f->coords[0], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[1], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[3], 0.0f, 0.0f, &f->colour);
                addVertexS(vertices + f->coords[2], 0.0f, 0.0f, &f->colour);
                break;
            }

            case TypeFT4:
            {
                const FT4* f = poly.ft4++;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                renderSetTexture(t);
                stateCheck();

                addQuad();
                addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, &f->colour);
                addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, &f->colour);
                addVertexS(vertices + f->coords[3], f->u3 * t->iw, f->v3 * t->ih, &f->colour);
                addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, &f->colour);
                break;
            }

            case TypeLSFT4:
            {
                const LSFT4* f = poly.lsft4++;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                renderSetTexture(t);
                stateCheck();

                addQuad();
                addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, &f->colour);
                addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, &f->colour);
                addVertexS(vertices + f->coords[3], f->u3 * t->iw, f->v3 * t->ih, &f->colour);
                addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, &f->colour);
                break;
            }

            case TypeG3:
            {
                const G3* f = poly.g3++;

                renderSetTexture(NULL);
                stateCheck();

                addTriangle();
                addVertexS(vertices + f->coords[0], 0.0f, 0.0f, f->colour + 0);
                addVertexS(vertices + f->coords[1], 0.0f, 0.0f, f->colour + 1);
                addVertexS(vertices + f->coords[2], 0.0f, 0.0f, f->colour + 2);
                break;
            }

            case TypeLSG3:
            {
                const LSG3* f = poly.lsg3++;

                renderSetTexture(NULL);
                stateCheck();

                addTriangle();
                addVertexS(vertices + f->coords[0], 0.0f, 0.0f, f->colour + 0);
                addVertexS(vertices + f->coords[1], 0.0f, 0.0f, f->colour + 1);
                addVertexS(vertices + f->coords[2], 0.0f, 0.0f, f->colour + 2);
                break;
            }

            case TypeGT3:
            {
                const GT3* f = poly.gt3++;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                renderSetTexture(t);

                if (f->flag & ShipEngine)
                {
                    uint32 color = gCurState.color;

                    renderSetColor(0xFFFFFFFF);
                    renderSetBlend(BLEND_ADD);
                    stateCheck();

                    addTriangle();
                    addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, &colorWhite);
                    addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, &colorWhite);
                    addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, &colorWhite);

                    renderSetBlend(BLEND_NONE);
                    renderSetColor(color);
                }
                else
                {
                    stateCheck();

                    addTriangle();
                    addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, f->colour + 0);
                    addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, f->colour + 1);
                    addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, f->colour + 2);
                }
                break;
            }

            case TypeLSGT3:
            {
                const LSGT3* f = poly.lsgt3++;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                renderSetTexture(t);
                stateCheck();

                addTriangle();
                addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, f->colour + 0);
                addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, f->colour + 1);
                addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, f->colour + 2);
                break;
            }

            case TypeG4:
            {
                const G4* f = poly.g4++;

                renderSetTexture(NULL);
                stateCheck();

                addQuad();
                addVertexS(vertices + f->coords[0], 0.0f, 0.0f, f->colour + 0);
                addVertexS(vertices + f->coords[1], 0.0f, 0.0f, f->colour + 1);
                addVertexS(vertices + f->coords[3], 0.0f, 0.0f, f->colour + 3);
                addVertexS(vertices + f->coords[2], 0.0f, 0.0f, f->colour + 2);
                break;
            }

            case TypeLSG4:
            {
                const LSG4* f = poly.lsg4++;

                renderSetTexture(NULL);
                stateCheck();

                addQuad();
                addVertexS(vertices + f->coords[0], 0.0f, 0.0f, f->colour + 0);
                addVertexS(vertices + f->coords[1], 0.0f, 0.0f, f->colour + 1);
                addVertexS(vertices + f->coords[3], 0.0f, 0.0f, f->colour + 3);
                addVertexS(vertices + f->coords[2], 0.0f, 0.0f, f->colour + 2);
                break;
            }

            case TypeGT4:
            {
                const GT4* f = poly.gt4++;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                renderSetTexture(t);
                stateCheck();

                addQuad();
                addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, f->colour + 0);
                addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, f->colour + 1);
                addVertexS(vertices + f->coords[3], f->u3 * t->iw, f->v3 * t->ih, f->colour + 3);
                addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, f->colour + 2);
                break;
            }

            case TypeLSGT4:
            {
                const LSGT4* f = poly.lsgt4++;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                renderSetTexture(t);
                stateCheck();

                addQuad();
                addVertexS(vertices + f->coords[0], f->u0 * t->iw, f->v0 * t->ih, f->colour + 0);
                addVertexS(vertices + f->coords[1], f->u1 * t->iw, f->v1 * t->ih, f->colour + 1);
                addVertexS(vertices + f->coords[3], f->u3 * t->iw, f->v3 * t->ih, f->colour + 3);
                addVertexS(vertices + f->coords[2], f->u2 * t->iw, f->v2 * t->ih, f->colour + 2);
                break;
            }

            case TypeTSPR:
            case TypeBSPR:
            {
                const SPR* f = poly.spr;

                int w, h0, h1;
                int sx, sz;
                TextureGL* t = (TextureGL*)TextureTable[f->texture];

                w = f->width >> 1;
                if (poly.primitive->type == TypeTSPR)
                {
                    h0 = 0;
                    h1 = f->height;
                }
                else
                {
                    h0 = f->height;
                    h1 = 0;
                }
                poly.spr++;

                v = vertices + f->coord;
                sx = gCylX * w >> 12;
                sz = gCylZ * w >> 12;

                renderSetTexture(t);
                stateCheck();

                addQuad();
                addVertex(v->x - sx, v->y - h0, v->z - sz, 0.0f, 0.0f, &f->colour);
                addVertex(v->x + sx, v->y - h0, v->z + sz, 1.0f, 0.0f, &f->colour);
                addVertex(v->x + sx, v->y + h1, v->z + sz, 1.0f, 1.0f, &f->colour);
                addVertex(v->x - sx, v->y + h1, v->z - sz, 0.0f, 1.0f, &f->colour);
                break;
            }

            case TypeInfiniteLight:
                poly.infiniteLight++;
                break;

            case TypePointLight:
                poly.pointLight++;
                break;

            case TypeSpotLight:
                poly.spotLight++;
                break;

            default:
                ASSERT(0);
                break;
        }
    }
}

void renderObject(const Object* object)
{
    const MATRIX* matrix;
    int cx, cy, cz;

    matrix = GetSkeletonMatrix(object->skeleton);

    cx = matrix->t[0];
    cy = matrix->t[1];
    cz = matrix->t[2];

    renderSetSkeleton(object->skeleton, cx, cy, cz);
    renderSetShader(&shaderObject);

    renderMesh(object->vertices, object->primitives, object->primitiveCount);
}

void renderShip(const Object* object, uint32 color)
{
    const MATRIX* matrix;
    int cx, cy, cz;

    matrix = GetSkeletonMatrix(object->skeleton);

    cx = matrix->t[0];
    cy = matrix->t[1];
    cz = matrix->t[2];

    renderSetSkeleton(object->skeleton, cx, cy, cz);
    renderSetShader(&shaderShip);
    renderSetColor(color);

    renderMesh(object->vertices, object->primitives, object->primitiveCount);

    renderSetColor(0xFFFFFFFF);
}

void renderShield(const Object* object, int32 counter)
{
    const MATRIX* matrix;
    int cx, cy, cz;

    matrix = GetSkeletonMatrix(object->skeleton);

    cx = matrix->t[0];
    cy = matrix->t[1];
    cz = matrix->t[2];

    renderSetSkeleton(object->skeleton, cx, cy, cz);
    renderSetShader(&shaderShield);
    renderSetBlend(BLEND_ADD);
    renderSetColor(counter);

    renderMesh(object->vertices, object->primitives, object->primitiveCount);

    renderSetBlend(BLEND_NONE);
}

void renderSky(const Object* object)
{
    object->skeleton->update = 1;

    const MATRIX* matrix;
    int cx, cy, cz;

    matrix = GetSkeletonMatrix(object->skeleton);

    cx = matrix->t[0];
    cy = matrix->t[1];
    cz = matrix->t[2];

    renderSetSkeleton(object->skeleton, cx, cy, cz);
    renderSetShader(&shaderSky);
    renderDepthTest(0);

    renderMesh(object->vertices, object->primitives, object->primitiveCount);

    renderDepthTest(1);
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
    TextureGL* tex;
    int32 w, h;
    int i, j;

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
        ASSERT(0);
    }

    if (HACK_LOAD_TRACK)
    {
        ASSERT(w == 32);
        ASSERT(h == 32);
        FillTrackTex(tempData);
    #ifdef _DEBUG
        return (Texture*)tim; // for CalcShadePalettes
    #else
        return NULL;
    #endif
    }
    else
    {
        tex = (TextureGL*)malloc(sizeof(TextureGL));
        tex->width = w;
        tex->height = h;
        tex->iw = 1.0f / w;
        tex->ih = 1.0f / h;

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

        return (Texture*)tex;
    }
}

void renderDeleteTexture(Texture* texture)
{
    // TODO
}
