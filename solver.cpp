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

	void dfs(std::vector<std::string> scramble, std::chrono::seconds max_time = std::chrono::seconds(60)) {
		int MAX_DEPTH = DEPTH_PHASE_1;
		std::vector<std::string> move_space = MOVES;

		std::vector<std::unique_ptr<DFSEntry>> dfs_nodes;

		// Safety cap so a runaway search can't hang benchmarking runs indefinitely
		auto dfs_start_time = std::chrono::steady_clock::now();
		long long nodes_since_time_check = 0;

		for (int search_depth = 1; search_depth <= MAX_DEPTH; search_depth++) {
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

				nodes_since_time_check++;
				if (nodes_since_time_check >= 1000) {
					nodes_since_time_check = 0;
					if (std::chrono::steady_clock::now() - dfs_start_time > max_time) {
						std::cout << "Solve exceeded safety cap of " << max_time.count() << "s, aborting.\n";
						return;
					}
				}

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

// --- bmark-7: additive benchmarking instrumentation below (final benchmark) ---

struct BenchmarkResult {
	long long states_explored;
	int max_depth_reached;
	long long elapsed_ms;
	long long total_depth_searched = 0;
};

// Mirrors the DFS traversal mechanics used by Solver::dfs (iterative deepening,
// banned same-face-twice pruning, visited-state dedup, unique_ptr-owned DFSEntry
// parent chain freed at the end of each depth's search) so that the states/sec
// figure is representative of this commit's actual search mechanics.
BenchmarkResult explore_benchmark(const std::vector<std::string>& move_space, int search_depth, std::chrono::seconds duration) {
	Cube cube;

	std::unordered_map<char, std::unordered_set<char>> banned_next_moves {{'U', std::unordered_set<char>{'U', 'D'}},
								{'D', std::unordered_set<char>{'D'}}, {'F', std::unordered_set<char>{'F', 'B'}},
								{'B', std::unordered_set<char>{'B'}}, {'R', std::unordered_set<char>{'R', 'L'}},
								{'L', std::unordered_set<char>{'L'}}};

	std::pair<long,long> solved_state = cube.get_solved_state();

	auto start_time = std::chrono::steady_clock::now();
	long long states_explored = 0;
	int max_depth_reached = 0;
	long long total_depth_searched = 0;

	// Single continuous DFS up to search_depth, with one visited set for the whole run -
	// matches bmark-3/5's explore_benchmark() structure. (A prior version of this function
	// wrapped the DFS in an outer per-depth-limit restart loop that reset the visited set
	// from scratch at every depth level - the same benchmark-harness bug found and fixed on
	// bmark-4/bmark-6: it forces repeated re-exploration of the same shallow states instead
	// of completing one deep pass, capping max depth reached far below the real cap.)
	std::unordered_set<std::pair<long,long>, StateHash> visited;
	std::stack<DFSEntry> states;
	std::vector<std::unique_ptr<DFSEntry>> dfs_nodes;

	visited.insert(solved_state);
	states.push(DFSEntry{nullptr, solved_state.first, solved_state.second, "", 0});

	while (states.size() > 0) {
		DFSEntry dfs_state(states.top());
		states.pop();

		states_explored++;
		total_depth_searched += dfs_state.depth;
		if (dfs_state.depth > max_depth_reached) {
			max_depth_reached = dfs_state.depth;
		}

		if (states_explored % 1000 == 0) {
			if (std::chrono::steady_clock::now() - start_time >= duration) {
				break;
			}
		}

		if (dfs_state.depth < search_depth) {
			for (const std::string& mv : move_space) {
				if (dfs_state.depth > 0 && banned_next_moves[dfs_state.move[0]].count(mv[0])) {
					continue;
				}
				std::pair<long,long> state = cube.move(mv, dfs_state.corners, dfs_state.edges);

				if (!visited.count(state)) {
					visited.insert(state);
					std::unique_ptr<DFSEntry> parent = std::make_unique<DFSEntry>(dfs_state);
					DFSEntry* parent_ptr = parent.get();
					dfs_nodes.push_back(std::move(parent));
					states.push(DFSEntry{parent_ptr, state.first, state.second, mv, dfs_state.depth + 1});
				}
			}
		}
	}

	long long elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count();
	return BenchmarkResult{states_explored, max_depth_reached, elapsed_ms, total_depth_searched};
}

struct IddfsResult {
	long long states_explored = 0;
	int max_depth_completed = 0;
	long long elapsed_ms = 0;
};

// True IDDFS: restarts a fresh bounded DFS pass (its own visited set, from the
// solved state) at increasing depth limits 1, 2, 3, ..., stopping once the total
// time budget is used up. A depth level only counts as "reached" if that entire
// pass finished within budget - unlike explore_benchmark() above, which is a
// single continuous unpruned DFS pass that can reach a nominal depth almost
// instantly just by descending one path without ever completing a full level.
// Uses the same orientation-lookup pruning bound and banned-move check
// Solver::dfs() uses (a plain stack here, not DFSEntry - that's a separate,
// orthogonal optimisation to how deep this benchmark can search).
IddfsResult iddfs_benchmark(const std::vector<std::string>& move_space, int max_depth_limit, std::chrono::seconds duration) {
	std::unordered_map<char, std::unordered_set<char>> banned_next_moves {{'U', std::unordered_set<char>{'U', 'D'}},
								{'D', std::unordered_set<char>{'D'}}, {'F', std::unordered_set<char>{'F', 'B'}},
								{'B', std::unordered_set<char>{'B'}}, {'R', std::unordered_set<char>{'R', 'L'}},
								{'L', std::unordered_set<char>{'L'}}};

	Cube cube;
	std::pair<std::vector<int>, std::vector<int>> orientations = cube.generate_orientations();
	std::vector<int> corner_orientations = orientations.first;
	std::vector<int> edge_orientations = orientations.second;
	std::pair<long,long> solved = cube.get_solved_state();

	IddfsResult result;
	auto start = std::chrono::steady_clock::now();

	for (int depth_limit = 1; depth_limit <= max_depth_limit; depth_limit++) {
		std::stack<int> depths;
		std::stack<char> last_move_chars;
		std::stack<long> corner_states;
		std::stack<long> edge_states;
		std::unordered_set<std::pair<long,long>,StateHash> visited;

		depths.push(0);
		last_move_chars.push('\0');
		corner_states.push(solved.first);
		edge_states.push(solved.second);

		bool timed_out = false;
		while (depths.size() > 0) {
			if (result.states_explored % 1000 == 0 && std::chrono::steady_clock::now() - start >= duration) {
				timed_out = true;
				break;
			}

			int depth = depths.top();
			char last_move = last_move_chars.top();
			long corners = corner_states.top();
			long edges = edge_states.top();

			depths.pop();
			last_move_chars.pop();
			corner_states.pop();
			edge_states.pop();

			visited.insert({corners, edges});
			result.states_explored++;

			std::pair<int,int> min_sol_moves = cube.ori_to_int(corners, edges);
			if (std::max(corner_orientations[min_sol_moves.first], edge_orientations[min_sol_moves.second]) <= depth_limit - depth) {
				for (const std::string& mv : move_space) {
					if (last_move != '\0' && banned_next_moves[last_move].count(mv[0])) {
						continue;
					}
					if (depth < depth_limit) {
						std::pair<long,long> next_state = cube.move(mv, corners, edges);
						if (!visited.count(next_state)) {
							depths.push(depth + 1);
							last_move_chars.push(mv[0]);
							corner_states.push(next_state.first);
							edge_states.push(next_state.second);
						}
					}
				}
			}
		}

		if (timed_out) break;
		result.max_depth_completed = depth_limit;
	}

	result.elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
	return result;
}

void log_iddfs_result(std::ostream& out, const std::string& label, const IddfsResult& r) {
	out << "=== " << label << " (true IDDFS) ===\n";
	out << "States explored: " << r.states_explored << "\n";
	out << "Max depth FULLY COMPLETED: " << r.max_depth_completed << "\n";
	out << "Elapsed: " << r.elapsed_ms << "ms\n\n";
}

void log_explore_result(const std::string& label, const BenchmarkResult& result) {
	double states_per_sec = result.elapsed_ms > 0 ? (double)result.states_explored / (result.elapsed_ms / 1000.0) : 0.0;

	std::cout << label << ":\n";
	std::cout << "  States Explored: " << result.states_explored << "\n";
	std::cout << "  Max Depth Reached: " << result.max_depth_reached << "\n";
	std::cout << "  Total depth searched: " << result.total_depth_searched << "\n";
	std::cout << "  Elapsed: " << result.elapsed_ms << "ms\n";
	std::cout << "  States/sec: " << states_per_sec << "\n";
	if (result.states_explored > 0) {
		std::cout << "  Average depth per state explored: " << ((double)result.total_depth_searched / result.states_explored) << "\n";
	}
	if (result.max_depth_reached > 0) {
		std::cout << "  Average states per depth level (search breadth): " << ((double)result.states_explored / result.max_depth_reached) << "\n";
	}
	std::cout << "\n";
}

void benchmark_states_explored() {
	std::vector<std::string> MOVES = {"R", "R'", "R2", "L", "L'", "L2", "U", "U'", "U2", "D", "D'", "D2", "F", "F'", "F2", "B", "B'", "B2"};
	std::vector<std::string> DOMINO_MOVES = {"R2", "L2", "F2", "B2", "U", "U'", "U2", "D", "D'", "D2"};

	BenchmarkResult non_domino_result = explore_benchmark(MOVES, 20, std::chrono::seconds(10));
	log_explore_result("Non domino reduced search (bmark-7, final)", non_domino_result);

	BenchmarkResult domino_result = explore_benchmark(DOMINO_MOVES, 20, std::chrono::seconds(10));
	log_explore_result("Domino reduced search (bmark-7, final)", domino_result);

	IddfsResult iddfs_non_domino = iddfs_benchmark(MOVES, 30, std::chrono::seconds(10));
	log_iddfs_result(std::cout, "Non domino reduced search (bmark-7)", iddfs_non_domino);

	IddfsResult iddfs_domino = iddfs_benchmark(DOMINO_MOVES, 30, std::chrono::seconds(10));
	log_iddfs_result(std::cout, "Domino reduced search (bmark-7)", iddfs_domino);
}

// Solve-time benchmark at a single, well-chosen scramble size (bmark-1/bmark-2 reporting style):
// solved-count, avg/median/min/max solve time, and avg moves in solution.
std::pair<int,double> benchmark_solve_times(int scramble_size) {
	Cube cube;
	Solver solver = Solver(cube);
	Timer timer = Timer();

	const int NUM_SOLVES = 30;
	std::vector<long> times;
	std::vector<int> move_counts;
	int solved_count = 0;

	std::cout << "=== Solves of size " << scramble_size << " (bmark-7, final) ===\n";

	for (int solve_num = 0; solve_num < NUM_SOLVES; solve_num++) {
		std::pair<std::vector<std::string>,std::pair<long,long>> p = cube.random_scramble(scramble_size, solver.DEPTH_PHASE_1);
		std::vector<std::string> scramble = p.first;

		solver.reset_full();
		solver.reset_dfs(scramble);

		timer.start();
		solver.dfs(scramble);
		auto solve_time = timer.stop(scramble_size);

		std::pair<std::list<std::string>,std::list<std::string>> solution = solver.get_solution();
		bool solved = solution.second.size() > 0;

		std::cout << "Solve " << (solve_num + 1) << ": " << (solved ? "SOLVED" : "NO SOLUTION FOUND")
				   << " | Time: " << solve_time.count() << "ms\n";

		if (solved) {
			solved_count++;
			times.push_back(solve_time.count());
			move_counts.push_back((int)(solution.first.size() + solution.second.size()));
		}
	}

	std::cout << "Solved " << solved_count << "/" << NUM_SOLVES << " scrambles of size " << scramble_size;
	double avg_time = 0.0;
	if (solved_count > 0) {
		avg_time = timer.avg(times);
		std::cout << ". Average solve time (successful solves only): " << avg_time << "ms\n";
		double avg_moves = (double)std::accumulate(move_counts.begin(), move_counts.end(), 0) / move_counts.size();
		std::cout << "Median solve time: " << timer.median(times) << "ms | Min: " << *std::min_element(times.begin(), times.end())
				   << "ms | Max: " << *std::max_element(times.begin(), times.end()) << "ms | Average moves in solution: " << avg_moves << "\n";
	} else {
		std::cout << ". No successful solves to average.\n";
	}
	return std::make_pair(solved_count, avg_time);
}

int main(int argc, char** argv) {
	benchmark_states_explored();

	// Full scaling table: step size by 5 starting at 5, stop once average solve time
	// exceeds 30s (or solve rate drops meaningfully), still reporting that final row.
	// Raised from a 25 cap to 40: at size 25 both this branch and bmark-6 were still
	// averaging well under the 30s stop threshold, so 25 was an arbitrary cutoff (on
	// the "God's number ~20 in HTM" theory) rather than the real break condition
	// actually firing - extending lets the natural stop condition find where each
	// branch really tops out, for a direct bmark-6 vs bmark-7 comparison at harder sizes.
	for (int size = 5; size <= 40; size += 5) {
		std::pair<int,double> result = benchmark_solve_times(size);
		int solved_count = result.first;
		double avg_time = result.second;

		if (avg_time > 30000.0 || solved_count < 7) {
			std::cout << "=== Stopping scaling sweep at size " << size << " ===\n";
			break;
		}
	}
	// std::vector<std::string> scramble = {"F", "U'", "F2", "D'", "B", "U", "R'", "F", "L", "D'", "R'", "U'", "L", "U", "B'", "D2", "R'", "F", "U2", "D2"};

	// solve(scramble);

	return 0;
}
