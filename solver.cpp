#include "rotation.h"
#include "timer.h"
#include "hash.h"

#include <string>
#include <list>
#include <vector>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <iostream>
#include <fstream>

#include <algorithm>

#include <random>

class Cube {
private:
	// Corner Encoding: Where the W/Y side is facing. TB = 0, FB = 1, RL = 2.
	// Edge Encoding: If the higher in the colour hierarchy is the higher value in the face hierarchy then 0 else 1
	// Hierarchy: WY/GB/RO TB/FB/RL
	long CORNERS_SOLVED = 0b00111'00110'00101'00100'00011'00010'00001'00000;
	long EDGES_SOLVED = 0b01011'01010'01001'01000'00111'00110'00101'00100'00011'00010'00001'00000;

	std::vector<std::string> scramble_moves;

	std::mt19937 gen;
	std::uniform_int_distribution<> dist;

	std::vector<std::string> MOVES = {"R", "R'", "R2", "L", "L'", "L2", "U", "U'", "U2", "D", "D'", "D2", "F", "F'", "F2", "B", "B'", "B2"};

	std::unordered_map<char,std::function<long(long,int)>> corner_cycles = {{'R', R_CORNER_CYCLE}, {'L', L_CORNER_CYCLE}, {'U', U_CORNER_CYCLE},
								 {'D', D_CORNER_CYCLE}, {'F', F_CORNER_CYCLE}, {'B', B_CORNER_CYCLE}};
	std::unordered_map<char,std::function<long(long,int)>> edge_cycles = {{'R', R_EDGE_CYCLE}, {'L', L_EDGE_CYCLE}, {'U', U_EDGE_CYCLE},
								 {'D', D_EDGE_CYCLE}, {'F', F_EDGE_CYCLE}, {'B', B_EDGE_CYCLE}};

	std::unordered_map<char, std::unordered_set<char>> banned_next_moves {{'U', std::unordered_set<char>{'U', 'D'}}, 
									{'D', std::unordered_set<char>{'D'}}, {'F', std::unordered_set<char>{'F', 'B'}}, 
									{'B', std::unordered_set<char>{'B'}}, {'R', std::unordered_set<char>{'R', 'L'}}, 
									{'L', std::unordered_set<char>{'L'}}};

public:
	long& corners;
	long& edges;

	Cube() : corners(CORNERS_SOLVED), edges(EDGES_SOLVED) {
		std::random_device rd;
		gen = std::mt19937(rd());
		dist = std::uniform_int_distribution<>(0, MOVES.size() - 1);
	}

	std::vector<std::string> scramble(int scramble_size) {
		scramble_moves.clear();
		std::string last = "";
		for (int i = 0; i < scramble_size; i++) {
			std::string move = MOVES[dist(gen)];
			if (i != 0 && banned_next_moves[move[0]].count(last[0])) {
				i--;
				continue;
			}
			last = move;
			scramble_moves.push_back(move);
		}
		execute_moves(scramble_moves);
		return scramble_moves;
	}

	void set_scramble(std::vector<std::string> scramble_mvs) {
		reset();
		scramble_moves = scramble_mvs;
		execute_moves(scramble_moves);
	}

	void to_phase(std::list<std::string> p1_moves) {
		reset();
		execute_moves(scramble_moves);
		execute_moves(p1_moves);
	}

	std::vector<std::string> get_scramble() {
		return scramble_moves;
	}

	void set_state(long new_corners, long new_edges) {
		corners = new_corners;
		edges = new_edges;
	}

	void reset() {
		set_state(CORNERS_SOLVED, EDGES_SOLVED);
	}

	std::pair<long&,long&> get_state() {
		return {corners, edges};
	}

	std::pair<long,long> get_solved_state() {
		return {CORNERS_SOLVED, EDGES_SOLVED};
	}

	void move(std::string mv) {
		if (mv.size() == 1) {
			cycle(1, mv[0]);
		} else if (mv[1] == '2') {
			cycle(2, mv[0]);
		} else if (mv[1] == '\'') {
			cycle(-1, mv[0]);
		} else {
			std::cout << "Invalid move given: " << mv << "\n";
		}
	}

	void cycle(int direction, char move_type) {
		// direction: Either 1, -1, 2 depending on whether you are doing an R, R', or R2 for example. 
		std::function<long(long,int)> corner_cycler = corner_cycles[move_type];
		std::function<long(long,int)> edge_cycler = edge_cycles[move_type];

		corners = corner_cycler(corners, direction);
		edges = edge_cycler(edges, direction);
	}

