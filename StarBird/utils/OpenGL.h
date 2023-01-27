///------------------------------------------------------------------------------------------------
///  MathUtils.h
///  StarBird
///
///  Created by Alex Koukoulas on 20/11/2019.
///-----------------------------------------------------------------------------------------------

#ifndef OpenGL_h
#define OpenGL_h

///-----------------------------------------------------------------------------------------------

#define GLES_SILENCE_DEPRECATION
#include <OpenGLES/ES3/gl.h>

#define GL_CALL(func) do { func; auto err = glGetError(); if (err != GL_NO_ERROR) { printf("GLError: %d\n", err); assert(false); } } while (0)
#define GL_NO_CHECK_CALL(func) func

///-----------------------------------------------------------------------------------------------

#endif /* OpenGL_h */
