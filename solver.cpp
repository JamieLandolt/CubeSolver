#include "rotation.h"
#include "timer.h"
#include "hash.h"
#include <string>
#include <list>
#include <vector>
#include <stack>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <cmath>

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

	std::pair<std::vector<int>, std::vector<int>> orientations;

	std::vector<std::string> scramble_moves;

	std::mt19937 gen;
	std::uniform_int_distribution<> dist;
	std::uniform_int_distribution<> dom_dist;

	std::vector<std::string> MOVES = {"R", "R'", "R2", "L", "L'", "L2", "U", "U'", "U2", "D", "D'", "D2", "F", "F'", "F2", "B", "B'", "B2"};
	std::vector<std::string> DOMINO_MOVES = {"R2", "L2", "F2", "B2", "U", "U'", "U2", "D", "D'", "D2"};

	std::unordered_map<char,std::function<long(long,int)>> corner_cycles = {{'R', R_CORNER_CYCLE}, {'L', L_CORNER_CYCLE}, {'U', U_CORNER_CYCLE},
								 {'D', D_CORNER_CYCLE}, {'F', F_CORNER_CYCLE}, {'B', B_CORNER_CYCLE}};
	std::unordered_map<char,std::function<long(long,int)>> edge_cycles = {{'R', R_EDGE_CYCLE}, {'L', L_EDGE_CYCLE}, {'U', U_EDGE_CYCLE},
								 {'D', D_EDGE_CYCLE}, {'F', F_EDGE_CYCLE}, {'B', B_EDGE_CYCLE}};

	std::unordered_map<char, std::unordered_set<char>> banned_next_moves {{'U', std::unordered_set<char>{'U', 'D'}}, 
									{'D', std::unordered_set<char>{'D'}}, {'F', std::unordered_set<char>{'F', 'B'}}, 
									{'B', std::unordered_set<char>{'B'}}, {'R', std::unordered_set<char>{'R', 'L'}}, 
									{'L', std::unordered_set<char>{'L'}}};