	void execute_moves(std::vector<std::string> moves) {
		for (std::string m : moves) {
			move(m);
		}
	}

	void execute_moves(std::list<std::string> moves) {
		for (std::string m : moves) {
			move(m);
		}
	}

	void display() {
		
	}
};

class Solver {
private:
	// Valid Moves after 
	std::vector<std::string> MOVES = {"R", "R'", "R2", "L", "L'", "L2", "U", "U'", "U2", "D", "D'", "D2", "F", "F'", "F2", "B", "B'", "B2"};
	std::vector<std::string> DOMINO_MOVES = {"R2", "L2", "F2", "B2", "U", "U'", "U2", "D", "D'", "D2"};

	int DEPTH_PHASE_1 = 8;
	int DEPTH_PHASE_2 = 12;
	int SOLVER_PHASE = 0; // 0 -> Performing Domino Reduction. 1 -> Solving the cube with reduced move space.

	std::stack<int> depths;
	std::stack<std::list<std::string>> moves;
	std::stack<long> corner_states;
	std::stack<long> edge_states;
	std::unordered_set<std::pair<long,long>,StateHash> visited;

	std::pair<std::list<std::string>,std::list<std::string>> solution;
	Cube& cube;

	std::pair<long&,long&> state;
	long& corner_state;
	long& edge_state;

	std::unordered_map<char, std::unordered_set<char>> banned_next_moves {{'U', std::unordered_set<char>{'U', 'D'}}, 
									{'D', std::unordered_set<char>{'D'}},
									{'F', std::unordered_set<char>{'F', 'B'}}, {'B', std::unordered_set<char>{'B'}},
									{'R', std::unordered_set<char>{'R', 'L'}}, {'L', std::unordered_set<char>{'L'}}};
	
public:
	Solver(Cube& external_cube) : cube(external_cube), corner_state(state.first), edge_state(state.second), state(cube.get_state()) {}

	std::pair<std::list<std::string>,std::list<std::string>> get_solution() {
		return solution;
	}

	void reset_solution() {
		solution = {};
	}

	void set_depth(int type, int depth) {
		if (type == 1) {
			DEPTH_PHASE_1 = depth;
		} else {
			DEPTH_PHASE_2 = depth;
		}
	}

	void reset() {
		// Clear all stacks
		std::stack<int> empty_depths;
		std::swap(depths, empty_depths);
			
		std::stack<std::list<std::string>> empty_moves;
		std::swap(moves, empty_moves);

		std::stack<long> empty_corners;
		std::swap(corner_states, empty_corners);

		std::stack<long> empty_edges;
		std::swap(edge_states, empty_edges);

		visited.clear();
		
		cube.to_phase(solution.first);
		auto state = cube.get_state();

		depths.push(0);
		moves.push(std::list<std::string>{});
		corner_states.push(state.first);
		edge_states.push(state.second);
	}

	void dfs() {
		int MAX_DEPTH = DEPTH_PHASE_1;
		std::vector<std::string> move_space = MOVES;
		for (int search_depth = 0; search_depth < MAX_DEPTH; search_depth++) {
			std::cout << "Searching Depth: " << search_depth << "\n";
			
			// Reset dfs state
			reset();

			std::ofstream file("debug.txt");
			while (depths.size() > 0) {
				// Get next node to visit
				int depth = depths.top();

				long corners = corner_states.top();
				long edges = edge_states.top();

				std::list<std::string> state_moves(moves.top());

				depths.pop();
				moves.pop();
				corner_states.pop();
				edge_states.pop();

				// Hash and store visited state
				visited.insert({corners, edges});
				
				// Check for target state
				int phase_complete = check_state(corners, edges);

				if (phase_complete == 2) {
					// Solved state has been found
					solution.second = state_moves;
					return;
				}

				// Domino reduced state has been found for the first time
				if (phase_complete == 1 and SOLVER_PHASE == 0) {
					SOLVER_PHASE++;
					move_space = DOMINO_MOVES;
					search_depth = 1;
					MAX_DEPTH = DEPTH_PHASE_2;

					std::cout << "Domino reduction complete with: ";
					for (std::string s : state_moves) {
						std::cout << s << ", ";
					}
					std::cout << "\n";

					// Save moves to get to that state
					solution.first = state_moves;
					state_moves.clear();

					// Clear all stacks
					std::stack<int> empty_depths;
					std::swap(depths, empty_depths);
					
					std::stack<std::list<std::string>> empty_moves;
					std::swap(moves, empty_moves);

					std::stack<long> empty_corners;
					std::swap(corner_states, empty_corners);

					std::stack<long> empty_edges;
					std::swap(edge_states, empty_edges);

					visited.clear();

					// Setup next phase of search to start from domino reduced state
					depths.push(0);
					moves.push(std::list<std::string>{});
					corner_states.push(corners);
					edge_states.push(edges);
					continue;
				}

				// Search child nodes
				for (std::string move : move_space) {
					if (state_moves.size() > 0 && banned_next_moves[state_moves.back()[0]].count(move[0])) {
						// Avoid moving the same side twice in a row
						continue;
					}
					if (depth <= search_depth) {
						std::list<std::string> next_state_moves = state_moves;
						// Do the move
						cube.set_state(corners, edges);
						cube.move(move);
						std::pair<long,long> p = cube.get_state();
							
						// Store if we haven't been in the state before
						if (!visited.count(p)) {
							depths.push(depth + 1);
							next_state_moves.push_back(move);
							moves.push(next_state_moves);

							// Store new corner/edge states
							corner_states.push(p.first);
							edge_states.push(p.second);
						}
					}
				}
			}
		}
	}

