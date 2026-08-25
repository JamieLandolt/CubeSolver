all:
	g++ -std=c++17 solver.cpp -O2 -o solver
debug:
	g++ -std=c++17 -g solver.cpp -o debugSolver
clean:
	rm solver
	rm debugSolver
