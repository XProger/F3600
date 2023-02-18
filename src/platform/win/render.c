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

#define i255    (1.0f / 255.0f)
#define i4096   (1.0f / 4096.0f)

#define Z_NEAR  64
#define Z_FAR   65535

#if defined(__WIN32__) || defined(__LINUX__)
    PFNGLGETSTRINGIPROC                 glGetStringi;

    // Textures
    PFNGLGENERATEMIPMAPPROC             glGenerateMipmap;
    PFNGLTEXIMAGE3DPROC                 glTexImage3D;

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

#define EXT_DECL(v) uint8 v;
#define EXT_CHECK(v) if (ext && strstr(ext, "_"#v)) support.##v = 1;

#define EXT_LIST(E) \
    E( texture_array    )

#define CHECK_EXT(name) 

typedef struct
{
    EXT_LIST(EXT_DECL)
} Support;

Support support;

typedef uint16 Index;

typedef struct
{
    float x, y, z;
    float s, t;
    uint32 color;
} Vertex;

typedef struct
{
    float e[4][4];
} mat4;

#define MAX_RENDER_VERTICES     (256)
#define MAX_RENDER_INDICES      (256)

int32 gIndicesCount;
int32 gVerticesCount;
Index gIndices[MAX_RENDER_VERTICES];
Vertex gVertices[MAX_RENDER_INDICES];

mat4 mView, mProj, mProj3D;

int32 gMaxAniso;

uint16 tempData[320 * 240];

GLuint whiteTex;
const Texture* gCurTexture;

int32 gRenderMode;

int32 xOffset;
int32 yOffset;

int32 gCylX, gCylZ;

GLuint VAO;
GLuint VBO[2];

#define MAX_TRACK_TEX   22
Texture gTrackTexArray;
Texture gTrackTex[MAX_TRACK_TEX];
uint16 gTrackTexData[MAX_TRACK_TEX][128 * 128];

extern Texture* gEffectTextures;

const uint32 colorGrey = 0xFF808080;
const uint32 colorWhite = 0xFFFFFFFF;

// Shaders
#define DECL_ENUM(v) v,
#define DECL_STR(v)  #v,

#define SHADER_UNIFORMS(E) \
    E( uProjMatrix      ) \
    E( uViewMatrix      ) \
    E( uColor           )

#define SHADER_ATTRIBS(E) \
    E( aCoord           ) \
    E( aTexCoord        ) \
    E( aColor           )

typedef enum
{
    SHADER_UNIFORMS(DECL_ENUM)
    uMAX
} UniformType;

typedef enum
{
    SHADER_ATTRIBS(DECL_ENUM)
    aMAX
} UniformType;

const char* const SHADER_UNIFORM_NAMES[uMAX] = { SHADER_UNIFORMS(DECL_STR) };
const char* const SHADER_ATTRIB_NAMES[aMAX] = { SHADER_ATTRIBS(DECL_STR) };

typedef struct
{
    GLuint id;
    GLint uid[uMAX];
} Shader;

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
    "varying vec3 vTexCoord;\n"
    "varying vec3 vColor;\n"

    "#ifdef VERTEX\n"
        "uniform mat4 uProjMatrix;\n"
        "uniform mat4 uViewMatrix;\n"

        "attribute vec3 aCoord;\n"
        "attribute vec2 aTexCoord;\n"
        "attribute vec4 aColor;\n"

        "void main() {\n"
            "vTexCoord.xy = aTexCoord;\n"
        "#ifdef USE_TEXTURE_ARRAY\n"
            "vTexCoord.z = floor(aColor.w * 255.0);\n"
        "#else\n"
            "vTexCoord.z = 0.0;\n"
        "#endif\n"
            "vColor = aColor.xyz * 2.0;\n"
            "gl_Position = uProjMatrix * (uViewMatrix * vec4(aCoord, 1.0));\n"
        "}\n"

    "#else\n"
        "#ifdef USE_TEXTURE_ARRAY\n"
            "uniform sampler2DArray sDiffuse;\n"
        "#else\n"
            "uniform sampler2D sDiffuse;\n"
        "#endif\n"

        "void main() {\n"
        "#ifdef USE_TEXTURE_ARRAY\n"
            "fragColor = texture(sDiffuse, vTexCoord);\n"
        "#else\n"
            "fragColor = texture2D(sDiffuse, vTexCoord.xy);\n"
        "#endif\n"
            "fragColor.xyz *= vColor;\n"
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
            "vec3 ambient = uColor.xyz + 0.125;\n"
            "vColor = vec4(aColor.xyz * 4.0 * ambient, 1.0);\n"
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
            "vColor = vec4(a * 0.5 + b, 0.25 + b, 0.5 + b, 2.0) * 0.5;\n"
            "gl_Position = uProjMatrix * (uViewMatrix * vec4(pos, 1.0));\n"
        "}\n"

    "#else\n"

        "void main() {\n"
            "fragColor = vColor;\n"
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
            "vIntensity = aColor.w * 2.0;\n"
            "gl_Position = uProjMatrix * (uViewMatrix * vec4(aCoord, 1.0));\n"
        "}\n"

    "#else\n"

        "uniform sampler2D sDiffuse;\n"

        "void main() {\n"
            "float s = 1.0 - clamp(texture2D(sDiffuse, vTexCoord).x * vIntensity, 0.0, 1.0);\n"
            "fragColor = vec4(s, s, s, 1.0);\n"
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
            "vColor = vec4(aColor.xyz, 1.0);\n"
            "gl_Position = uProjMatrix * vec4(aCoord, 0.0, 1.0);\n"
        "}\n"

    "#else\n"

        "uniform sampler2D sDiffuse;\n"

        "void main() {\n"
            "fragColor = texture2D(sDiffuse, vTexCoord) * vColor;\n"
            "if (fragColor.a <= 0.0) discard;\n"
        "}\n"

    "#endif\n";

