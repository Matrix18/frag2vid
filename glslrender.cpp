/**
 * frag2vid
 * render fragment shader on fullscreen quad to mpeg video
 * TODO: =clean up shader programs and objects on exit (atexit(deinit))
 *       =parameterize rendering resolution
 *       =texture uniforms 
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> //glm::value_ptr()

#include "shaders.h"
#include "encoder.h"

enum Constants { SCREENSHOT_MAX_FILENAME = 256 };
static GLubyte *pixels = NULL;

static const unsigned int HEIGHT = 480;
static const unsigned int WIDTH = 640;

static int offscreen = 0;
static unsigned int max_nframes = 100;
static unsigned int nframes = 0;
static unsigned int time0;

static uint8_t *rgb = NULL;

GLuint vao, vbo;
GLuint shaderprogram;
const char *vertsrc = "def.vert"; //default filename
const char *fragsrc = "def.frag";
const GLfloat quad[4][2] = {
    {  -1.0,  1.0  }, 
    {  -1.0, -1.0  }, 
    {   1.0,  1.0  }, 
    {   1.0, -1.0  } };

//shader uniforms
static glm::vec2 uResolution((float)HEIGHT, (float)WIDTH);
static float time = 0.0;

static const float MS_PER_SECOND = 1000.0;
static unsigned int startTime;

static int model_finished(void) {
    return nframes >= max_nframes;
}

static void init(void)  {
    
    glReadBuffer(GL_BACK); //originally GL_BACK
    glDrawBuffer(GL_BACK); //GL_FRONT_AND_BACK subverts double buffering but then glReadPixels performs correctly...
    
    //GL predrawing
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glViewport(0, 0, WIDTH, HEIGHT);
    
    time0 = glutGet(GLUT_ELAPSED_TIME);
    encoderStart("tmp.mpg", AV_CODEC_ID_MPEG1VIDEO, 25, WIDTH, HEIGHT);
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
  glUniform2fv( glGetUniformLocation(shaderprogram, "resolution"), 1, glm::value_ptr(uResolution) );

  time = (float)(glutGet(GLUT_ELAPSED_TIME) - startTime) / MS_PER_SECOND; 

  glUniform1f( glGetUniformLocation(shaderprogram, "time"), time);

  
  glClearColor(0.0, 0.5, 0.5, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

}

static void display(void) {

    char extension[SCREENSHOT_MAX_FILENAME];
    char filename[SCREENSHOT_MAX_FILENAME];

    draw_scene();
    glReadBuffer(GL_BACK);
    encoderReadGL(&rgb, &pixels, WIDTH, HEIGHT); //call glReadPixels before buffer swap
    
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

    GLint glut_display;
    glutInit(&argc, argv);
    
    if (argc > 1)
        offscreen = 0;

    glutInitWindowSize(WIDTH, HEIGHT);
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
