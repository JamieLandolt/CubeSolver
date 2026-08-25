all:
	g++ solver.cpp -O2 -o solver
debug:
	g++ -g solver.cpp -o debugSolver
clean:
	rm solver
	rm debugSolver