public:
	Cube() {
		std::random_device rd;
		gen = std::mt19937(rd());
		dist = std::uniform_int_distribution<>(0, MOVES.size() - 1);
		dom_dist = std::uniform_int_distribution<>(0, DOMINO_MOVES.size() - 1);
		orientations = generate_orientations();
	}

	std::pair<std::vector<std::string>,std::pair<long,long>> random_scramble(int scramble_size, int DEPTH_PHASE_1) {
		// Currently implemented so that it will make only scrambles that the solver can search far enough to find
		// Based on DEPTH_PHASE_1&2
		scramble_moves.clear();
		std::string last = "";
		for (int i = 0; i < scramble_size; i++) {
			std::string move;
			if (i < DEPTH_PHASE_1) {
				move = DOMINO_MOVES[dom_dist(gen)];
			} else {
				move = MOVES[dist(gen)];
			}
			if (i != 0 && banned_next_moves[move[0]].count(last[0])) {
				i--;
				continue;
			}
			last = move;
			scramble_moves.push_back(move);
		}
		
		std::pair<long,long> state = get_solved_state();
		return std::make_pair(scramble_moves, execute_moves(scramble_moves, state.first, state.second));
	}

	std::pair<std::vector<std::string>,std::pair<long,long>> scramble(std::vector<std::string> scramble_mvs) {
		scramble_moves = scramble_mvs;
		
		std::pair<long,long> state = get_solved_state();
		return std::make_pair(scramble_moves, execute_moves(scramble_moves, state.first, state.second));
	}

	std::pair<long,long> to_phase(std::vector<std::string> scramble_moves, std::list<std::string> p1_moves) {
		std::pair<long,long> state = get_solved_state();
		state = execute_moves(scramble_moves, state.first, state.second);
		return execute_moves(p1_moves, state.first, state.second);
	}

	std::pair<long,long> get_solved_state() {
		return {CORNERS_SOLVED, EDGES_SOLVED};
	}

	std::pair<long,long> move(std::string mv, long corners, long edges) {
		if (mv.size() == 1) {
			return cycle(1, mv[0], corners, edges);
		} else if (mv[1] == '2') {
			return cycle(2, mv[0], corners, edges);
		} else if (mv[1] == '\'') {
			return cycle(-1, mv[0], corners, edges);
		} else {
			std::cout << "Invalid move given: " << mv << "\n";
		}
		return std::make_pair(-1, -1);
	}

	std::pair<long,long> cycle(int direction, char move_type, long corners, long edges) {
		// direction: Either 1, -1, 2 depending on whether you are doing an R, R', or R2 for example. 
		std::function<long(long,int)> corner_cycler = corner_cycles[move_type];
		std::function<long(long,int)> edge_cycler = edge_cycles[move_type];
		corners = corner_cycler(corners, direction);
		edges = edge_cycler(edges, direction);

		return std::make_pair(corners, edges);
	}

	std::pair<long,long> execute_moves(std::vector<std::string> moves, long corners, long edges) {
		for (std::string m : moves) {
			std::pair<long,long> state = move(m, corners, edges);
			corners = state.first;
			edges = state.second;
		}
		return std::make_pair(corners, edges);
	}

	std::pair<long,long> execute_moves(std::list<std::string> moves, long corners, long edges) {
		for (std::string m : moves) {
			std::pair<long,long> state = move(m, corners, edges);
			corners = state.first;
			edges = state.second;
		}
		return std::make_pair(corners, edges);
	}

	void display(std::pair<long,long> state) {
		long corners = state.first;
		long edges = state.second;

		std::cout << "Corners:\n";
		for (int i = 0; i < 8; i++) {
			std::cout << i << ": ";
			std::cout << ((corners & (0b111ULL << i * 5)) >> i * 5) << ", " << ((corners & (0b11000ULL << i * 5)) >> i * 5 + 3) << "\n";
		}


		std::cout << "\nEdges:\n";
		for (int i = 0; i < 12; i++) {
			std::cout << i << ": ";
			std::cout << ((edges & (0b1111ULL << i * 5)) >> i * 5) << ", " << ((edges & (0b10000ULL << i * 5)) >> i * 5 + 4)<< "\n";
		}
		std::cout << "\n";
	}

	std::pair<int,int> ori_to_int(long corners, long edges) {
		// Make a base 3 and 2 number from the orientations
		int corner_b3 = 0;
		int edge_b2 = 0;
		for (int i = 0; i < 11; i++) {
			if (i < 7) {
				corner_b3 += pow(3, i) * ((corners & (0b11000ULL << i * 5)) >> i * 5 + 3);
			}
			edge_b2 += pow(2, i) * ((edges & (0b10000ULL << i * 5)) >> i * 5 + 4);
		}
		return std::make_pair(corner_b3, edge_b2);
	}

	std::pair<int,int> ori_to_int(std::pair<long,long> state) {
		// Make a base 3 and 2 number from the orientations
		int corner_b3 = 0;
		int edge_b2 = 0;
		for (int i = 0; i < 11; i++) {
			if (i < 7) {
				corner_b3 += pow(3, i) * ((state.first & (0b11000ULL << i * 5)) >> i * 5 + 3);
			}
			edge_b2 += pow(2, i) * ((state.second & (0b10000ULL << i * 5)) >> i * 5 + 4);
		}
		return std::make_pair(corner_b3, edge_b2);
	}

	int pow(int x, int n) {
		int ans = 1;
		for (int i = 0; i < n; i++) {
			ans *= x;
		}
		return ans;
	}

	std::pair<std::vector<int>, std::vector<int>> generate_orientations() {
		std::pair<long,long> solved_state = get_solved_state();

		int completed = 0;
		int num_corner_orientations = pow(3, 7);
		int num_edge_orientations = pow(2, 11);
		std::vector<int> corner_orientations(num_corner_orientations, -1);
		std::vector<int> edge_orientations(num_edge_orientations, -1);

		std::queue<std::pair<long,long>> states;
		states.push(solved_state);

		auto [corners_b3, edges_b2] = ori_to_int(solved_state);
		if (corner_orientations.at(corners_b3) == -1) {
			corner_orientations[corners_b3] = 0;
			completed++;
		}
		if (edge_orientations.at(edges_b2) == -1) {
			edge_orientations[edges_b2] = 0;
			completed++;
		}

		std::queue<int> moves;
		moves.push(0);

		// Find the minimum moves it takes to correctly orient each corner by making every sequence of moves
		while (completed < num_corner_orientations + num_edge_orientations) {
			std::pair<long,long> state = states.front();
			int num_moves = moves.front();

			states.pop();
			moves.pop();

			for (std::string mv : MOVES) {
				std::pair<long,long> next_state = move(mv, state.first, state.second);
				auto [corner_hash, edge_hash] = ori_to_int(next_state);
				// If there's a new orientation, explore it
				if (corner_orientations.at(corner_hash) == -1 || edge_orientations.at(edge_hash) == -1) {
					states.push(next_state);
					moves.push(num_moves + 1);

					// If we haven't stored that corner/edge state yet store the min moves to get to it
					if (corner_orientations.at(corner_hash) == -1) {
						corner_orientations[corner_hash] = num_moves + 1;
						completed++;
					}
					if (edge_orientations.at(edge_hash) == -1) {
						edge_orientations[edge_hash] = num_moves + 1;
						completed++;
					}

				}
			}
		}

		return std::make_pair(corner_orientations, edge_orientations);
	}
};

