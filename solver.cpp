#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <unordered_map>
#include <functional>
#include <algorithm>

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


class Cube {
private:
	// Corner Encoding: Where the W/Y side is facing. TB = 0, FB = 1, RL = 2.
	// Edge Encoding: If the higher in the colour hierachy is the higher value in the face hierarchy then 0 else 1
	std::vector<std::pair<int,int>> CORNERS_SOLVED;
	std::vector<std::pair<int,int>> EDGES_SOLVED;

	std::vector<std::pair<int,int>> corners;
	std::vector<std::pair<int,int>> edges;

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
	std::vector<int> RL_EDGE_ROTATION {0, 1, 0, 1};
	std::vector<int> UD_EDGE_ROTATION {0, 0, 0, 0};
	std::vector<int> FB_EDGE_ROTATION {1, 1, 1, 1};

	std::unordered_map<char,std::vector<int>> corner_cycles = {{'R', R_CORNER_CYCLE}, {'L', L_CORNER_CYCLE}, {'U', U_CORNER_CYCLE},
								 {'D', D_CORNER_CYCLE}, {'F', F_CORNER_CYCLE}, {'B', B_CORNER_CYCLE}};
	std::unordered_map<char,std::vector<int>> edge_cycles = {{'R', R_EDGE_CYCLE}, {'L', L_EDGE_CYCLE}, {'U', U_EDGE_CYCLE},
								 {'D', D_EDGE_CYCLE}, {'F', F_EDGE_CYCLE}, {'B', B_EDGE_CYCLE}};
	std::unordered_map<char,std::function<int(int)>> corner_rotations = {{'R', RL_CORNER_MAPPING}, {'L', RL_CORNER_MAPPING}, {'U', UD_CORNER_MAPPING},
								 {'D', UD_CORNER_MAPPING}, {'F', FB_CORNER_MAPPING}, {'B', FB_CORNER_MAPPING}};
	// Hierarchy: WY/GB/RO TB/FB/RL
	std::unordered_map<char,std::vector<int>> edge_rotations = {{'R', RL_EDGE_ROTATION}, {'L', RL_EDGE_ROTATION}, {'U', UD_EDGE_ROTATION},
								 {'D', UD_EDGE_ROTATION}, {'F', FB_EDGE_ROTATION}, {'B', FB_EDGE_ROTATION}};

public:
	Cube() {
		for (int i = 0; i < 8; i++) {
			CORNERS_SOLVED.push_back(std::pair<int,int>{0, i});
		}
		for (int i = 0; i < 12; i++) {
			EDGES_SOLVED.push_back(std::pair<int,int>{0, i});
		}

		corners = CORNERS_SOLVED;
		edges = EDGES_SOLVED;
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

	void cycle(std::vector<std::pair<int,int>> corners, std::vector<std::pair<int,int>> edges, std::vector<int> corner_cycle,
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

		        std::function<int(int)> rev_corner_rotation(corner_rotation);
			std::reverse(rev_corner_rotation.begin(), rev_corner_rotation.end());
			corner_rotation = rev_corner_rotation;

		        std::vector<int> rev_edge_rotation(edge_rotation);					
			std::reverse(rev_edge_rotation.begin(), rev_edge_rotation.end());
			edge_rotation = rev_edge_rotation;
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
			std::function<int(int)> rot = corner_rotation[i];

			int next_edge_pos = edge_cycle[(i + 1) % affected_pieces];
			int edge_rot = edge_rotation[i];

			// Get corner and copy the next before replacing it
			std::pair<int,int> curr_corner = next_corner;
			next_corner(corners[next_corner_pos]);

			// Get edge and copy the next before replacing it
			std::pair<int,int> curr_edge = next_edge;
			next_edge(edges[next_edge_pos]);

			// Rotate FIRST then Move
			curr_corner.first = rot(curr_corner.first);
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

	void display() {
		for (int i = 0; i < 7; i++) {
			std::pair<int,int> p = get_piece_at(1, i);
			std::cout << p.first << " | " << p.second << "\n";
		}

		for (int i = 0; i < 11; i++) {
			std::pair<int,int> p = get_piece_at(0, i);
			std::cout << p.first << " | " << p.second << "\n";
		}
	}
};


int main(int argc, char** argv) {
	Cube cube;
	cube.execute_moves(std::vector<std::string>{"R", "U", "R'", "U'", "R'", "F", "R2", "U'", "R'", "U'", "R", "U", "R'", "F'"});
	cube.display();
	cube.execute_moves(std::vector<std::string>{"R", "U", "R'", "U'", "R'", "F", "R2", "U'", "R'", "U'", "R", "U", "R'", "F'"});
	cube.display();
	
	return 0;
}
