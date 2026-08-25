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
#include <memory>

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

	std::pair<std::vector<int>,std::vector<int>> generate_orientations() {
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

	std::string invert_move(std::string move) {
		// Inverts domino reduction moves
		if (move[0] == 'U' || move[0] == 'D') {
			if (move.size() == 1) {
				return std::string(1, move[0]) + '\'';
			} else if (move[1] == '\'') {
				return std::string(1, move[0]);
			} else {
				return move;
			}
		}
		return move;
	}

	std::unordered_map<std::pair<long,long>,PathEntry,StateHash> generate_solution_lookup(int MAX_SOL_SEARCH_DEPTH) {
        std::cout << "Generating Phase 3 steps...\n";
		std::pair<long,long> solved_state = get_solved_state();

		std::unordered_map<std::pair<long,long>,PathEntry,StateHash> solution_paths;
        // Preallocate enough space to never need to rehash everything, for MAX_SOL_SEARCH_DEPTH = 9 this is good
        if (MAX_SOL_SEARCH_DEPTH != 9) {
        std::cerr << "CHANGE solution_paths RESERVE VAL\n";
        }
		solution_paths.reserve(28000000);
		PathEntry solved_path{solved_state, ""};
		solution_paths[solved_state] = solved_path;

		std::queue<std::pair<long,long>> states;
		std::queue<std::string> last_moves;
		std::queue<int> depths;

		states.push(solved_state);
		last_moves.push("");
		depths.push(0);

		while (states.size()) {
			std::pair<long,long> state = states.front();
			std::string last_move = last_moves.front();
			int depth = depths.front();

			states.pop();
			last_moves.pop();
			depths.pop();

			for (std::string mv : DOMINO_MOVES) {
				std::pair<long,long> next_state = move(mv, state.first, state.second);
				// Avoid moving the same side twice in a row to reduce the search space
				if (last_move.size() > 0 && banned_next_moves[last_move[0]].count(mv[0]) 
					|| depth > MAX_SOL_SEARCH_DEPTH - 1) {
					continue;
				}
				// If we haven't seen the next_state yet add it for exploration and store how to get from there to the solved state
				if (!solution_paths.count(next_state)) {
					states.push(next_state);
					last_moves.push(mv);
					depths.push(depth + 1);

					solution_paths[next_state] = PathEntry{state, invert_move(mv)};
				}
			}
		}
        
        std::cout << "Finished generating Phase 3 steps.\n";
		return solution_paths;
	}
};

class Solver {
private:
	// Valid Moves after 
	std::vector<std::string> MOVES = {"R", "R'", "R2", "L", "L'", "L2", "U", "U'", "U2", "D", "D'", "D2", "F", "F'", "F2", "B", "B'", "B2"};
	std::vector<std::string> DOMINO_MOVES = {"R2", "L2", "F2", "B2", "U", "U'", "U2", "D", "D'", "D2"};

	int SOLVER_PHASE = 0; // 0 -> Performing Domino Reduction. 1 -> Solving the cube with reduced move space.

	std::stack<DFSEntry> states;
	std::unordered_set<std::pair<long,long>,StateHash> visited;

	std::unordered_map<std::pair<long,long>,PathEntry,StateHash> solution_paths;

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
	int DEPTH_PHASE_1 = 12;
	int DEPTH_PHASE_2 = 12;

	Solver(Cube& external_cube) : cube(external_cube) {
		solution_paths = cube.generate_solution_lookup(9);
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
		std::stack<DFSEntry> empty_states;
		std::swap(states, empty_states);

		std::pair<long,long> state = cube.to_phase(scramble_moves, solution.first);

		visited.clear();
        visited.insert(state);

		states.push(DFSEntry{nullptr, state.first, state.second, "", 0});
	}

