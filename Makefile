all:
	g++ -std=c++17 -Wno-shift-op-parentheses -O2 solver.cpp -o solver
debug:
	g++ -std=c++17 -Wno-shift-op-parentheses -g solver.cpp -o debugSolver
gprof:
	g++ -std=c++17 -Wno-shift-op-parentheses -O2 -pg solver.cpp -o solver_gprof
clean:
	rm solver
	rm debugSolver