#define SHADER_IMPORT(name) Shader shader_ ## name;
#define SHADER_INIT(name)   initShader(&shader_ ## name, sh_ ## name)

SHADER_IMPORT(gui);
SHADER_IMPORT(sky);
SHADER_IMPORT(track);
SHADER_IMPORT(object);
SHADER_IMPORT(ship);
SHADER_IMPORT(shield);
SHADER_IMPORT(blob);

// render state
const Texture* gCurTexture;
const Shader* gCurShader;
int32 gCurBlend;
int32 gCurDepthTest;
int32 gCurDepthWrite;
int32 gCurView = -1;

void matrixIdentity(mat4* m)
{
    memset(m, 0, sizeof(mat4));
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

void setFilter(GLenum target, int32 bilinear, int32 mipmap)
{
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, mipmap ? (bilinear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST) : (bilinear ? GL_LINEAR : GL_NEAREST));
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, bilinear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    if (mipmap && gMaxAniso)
    {
        glTexParameteri(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, gMaxAniso);
    }
}

GLuint genTex(uint32 color)
{
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);
    setFilter(GL_TEXTURE_2D, 0, 0);
    return id;
}

void initShader(Shader* shader, const char* text)
{
    int32 i;
    GLchar info[1024];
    GLuint obj;
    const GLenum type[2] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };

#if defined(__MAC__)
    const GLchar *code[2][4] = {
        { GLSL_HEADER_VERT_GL2, "", "#line 0\n", text },
        { GLSL_HEADER_FRAG_GL2, "", "#line 0\n", text }
    };
#else
    const GLchar *code[2][4] = {
        { GLSL_HEADER_VERT_GL3, "", "#line 0\n", text },
        { GLSL_HEADER_FRAG_GL3, "", "#line 0\n", text }
    };
