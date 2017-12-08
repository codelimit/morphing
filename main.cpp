#include "eng.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

#define PI 3.14159265

int main(int argc, char *argv[]) {

  engine eng;
  bool running{true};
  std::string pressedBtn;
  image img1, img2;

  if (!(eng.initAll())) {
    running = false;
  }

  if (!(eng.initWindow(512, 512))) {
    running = false;
  }

  int counter = 0;
  if (!(startCounter(counter))) {
    running = false;
  };

  img1 = eng.loadTexture("sample.png");
  if (img1.id < 1) {
    running = false;
  }

  img2 = eng.loadTexture("wot3.png");
  if (img2.id < 1) {
    running = false;
  }

  while (running) {
    std::string btn = eng.getEvent();
    if (!btn.empty()) {
      if (btn == "QUIT") {
        running = false;
      }
      std::cout << "Button pressed: " << btn << std::endl;
    }

    std::ifstream file("vertexes.txt");
    if (!file) {
      std::cerr << "Could not open the vertex data file for morphing "
                << std::endl;
      running = false;
    }
    std::ifstream file_img("imgvertexes.txt");
    if (!file_img) {
      std::cerr << "Could not open the vertex data file for texturing"
                << std::endl;
      running = false;
    }

    triangle tr_in1a;
    triangle tr_in1b;
    triangle tr_in2a;
    triangle tr_in2b;
    triangle tr_outa;
    triangle tr_outb;
    triangle tr1a;
    triangle tr1b;
    triangle im1a;
    triangle im1b;
    triangle tr2a;
    triangle tr2b;
    triangle im2a;
    triangle im2b;

    file >> tr_in1a >> tr_in2a >> tr_in1b >> tr_in2b;
    file_img >> tr1a >> tr1b >> im1a >> im1b >> tr2a >> tr2b >> im2a >> im2b;

    float al = sin((counter * PI) / 180);
    tr_outa = eng.morph(tr_in1a, tr_in1b, al);
    tr_outb = eng.morph(tr_in2a, tr_in2b, al);

    eng.renderTriangle(tr_outa);
    eng.renderTriangle(tr_outb);

    eng.renderTexture(tr1a, im1a, img1);
    eng.renderTexture(tr1b, im1b, img1);

    float mx = sin((counter * PI) / 180);
    float my = cos((counter * PI) / 180);
    scale scl = eng.scaleTexture(im2a, im2b, img2);
    for (int i = 0; i < 3; i++) {
      im2a.v[i].x += ((mx * ((1 - scl.sx) / 2) + ((1 - scl.sx) / 2)));
      im2b.v[i].x += ((mx * ((1 - scl.sx) / 2) + ((1 - scl.sx) / 2)));
      im2a.v[i].y += ((my * ((1 - scl.sy) / 2) + ((1 - scl.sy) / 2)));
      im2b.v[i].y += ((my * ((1 - scl.sy) / 2) + ((1 - scl.sy) / 2)));
    }

    eng.renderTexture(tr2a, im2a, img2);
    eng.renderTexture(tr2b, im2b, img2);

    eng.swapBuffers();
  }
  counter = -1;
  eng.shutDown();
  return 0;
}
