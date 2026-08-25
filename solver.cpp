#include "solver_core.h"

void solve(std::vector<std::string> scramble) {
	Cube cube = Cube();
	std::pair<std::vector<std::string>,std::pair<long,long>> p = cube.scramble(scramble);
	std::pair<long,long> state = p.second;
	Solver solver = Solver(cube);
	solver.dfs(p.first);

	std::pair<std::list<std::string>,std::list<std::string>> solution = solver.get_solution();
	if (!solution.second.size()) {
		std::cout << "No solution could be found with the current depth\n";
	} else {
		std::cout << "First Phase:\n";
		for (std::string move : solution.first) {
			std::cout << move << ", ";
		}
		std::cout << "\nSecond Phase:\n";
		for (std::string move : solution.second) {
			std::cout << move << ", ";
		}
		std::cout << "\nSolution complete with: ";
		solution.first.splice(solution.first.end(), solution.second);
		for (std::string move : solution.first) {
			std::cout << move << ", ";
		}
		std::cout <<"\n";
	}
}

void benchmark_solves() {
    Cube cube;
	Solver solver = Solver(cube);
	Timer timer = Timer();

    std::vector<long> times;

	int scramble_size = 25;
	for (int scramble_num = 0; scramble_num < 80; scramble_num++) {
		std::cout << "Scramble " << scramble_num << ":\n";

		std::pair<std::vector<std::string>,std::pair<long,long>> p = cube.random_scramble(scramble_size, solver.DEPTH_PHASE_1);
		std::vector<std::string> scramble = p.first;
		std::pair<long,long> state = p.second;

		int i;
		std::cout << "Scramble Moves: ";
		for (i = 0; i < scramble.size() - 1; i++) {
			std::cout << scramble[i] << ", ";
		}
		std::cout << scramble[i] << "\n";

		solver.reset_full();
		solver.reset_dfs(scramble);

		// Time Solve
		timer.start();
		solver.dfs(scramble);
		auto solve_time = timer.stop(scramble_size);

		long time = solve_time.count();
		std::cout << "Time: " << time << "ms | ";
		times.push_back(time);

		// If a non middle layer edge is in a middle layer edge position
		std::pair<std::list<std::string>,std::list<std::string>> solution = solver.get_solution();
		std::list<std::string> p1_sol = solution.first;
		std::list<std::string> p2_sol = solution.second;

		i = 0;
		std::cout << "(Phase 1) ";
		for (std::string mv : p1_sol) {
			i++;
			if (i < p1_sol.size()) {
				std::cout << mv << ", ";
			} else {
				std::cout << mv << " | ";
			}
		}

		i = 0;
		std::cout << "(Phase 2) ";
		for (std::string mv : p2_sol) {
			if (i < p2_sol.size()) {
				std::cout << mv << ", ";
			} else {
				std::cout << mv;
			}
		}
		std::cout << "\n\n";
	}
	std::cout << "Min Solve Time: " << *std::min_element(times.begin(), times.end()) << "ms" << std::endl;
	std::cout << "Average Solve Time: " << timer.avg(times) << "ms" << std::endl;
	std::cout << "Median Solve Time: " << timer.median(times) << "ms" << std::endl;
	std::cout << "Max Solve Time: " << *std::max_element(times.begin(), times.end()) << "ms" << std::endl;
	std::cout << "Total Time: " << std::accumulate(times.begin(), times.end(), 0) << "ms" << std::endl;
}

int main(int argc, char** argv) {
	// benchmark_solves();
	std::vector<std::string> scramble = {"U", "R2", "F", "B", "R", "B2", "R", "U2", "L", "B2", "R", "U'", "D'", "R2", "F", "R'", "L", "B2", "U2", "F2"};


	solve(scramble);

	return 0;
}
