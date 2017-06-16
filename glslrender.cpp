/**
 * frag2vid
 * render fragment shader on fullscreen quad to mpeg video
 * TODO: =clean up shader programs and objects on exit (atexit(deinit))
 *       =texture uniforms
 *       =add cycling timer and bpm parameter 
 */
#include <iostream>
#include <vector>

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> //glm::value_ptr()

#include "optionparser.h"

#include "shaders.h"
#include "encoder.h"

enum Constants { SCREENSHOT_MAX_FILENAME = 256 };
static GLubyte *pixels = NULL;

static unsigned int width = 640;
static unsigned int height = 480;

static int offscreen = 0;
static unsigned int max_nframes = 100;
static unsigned int nframes = 0;
static unsigned int time0;

static uint8_t *rgb = NULL;

//gl rendering
GLuint vao, vbo;
GLuint shaderprogram;
const char *vertsrc = "def.vert"; //default filenames
char *fragsrc = "def.frag";  //can be altered by cli arg
const GLfloat quad[4][2] = {
    {  -1.0,  1.0  }, 
    {  -1.0, -1.0  }, 
    {   1.0,  1.0  }, 
    {   1.0, -1.0  } };

//shader uniforms
static glm::vec2 uResolution;
static float uTime = 0.0;

//timing variables
static const float MS_PER_SECOND = 1000.0;
static unsigned int startTime;
static unsigned int maxTime = 0; 

//option parsing
struct Arg: public option::Arg
{ 
  static option::ArgStatus Required(const option::Option& option, bool msg)
  {
    if(option.arg != 0)
      return option::ARG_OK;

    if(msg) {
      std::cout << option.name << " requires an argument\n";
      return option::ARG_ILLEGAL;
    }
  }
};

enum  optionIndex { UNKNOWN, HELP, FILENAME, WIDTH, HEIGHT, RUNTIME };

const option::Descriptor usage[] =
{
  {HELP, 0, "?", "help", option::Arg::None, "-? \t--help \tPrint usage and exit"},
  {FILENAME, 0, "f", "file", Arg::Required, "-f \t--file \tFilename of fragment shader to render"},
  {WIDTH, 0, "w", "width", Arg::Required, "-w \t--width \tWidth at which to render, must be less than screen width"},
  {HEIGHT, 0 , "h", "height", Arg::Required, "-h \t--height \tHeight at which to render, must be less than screen height"},
  {RUNTIME, 0, "t", "time", Arg::Required, "-t \t--time \tTotal time to render in seconds"},
  {0,0,0,0,0,0} //necessary for accuracy when fed into option::Stats
};


static int model_finished(void) {

  if(maxTime > 0) {

    return glutGet(GLUT_ELAPSED_TIME) >= maxTime;
    
  } else {
    
    return nframes >= max_nframes;
  }
}

static void init(void)  {
    
  glReadBuffer(GL_BACK); //originally GL_BACK
  glDrawBuffer(GL_BACK); //GL_FRONT_AND_BACK subverts double buffering
    
  //GL predrawing
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glEnable(GL_DEPTH_TEST);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glViewport(0, 0, width, height);
    
  time0 = glutGet(GLUT_ELAPSED_TIME);
  encoderStart("tmp.mpg", AV_CODEC_ID_MPEG1VIDEO, 25, width, height);
  //ffmpeg_encoder_start

  shaderprogram = makeShader(vertsrc,fragsrc);

  glGenVertexArrays(1, &vao);//Allocate and assign a Vertex Array Object to handle vao
  glBindVertexArray(vao);    //Bind vao as the current used object

  glGenBuffers(1, &vbo);     //Allocate and assign one Vertex Buffer Objects to our handle
  glBindBuffer(GL_ARRAY_BUFFER, vbo); //make vbo active buffer
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), quad, GL_STATIC_DRAW); //Copy the vertex data from quad to buffer
    
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0); //position into attrib 0, 2 floats per vert
  glEnableVertexAttribArray(0);
}

static void deinit(void)  {

  printf("FPS = %f\n", 1000.0 * nframes / (double)(glutGet(GLUT_ELAPSED_TIME) - time0));
  free(pixels);
  encoderFinish(); //ffmpeg_encoder_finish();
  free(rgb);
}

static void draw_scene(void) {

  glUseProgram(shaderprogram);

  //set uniforms
  uResolution.y = (float)height;
  uResolution.x = (float)width;
  
  glUniform2fv( glGetUniformLocation(shaderprogram, "resolution"), 1, glm::value_ptr(uResolution) );

  uTime = (float)(glutGet(GLUT_ELAPSED_TIME) - startTime) / MS_PER_SECOND; 

  glUniform1f( glGetUniformLocation(shaderprogram, "time"), uTime);

  
  glClearColor(0.0, 0.5, 0.5, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

}

static void display(void) {

  char extension[SCREENSHOT_MAX_FILENAME];
  char filename[SCREENSHOT_MAX_FILENAME];

  draw_scene();
  glReadBuffer(GL_BACK);
  encoderReadGL(&rgb, &pixels, width, height); //call glReadPixels before buffer swap
    
  glutSwapBuffers(); //consider ordering of drawing, swapping buffers, and reading buffer

  setFramepts(nframes); //frame->pts = nframes;
  encodeFrame(rgb);

  nframes++;
  if (model_finished())
      exit(EXIT_SUCCESS);
}

static void idle(void) {
  glutPostRedisplay();
}

int main(int argc, char **argv) {

  argc -= (argc > 0); argv += (argc > 0); //skip program name argv[0] if present

  
  option::Stats stats(usage, argc, argv);
  std::vector<option::Option> options(stats.options_max);
  std::vector<option::Option> buffer(stats.buffer_max);
  option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);

  if(parse.error())
    return 1;

  if(options[HELP]) {
    option::printUsage(std::cout, usage);
    return 0;
  }

  for(int i = 0; i < parse.optionsCount(); ++i) {

    option::Option& opt = buffer[i];
    switch(opt.index()) {

      case FILENAME:
	fragsrc = opt.arg;
	break;
	
      case WIDTH:
	width = atoi(opt.arg);
	break;
	
      case HEIGHT:
	height = atoi(opt.arg);
	break;

      case RUNTIME:
	maxTime = atoi(opt.arg) * MS_PER_SECOND;
    }
  }
  
  GLint glut_display;
  glutInit(&argc, argv);
    
  if (argc > 1)
      offscreen = 0;

  glutInitWindowSize(width, height);
  glutInitWindowPosition(100, 100);
  glut_display = GLUT_DOUBLE;
    
  glutInitDisplayMode(glut_display | GLUT_RGBA | GLUT_DEPTH);
  glutCreateWindow(argv[0]);

  GLenum err = glewInit();
  if (GLEW_OK != err) {
    /* Problem: glewInit failed, something is seriously wrong. */
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
  }

  startTime = glutGet(GLUT_ELAPSED_TIME);
    
  init();
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  atexit(deinit);
  glutMainLoop();
  return EXIT_SUCCESS;
}
