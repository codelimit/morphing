#include "eng.h"
#include "picopng.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
//#include <SDL2/SDL_opengl.h>
#include <iostream>
#include <string>
#include <vector>

#define GL_CHECK()                                                             \
  {                                                                            \
    int error = glGetError();                                                  \
    if (error != GL_NO_ERROR) {                                                \
      switch (error) {                                                         \
      case GL_INVALID_ENUM:                                                    \
        std::cerr << GL_INVALID_ENUM << ": GL_INVALID_ENUM." << std::endl;     \
        break;                                                                 \
      case GL_INVALID_VALUE:                                                   \
        std::cerr << GL_INVALID_VALUE << ": GL_INVALID_VALUE" << std::endl;    \
        break;                                                                 \
      case GL_INVALID_OPERATION:                                               \
        std::cerr << GL_INVALID_OPERATION << ": GL_INVALID_OPERATION"          \
                  << std::endl;                                                \
        break;                                                                 \
      case GL_STACK_OVERFLOW:                                                  \
        std::cerr << GL_STACK_OVERFLOW << ": GL_STACK_OVERFLOW" << std::endl;  \
        break;                                                                 \
      case GL_STACK_UNDERFLOW:                                                 \
        std::cerr << GL_STACK_UNDERFLOW << ": GL_STACK_UNDERFLOW"              \
                  << std::endl;                                                \
        break;                                                                 \
      case GL_OUT_OF_MEMORY:                                                   \
        std::cerr << GL_OUT_OF_MEMORY << ": GL_OUT_OF_MEMORY" << std::endl;    \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
  }
#define MORPHRATE 20
std::istream &operator>>(std::istream &is, vertex &v) {
  is >> v.x;
  is >> v.y;
  return is;
}

std::istream &operator>>(std::istream &is, triangle &t) {
  is >> t.v[0];
  is >> t.v[1];
  is >> t.v[2];
  return is;
}

Uint32 setMorphSpeed(Uint32 interval, void *param) {
  int *s = (int *)param;
  if (*s < 0) {
    return interval;
  }
  if (*s < 360) {
    *s = *s + 1;
    startCounter(*s);
    return 0;
  }
  *s = 1;
  startCounter(*s);
  return 0;
}

bool startCounter(int &rad) {
  SDL_TimerID timer;
  timer = SDL_AddTimer(MORPHRATE, setMorphSpeed, &rad);
  if (timer == 0) {
    std::cerr << "Can't initialize timer: " << SDL_GetError() << std::endl;
    return false;
  }
  return true;
}

GLuint loadShader(const char *shaderSrc, GLenum type) {
  GLuint shader;
  GLint compiled;
  shader = glCreateShader(type);
  GL_CHECK();
  if (shader == 0)
    return 0;
  glShaderSource(shader, 1, &shaderSrc, NULL);
  GL_CHECK();
  glCompileShader(shader);
  GL_CHECK();
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  GL_CHECK();
  if (!compiled) {
    GLint infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
    GL_CHECK();
    if (infoLen > 1) {
      char *infoLog = new char[infoLen];
      glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
      GL_CHECK();
      std::string log = infoLog;
      std::cerr << "Error compiling shader: " << log;
      delete infoLog;
    }
    glDeleteShader(shader);
    GL_CHECK();
    return 0;
  }
  return shader;
}

bool engine::initAll() {

  SDL_version compiled;
  SDL_version linked;

  SDL_VERSION(&compiled);
  SDL_GetVersion(&linked);

  if (SDL_COMPILEDVERSION !=
      SDL_VERSIONNUM(linked.major, linked.minor, linked.patch)) {
    std::cerr << "SDL2 compiled and linked versions mismatch. Errors or "
                 "crashes may be occurred: "
              << SDL_GetError() << std::endl;
  }
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
    return false;
  }
  return true;
}

bool engine::initWindow(int height, int width) {
  SDL_Window *window = SDL_CreateWindow(
      "An SDL2 window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, height,
      width, SDL_WINDOW_OPENGL);
  if (window == nullptr) {
    std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
    return false;
  }
  windowId = SDL_GetWindowID(window);
  //---------------------------------------------------------------------------/
  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  if (gl_context == nullptr) {
    std::cerr << "Could not create GL_context for specified window: "
              << SDL_GetError() << std::endl;
    return false;
  }
  //---------------------------------------------------------------------------/
  int gl_major{};
  int gl_minor{};
  if ((SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &gl_major)) != 0) {
    std::cerr << "Could not get GL attribute: " << SDL_GetError() << std::endl;
  }
  if ((SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &gl_minor) != 0)) {
    std::cerr << "Could not get GL attribute: " << SDL_GetError() << std::endl;
  }
  std::cout << "OpenGL current version: " << gl_major << '.' << gl_minor
            << std::endl;
  if (gl_major <= 2 && gl_minor < 1) {
    std::cerr << "OpenGL current version: " << gl_major << '.' << gl_minor
              << std::endl
              << "need at least version: 2.1" << std::endl;
    return false;
  }
  //---------------------------------------------------------------------------/
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cerr << "GLEW initialization failed. " << glewGetErrorString(err)
              << std::endl;
    return false;
  }

  //--------------------------------------------------------------------------/

  GLchar vShaderStr[] = "attribute vec2 vPosition; \n"
                        "void main() \n"
                        "{ \n"
                        " gl_Position = vec4(vPosition, 0.0, 1.0); \n"
                        "} \n";
  GLchar fShaderStr[] = "precision mediump float; \n"
                        "void main() \n"
                        "{ \n"
                        "gl_FragColor = vec4(0.1, 0.014, 0.7, 1.0); \n"
                        "} \n";
  GLchar vShaderStrImg[] = "attribute vec2 a_position; \n"
                           "attribute vec2 a_texCoord; \n"
                           "varying vec2 v_texCoord; \n"
                           "void main() \n"
                           "{ \n"
                           "gl_Position = vec4(a_position, 0.0, 1.0); \n"
                           "v_texCoord = a_texCoord; \n"
                           "} \n";
  GLchar fShaderStrImg[] =
      "precision mediump float; \n"
      "varying vec2 v_texCoord; \n"
      "uniform sampler2D s_texture; \n"
      "void main() \n"
      "{ \n"
      " gl_FragColor = texture2D(s_texture, v_texCoord); \n"
      "} \n";

  GLint linked;
  GLuint vertexShader = loadShader(vShaderStr, GL_VERTEX_SHADER);
  GL_CHECK();
  if (vertexShader == 0) {
    return false;
  }
  GLuint vertexShaderImg = loadShader(vShaderStrImg, GL_VERTEX_SHADER);
  GL_CHECK();
  if (vertexShader == 0) {
    return false;
  }
  GLuint fragmentShader = loadShader(fShaderStr, GL_FRAGMENT_SHADER);
  GL_CHECK();
  if (fragmentShader == 0) {
    return false;
  }
  GLuint fragmentShaderImg = loadShader(fShaderStrImg, GL_FRAGMENT_SHADER);
  GL_CHECK();
  if (fragmentShader == 0) {
    return false;
  }
  triangleProg = glCreateProgram();
  GL_CHECK();
  if (triangleProg == 0) {
    std::cerr << "Failed to create GL program" << std::endl;
    return false;
  }
  glAttachShader(triangleProg, vertexShader);
  GL_CHECK();
  glAttachShader(triangleProg, fragmentShader);
  GL_CHECK();
  glBindAttribLocation(triangleProg, 0, "vPosition");
  GL_CHECK();
  glLinkProgram(triangleProg);
  GL_CHECK();
  glGetProgramiv(triangleProg, GL_LINK_STATUS, &linked);
  GL_CHECK();
  if (!linked) {
    GLint infoLen = 0;
    glGetProgramiv(triangleProg, GL_INFO_LOG_LENGTH, &infoLen);
    GL_CHECK();
    if (infoLen > 1) {
      char *infoLog = new char[infoLen];
      glGetProgramInfoLog(triangleProg, infoLen, NULL, infoLog);
      GL_CHECK();
      std::string log = infoLog;
      std::cout << "Error linking program: " << log << std::endl;
      delete infoLog;
    }
    glDeleteProgram(triangleProg);
    GL_CHECK();
    return false;
  }
  textureProg = glCreateProgram();
  GL_CHECK();
  if (textureProg == 0) {
    std::cerr << "Failed to create GL program" << std::endl;
    return false;
  }
  glAttachShader(textureProg, vertexShaderImg);
  GL_CHECK();
  glAttachShader(textureProg, fragmentShaderImg);
  GL_CHECK();
  glBindAttribLocation(textureProg, 0, "vPosition");
  GL_CHECK();
  glLinkProgram(textureProg);
  GL_CHECK();
  glGetProgramiv(textureProg, GL_LINK_STATUS, &linked);
  GL_CHECK();
  if (!linked) {
    GLint infoLen = 0;
    glGetProgramiv(textureProg, GL_INFO_LOG_LENGTH, &infoLen);
    GL_CHECK();
    if (infoLen > 1) {
      char *infoLog = new char[infoLen];
      glGetProgramInfoLog(textureProg, infoLen, NULL, infoLog);
      GL_CHECK();
      std::string log = infoLog;
      std::cout << "Error linking program: " << log << std::endl;
      delete infoLog;
    }
    glDeleteProgram(textureProg);
    GL_CHECK();
    return false;
  }

  glEnable(GL_BLEND);
  GL_CHECK();
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  GL_CHECK();
  return true;
}

