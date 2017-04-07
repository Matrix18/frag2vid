frag2vid: glslrender.cpp encoder.cpp shaders.cpp
	g++ glslrender.cpp shaders.cpp encoder.cpp -lavcodec -lavformat -lavutil -lswscale -lGL -lGLU -lglut -lpng -lGLEW -lstdc++ -fpermissive -g
