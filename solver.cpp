#include <string>
#include <utility>
#include <iostream>

class Cube {
private:
	// Corner Encoding: whatever is facing up or down. 0 is W/Y, 1 is G/B, 2 is R/O
	long CORNERS_SOLVED = 0b00111'00110'00101'00100'00011'00010'00001'00000;
	// Edge Encoding: If the higher in the colour hierachy is the higher value in the face hierarchy then 0 else 1
	// Hierarchy: WY/GB/RO TB/FB/RL
	long EDGES_SOLVED   = 0b0000'01011'01010'01001'01000'00111'00110'00101'00100'00011'00010'00001'00000;

	long corners;
	long edges;
public:
	Cube() {
		corners = CORNERS_SOLVED;
		edges = EDGES_SOLVED;
	}

	int move(std::string mv) {
		if (mv == "R") {
			
		}
	}

	std::pair<int, int> get_piece_at(int corner, int pos) {
		// corner if corner = 1 else edge
		// num => the number of the corner or edge
		std::pair<int, int> p;
		if (corner) {
			if (!(0 <= pos && pos <= 7)) {
				std::cout << "Invalid Corner Chosen: " << pos << "\n";
				pos = 0;
			}
			
			p.first = corners >> (5 * pos) & 0b111;
			p.second = corners >> ((5 * pos) + 3) & 0b11;
		} else {
			if (!(0 <= pos && pos <= 11)) {
				std::cout << "Invalid Edge Chosen: " << pos << "\n";
				pos = 0;
			}

			p.first = edges >> (5 * pos) & 0b1111;
			p.second = edges >> ((5 * pos) + 4) & 0b1;
		}

		return p;
	}
};

int main(int argc, char** argv) {
	Cube cube = Cube();
	for (int i = 0; i < 7; i++) {
		cube.get_piece_at(1, i);
	}
	return 0;
}