#endif

    if (support.texture_array)
    {
        code[0][1] =
        code[1][1] = "#define USE_TEXTURE_ARRAY\n";
    }

    shader->id = glCreateProgram();

    for (i = 0; i < 2; i++)
    {
        obj = glCreateShader(type[i]);
        glShaderSource(obj, 4, code[i], NULL);
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

    // init attribs
    for (i = 0; i < aMAX; i++)
    {
        glBindAttribLocation(shader->id, i, SHADER_ATTRIB_NAMES[i]);
    }
    
    glLinkProgram(shader->id);
    glGetProgramInfoLog(shader->id, sizeof(info), NULL, info);
    if (info[0] && strlen(info) > 8)
    {
        LOG("%s\n", info);
        ASSERT(0);
    }

    // init uniforms
    for (i = 0; i < uMAX; i++)
    {
        shader->uid[i] = glGetUniformLocation(shader->id, SHADER_UNIFORM_NAMES[i]);
    }

    // set samplers
    i = 0;
    glUseProgram(shader->id);
    glUniform1iv(glGetUniformLocation(shader->id, "sDiffuse"), 1, &i);
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

#define DRAW_BEGIN(v,t,c)   gVertexPtr; gVertexPtr += c
#define DRAW_END()          
#define VERTEX_SET(v, _x, _y, _z, _s, _t, _color) v.x = (float)_x; v.y = (float)_y; v.z = (float)_z; v.s = (float)_s; v.t = (float)_t; v.color = _color
#define VERTEX_SET_FACE(i, face, vertices, v) VERTEX_SET(v[i], vertices[face->index[i]].x, vertices[face->index[i]].y, vertices[face->index[i]].z, face->uv[i][0] * iw, face->uv[i][1] * ih, face->color[i])

#define GetProcOGL(x) x=GetProc(#x)

void renderInit()
{
    Vertex* v = NULL;
#if defined(__WIN32__)
    int32 format;
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
        const int32 pixelAttribs[] = {
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

        const int32 contextAttribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 2,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };

        UINT numFormats;
        int32 status = wglChoosePixelFormatARB(hDC, pixelAttribs, NULL, 1, &format, &numFormats);
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
    GetProcOGL(glGetStringi);

    GetProcOGL(glGenerateMipmap);
    GetProcOGL(glTexImage3D);

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

    if (glGetStringi)
    {
        int32 i;
        GLint extCount = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &extCount);
        for (i = 0; i < extCount; i++)
        {
            const char *ext = (const char*)glGetStringi(GL_EXTENSIONS, i);
            EXT_LIST(EXT_CHECK)
        }
    }
    else
    {
        const char *ext = (const char*)glGetString(GL_EXTENSIONS);
        EXT_LIST(EXT_CHECK)
    }

    //support.texture_array = 0;

    // init shaders
    SHADER_INIT(gui);
    SHADER_INIT(track);
    SHADER_INIT(sky);
    SHADER_INIT(object);
    SHADER_INIT(ship);
    SHADER_INIT(shield);
    SHADER_INIT(blob);

    // init vertex buffer
    glGenVertexArrays(1, &VAO);
    glGenBuffers(2, VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_RENDER_INDICES * sizeof(Index), NULL, GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, MAX_RENDER_VERTICES * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);

    // set vertex layout
    glEnableVertexAttribArray(aCoord);
    glEnableVertexAttribArray(aTexCoord);
    glEnableVertexAttribArray(aColor);

    glVertexAttribPointer(aCoord, 3, GL_FLOAT, GL_FALSE, sizeof(*v), &v->x);
    glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(*v), &v->s);
    glVertexAttribPointer(aColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(*v), &v->color);

    whiteTex = genTex(0xFFFFFFFF);

    glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gMaxAniso);

    // pre-allocate track textures
    if (support.texture_array)
    {
        ASSERT(glTexImage3D);
        glGenTextures(1, &gTrackTexArray.id);
        glBindTexture(GL_TEXTURE_2D_ARRAY, gTrackTexArray.id);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 128, 128, MAX_TRACK_TEX, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, NULL);
    }
    else
    {
        int32 i;
        for (i = 0; i < MAX_TRACK_TEX; i++)
        {
            glGenTextures(1, &gTrackTex[i].id);
            glBindTexture(GL_TEXTURE_2D, gTrackTex[i].id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 128, 128, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, NULL);
        }
    }
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

void renderResize(int32 width, int32 height)
{
    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;
    gWidth = width;
    gHeight = height;
}

void renderOffset(int32 x, int32 y)
{
    xOffset = x;
    yOffset = y;
}

void flush()
{
    if (gIndicesCount == 0)
        return;

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, gIndicesCount * sizeof(Index), gIndices);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gVerticesCount * sizeof(Vertex), gVertices);
    glDrawElements(GL_TRIANGLES, gIndicesCount, GL_UNSIGNED_SHORT, NULL);

    gIndicesCount = 0;
    gVerticesCount = 0;
}

void addTriangle()
{
    if (gIndicesCount + 3 > MAX_RENDER_INDICES)
        flush();

    gIndices[gIndicesCount++] = gVerticesCount;
    gIndices[gIndicesCount++] = gVerticesCount + 1;
    gIndices[gIndicesCount++] = gVerticesCount + 2;
}

void addQuad()
{
    if (gIndicesCount + 6 > MAX_RENDER_INDICES)
        flush();

    gIndices[gIndicesCount++] = gVerticesCount;
    gIndices[gIndicesCount++] = gVerticesCount + 1;
    gIndices[gIndicesCount++] = gVerticesCount + 2;

    gIndices[gIndicesCount++] = gVerticesCount;
    gIndices[gIndicesCount++] = gVerticesCount + 2;
    gIndices[gIndicesCount++] = gVerticesCount + 3;
}

void addVertex(float x, float y, float z, float s, float t, uint32 color)
{
    Vertex* v;

    if (gVerticesCount >= MAX_RENDER_VERTICES)
        flush();

    v = gVertices + gVerticesCount++;
    v->x = x;
    v->y = y;
    v->z = z;
    v->s = s;
    v->t = t;
    v->color = color;
}

void addVertexS(const vec3s* pos, float s, float t, uint32 color)
{
    addVertex((float)pos->x, (float)pos->y, (float)pos->z, s, t, color);
}