	void dfs(std::vector<std::string> scramble) {
		int MAX_DEPTH = DEPTH_PHASE_1;
		std::vector<std::string> move_space = MOVES;

		std::vector<std::unique_ptr<DFSEntry>> dfs_nodes;

		for (int search_depth = 1; search_depth <= MAX_DEPTH; search_depth++) {
			std::cout << "Searching Depth: " << search_depth << "\n";
			
			// Reset dfs state
			reset_dfs(scramble);

			std::ofstream file("debug.txt");
			while (states.size() > 0) {
				// Get next node to visit

		 		DFSEntry dfs_state(states.top());

				int depth = dfs_state.depth;
				std::string move = dfs_state.move;
				long corners = dfs_state.corners;
				long edges = dfs_state.edges;

				states.pop();
				
				// Check for target state
				int phase_complete = check_state(corners, edges);

				if (phase_complete == 2) {
					// Solved state has been found
					solution.second = combine(dfs_state, solution_paths[std::make_pair(corners, edges)]);
					return;
				}

				// Domino reduced state has been found for the first time
				if (phase_complete == 1 and SOLVER_PHASE == 0) {
					SOLVER_PHASE++;
					move_space = DOMINO_MOVES;
					search_depth = 1;
					MAX_DEPTH = DEPTH_PHASE_2;

					// Save moves to get to that state
					solution.first = get_moves(&dfs_state);

					// Clear all stacks
				        std::stack<DFSEntry> empty_states;
				        std::swap(states, empty_states);

					visited.clear();
					visited.insert(std::make_pair(corners, edges));

				        states.push(DFSEntry{&dfs_state, corners, edges, "", 0});
					continue;
				}

				// Based on the orientation of the corners and edges
				// Finds the minimum moves needed to solve the case
				// Works for both Phase 1 & 2
				std::pair<int,int> min_sol_moves = cube.ori_to_int(corners, edges);
				// Search child nodes
				for (std::string move : move_space) {
					// If it takes more moves than are left in the search to solve, don't bother searching
					if (std::max(corner_orientations[min_sol_moves.first], edge_orientations[min_sol_moves.second]) > search_depth - depth) {
						break;
					}

					// Avoid moving the same side twice in a row
					if (depth > 0 && banned_next_moves[dfs_state.move[0]].count(move[0])) {
						continue;
					}
					if (depth < search_depth) {
						std::pair<long,long> state = cube.move(move, corners, edges);
							
						// Store if we haven't been in the state before
						if (!visited.count(state)) {
							visited.insert(state);
							std::unique_ptr<DFSEntry> parent = std::make_unique<DFSEntry>(dfs_state);
							DFSEntry* parent_ptr = parent.get();
							dfs_nodes.push_back(std::move(parent));
							DFSEntry next_state_moves = DFSEntry{std::move(parent_ptr), state.first, state.second, move, depth + 1};
							states.push(next_state_moves);
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

		if (solution_paths.count(std::make_pair(corners, edges))) {
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

    std::list<std::string> combine(DFSEntry sol_p2, PathEntry sol_p3) {
        std::list<std::string> solution = get_moves(&sol_p2);
        std::list<std::string> solution_p3 = get_moves(sol_p3);
        
        solution.splice(solution.end(), solution_p3);
        return solution;
    }


    std::list<std::string> get_moves(PathEntry path) {
        std::list<std::string> sol;
        while (path.move.size()) {
            sol.push_back(path.move);
            path = solution_paths[path.parent_state];
        }
        return sol;
    }

    std::list<std::string> get_moves(DFSEntry* path) {
        std::list<std::string> sol;
        while (path->move.size()) {
            sol.push_back(path->move);
            path = path->parent_state;
        }
	sol.reverse();
        return sol;
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
    Cube cube;
	Solver solver = Solver(cube);
	Timer timer = Timer();

    std::vector<long> times;

	int scramble_size = 25;
	for (int scramble_num = 0; scramble_num < 20; scramble_num++) {
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
	std::cout << "Min Solve Time: " << *std::min_element(times.begin(), times.end()) << std::endl;
	std::cout << "Average Solve Time: " << timer.avg(times) << std::endl;
	std::cout << "Median Solve Time: " << timer.median(times) << std::endl;
	std::cout << "Max Solve Time: " << *std::max_element(times.begin(), times.end()) << std::endl;
}

int main(int argc, char** argv) {
	// Hardcoded 25-move scramble empirically found to solve quickly (well under
	// a second of actual search time) on this commit (the most optimized
	// version), for gprof profiling purposes.
	std::vector<std::string> scramble = {"D", "F2", "U", "L2", "D", "B2", "D'", "F2", "B2", "D'", "R2", "D2", "L2", "B'", "L", "B", "U'", "D", "F", "L'", "F", "D2", "L'", "U2", "L'"};

	solve(scramble);

	return 0;
}
