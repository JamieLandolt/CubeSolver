all:
	g++ -std=c++17 -O2 solver.cpp -o solver
debug:
	g++ -std=c++17 -g -O2 solver.cpp -o debugSolver
clean:
	rm solver
	rm debugSolver