void renderSetBlend(int32 blend)
{
    if (gCurBlend == blend)
        return;
    flush();

    if (gCurBlend == BLEND_NONE)
    {
        glEnable(GL_BLEND);
    }

    gCurBlend = blend;

    switch (blend)
    {
        case BLEND_NONE:
            glDisable(GL_BLEND);
            break;
        case BLEND_ADD:
            glBlendFunc(GL_ONE, GL_ONE);
            break;
        case BLEND_MUL:
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
            break;
    }
}

void setDepthWrite(int32 enable)
{
    if (gCurDepthWrite == enable)
        return;
    flush();
    gCurDepthWrite = enable;

    glDepthMask(enable ? GL_TRUE : GL_FALSE);
}

void setDepthTest(int32 enable)
{
    if (gCurDepthTest == enable)
        return;
    flush();
    gCurDepthTest = enable;

    if (enable)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

void setTexture(const Texture* texture)
{
    if (gCurTexture == texture)
        return;
    flush();
    gCurTexture = texture;

    glBindTexture((texture == &gTrackTexArray) ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D, texture ? texture->id : whiteTex);
}

void setMatrix(const Matrix* matrix)
{
    flush();

    mView.e[0][0] = matrix->e00 * i4096;
    mView.e[0][1] = matrix->e10 * i4096;
    mView.e[0][2] = matrix->e20 * i4096;
    mView.e[0][3] = 0.0f;

    mView.e[1][0] = matrix->e01 * i4096;
    mView.e[1][1] = matrix->e11 * i4096;
    mView.e[1][2] = matrix->e21 * i4096;
    mView.e[1][3] = 0.0f;

    mView.e[2][0] = matrix->e02 * i4096;
    mView.e[2][1] = matrix->e12 * i4096;
    mView.e[2][2] = matrix->e22 * i4096;
    mView.e[2][3] = 0.0f;

    mView.e[3][0] = matrix->e03;
    mView.e[3][1] = matrix->e13;
    mView.e[3][2] = matrix->e23;
    mView.e[3][3] = 1.0f;
}

void setShader(const Shader* shader)
{
    flush();

    if (gCurShader != shader)
    {
        glUseProgram(shader->id);
    }

    gCurShader = shader;

    if (shader->uid[uProjMatrix] > -1)
    {
        glUniformMatrix4fv(shader->uid[uProjMatrix], 1, GL_FALSE, (GLfloat*)&mProj);
    }

    if (shader->uid[uViewMatrix] > -1)
    {
        glUniformMatrix4fv(shader->uid[uViewMatrix], 1, GL_FALSE, (GLfloat*)&mView);
    }
}

void setColor(uint32 color)
{
    if (gCurShader->uid[uColor] > -1)
    {
        GLfloat colorParam[4];
        colorParam[0] = i255 * (0xFF & (color));
        colorParam[1] = i255 * (0xFF & (color >> 8));
        colorParam[2] = i255 * (0xFF & (color >> 16));
        colorParam[3] = 1.0f;
        glUniform4fv(gCurShader->uid[uColor], 1, colorParam);
    }
}

void renderClear(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

int32 renderGlyph(const Glyph* g, int32 x, int32 y, int32 size, uint32 color)
{
#ifndef USE_FONT_SCALE
    size = 16;
#endif
    const Texture* t = g->texture;
    float u0 = g->x * t->iw;
    float v0 = g->y * t->ih;
    float u1 = (g->x + g->w) * t->iw;
    float v1 = (g->y + g->h) * t->ih;
    float x0 = x;
    float y0 = y;
    float x1 = x + g->w * size / 16;
    float y1 = y + g->h * size / 16;

    setTexture(t);

    addQuad();
    addVertex(x0, y0, 0, u0, v0, color);
    addVertex(x1, y0, 0, u1, v0, color);
    addVertex(x1, y1, 0, u1, v1, color);
    addVertex(x0, y1, 0, u0, v1, color);

    return g->w * size / 16;
}

void renderImage(const Texture* t, int32 x, int32 y)
{
    const uint32 color = 0xFFFFFFFF;

    float u0 = 0;
    float v0 = 0;
    float u1 = 1.0f;
    float v1 = 1.0f;
    float x0 = x;
    float y0 = y;
    float x1 = x + t->width;
    float y1 = y + t->height;

    setTexture(t);

    addQuad();
    addVertex(x0, y0, 0, u0, v0, color);
    addVertex(x1, y0, 0, u1, v0, color);
    addVertex(x1, y1, 0, u1, v1, color);
    addVertex(x0, y1, 0, u0, v1, color);
}

void renderBackground(const Texture* t)
{
    const uint32 color = 0xFFFFFFFF;

    float u0 = 0;
    float v0 = 0;
    float u1 = 1.0f;
    float v1 = 1.0f;
    float f = (guiB - guiT) / (t->height * 2.0f);
    float x0 = 160 - t->width * f;
    float y0 = guiT;
    float x1 = 160 + t->width * f;
    float y1 = guiB;

    setTexture(t);

    addQuad();
    addVertex(x0, y0, 0, u0, v0, color);
    addVertex(x1, y0, 0, u1, v0, color);
    addVertex(x1, y1, 0, u1, v1, color);
    addVertex(x0, y1, 0, u0, v1, color);
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

    setDepthWrite(0);
    renderSetBlend(BLEND_MUL);
    setMatrix(gMatrixPtr);
    setShader(&shader_blob);
    setTexture(gShadowTex);

    addQuad();
    addVertex(px + dx - rx, py + dy - ry, pz + dz - rz, 0.0f, 0.0f, color);
    addVertex(px + dx + rx, py + dy + ry, pz + dz + rz, 1.0f, 0.0f, color);
    addVertex(px - dx + rx, py - dy + ry, pz - dz + rz, 1.0f, 1.0f, color);
    addVertex(px - dx - rx, py - dy - ry, pz - dz - rz, 0.0f, 1.0f, color);

    renderSetBlend(BLEND_NONE);
    setDepthWrite(1);
}

void renderColorQuad(const ColorQuad* q)
{
    setTexture(NULL);

    addQuad();
    addVertex(q->x0, q->y0, 0, 0.0f, 0.0f, q->color[0]);
    addVertex(q->x1, q->y1, 0, 0.0f, 0.0f, q->color[1]);
    addVertex(q->x2, q->y2, 0, 0.0f, 0.0f, q->color[2]);
    addVertex(q->x3, q->y3, 0, 0.0f, 0.0f, q->color[3]);
}

void renderBackFill(int32 x, int32 y, int32 w, int32 h)
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
    q.color[3] = 0xFF808080;

    renderSetBlend(BLEND_MUL);
    renderColorQuad(&q);
    renderSetBlend(BLEND_NONE);
}

void prepareTrack(void)
{
    ASSERT(ctrlnumLib <= MAX_TRACK_TEX);

    if (support.texture_array)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, gTrackTexArray.id);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 128, 128, ctrlnumLib, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, gTrackTexData);
        setFilter(GL_TEXTURE_2D_ARRAY, 1, glGenerateMipmap != 0);
        if (glGenerateMipmap)
        {
            glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
        }
    }
    else
    {
        int32 i;
        for (i = 0; i < ctrlnumLib; i++)
        {
            glBindTexture(GL_TEXTURE_2D, gTrackTex[i].id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 128, 128, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, gTrackTexData[i]);

            setFilter(GL_TEXTURE_2D, 1, glGenerateMipmap != 0);
            if (glGenerateMipmap)
            {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
        }
    }
}

