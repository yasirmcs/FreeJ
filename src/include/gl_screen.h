/*  FreeJ
 *  (c) Copyright 2001 Denis Roio aka jaromil <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * "$Id$"
 *
 */

#include <config.h>

#ifndef __GL_SCREEN_H__
#define __GL_SCREEN_H__

//#include <SDL.h>
//#include <SDL_opengl.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <screen.h>

struct Vertex
{
    float tu, tv;
    float x, y, z;
};

class GlScreen : public ViewPort {
 public:
  GlScreen();
  ~GlScreen();

  bool init(int widt, int height);
  void set_magnification(int algo);
  void resize(int resize_w, int resize_h);

  void show();
  void drawframe();
  void clear();

  void fullscreen();
  void *get_surface();

  float x_translation;
  float y_translation;
  float x_rotation;
  float y_rotation;
  float rotation;
  float zoom;

  // opengl stuff
  bool glblit(Layer *layer);
  bool glblitX(Layer *layer);
  GLuint texturize(Layer *layer);
  Vertex g_quadVertices[4];
  bool create_gl_context;

  // whis is the main window surface
  SDL_Surface *surface;
  void *coords(int x, int y);

  bool lock();
  bool unlock();
 
 private:
  int setres(int wx, int hx);
  
  int glwin;

  // check gl error and print it
  bool check_opengl_error();

};

#endif 