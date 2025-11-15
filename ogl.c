#include <EGL/egl.h>
#include <cstddef>
#include <cstdlib>
#include <math.h>
#include <sixel.h>
#include <stdio.h>
#include <unistd.h>

#define EGL_EGLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <vector>

const int WIDTH = 512;
const int HEIGHT = 512;

const char *vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\n";

const char *fragmentShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n";

const int width = 240;
const int height = 160;

static int sixel_write(char *data, int size, void *priv) {
  FILE *f = (FILE *)priv;
  return write(f->_fileno, data, size);
}

int main(int argc, char *argue[]) {
  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  EGLint major;
  EGLint minor;
  eglInitialize(display, &major, &minor);
  eglBindAPI(EGL_OPENGL_API);
  EGLint configAttribs[] = {EGL_SURFACE_TYPE,
                            EGL_PBUFFER_BIT,
                            EGL_RED_SIZE,
                            8,
                            EGL_GREEN_SIZE,
                            8,
                            EGL_BLUE_SIZE,
                            8,
                            EGL_ALPHA_SIZE,
                            8,
                            EGL_DEPTH_SIZE,
                            24,
                            EGL_STENCIL_SIZE,
                            8,
                            EGL_RENDERABLE_TYPE,
                            EGL_OPENGL_BIT,
                            EGL_NONE};
  EGLConfig config;
  EGLint numConfigs;
  eglChooseConfig(display, configAttribs, &config, 1, &numConfigs);
  EGLint contextAttribs[] = {EGL_CONTEXT_MAJOR_VERSION,
                             3,
                             EGL_CONTEXT_MINOR_VERSION,
                             0,
                             EGL_CONTEXT_OPENGL_PROFILE_MASK,
                             EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
                             EGL_NONE};
  EGLContext eglContext =
      eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
  EGLint surfaceAttribs[] = {EGL_WIDTH, static_cast<int>(width), EGL_HEIGHT,
                             static_cast<int>(height), EGL_NONE};
  EGLSurface surface = eglCreatePbufferSurface(display, config, surfaceAttribs);
  eglMakeCurrent(display, surface, surface, eglContext);

  uint32_t vertex, fragment;

  // Compile vertex shader
  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &vertexShaderSource, NULL);
  glCompileShader(vertex);

  // Compile fragment shader
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragment);

  uint32_t program = 0;
  program = glCreateProgram();
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);

  sixel_dither_t *dither;
  sixel_output_t *context;

  int status = sixel_output_new(&context, sixel_write, stdout, NULL);
  if (SIXEL_FAILED(status)) {
    fprintf(stderr, "sixel_output_new failed\n");
    exit(1);
  }

  sixel_output_set_palette_type(context, PALETTETYPE_HLS);

  dither = sixel_dither_get(SIXEL_BUILTIN_XTERM256);
  sixel_dither_set_pixelformat(dither, SIXEL_PIXELFORMAT_RGB888);

  uint8_t *buf = (uint8_t *)malloc(width * height * 3);

  double theta = 0.0;
  while (true) {
    theta += 0.001;
    GLfloat vertices[] = {0.0f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f};
    for (size_t i = 0; i < 6; i += 2) {
      double x = vertices[i];
      double y = vertices[i + 1];
      vertices[i] = x * cos(theta) - y * sin(theta);
      vertices[i + 1] = x * sin(theta) + y * cos(theta);
    }

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    std::vector<GLubyte> pixels(width * height * 3);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    status = sixel_encode((unsigned char *)pixels.data(), width, height, 0,
                          dither, context);
    if (SIXEL_FAILED(status)) {
      fprintf(stderr, "sixel_encode failed during setup\n");
      exit(1);
    }
  }

  sixel_output_destroy(context);
  sixel_dither_unref(dither);
  free(buf);

  eglDestroyContext(display, eglContext);
  eglDestroySurface(display, surface);
  eglTerminate(display);

  return 0;
}
