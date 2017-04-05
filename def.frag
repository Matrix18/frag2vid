#version 300 es

precision highp float; //"some drivers required this next line to function properly"???

//in vec4 gl_FragCoord;

out vec4 fragColor;

uniform vec2 resolution;
uniform float time;

void main(void) {

  vec2 uv = gl_FragCoord.xy / resolution;
  
  fragColor = vec4(0.5,uv.y,0.0,1.0);
}