	int check_state(long corners, long edges) {
		// Checks if the current goal has been reached in the given position
		// Goal is determined by SOLVER_PHASE

		std::pair<long,long> cube_state = cube.get_solved_state();
		long CORNERS_SOLVED = cube_state.first;
		long EDGES_SOLVED = cube_state.second;

		std::unordered_set<int> mid_layer_edges = {4, 5, 6, 7};

		int num_corners = 8;
		int num_edges = 12;

		if (corners == CORNERS_SOLVED && edges == EDGES_SOLVED) {
			return 2;
		}

		if (SOLVER_PHASE == 0) {
			for (int i = 0; i < num_corners; i++) {
				if (corners & (ZERO_CORI_MASK << i * 5)) {
					return 0;
				}
			} 

			for (int i = 0; i < num_edges; i++) {
				if (edges & (ZERO_EORI_MASK << i * 5)) {
					return 0;
				}
				// If a non middle layer edge is in a middle layer edge position
				if (mid_layer_edges.count(i) && !mid_layer_edges.count((edges & (ZERO_EPOS_MASK << i * 5)) >> i * 5)) {
					return 0;
				}
			} 

			return 1;
		}

		return 0;
	}
};

void benchmark_solves() {
	Timer timer = Timer();
	// Setup Output File
	std::ofstream file("benchmarks.txt");

	if (!file) {
		std::cerr << "Failed to open file\n";
	}

	Cube cube;
	Solver solver = Solver(cube);
	for (int scramble_size = 2; scramble_size < 25; scramble_size++) {
		file << "Scrambles of size " << scramble_size << ":\n";
		std::cout << "Scrambles of size " << scramble_size << ":\n";

		for (int scr_num = 0; scr_num < 5; scr_num++) {
			std::cout << "Scramble: " << scr_num << "\n";
			std::vector<std::string> scramble = cube.scramble(scramble_size);

			// Time Solve
			timer.start();
			solver.dfs();
			auto solve_time = timer.stop(scramble_size);

			file << "Time: " << solve_time.count() << "ms | ";

			int i;
			// If a non middle layer edge is in a middle layer edge position
			file << "Scramble Moves: ";
			for (i = 0; i < scramble.size() - 1; i++) {
				file << scramble[i] << ", ";
			}
			file << scramble[i] << " | ";

			std::pair<std::list<std::string>,std::list<std::string>> solution = solver.get_solution();
			std::list<std::string> p1_sol = solution.first;
			std::list<std::string> p2_sol = solution.second;

			i = 0;
			file << "(Phase 1) ";
			for (std::string mv : p1_sol) {
				i++;
				if (i < p1_sol.size()) {
					file << mv << ", ";
				} else {
					file << mv << " | ";
				}
			}

			file << "(Phase 2) ";
			for (std::string mv : p2_sol) {
				if (i < p2_sol.size()) {
					file << mv << ", ";
				} else {
					file << mv;
				}
			}
			file << "\n";

			cube.reset();
			solver.reset();
			solver.reset_solution();;

			file.flush();
		}

		file << "\n";
		file << "Average time for scramble size " << scramble_size << ": " << timer.get_avg_times()[scramble_size] << "ms\n\n";
	}

	if (!file.good()) {
		std::cerr << "Write failed\n";
	}
}

void solve(std::vector<std::string> scramble) {
	Cube cube = Cube();
	cube.set_scramble(scramble);
	Solver solver = Solver(cube);
	solver.dfs();

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

int main(int argc, char** argv) {
	benchmark_solves();
	return 0;
}
