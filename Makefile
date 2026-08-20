all:
	g++ -std=c++17 -Wno-shift-op-parentheses -O2 solver.cpp -o solver
debug:
	g++ -std=c++17 -Wno-shift-op-parentheses -g solver.cpp -o debugSolver
clean:
	rm solver
	rm debugSolver
