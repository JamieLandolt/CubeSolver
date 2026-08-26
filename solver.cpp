#include <string>
#include <list>
#include <vector>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <iostream>
#include <fstream>

#include <functional>
#include <algorithm>
#include <cstdint>
#include <chrono>

#include <random>
#include <iomanip>

int RL_CORNER_MAPPING(int x) {
	if (x == 0) {
		return 1;
	} else if (x == 1) {
		return 0;
	} else if (x == 2) {
		return 2;
	} else {
		std::cout << "Invalid RL corner orientation: " << x << "\n";
	}
	return -1;
}

int UD_CORNER_MAPPING(int x) {
	if (x == 0) {
		return 0;
	} else if (x == 1) {
		return 2;
	} else if (x == 2) {
		return 1;
	} else {
		std::cout << "Invalid UD corner orientation: " << x << "\n";
	}
	return -1;
}

int FB_CORNER_MAPPING(int x) {
	if (x == 0) {
		return 2;
	} else if (x == 1) {
		return 1;
	} else if (x == 2) {
		return 0;
	} else {
		std::cout << "Invalid RL corner orientation: " << x << "\n";
	}
	return -1;
}


struct StateHash {
    size_t operator()(const std::pair<uint32_t, uint64_t>& p) const {
        size_t h1 = std::hash<uint32_t>()(p.first);
        size_t h2 = std::hash<uint64_t>()(p.second);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

uint64_t permRank(const std::vector<int>& perm) {
    int n = perm.size();
    std::vector<bool> used(n, false);
    uint64_t rank = 0;
    uint64_t fact = 1;
    for (int i = n - 2; i >= 0; i--) fact *= (i + 1); // (n-1)!

    for (int i = 0; i < n; i++) {
        int smaller = 0;
        for (int j = perm[i] - 1; j >= 0; j--) {
            if (!used[j]) smaller++;
        }
        rank += smaller * fact;
        used[perm[i]] = true;
        if (i < n - 1) fact /= (n - 1 - i);
    }
    return rank;
}

// vector 1: 8 pairs, first value 0-2, second value unique 0-7, order matters
uint32_t encodeVec1(const std::vector<std::pair<int,int>>& v) {
    std::vector<int> order(8);
    int arr[8];
    for (int i = 0; i < 8; i++) {
        order[i] = v[i].second;   // sequence of labels
        arr[v[i].second] = v[i].first;
    }
    uint32_t permPart = (uint32_t)permRank(order);   // 0 to 40319
    uint32_t valuePart = 0;
    for (int i = 0; i < 8; i++) valuePart = valuePart * 3 + arr[i]; // 0 to 6560

    return permPart * 6561 + valuePart;
}

// vector 2: 12 pairs, first value 0-1, second value unique 0-11, order matters
uint64_t encodeVec2(const std::vector<std::pair<int,int>>& v) {
    std::vector<int> order(12);
    uint32_t bitmask = 0;
    for (int i = 0; i < 12; i++) {
        order[i] = v[i].second;
        if (v[i].first == 1) bitmask |= (1u << v[i].second);
    }
    uint64_t permPart = permRank(order);   // 0 to 479001599
    uint64_t valuePart = bitmask;          // 0 to 4095

    return permPart * 4096ull + valuePart;
}


class Cube {
private:
	// Corner Encoding: Where the W/Y side is facing. TB = 0, FB = 1, RL = 2.
	// Edge Encoding: If the higher in the colour hierarchy is the higher value in the face hierarchy then 0 else 1
	// Hierarchy: WY/GB/RO TB/FB/RL
	std::vector<std::pair<int,int>> CORNERS_SOLVED;
	std::vector<std::pair<int,int>> EDGES_SOLVED;

	std::vector<int> R_CORNER_CYCLE {2, 1, 5, 6};
	std::vector<int> L_CORNER_CYCLE {0, 3, 7, 4};
	std::vector<int> U_CORNER_CYCLE {0, 1, 2, 3};
	std::vector<int> D_CORNER_CYCLE {7, 6, 5, 4};
	std::vector<int> F_CORNER_CYCLE {3, 2, 6, 7};
	std::vector<int> B_CORNER_CYCLE {1, 0, 4, 5};

	std::vector<int> R_EDGE_CYCLE {1, 5, 9, 6};
	std::vector<int> L_EDGE_CYCLE {3, 7, 11, 4};
	std::vector<int> U_EDGE_CYCLE {0, 1, 2, 3};
	std::vector<int> D_EDGE_CYCLE {10, 9, 8, 11};
	std::vector<int> F_EDGE_CYCLE {2, 6, 10, 7};
	std::vector<int> B_EDGE_CYCLE {0, 4, 8, 5};

	// Starts at the topmost edge on the side (before the turn) and goes clockwise around to each edge. 1 means its orientation changes.
	std::vector<int> RL_EDGE_ROTATION {0, 0, 0, 0};
	std::vector<int> UD_EDGE_ROTATION {0, 0, 0, 0};
	std::vector<int> FB_EDGE_ROTATION {1, 1, 1, 1};

	std::unordered_map<char,std::vector<int>> corner_cycles = {{'R', R_CORNER_CYCLE}, {'L', L_CORNER_CYCLE}, {'U', U_CORNER_CYCLE},
								 {'D', D_CORNER_CYCLE}, {'F', F_CORNER_CYCLE}, {'B', B_CORNER_CYCLE}};
	std::unordered_map<char,std::vector<int>> edge_cycles = {{'R', R_EDGE_CYCLE}, {'L', L_EDGE_CYCLE}, {'U', U_EDGE_CYCLE},
								 {'D', D_EDGE_CYCLE}, {'F', F_EDGE_CYCLE}, {'B', B_EDGE_CYCLE}};
	std::unordered_map<char,std::function<int(int)>> corner_rotations = {{'R', RL_CORNER_MAPPING}, {'L', RL_CORNER_MAPPING}, {'U', UD_CORNER_MAPPING},
								 {'D', UD_CORNER_MAPPING}, {'F', FB_CORNER_MAPPING}, {'B', FB_CORNER_MAPPING}};
	std::unordered_map<char,std::vector<int>> edge_rotations = {{'R', RL_EDGE_ROTATION}, {'L', RL_EDGE_ROTATION}, {'U', UD_EDGE_ROTATION},
								 {'D', UD_EDGE_ROTATION}, {'F', FB_EDGE_ROTATION}, {'B', FB_EDGE_ROTATION}};
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


		if (direction != -1 && direction != 1) {
			std::cout << "Invalid direction given: " << direction << "\n";
		}

		if (direction == -1) {
			std::vector<int> rev_corner_cycle(corner_cycle);
			std::reverse(rev_corner_cycle.begin(), rev_corner_cycle.end());
			corner_cycle = rev_corner_cycle;

		        std::vector<int> rev_edge_cycle(edge_cycle);
			std::reverse(rev_edge_cycle.begin(), rev_edge_cycle.end());
			edge_cycle = rev_edge_cycle;
		}

		// ATP whether the direction was -1 or 1 the variables have been updated so that the below code should cycle correctly, theoretically

		// Number of edges/corners that are affected by a move (size of corner_cycle or edge_cycle)
		int affected_pieces = 4;

		int corner_pos = corner_cycle[0];
		std::pair<int,int> next_corner(corners[corner_pos]);

		int edge_pos = edge_cycle[0];
		std::pair<int,int> next_edge(edges[edge_pos]);

		// Update corner and edge rotation
		for (int i = 0; i < affected_pieces; i++) {
			int next_corner_pos = corner_cycle[(i + 1) % affected_pieces];

			int next_edge_pos = edge_cycle[(i + 1) % affected_pieces];
			int edge_rot = edge_rotation[i];

			// Get corner and copy the next before replacing it
			std::pair<int,int> curr_corner = next_corner;
			next_corner = corners[next_corner_pos];

			// Get edge and copy the next before replacing it
			std::pair<int,int> curr_edge = next_edge;
			next_edge = edges[next_edge_pos];

			// Rotate FIRST then Move
			curr_corner.first = corner_rotation(curr_corner.first);
			corners[next_corner_pos] = curr_corner;

			// On each move edge_rot is either 0 or 1 corresponding to whether the EO was flipped
			// Mod 2 keeps rotation between 0 and 1
			curr_edge.first = (curr_edge.first + edge_rot) % 2;
			edges[next_edge_pos] = curr_edge;
		}
	}

	std::pair<int,int> get_piece_at(int corner, int pos) {
		// corner if corner = 1 else edge
		// num => the number of the corner or edge
		std::pair<int,int> p;

		if (corner) {
			if (!(0 <= pos && pos <= 7)) {
				std::cout << "Invalid Corner Chosen: " << pos << "\n";
				pos = 0;
			} else {
				p = corners[pos];
			}
		} else {
			if (!(0 <= pos && pos <= 11)) {
				std::cout << "Invalid Edge Chosen: " << pos << "\n";
				pos = 0;
			} else {
				p = edges[pos];
			}

		}

		return p;
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
		std::cout << "\nCorners: \n";
		for (int i = 0; i < 8; i++) {
			std::pair<int,int> p = get_piece_at(1, i);
			std::cout << p.first << " | " << p.second << "\n";
		}

		std::cout << "\nEdges: \n";
		for (int i = 0; i < 12; i++) {
			std::pair<int,int> p = get_piece_at(0, i);
			std::cout << p.first << " | " << p.second << "\n";
		}
	}
};

class Solver {
private:
	// Valid Moves after 
	std::vector<std::string> MOVES = {"R", "R'", "R2", "L", "L'", "L2", "U", "U'", "U2", "D", "D'", "D2", "F", "F'", "F2", "B", "B'", "B2"};
	std::vector<std::string> DOMINO_MOVES = {"R2", "L2", "F2", "B2", "U", "U'", "U2", "D", "D'", "D2"};