void fillTrackTex(uint16* data)
{
    int32 id, i, j, k, x, y;

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

void renderFrameBegin(void)
{
    gRenderMode = RENDER_MODE_3D;
    renderSetMode(RENDER_MODE_2D);

    glViewport(0, 0, gWidth, gHeight);
}

void renderFrameEnd(void)
{
    flush();
}

void drawSection(const TrackSection* section, int32 specialed)
{
    const vec3i* vertices = gTrack.vertices;
    const Face* face = gTrack.faces + section->faceStart;
    const Face* last = face + section->faceCount;
    const vec3i* v;
    int32 cx = gCameraViewPos.x;
    int32 cy = gCameraViewPos.y;
    int32 cz = gCameraViewPos.z;
    float u0, u1;
    uint32 color1, color2;
    uint16 flags;
    int32 texture;
    int32 angle;

    while (face < last)
    {
        flags = face->flags;

        if (flags & TRACK_FACE_FLIP)
        {
            u0 = 0.0f;
            u1 = 1.0f;
        }
        else
        {
            u0 = 1.0f;
            u1 = 0.0f;
        }

        color1 = color2 = face->color;

        if (!specialed)
        {
            if (flags & TRACK_FACE_SPEED)
            {
                angle = (section - gTrack.sections) * 2 - gFrameIndex;
                color1 = speedColor((angle) << 9);
                color2 = speedColor((angle + 2) << 9);
            }
            else
            {
                int32 weapon = GET_TRACK_FACE_WEAPON(flags);
                if (weapon > 0 && weapon < 0xFF)
                {
                    angle = (gFrameIndex + (face - gTrack.faces) * 64) * 32;
                    color1 = color2 = weaponColor(angle);
                }
            }
        }

        texture = GET_TRACK_FACE_TEXTURE(flags);

        if (support.texture_array)
        {
            color1 |= (texture << 24);
            color2 |= (texture << 24);
        }
        else
        {
            setTexture(gTrackTex + texture);
        }

        addQuad();

        v = vertices + face->vertex[3];
        addVertex(v->x - cx, v->y - cy, v->z - cz, u0, 1.0f, color1);

        v = vertices + face->vertex[2];
        addVertex(v->x - cx, v->y - cy, v->z - cz, u1, 1.0f, color1);

        v = vertices + face->vertex[1];
        addVertex(v->x - cx, v->y - cy, v->z - cz, u1, 0.0f, color2);

        v = vertices + face->vertex[0];
        addVertex(v->x - cx, v->y - cy, v->z - cz, u0, 0.0f, color2);

        face++;
    }
}

void renderTrack(int32 playerIndex, int32 specialed)
{
    int32 i, j, s;
    int32 dir;
    int16* counts;
    int16** sections;
    TrackSection* section;
    TrackCamera* camera = &gPlayers[playerIndex].camera;

    dir = TrackDirection(camera);

    setMatrix(gMatrixPtr);
    setShader(&shader_track);

    switch (dir)
    {
        case VIEW_DIR_NORTH:
            counts = camera->section->northViewCount;
            sections = camera->section->northViewSection;
            break;
        case VIEW_DIR_SOUTH:
            counts = camera->section->southViewCount;
            sections = camera->section->southViewSection;
            break;
        case VIEW_DIR_WEST:
            counts = camera->section->westViewCount;
            sections = camera->section->westViewSection;
            break;
        default: // VIEW_DIR_EAST
            counts = camera->section->eastViewCount;
            sections = camera->section->eastViewSection;
    }

    if (support.texture_array)
    {
        setTexture(&gTrackTexArray);
    }

    section = gTrack.sections + sections[0][0];
    drawSection(section->prevSection, specialed);

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < counts[i]; j += 2)
        {
            section = gTrack.sections + sections[i][j];

            for (s = 0; s < sections[i][j + 1]; s++)
            {
                drawSection(section, specialed);
                section = section->nextSection;
            }
        }
    }
}

