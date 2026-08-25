all:
	g++ -std=c++17 solver.cpp -o solver
debug:
	g++ -std=c++17 -g solver.cpp -o debugSolver
gprof:
	g++ -std=c++17 -O2 -pg solver.cpp -o solver_gprof
clean:
	rm solver
	rm debugSolver
