all:
	g++ -std=c++17 -Wno-shift-op-parentheses -O2 solver.cpp -o solver
debug:
	g++ -std=c++17 -Wno-shift-op-parentheses -g solver.cpp -o debugSolver
RAYLIB_PREFIX := $(shell brew --prefix raylib 2>/dev/null)
visualiser:
	g++ -std=c++17 -Wno-shift-op-parentheses -O2 visualiser.cpp -o visualiser \
		-I$(RAYLIB_PREFIX)/include -L$(RAYLIB_PREFIX)/lib \
		-lraylib -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
clean:
	rm solver
	rm debugSolver
