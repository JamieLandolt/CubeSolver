all:
	g++ solver.cpp -o solver
debug:
	g++ -g solver.cpp -o debugSolver
clean:
	rm solver
	rm debugSolver