class Solver {
private:
	// Valid Moves after 
	std::vector<std::string> MOVES = {"R", "R'", "R2", "L", "L'", "L2", "U", "U'", "U2", "D", "D'", "D2", "F", "F'", "F2", "B", "B'", "B2"};
	std::vector<std::string> DOMINO_MOVES = {"R2", "L2", "F2", "B2", "U", "U'", "U2", "D", "D'", "D2"};

	int SOLVER_PHASE = 0; // 0 -> Performing Domino Reduction. 1 -> Solving the cube with reduced move space.

	std::stack<int> depths;
	std::stack<std::list<std::string>> moves;
	std::stack<long> corner_states;
	std::stack<long> edge_states;
	std::unordered_set<std::pair<long,long>,StateHash> visited;

	std::pair<std::vector<int>, std::vector<int>> orientations;
	std::vector<int> corner_orientations;
	std::vector<int> edge_orientations;

	std::pair<std::list<std::string>,std::list<std::string>> solution;
	Cube& cube;

	std::unordered_map<char, std::unordered_set<char>> banned_next_moves {{'U', std::unordered_set<char>{'U', 'D'}}, 
								{'D', std::unordered_set<char>{'D'}},
								{'F', std::unordered_set<char>{'F', 'B'}}, {'B', std::unordered_set<char>{'B'}},
								{'R', std::unordered_set<char>{'R', 'L'}}, {'L', std::unordered_set<char>{'L'}}};
	
public:
	int DEPTH_PHASE_1 = 9;
	int DEPTH_PHASE_2 = 9;

	Solver(Cube& external_cube) : cube(external_cube) {
		orientations = cube.generate_orientations();
		corner_orientations = orientations.first;
		edge_orientations = orientations.second;
	}

	std::pair<std::list<std::string>,std::list<std::string>> get_solution() {
		return solution;
	}

	void reset_full() {
		solution = {};
		SOLVER_PHASE = 0;
	}

	void set_depth(int type, int depth) {
		if (type == 1) {
			DEPTH_PHASE_1 = depth;
		} else {
			DEPTH_PHASE_2 = depth;
		}
	}

	void reset_dfs(std::vector<std::string> scramble_moves) {
		// Auto resets to the DR state if a solution for that has been found (AKA solution.first is not empty)
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
		
		std::pair<long,long> state = cube.to_phase(scramble_moves, solution.first);

		depths.push(0);
		moves.push(std::list<std::string>{});
		corner_states.push(state.first);
		edge_states.push(state.second);
	}

