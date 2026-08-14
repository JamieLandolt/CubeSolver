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

	std::unordered_map<char, std::unordered_set<char>> banned_next_moves {{'U', std::unordered_set<char>{'U', 'D'}}, 
									{'D', std::unordered_set<char>{'D'}}, {'F', std::unordered_set<char>{'F', 'B'}}, 
									{'B', std::unordered_set<char>{'B'}}, {'R', std::unordered_set<char>{'R', 'L'}}, 
									{'L', std::unordered_set<char>{'L'}}};

public:
	std::vector<std::pair<int,int>> corners;
	std::vector<std::pair<int,int>> edges;

	Cube() {
		// Setup solved cube state
		for (int i = 0; i < 8; i++) {
			CORNERS_SOLVED.push_back(std::pair<int,int>{0, i});
		}
		for (int i = 0; i < 12; i++) {
			EDGES_SOLVED.push_back(std::pair<int,int>{0, i});
		}
		corners = CORNERS_SOLVED;
		edges = EDGES_SOLVED;
		

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

	void set_state(std::vector<std::pair<int,int>> new_corners, std::vector<std::pair<int,int>> new_edges) {
		corners = new_corners;
		edges = new_edges;
	}

	void reset() {
		set_state(CORNERS_SOLVED, EDGES_SOLVED);
	}

	std::pair<std::vector<std::pair<int,int>>&,std::vector<std::pair<int,int>>&> get_state() {
		return {corners, edges};
	}

	std::pair<std::vector<std::pair<int,int>>,std::vector<std::pair<int,int>>> get_solved_state() {
		return {CORNERS_SOLVED, EDGES_SOLVED};
	}

	void move(std::string mv) {
		if (mv.size() == 1) {
			cycle(corners, edges, corner_cycles[mv[0]], edge_cycles[mv[0]], corner_rotations[mv[0]], edge_rotations[mv[0]], 1);
		} else if (mv[1] == '2') {
			cycle(corners, edges, corner_cycles[mv[0]], edge_cycles[mv[0]], corner_rotations[mv[0]], edge_rotations[mv[0]], 1);
			cycle(corners, edges, corner_cycles[mv[0]], edge_cycles[mv[0]], corner_rotations[mv[0]], edge_rotations[mv[0]], 1);
		} else if (mv[1] == '\'') {
			cycle(corners, edges, corner_cycles[mv[0]], edge_cycles[mv[0]], corner_rotations[mv[0]], edge_rotations[mv[0]], -1);
		} else {
			std::cout << "Invalid move given: " << mv << "\n";
		}
	}

	void cycle(std::vector<std::pair<int,int>>& corners, std::vector<std::pair<int,int>>& edges, std::vector<int> corner_cycle,
		       std::vector<int> edge_cycle, std::function<int(int)> corner_rotation,
		       std::vector<int> edge_rotation, int direction) {
		// corners: Holds pairs corresponding to the rotation and corner number in each of the 7 corner positions in order from 0 to 7
		// edges: Holds pairs corresponding to the rotation and edge number in each of the 11 edge positions in order from 0 to 11
		// For corner and edge cycle vectors, the corner in the last position should cycle to the position in the first slot of the vector
		// corner_cycle: For the given move it holds the corners in the order that their rotation should be updated (according to 
		// corner_rotation) and also each corner in the position given by the number in the vector moves to the position of the the next 
		// corner
		// edge_cycle: For the move being performed, the edges that are currently in the positions that are stored in this vector should 
		// cycle to the next position in the vector
		// corner_rotation: Stores functions that should update the rotations of the corners in the positions stored in corner_cycle
		// edge_rotation: Same as corner roation but for edges and you just add the number % 2 to the edge rotation instead of a function
		// direction: Either 1, -1, 2 depending on whether you are doing an R, R', or R2 for example. 
		// Adjustments to the corner and edge cycle should be made (or they should be performed multiple times) if R2 for example
		

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
	std::stack<std::vector<std::pair<int,int>>> corner_states;
	std::stack<std::vector<std::pair<int,int>>> edge_states;
	std::unordered_set<std::pair<uint32_t, uint64_t>, StateHash> visited;

	std::pair<std::list<std::string>,std::list<std::string>> solution;
	Cube& cube;

	std::pair<std::vector<std::pair<int,int>>&,std::vector<std::pair<int,int>>&> state;
	std::vector<std::pair<int,int>>& corner_state;
	std::vector<std::pair<int,int>>& edge_state;

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

	std::pair<uint32_t,uint64_t> hash(std::vector<std::pair<int,int>> corners, std::vector<std::pair<int,int>> edges) {
		uint32_t v1code = encodeVec1(corners);
		uint64_t v2code = encodeVec2(edges);
		return std::make_pair(v1code, v2code); 
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

		std::stack<std::vector<std::pair<int,int>>> empty_corners;
		std::swap(corner_states, empty_corners);

		std::stack<std::vector<std::pair<int,int>>> empty_edges;
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

				std::vector<std::pair<int,int>> corners = corner_states.top();
				std::vector<std::pair<int,int>> edges = edge_states.top();

				std::list<std::string> state_moves(moves.top());

				depths.pop();
				moves.pop();
				corner_states.pop();
				edge_states.pop();

				// Hash and store visited state
				visited.insert(hash(corners, edges));
				
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

					std::stack<std::vector<std::pair<int,int>>> empty_corners;
					std::swap(corner_states, empty_corners);

					std::stack<std::vector<std::pair<int,int>>> empty_edges;
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
						std::pair<std::vector<std::pair<int,int>>,std::vector<std::pair<int,int>>> p = cube.get_state();
							
						// Store if we haven't been in the state before
						if (!visited.count(hash(p.first, p.second))) {
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

	int check_state(std::vector<std::pair<int,int>> corners, std::vector<std::pair<int,int>> edges) {
		// Checks if the current goal has been reached in the given position
		// Goal is determined by SOLVER_PHASE

		std::pair<std::vector<std::pair<int,int>>,std::vector<std::pair<int,int>>> cube_state = cube.get_solved_state();
		std::vector<std::pair<int,int>> CORNERS_SOLVED = cube_state.first;
		std::vector<std::pair<int,int>> EDGES_SOLVED = cube_state.second;

		std::unordered_set<int> mid_layer_edges = {4, 5, 6, 7};

		if (corners == CORNERS_SOLVED && edges == EDGES_SOLVED) {
			return 2;
		}

		if (SOLVER_PHASE == 0) {
			for (int i = 0; i < corners.size(); i++) {
				if (corners.at(i).first) {
					return 0;
				}
			} 

			for (int i = 0; i < edges.size(); i++) {
				if (edges.at(i).first) {
					return 0;
				}
				if (mid_layer_edges.count(i) && !mid_layer_edges.count(edges.at(i).second)) {
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