void renderEffects(const Effect* fx)
{
    int32 x, y, z, rx, ry, rz, ux, uy, uz, size;

    renderSetBlend(BLEND_ADD);
    setDepthWrite(0);
    setMatrix(gMatrixPtr);
    setShader(&shader_object);

    while (fx)
    {
        x = fx->pos.x - gCameraViewPos.x;
        y = fx->pos.y - gCameraViewPos.y;
        z = fx->pos.z - gCameraViewPos.z;
        size = fx->size * 8;

        rx = gMatrixPtr->e00 * size >> FIXED_SHIFT;
        ry = gMatrixPtr->e01 * size >> FIXED_SHIFT;
        rz = gMatrixPtr->e02 * size >> FIXED_SHIFT;

        ux = gMatrixPtr->e10 * size >> FIXED_SHIFT;
        uy = gMatrixPtr->e11 * size >> FIXED_SHIFT;
        uz = gMatrixPtr->e12 * size >> FIXED_SHIFT;

        setTexture(gEffectTextures + fx->type);
        setShader(&shader_object);

        addQuad();
        addVertex(x - rx - ux, y - ry - uy, z - rz - uz, 0.0f, 0.0f, 0xFF808080);
        addVertex(x + rx - ux, y - ry - uy, z + rz - uz, 1.0f, 0.0f, 0xFF808080);
        addVertex(x + rx + ux, y + ry + uy, z + rz + uz, 1.0f, 1.0f, 0xFF808080);
        addVertex(x - rx + ux, y + ry + uy, z - rz + uz, 0.0f, 1.0f, 0xFF808080);

        fx = fx->next;
    }

    setDepthWrite(1);
    renderSetBlend(BLEND_NONE);
}