triangle engine::morph(triangle &a, triangle &b, float alpha) {
  triangle c;
  if (alpha < 0) {
    alpha *= -1;
  }
  if (alpha >= 0.f && alpha <= 1.001) {
    for (int i = 0; i < 3; i++) {
      c.v[i].x = (a.v[i].x) * (1 - alpha) + (b.v[i].x) * alpha;
      c.v[i].y = (a.v[i].y) * (1 - alpha) + (b.v[i].y) * alpha;
    }
    return c;
  }
  std::cout << "Alpha is out of range: " << alpha << std::endl;
  return c;
}

void engine::renderTriangle(const triangle &t) {
  glUseProgram(triangleProg);
  GL_CHECK();
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), &t.v[0]);
  GL_CHECK();
  glEnableVertexAttribArray(0);
  GL_CHECK();
  glDrawArrays(GL_TRIANGLES, 0, 3);
  GL_CHECK();
  return;
}

image engine::loadTexture(const char *filename) {
  image img;
  std::vector<unsigned char> buffer, image;
  std::vector<unsigned char> swap, tmp;
  loadFile(buffer, filename);
  unsigned long w, h;
  int error = decodePNG(image, w, h, buffer.empty() ? 0 : &buffer[0],
                        (unsigned long)buffer.size());
  if (error != 0) {
    std::cerr << "Unable to load texture. Error: " << error << std::endl;
    return img;
  }
  img.h = h;
  img.w = w;
  for (unsigned long i = 0; i < h; i++) {
    for (unsigned long j = 0; j < (w * 4); j++) {
      tmp.push_back(image.back());
      image.pop_back();
    }
    for (unsigned long j = 0; j < (w * 4); j++) {
      swap.push_back(tmp.back());
      tmp.pop_back();
    }
    tmp.clear();
  }
  img.id = loadGL(img, swap);
  if (img.id == 0) {
    std::cerr << "Error while generating GL texture units";
    return img;
  }
  std::cout << "Texture " << img.id << " loaded with" << std::endl;
  if (swap.size() > 4) {
    std::cout << "width: " << w << " height: " << h << std::endl;
  }
  return img;
}

