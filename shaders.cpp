#include <iostream>

#include <GL/glew.h>
#include <GL/gl.h>


char* filetobuf(const char *file)
{
    FILE *fptr;
    long length;
    char *buf;

    fptr = fopen(file, "rb"); /* Open file for reading */
    if (!fptr) /* Return NULL on failure */
        return NULL;
    fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
    length = ftell(fptr); /* Find out how many bytes into the file we are */
    buf = (char*)malloc(length+1); /* Allocate a buffer for the entire length of the file and a null terminator */
    fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
    fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
    fclose(fptr); /* Close the file */
    buf[length] = 0; /* Null terminator */

    return buf; /* Return the buffer */
}

/**
 * Compiles links and returns a shader from vert and frag filenames
 * - adapted from khronos.org opengl wiki tut2
 * - must decide on standard attributes to bind (position, color, time, etc)
 */
GLuint makeShader(const char *vertsrc,const char *fragsrc)
{
  GLchar *vertexsource, *fragmentsource;
  GLuint vertexshader, fragmentshader, shaderprogram;
  int IsCompiled_VS, IsCompiled_FS;
  int IsLinked;
  int maxLength;
  char *vertexInfoLog, *fragmentInfoLog, *shaderProgramInfoLog;
  
  vertexsource = filetobuf(vertsrc);
  fragmentsource = filetobuf(fragsrc);

  vertexshader = glCreateShader(GL_VERTEX_SHADER);

  /* Send the vertex shader source code to GL */
  /* Note that the source code is NULL character terminated. */
  /* GL will automatically detect that therefore the length info can be 0 in this case (the last parameter) */
  glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);

  /* Compile the vertex shader */
  glCompileShader(vertexshader);

  glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &IsCompiled_VS);

  if(IsCompiled_VS == false)
  {
     glGetShaderiv(vertexshader, GL_INFO_LOG_LENGTH, &maxLength);

     /* The maxLength includes the NULL character */
     vertexInfoLog = (char *)malloc(maxLength);

     glGetShaderInfoLog(vertexshader, maxLength, &maxLength, vertexInfoLog);

     /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
     /* In this simple program, we'll just leave */
     std::cout << vertexInfoLog << "\n";

     free(vertexInfoLog);
     return NULL;
  }  

  /* Create an empty fragment shader handle */
  fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);

  /* Send the fragment shader source code to GL */
  /* Note that the source code is NULL character terminated. */
  /* GL will automatically detect that therefore the length info can be 0 in this case (the last parameter) */
  glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);

  /* Compile the fragment shader */
  glCompileShader(fragmentshader);

  glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &IsCompiled_FS);

  if(IsCompiled_FS == false)
  {
     glGetShaderiv(fragmentshader, GL_INFO_LOG_LENGTH, &maxLength);

     /* The maxLength includes the NULL character */
     fragmentInfoLog = (char *)malloc(maxLength);

     glGetShaderInfoLog(fragmentshader, maxLength, &maxLength, fragmentInfoLog);

     /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
     /* In this simple program, we'll just leave */
     std::cout << fragmentInfoLog << "\n";
     
     free(fragmentInfoLog);
     return NULL;
  }

  /* If we reached this point it means the vertex and fragment shaders compiled and are syntax error free. */
  /* We must link them together to make a GL shader program */
  /* GL shader programs are monolithic. It is a single piece made of 1 vertex shader and 1 fragment shader. */
  /* Assign our program handle a "name" */
  shaderprogram = glCreateProgram();

  /* Attach our shaders to our program */
  glAttachShader(shaderprogram, vertexshader);
  glAttachShader(shaderprogram, fragmentshader);

  /* Bind attribute index 0 (coordinates) to in_Position and attribute index 1 (color) to in_Color */
  /* Attribute locations must be setup before calling glLinkProgram. */
  //gotta standardize these, add timer etc
  glBindAttribLocation(shaderprogram, 0, "in_Position");

  /* Link our program */
  /* At this stage, the vertex and fragment programs are inspected, optimized and a binary code is generated for the shader. */
  /* The binary code is uploaded to the GPU, if there is no error. */
  glLinkProgram(shaderprogram);

  /* Again, we must check and make sure that it linked. If it fails, it would mean either there is a mismatch between the vertex */
  /* and fragment shaders. It might be that you have surpassed your GPU's abilities. Perhaps too many ALU operations or */
  /* too many texel fetch instructions or too many interpolators or dynamic loops. */

  glGetProgramiv(shaderprogram, GL_LINK_STATUS, (int *)&IsLinked);

  if(IsLinked == false)
  {
     /* Noticed that glGetProgramiv is used to get the length for a shader program, not glGetShaderiv. */
     glGetProgramiv(shaderprogram, GL_INFO_LOG_LENGTH, &maxLength);

     /* The maxLength includes the NULL character */
     shaderProgramInfoLog = (char *)malloc(maxLength);

     /* Notice that glGetProgramInfoLog, not glGetShaderInfoLog. */
     glGetProgramInfoLog(shaderprogram, maxLength, &maxLength, shaderProgramInfoLog);

     /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
     /* In this simple program, we'll just leave after printing*/
     std::cout << shaderProgramInfoLog << "\n";

     free(shaderProgramInfoLog);
     return NULL;
  }
  
  return shaderprogram;
}