void drawMesh(const Object* obj)
{
    float iw, ih;
    const vec3s* vertices = obj->vertices;
    const ObjFace* face = obj->faces;
    const ObjFace* fLast = face + obj->fCount;
    const ObjSprite* sprite = obj->sprites;
    const ObjSprite* sLast = sprite + obj->sCount;

    while (face < fLast)
    {
        uint32 texIndex = FACE_TEXTURE(face);

        // TODO precalc float uv's
        if (texIndex != NO_TEXTURE)
        {
            Texture* t = gTextures + FACE_TEXTURE(face);
            setTexture(t);
            iw = t->iw;
            ih = t->ih;
        }
        else
        {
            setTexture(NULL);
            iw = 0.0f;
            ih = 0.0f;
        }

        if (FACE_QUAD(face))
        {
            addQuad();
            addVertexS(vertices + face->index[0], face->uv[0][0] * iw, face->uv[0][1] * ih, face->color[0]);
            addVertexS(vertices + face->index[1], face->uv[1][0] * iw, face->uv[1][1] * ih, face->color[1]);
            addVertexS(vertices + face->index[2], face->uv[2][0] * iw, face->uv[2][1] * ih, face->color[2]);
            addVertexS(vertices + face->index[3], face->uv[3][0] * iw, face->uv[3][1] * ih, face->color[3]);
        }
        else
        {
            addTriangle();
            addVertexS(vertices + face->index[0], face->uv[0][0] * iw, face->uv[0][1] * ih, face->color[0]);
            addVertexS(vertices + face->index[1], face->uv[1][0] * iw, face->uv[1][1] * ih, face->color[1]);
            addVertexS(vertices + face->index[2], face->uv[2][0] * iw, face->uv[2][1] * ih, face->color[2]);
        }

        face++;
    }

    while (sprite < sLast)
    {
        const vec3s* p = vertices + sprite->index;
        const Texture* t = gTextures + SPRITE_TEXTURE(sprite);
        int32 w = sprite->width >> 1;
        int32 wx = gCylX * w >> FIXED_SHIFT;
        int32 wz = gCylZ * w >> FIXED_SHIFT;
        int32 h0, h1;

        if (SPRITE_FLIP(sprite))
        {
            h0 = sprite->height;
            h1 = 0;
        }
        else
        {
            h0 = 0;
            h1 = sprite->height;
        }

        setTexture(t);

        addQuad();
        addVertex(p->x - wx, p->y - h0, p->z - wz, 0.0f, 0.0f, sprite->color);
        addVertex(p->x + wx, p->y - h0, p->z + wz, 1.0f, 0.0f, sprite->color);
        addVertex(p->x + wx, p->y + h1, p->z + wz, 1.0f, 1.0f, sprite->color);
        addVertex(p->x - wx, p->y + h1, p->z - wz, 0.0f, 1.0f, sprite->color);

        sprite++;
    }
}

void drawMeshShip(const Object* obj, uint32 color)
{
    float iw, ih;
    const vec3s* vertices = obj->vertices;
    const ObjFace* face = obj->faces;
    const ObjFace* fLast = face + obj->fCount;

// opaque
    setColor(color);

    while (face < fLast)
    {
        if ((face->flags & FACE_FLAG_ENGINE) == 0)
        {
            uint32 texIndex = FACE_TEXTURE(face);

            // TODO precalc float uv's
            if (texIndex != NO_TEXTURE)
            {
                Texture* t = gTextures + FACE_TEXTURE(face);
                setTexture(t);
                iw = t->iw;
                ih = t->ih;
            }
            else
            {
                setTexture(NULL);
                iw = 0.0f;
                ih = 0.0f;
            }

            if (FACE_QUAD(face))
            {
                addQuad();
                addVertexS(vertices + face->index[0], face->uv[0][0] * iw, face->uv[0][1] * ih, face->color[0]);
                addVertexS(vertices + face->index[1], face->uv[1][0] * iw, face->uv[1][1] * ih, face->color[1]);
                addVertexS(vertices + face->index[2], face->uv[2][0] * iw, face->uv[2][1] * ih, face->color[2]);
                addVertexS(vertices + face->index[3], face->uv[3][0] * iw, face->uv[3][1] * ih, face->color[3]);
            }
            else
            {
                addTriangle();
                addVertexS(vertices + face->index[0], face->uv[0][0] * iw, face->uv[0][1] * ih, face->color[0]);
                addVertexS(vertices + face->index[1], face->uv[1][0] * iw, face->uv[1][1] * ih, face->color[1]);
                addVertexS(vertices + face->index[2], face->uv[2][0] * iw, face->uv[2][1] * ih, face->color[2]);
            }
        }

        face++;
    }

// semi-transparent
    renderSetBlend(BLEND_ADD);
    setColor(0xFF202020);

    face = obj->faces;
    while (face < fLast)
    {
        if ((face->flags & FACE_FLAG_ENGINE) != 0)
        {
            uint32 texIndex = FACE_TEXTURE(face);

            // TODO precalc float uv's
            if (texIndex != NO_TEXTURE)
            {
                Texture* t = gTextures + FACE_TEXTURE(face);
                setTexture(t);
                iw = t->iw;
                ih = t->ih;
            }
            else
            {
                setTexture(NULL);
                iw = 0.0f;
                ih = 0.0f;
            }
    
            ASSERT(FACE_QUAD(face) == 0); // triangles only

            addTriangle();
            addVertexS(vertices + face->index[0], face->uv[0][0] * iw, face->uv[0][1] * ih, face->color[0]);
            addVertexS(vertices + face->index[1], face->uv[1][0] * iw, face->uv[1][1] * ih, face->color[1]);
            addVertexS(vertices + face->index[2], face->uv[2][0] * iw, face->uv[2][1] * ih, face->color[2]);
        }

        face++;
    }

    renderSetBlend(BLEND_NONE);
}

