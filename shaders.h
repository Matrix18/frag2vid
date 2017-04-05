#ifndef _SHADERS_H_
#define _SHADERS_H_

#include <GL/gl.h>

char* filetobuf(char *file);
GLuint makeShader(const char *vertsrc, const char *fragsrc);


#endif
