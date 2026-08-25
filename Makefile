all:
	g++ solver.cpp -o solver
debug:
	g++ -g solver.cpp -o debugSolver
gprof:
	g++ -O2 -pg solver.cpp -o solver_gprof
clean:
	rm solver
	rm debugSolver