void renderObject(const Object* object)
{
    setMatrix(gMatrixPtr);
    setShader(&shader_object);

    drawMesh(object);
}

void renderShip(const Object* object, uint32 color)
{
    setMatrix(gMatrixPtr);
    setShader(&shader_ship);

    drawMeshShip(object, color);
}

void renderShield(const Object* object, int32 counter)
{
    renderSetBlend(BLEND_ADD);

    setMatrix(gMatrixPtr);
    setShader(&shader_shield);
    setColor(counter);
    drawMesh(object);

    renderSetBlend(BLEND_NONE);
}

void renderSky(const Object* object)
{
    setDepthTest(0);

    setMatrix(gMatrixPtr);
    setShader(&shader_sky);

    drawMesh(object);

    setDepthTest(1);
}

void renderTargetIcon(Texture* tex)
{
    float px, py, pz, size;
    Matrix* m = gMatrixPtr;

    pz = mProj3D.e[0][2] * m->e03 + mProj3D.e[1][2] * m->e13 + mProj3D.e[2][2] * m->e23 + mProj3D.e[3][2];

    if (pz <= Z_NEAR)
        return;

    px = mProj3D.e[0][0] * m->e03 + mProj3D.e[1][0] * m->e13 + mProj3D.e[2][0] * m->e23 + mProj3D.e[3][0];
    py = mProj3D.e[0][1] * m->e03 + mProj3D.e[1][1] * m->e13 + mProj3D.e[2][1] * m->e23 + mProj3D.e[3][1];

    px = guiL + (px / pz * 0.5f + 0.5f) * (guiR - guiL);
    py = guiT + (0.5f - py / pz * 0.5f) * (guiB - guiT);

    size = tex->width >> 1;

    renderSetBlend(BLEND_ADD);
    setTexture(tex);

    addQuad();
    addVertex(px - size, py - size, 0, 0.0f, 1.0f, 0xFF808080);
    addVertex(px + size, py - size, 0, 1.0f, 1.0f, 0xFF808080);
    addVertex(px + size, py + size, 0, 1.0f, 0.0f, 0xFF808080);
    addVertex(px - size, py + size, 0, 0.0f, 0.0f, 0xFF808080);

    renderSetBlend(BLEND_NONE);
}

void renderSetMode(int32 renderMode)
{
    int32 x = 0;
    int32 y = 0;
    int32 w = gWidth;
    int32 h = gHeight;

    if (gCurView != -1)
    {
        if (gNumPlayers > 2)
        {
            w >>= 1;
            h >>= 1;

            switch (gCurView)
            {
                case 0:
                    y = h;
                    break;
                case 1:
                    x = w;
                    y = h;
                    break;
                case 2:
                    if (gNumPlayers == 3)
                    {
                        x = w >> 1;
                    }
                    break;
                case 3:
                    x = w;
                    break;
            }
        }
        else
        {
            w >>= 1;
            if (gCurView == 1)
            {
                x = w;
            }
        }
    }

    flush();

    glViewport(x, y, w, h);

    if (renderMode == RENDER_MODE_2D)
    {
        //setFilter(0);

        matrixOrtho(&mProj, guiL, guiR, guiB, guiT, 0, 1);

        setDepthTest(0);
        renderSetBlend(BLEND_NONE);

        setShader(&shader_gui);
    }
    else
    {
        //setFilter(1);

        matrixSetViewMenu();

        matrixPerspective(&mProj, gCameraFOV / 4096.0f, (float)w / (float)h, -xOffset / ((guiR - guiL) * 0.5f), yOffset / ((guiB - guiT) * 0.5f), Z_NEAR, Z_FAR);

        mProj3D = mProj;

        setDepthTest(1);
        renderSetBlend(BLEND_NONE);

        // get fixed right vector for sprites
        rsincos(gCameraRotY, gCylZ, gCylX);
    }
}

void renderSetView(int32 index)
{
    gCurView = index;
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
/*
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
*/
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

        fillTrackTex(tempData);

        tex->id = 0;
    }
    else
    {
        glGenTextures(1, &tex->id);
        glBindTexture(GL_TEXTURE_2D, tex->id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, tempData);

        setFilter(GL_TEXTURE_2D, HACK_TEX_FILTER, 0);
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