	void dfs(std::vector<std::string> scramble) {
		int MAX_DEPTH = DEPTH_PHASE_1;
		std::vector<std::string> move_space = MOVES;
		for (int search_depth = 1; search_depth <= MAX_DEPTH; search_depth++) {
			// Reset dfs state
			reset_dfs(scramble);

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
					// Based on the orientation of the corners and edges
					// Finds the minimum moves needed to solve the case
					// Works for both Phase 1 & 2
					std::pair<int,int> min_sol_moves = cube.ori_to_int(corners, edges);

					// If it takes more moves than are left in the search to solve, don't bother searching
					if (std::max(corner_orientations[min_sol_moves.first], edge_orientations[min_sol_moves.second]) > search_depth - depth) {
						break;
					}

					// Avoid moving the same side twice in a row
					if (state_moves.size() > 0 && banned_next_moves[state_moves.back()[0]].count(move[0])) {
						continue;
					}
					if (depth < search_depth) {
						std::list<std::string> next_state_moves = state_moves;
						std::pair<long,long> state = cube.move(move, corners, edges);
							
						// Store if we haven't been in the state before
						if (!visited.count(state)) {
							depths.push(depth + 1);
							next_state_moves.push_back(move);
							moves.push(next_state_moves);

							// Store new corner/edge states
							corner_states.push(state.first);
							edge_states.push(state.second);
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
	Timer timer = Timer();
	// Setup Output File
	std::ofstream file("benchmarks.txt");

	if (!file) {
		std::cerr << "Failed to open file\n";
	}

	Cube cube;
	Solver solver = Solver(cube);
	for (int scramble_size = 2; scramble_size < solver.DEPTH_PHASE_1 + solver.DEPTH_PHASE_2; scramble_size++) {
		file << "Scrambles of size " << scramble_size << ":\n";
		std::cout << "Scrambles of size " << scramble_size << ":\n";

		for (int scr_num = 0; scr_num < 5; scr_num++) {
			std::cout << "Scramble: " << scr_num << "\n";
			std::pair<std::vector<std::string>,std::pair<long,long>> p = cube.random_scramble(scramble_size, solver.DEPTH_PHASE_1);
			std::vector<std::string> scramble = p.first;
			std::pair<long,long> state = p.second;

			int i;
			file << "Scramble Moves: ";
			for (i = 0; i < scramble.size() - 1; i++) {
				file << scramble[i] << ", ";
			}
			file << scramble[i] << " | ";

			solver.reset_full();
			solver.reset_dfs(scramble);

			// Time Solve
			timer.start();
			solver.dfs(scramble);
			auto solve_time = timer.stop(scramble_size);

			file << "Time: " << solve_time.count() << "ms | ";

			// If a non middle layer edge is in a middle layer edge position
			std::pair<std::list<std::string>,std::list<std::string>> solution = solver.get_solution();
			std::list<std::string> p1_sol = solution.first;
			std::list<std::string> p2_sol = solution.second;

			// Write moves to file
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

			i = 0;
			file << "(Phase 2) ";
			for (std::string mv : p2_sol) {
				if (i < p2_sol.size()) {
					file << mv << ", ";
				} else {
					file << mv;
				}
			}
			file << "\n";

			file.flush();
		}

		file << "\n";
		file << "Average time for scramble size " << scramble_size << ": " << timer.get_avg_times()[scramble_size] << "ms\n\n";
	}

	if (!file.good()) {
		std::cerr << "Write failed\n";
	}
}

int main(int argc, char** argv) {
	std::vector<std::string> scramble = {"D", "R2", "U", "F2", "L2", "U", "D2", "F2", "U", "D2"};
	auto start = std::chrono::steady_clock::now();
	solve(scramble);
	auto end = std::chrono::steady_clock::now();
	std::cout << "Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";
	return 0;
}