	int DEPTH_PHASE_1 = 12;
	int DEPTH_PHASE_2 = 20;
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
		auto dfs_start_time = std::chrono::steady_clock::now();
		long long dfs_iteration_count = 0;
		for (int search_depth = 1; search_depth < MAX_DEPTH; search_depth++) {
			// Reset dfs state
			reset();

			std::ofstream file("debug.txt");
			while (depths.size() > 0) {
				// Safety cap: bail out if the search has been running too long
				dfs_iteration_count++;
				if (dfs_iteration_count % 1000 == 0) {
					auto dfs_elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - dfs_start_time);
					if (dfs_elapsed.count() > 20) {
						std::cout << "dfs() timed out after " << dfs_elapsed.count() << "s, returning early\n";
						return;
					}
				}

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

class Timer {
private:
	std::chrono::high_resolution_clock::time_point start_time;
	std::unordered_map<int,std::vector<std::chrono::milliseconds>> times;
public:
	void start() {
		start_time = std::chrono::high_resolution_clock::now();
	}

	auto stop(int scramble_size) {
		std::chrono::milliseconds time = std::chrono::duration_cast<std::chrono::milliseconds>(
							std::chrono::high_resolution_clock::now() - start_time);
		times[scramble_size].push_back(time);
		return time;

	}

	std::unordered_map<int,std::vector<std::chrono::milliseconds>>& get_times() {
		return times;
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

			file.flush();
		}


		file << "\n";
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

struct BenchmarkResult {
	long long states_explored;
	int max_depth_reached;
	long long elapsed_ms;
	long long total_depth_searched = 0;
};

void log_benchmark_result(std::ostream& out, std::string label, BenchmarkResult result) {
	double states_per_sec = result.elapsed_ms > 0 ? (double)result.states_explored / (result.elapsed_ms / 1000.0) : 0.0;
	out << label << ":\n";
	out << "  States Explored: " << result.states_explored << "\n";
	out << "  Max Depth Reached: " << result.max_depth_reached << "\n";
	out << "  Total depth searched: " << result.total_depth_searched << "\n";
	out << "  Elapsed: " << result.elapsed_ms << "ms\n";
	out << "  States/sec: " << states_per_sec << "\n";
	if (result.states_explored > 0) {
		out << "  Average depth per state explored: " << ((double)result.total_depth_searched / result.states_explored) << "\n";
	}
	if (result.max_depth_reached > 0) {
		out << "  Average states per depth level (search breadth): " << ((double)result.states_explored / result.max_depth_reached) << "\n";
	}
	out << "\n";
}

// Mirrors the stack-based DFS traversal in Solver::dfs(), including the banned_next_moves
// pruning check, so that states-explored/sec can be measured for this feature in isolation.
BenchmarkResult explore_benchmark(const std::vector<std::string>& move_space, int search_depth, std::chrono::seconds duration) {
	std::unordered_map<char, std::unordered_set<char>> banned_next_moves {{'U', std::unordered_set<char>{'U', 'D'}},
									{'D', std::unordered_set<char>{'D'}},
									{'F', std::unordered_set<char>{'F', 'B'}}, {'B', std::unordered_set<char>{'B'}},
									{'R', std::unordered_set<char>{'R', 'L'}}, {'L', std::unordered_set<char>{'L'}}};

	Cube cube;

	std::stack<int> depths;
	std::stack<std::list<std::string>> moves;
	std::stack<std::vector<std::pair<int,int>>> corner_states;
	std::stack<std::vector<std::pair<int,int>>> edge_states;
	std::unordered_set<std::pair<uint32_t, uint64_t>, StateHash> visited;

	std::pair<std::vector<std::pair<int,int>>&,std::vector<std::pair<int,int>>&> state = cube.get_state();
	depths.push(0);
	moves.push(std::list<std::string>{});
	corner_states.push(state.first);
	edge_states.push(state.second);

	long long states_explored = 0;
	int max_depth_reached = 0;
	long long iteration_count = 0;
	long long total_depth_searched = 0;

	auto start_time = std::chrono::steady_clock::now();

	while (depths.size() > 0) {
		iteration_count++;
		if (iteration_count % 1000 == 0) {
			if (std::chrono::steady_clock::now() - start_time >= duration) {
				break;
			}
		}

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
		visited.insert(std::make_pair(encodeVec1(corners), encodeVec2(edges)));

		states_explored++;
		total_depth_searched += depth;
		if (depth > max_depth_reached) max_depth_reached = depth;

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
				if (!visited.count(std::make_pair(encodeVec1(p.first), encodeVec2(p.second)))) {
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

	BenchmarkResult result;
	result.states_explored = states_explored;
	result.max_depth_reached = max_depth_reached;
	result.elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count();
	result.total_depth_searched = total_depth_searched;
	return result;
}

int main(int argc, char** argv) {
	std::vector<std::string> MOVES = {"R", "R'", "R2", "L", "L'", "L2", "U", "U'", "U2", "D", "D'", "D2", "F", "F'", "F2", "B", "B'", "B2"};
	std::vector<std::string> DOMINO_MOVES = {"R2", "L2", "F2", "B2", "U", "U'", "U2", "D", "D'", "D2"};

	BenchmarkResult non_domino_result = explore_benchmark(MOVES, 20, std::chrono::seconds(10));
	log_benchmark_result(std::cout, "Non domino reduced search (bmark-3, banned_next_moves pruning)", non_domino_result);

	BenchmarkResult domino_result = explore_benchmark(DOMINO_MOVES, 20, std::chrono::seconds(10));
	log_benchmark_result(std::cout, "Domino reduced search (bmark-3, banned_next_moves pruning)", domino_result);

	// Solve-time scaling table across scramble sizes (existing Solver/dfs() path, unmodified).
	// Steps size by 5 (5, 10, 15, ...), 10 solves per size, stopping once the average solve
	// time exceeds 30s or the solve rate drops below half, but always reporting the size that
	// triggered the stop.
	const int NUM_SCRAMBLES = 30;
	const int SIZE_STEP = 5;
	const int MAX_SIZE = 50;

	struct SizeResult {
		int scramble_size;
		int solved_count;
		double avg_ms;
		long long median_ms;
		long long min_ms;
		long long max_ms;
		double avg_moves;
	};

	std::vector<SizeResult> table;

	std::cout << "=== Solve-time scaling table (bmark-3, DFS fixed-depth + banned_next_moves pruning) ===\n";

	for (int scramble_size = SIZE_STEP; scramble_size <= MAX_SIZE; scramble_size += (scramble_size < 10 ? 1 : SIZE_STEP)) {
		long long total_ms = 0;
		int solved_count = 0;
		std::vector<long long> solve_times_ms;
		std::vector<size_t> solve_move_counts;

		std::cout << "--- Scramble size " << scramble_size << " ---\n";

		for (int n = 0; n < NUM_SCRAMBLES; n++) {
			Cube cube = Cube();
			std::vector<std::string> scramble = cube.scramble(scramble_size);
			Solver solver = Solver(cube);

			auto solve_start = std::chrono::steady_clock::now();
			solver.dfs();
			auto solve_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - solve_start).count();

			std::pair<std::list<std::string>,std::list<std::string>> solution = solver.get_solution();
			bool solved = solution.second.size() > 0;

			std::cout << "Solve " << (n + 1) << ": " << (solved ? "SOLVED" : "NO SOLUTION FOUND")
				<< " | Time: " << solve_ms << "ms\n";

			if (solved) {
				total_ms += solve_ms;
				solved_count++;
				solve_times_ms.push_back(solve_ms);
				solve_move_counts.push_back(solution.first.size() + solution.second.size());
			}
		}

		SizeResult result{};
		result.scramble_size = scramble_size;
		result.solved_count = solved_count;

		if (solved_count > 0) {
			std::vector<long long> sorted_times(solve_times_ms);
			std::sort(sorted_times.begin(), sorted_times.end());
			result.avg_ms = (double)total_ms / solved_count;
			result.median_ms = sorted_times[sorted_times.size() / 2];
			result.min_ms = sorted_times.front();
			result.max_ms = sorted_times.back();

			size_t total_moves = 0;
			for (size_t mc : solve_move_counts) total_moves += mc;
			result.avg_moves = (double)total_moves / solve_move_counts.size();
		}

		table.push_back(result);

		bool avg_over_30s = solved_count > 0 && result.avg_ms > 30000.0;
		bool solve_rate_low = solved_count < (NUM_SCRAMBLES / 2);

		if (avg_over_30s || solve_rate_low) {
			std::cout << "Stopping: " << (avg_over_30s ? "average solve time exceeded 30s" : "solve rate dropped below half") << "\n";
			break;
		}
	}

	// Print final table
	std::cout << "\n=== SUMMARY TABLE ===\n";
	std::cout << std::left << std::setw(6) << "Size" << std::setw(8) << "Solved"
		<< std::setw(12) << "Avg(ms)" << std::setw(12) << "Median(ms)"
		<< std::setw(10) << "Min(ms)" << std::setw(10) << "Max(ms)" << "AvgMoves\n";
	for (const SizeResult& r : table) {
		std::cout << std::left << std::setw(6) << r.scramble_size
			<< std::setw(8) << (std::to_string(r.solved_count) + "/" + std::to_string(NUM_SCRAMBLES))
			<< std::setw(12) << (r.solved_count > 0 ? std::to_string((long long)r.avg_ms) : "-")
			<< std::setw(12) << (r.solved_count > 0 ? std::to_string(r.median_ms) : "-")
			<< std::setw(10) << (r.solved_count > 0 ? std::to_string(r.min_ms) : "-")
			<< std::setw(10) << (r.solved_count > 0 ? std::to_string(r.max_ms) : "-")
			<< (r.solved_count > 0 ? std::to_string(r.avg_moves) : "-") << "\n";
	}

	return 0;
}