unsigned int engine::loadGL(image &img, std::vector<unsigned char> &buffer) {
  GLuint textureId;
  textureCounter++;
  glGenTextures(1, &textureId);
  GL_CHECK();
  glActiveTexture(GL_TEXTURE0 + textureCounter);
  GL_CHECK();
  glBindTexture(GL_TEXTURE_2D, textureId);
  GL_CHECK();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.w, img.h, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, &buffer[0]);
  GL_CHECK();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  GL_CHECK();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  GL_CHECK();

  return textureCounter;
}

scale engine::scaleTexture(triangle &tr1, triangle &tr2, image &img) {
  scale scl;
  int h, w;
  SDL_Window *window = SDL_GetWindowFromID(windowId);
  SDL_GetWindowSize(window, &h, &w);
  scl.sy = (static_cast<float>(h) / static_cast<float>(img.h)) / 4;
  scl.sx = (static_cast<float>(w) / static_cast<float>(img.w)) / 4;
  for (int i = 0; i < 3; i++) {
    tr1.v[i].x *= scl.sx;
    tr2.v[i].x *= scl.sx;
    tr1.v[i].y *= scl.sy;
    tr2.v[i].y *= scl.sy;
  }

  return scl;
}

void engine::renderTexture(const triangle &tr, const triangle &im, image &img) {

  glUseProgram(textureProg);
  GL_CHECK();

  int location = glGetUniformLocation(textureProg, "s_texture");
  GL_CHECK();
  if (location == -1) {
    std::cerr << "Unable to find uniform for shader" << std::endl;
    return;
  }
  glUniform1i(location, img.id);
  GL_CHECK();
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), &tr.v[0]);
  GL_CHECK();
  glEnableVertexAttribArray(0);
  GL_CHECK();
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), &im.v[0]);
  GL_CHECK();
  glEnableVertexAttribArray(1);
  GL_CHECK();
  glDrawArrays(GL_TRIANGLES, 0, 3);
  return;
}

void engine::swapBuffers() {
  SDL_Window *window = SDL_GetWindowFromID(windowId);
  SDL_GL_SwapWindow(window);
  glClearColor(0.f, 0.f, 0.f, 0.0f);
  GL_CHECK();
  glClear(GL_COLOR_BUFFER_BIT);
  GL_CHECK();
}

std::string engine::getEvent() {

  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    if (event.key.type == SDL_KEYDOWN) {
      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE:
        return "QUIT";
        break;
      case SDLK_RETURN:
        return "START";
        break;
      case SDLK_BACKSPACE:
        return "SELECT";
        break;
      case SDLK_UP:
        return "UP";
        break;
      case SDLK_DOWN:
        return "DOWN";
        break;
      case SDLK_LEFT:
        return "LEFT";
        break;
      case SDLK_RIGHT:
        return "RIGHT";
        break;
      case SDLK_SPACE:
        return "FIRE1";
        break;
      case SDLK_LCTRL:
        return "FIRE2";
        break;
      }
    }
  }
  return "";
}

void engine::shutDown() {
  std::cout << "System is shutting down" << std::endl;
  SDL_Delay(2000);
  SDL_Window *window = SDL_GetWindowFromID(windowId);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
