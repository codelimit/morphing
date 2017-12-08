#include <iostream>
#include <list>
#include <vector>

#ifndef ENG_H_
#define ENG_H_

#ifdef _WIN32
#ifdef BUILD_ENG
#define ENG __declspec(dllexport)
#else
#define ENG __declspec(dllimport)
#endif
#else
#define ENG
#endif

struct scale {
  float sx = 0.f;
  float sy = 0.f;
};

struct vertex {
  float x;
  float y;
  vertex() : x(0.f), y(0.f) {}
};

struct ENG triangle {
  vertex v[3];
  triangle() {
    v[0] = vertex();
    v[1] = vertex();
    v[2] = vertex();
  }
};

struct image {
  unsigned int id = 0;
  unsigned long w = 0;
  unsigned long h = 0;
};

ENG std::istream &operator>>(std::istream &is, vertex &);
ENG std::istream &operator>>(std::istream &is, triangle &);

ENG bool startCounter(int &);

class ENG engine {
private:
  uint32_t windowId;
  uint32_t triangleProg;
  uint32_t textureProg;
  uint32_t actionProg;
  int textureCounter = {0};

  unsigned int loadGL(image &, std::vector<unsigned char> &);

public:
  triangle morph(triangle &, triangle &, float);

  bool initAll();

  bool initWindow(int a, int b);

  void renderTriangle(const triangle &);

  image loadTexture(const char *);

  scale scaleTexture(triangle &, triangle &, image &);

  void renderTexture(const triangle &, const triangle &, image &);

  void swapBuffers();

  std::string getEvent();

  void shutDown();
};

#endif /* ENG_H_ */
