#ifndef ROTATION_H
#define ROTATION_H

#include <iostream>

long ZERO_MASK = 0b11111;
long ONE_MASK = ZERO_MASK << (5 * 1);
long TWO_MASK = ZERO_MASK << (5 * 2);
long THREE_MASK = ZERO_MASK << (5 * 3);
long FOUR_MASK = ZERO_MASK << (5 * 4);
long FIVE_MASK = ZERO_MASK << (5 * 5);
long SIX_MASK = ZERO_MASK << (5 * 6);
long SEVEN_MASK = ZERO_MASK << (5 * 7);
long EIGHT_MASK = ZERO_MASK << (5 * 8);
long NINE_MASK = ZERO_MASK << (5 * 9);
long TEN_MASK = ZERO_MASK << (5 * 10);
long ELEVEN_MASK = ZERO_MASK << (5 * 11);

// Masks for corner positions
long ZERO_CPOS_MASK = 0b00111;
long ONE_CPOS_MASK = ZERO_CPOS_MASK << (5 * 1);
long TWO_CPOS_MASK = ZERO_CPOS_MASK << (5 * 2);
long THREE_CPOS_MASK = ZERO_CPOS_MASK << (5 * 3);
long FOUR_CPOS_MASK = ZERO_CPOS_MASK << (5 * 4);
long FIVE_CPOS_MASK = ZERO_CPOS_MASK << (5 * 5);
long SIX_CPOS_MASK = ZERO_CPOS_MASK << (5 * 6);
long SEVEN_CPOS_MASK = ZERO_CPOS_MASK << (5 * 7);

// Masks for edge positions
long ZERO_EPOS_MASK = 0b01111;
long ONE_EPOS_MASK = ZERO_EPOS_MASK << (5 * 1);
long TWO_EPOS_MASK = ZERO_EPOS_MASK << (5 * 2);
long THREE_EPOS_MASK = ZERO_EPOS_MASK << (5 * 3);
long FOUR_EPOS_MASK = ZERO_EPOS_MASK << (5 * 4);
long FIVE_EPOS_MASK = ZERO_EPOS_MASK << (5 * 5);
long SIX_EPOS_MASK = ZERO_EPOS_MASK << (5 * 6);
long SEVEN_EPOS_MASK = ZERO_EPOS_MASK << (5 * 7);
long EIGHT_EPOS_MASK = ZERO_EPOS_MASK << (5 * 8);
long NINE_EPOS_MASK = ZERO_EPOS_MASK << (5 * 9);
long TEN_EPOS_MASK = ZERO_EPOS_MASK << (5 * 10);
long ELEVEN_EPOS_MASK = ZERO_EPOS_MASK << (5 * 11);

// Masks for corner orientations
long ZERO_CORI_MASK = 0b11000;
long ONE_CORI_MASK = ZERO_CORI_MASK << (5 * 1);
long TWO_CORI_MASK = ZERO_CORI_MASK << (5 * 2);
long THREE_CORI_MASK = ZERO_CORI_MASK << (5 * 3);
long FOUR_CORI_MASK = ZERO_CORI_MASK << (5 * 4);
long FIVE_CORI_MASK = ZERO_CORI_MASK << (5 * 5);
long SIX_CORI_MASK = ZERO_CORI_MASK << (5 * 6);
long SEVEN_CORI_MASK = ZERO_CORI_MASK << (5 * 7);

// Masks for edge orientations
long ZERO_EORI_MASK = 0b10000;
long ONE_EORI_MASK = ZERO_EORI_MASK << (5 * 1);
long TWO_EORI_MASK = ZERO_EORI_MASK << (5 * 2);
long THREE_EORI_MASK = ZERO_EORI_MASK << (5 * 3);
long FOUR_EORI_MASK = ZERO_EORI_MASK << (5 * 4);
long FIVE_EORI_MASK = ZERO_EORI_MASK << (5 * 5);
long SIX_EORI_MASK = ZERO_EORI_MASK << (5 * 6);
long SEVEN_EORI_MASK = ZERO_EORI_MASK << (5 * 7);
long EIGHT_EORI_MASK = ZERO_EORI_MASK << (5 * 8);
long NINE_EORI_MASK = ZERO_EORI_MASK << (5 * 9);
long TEN_EORI_MASK = ZERO_EORI_MASK << (5 * 10);
long ELEVEN_EORI_MASK = ZERO_EORI_MASK << (5 * 11);

long R_CORNER_CYCLE(long corners, int direction) {
	// 00111'00110'00101'00100'00011'00010'00001'00000;
	// std::vector<long> R_CORNER_CYCLE {2, 1, 5, 6};
	// Same as L for orientation: 0 -> 0, 1 -> 2, 2 -> 1
	// 0 -> 1, 1 -> 0, 2 -> 2
	// 00 -> 01, 01 -> 00, 10 -> 10 (| + ~ = 2nd bit, first bit stays the same)
	
	// Cycle positions

	long bit1;
	long bit2;
	long bit2_pos;

	long two_orientation;
	long one_orientation;
	long five_orientation;
	long six_orientation;
	if (direction != 2) {
		bit1 = corners & 1ULL << 2 * 5 + 4;
		bit2 = corners & 1ULL << 2 * 5 + 3;
		bit2_pos = 1ULL << 2 * 5 + 3;
		two_orientation = bit1 | (~(bit1 >> 1 | bit2) & bit2_pos);

		bit1 = corners & 1ULL << 1 * 5 + 4;
		bit2 = corners & 1ULL << 1 * 5 + 3;
		bit2_pos = 1ULL << 1 * 5 + 3;
		one_orientation = bit1 | (~(bit1 >> 1 | bit2) & bit2_pos);

		bit1 = corners & 1ULL << 5 * 5 + 4;
		bit2 = corners & 1ULL << 5 * 5 + 3;
		bit2_pos = 1ULL << 5 * 5 + 3;
		five_orientation = bit1 | (~(bit1 >> 1 | bit2) & bit2_pos);

		bit1 = corners & 1ULL << 6 * 5 + 4;
		bit2 = corners & 1ULL << 6 * 5 + 3;
		bit2_pos = 1ULL << 6 * 5 + 3;
		six_orientation = bit1 | (~(bit1 >> 1 | bit2) & bit2_pos);
	 } else {
		// Orientation stays the same for any double move
		two_orientation = corners & TWO_CORI_MASK;
		one_orientation = corners & ONE_CORI_MASK;
		five_orientation = corners & FIVE_CORI_MASK;
		six_orientation = corners & SIX_CORI_MASK;
	 }

	// std::vector<long> R_CORNER_CYCLE {2, 1, 5, 6};
	int two_shift;
	int one_shift;
	int five_shift;
	int six_shift;
	long cycled = 2;
	if (direction == 1) {
		two_shift = 5;
		one_shift = 4 * 5;
		five_shift = 5;
		six_shift = 4 * 5;

		cycled = ((corners & TWO_CPOS_MASK) | two_orientation) >> two_shift | ((corners & ONE_CPOS_MASK) | one_orientation) << one_shift
		| ((corners & FIVE_CPOS_MASK) | five_orientation) << five_shift | ((corners & SIX_CPOS_MASK) | six_orientation) >> six_shift
		| (corners & ~(ONE_MASK | TWO_MASK | FIVE_MASK | SIX_MASK));
	} else if (direction == 2) {
		two_shift = 3 * 5;
		one_shift = 5 * 5;
		five_shift = 3 * 5;
		six_shift = 5 * 5;

		cycled = ((corners & TWO_CPOS_MASK) | two_orientation) << two_shift | ((corners & ONE_CPOS_MASK) | one_orientation) << one_shift
		| ((corners & FIVE_CPOS_MASK) | five_orientation) >> five_shift | ((corners & SIX_CPOS_MASK) | six_orientation) >> six_shift
		| (corners & ~(ONE_MASK | TWO_MASK | FIVE_MASK | SIX_MASK));
	} else if (direction == -1) {
		two_shift = 4 * 5;
		one_shift = 5;
		five_shift = 4 * 5;
		six_shift = 5;

		// NOTE: This cycle may be different to the cycle in direction == 1 despite looking the same, some << or >> are switched directions
		cycled = ((corners & TWO_CPOS_MASK) | two_orientation) << two_shift | ((corners & ONE_CPOS_MASK) | one_orientation) << one_shift
		| ((corners & FIVE_CPOS_MASK) | five_orientation) >> five_shift | ((corners & SIX_CPOS_MASK) | six_orientation) >> six_shift
		| (corners & ~(ONE_MASK | TWO_MASK | FIVE_MASK | SIX_MASK));
	} else {
		std::cerr << "Invalid direction passed to R_Corner_Cycle: " << direction << "\n";
	}

	return cycled;
}

long L_CORNER_CYCLE(long corners, int direction) {
	// 00111'00110'00101'00100'00011'00010'00001'00000;
        // std::vector<long> L_CORNER_CYCLE {0, 3, 7, 4};
	// Same as R for orientation: 0 -> 1, 1 -> 0, 2 -> 2
	// 00 -> 01, 01 -> 00, 10 -> 10 (| + ~ = 2nd bit, first bit stays the same)
	
	// Cycle positions

	long bit1;
	long bit2;
	long bit2_pos;

	long zero_orientation;
	long three_orientation;
	long seven_orientation;
	long four_orientation;
	if (direction != 2) {
		bit1 = corners & 1ULL << 0 * 5 + 4;
		bit2 = corners & 1ULL << 0 * 5 + 3;
		bit2_pos = 1ULL << 0 * 5 + 3;
		zero_orientation = bit1 | (~(bit1 >> 1 | bit2) & bit2_pos);

		bit1 = corners & 1ULL << 3 * 5 + 4;
		bit2 = corners & 1ULL << 3 * 5 + 3;
		bit2_pos = 1ULL << 3 * 5 + 3;
		three_orientation = bit1 | (~(bit1 >> 1 | bit2) & bit2_pos);

		bit1 = corners & 1ULL << 7 * 5 + 4;
		bit2 = corners & 1ULL << 7 * 5 + 3;
		bit2_pos = 1ULL << 7 * 5 + 3;
		seven_orientation = bit1 | (~(bit1 >> 1 | bit2) & bit2_pos);

		bit1 = corners & 1ULL << 4 * 5 + 4;
		bit2 = corners & 1ULL << 4 * 5 + 3;
		bit2_pos = 1ULL << 4 * 5 + 3;
		four_orientation = bit1 | (~(bit1 >> 1 | bit2) & bit2_pos);
	 } else {
		// Orientation stays the same for any double move
		zero_orientation = corners & ZERO_CORI_MASK;
		three_orientation = corners & THREE_CORI_MASK;
		seven_orientation = corners & SEVEN_CORI_MASK;
		four_orientation = corners & FOUR_CORI_MASK;
	 }

        // std::vector<long> L_CORNER_CYCLE {0, 3, 7, 4};
	int zero_shift;
	int three_shift;
	int seven_shift;
	int four_shift;
	long cycled = 2;
	if (direction == 1) {
		zero_shift = 3 * 5;
		three_shift = 4 * 5;
		seven_shift = 3 * 5;
		four_shift = 4 * 5;

		cycled = ((corners & ZERO_CPOS_MASK) | zero_orientation) << zero_shift | ((corners & THREE_CPOS_MASK) | three_orientation) << three_shift
		| ((corners & SEVEN_CPOS_MASK) | seven_orientation) >> seven_shift | ((corners & FOUR_CPOS_MASK) | four_orientation) >> four_shift
		| (corners & ~(ZERO_MASK | THREE_MASK | SEVEN_MASK | FOUR_MASK));
	} else if (direction == 2) {
		zero_shift = 7 * 5;
		three_shift = 5;
		seven_shift = 7 * 5;
		four_shift = 5;

		cycled = ((corners & ZERO_CPOS_MASK) | zero_orientation) << zero_shift | ((corners & THREE_CPOS_MASK) | three_orientation) << three_shift
		| ((corners & SEVEN_CPOS_MASK) | seven_orientation) >> seven_shift | ((corners & FOUR_CPOS_MASK) | four_orientation) >> four_shift
		| (corners & ~(ZERO_MASK | THREE_MASK | SEVEN_MASK | FOUR_MASK));
	} else if (direction == -1) {
		zero_shift = 4 * 5;
		three_shift = 3 * 5;
		seven_shift = 4 * 5;
		four_shift = 3 * 5;

		// NOTE: This cycle may be different to the cycle in direction == 1 despite looking the same, some << or >> are switched directions
		cycled = ((corners & ZERO_CPOS_MASK) | zero_orientation) << zero_shift | ((corners & THREE_CPOS_MASK) | three_orientation) >> three_shift
		| ((corners & SEVEN_CPOS_MASK) | seven_orientation) >> seven_shift | ((corners & FOUR_CPOS_MASK) | four_orientation) << four_shift
		| (corners & ~(ZERO_MASK | THREE_MASK | SEVEN_MASK | FOUR_MASK));
	} else {
		std::cerr << "Invalid direction passed to L_Corner_Cycle: " << direction << "\n";
	}

	return cycled;
}

long U_CORNER_CYCLE(long corners, int direction) {
	// 00111'00110'00101'00100'00011'00010'00001'00000;
        // std::vector<long> U_CORNER_CYCLE {0, 1, 2, 3};
	// Same as D for orientation: 0 -> 0, 1 -> 2, 2 -> 1
	// 00 -> 00, 01 -> 10, 10 -> 01 (Flip the bits)
	
	// Cycle positions

	long bit1;
	long bit2;

	long zero_orientation;
	long one_orientation;
	long two_orientation;
	long three_orientation;
	if (direction != 2) {
		bit1 = corners & 1ULL << 0 * 5 + 4;
		bit2 = corners & 1ULL << 0 * 5 + 3;
		zero_orientation = (bit1 >> 1) | (bit2 << 1);

		bit1 = corners & 1ULL << 1 * 5 + 4;
		bit2 = corners & 1ULL << 1 * 5 + 3;
		one_orientation = (bit1 >> 1) | (bit2 << 1);

		bit1 = corners & 1ULL << 2 * 5 + 4;
		bit2 = corners & 1ULL << 2 * 5 + 3;
		two_orientation = (bit1 >> 1) | (bit2 << 1);

		bit1 = corners & 1ULL << 3 * 5 + 4;
		bit2 = corners & 1ULL << 3 * 5 + 3;
		three_orientation = (bit1 >> 1) | (bit2 << 1);
	 } else {
		// Orientation stays the same for any double move
		zero_orientation = corners & ZERO_CORI_MASK;
		one_orientation = corners & ONE_CORI_MASK;
		two_orientation = corners & TWO_CORI_MASK;
		three_orientation = corners & THREE_CORI_MASK;
	 }

        // std::vector<long> U_CORNER_CYCLE {0, 1, 2, 3};
	int zero_shift;
	int one_shift;
	int two_shift;
	int three_shift;
	long cycled = 2;
	if (direction == 1) {
		zero_shift = 5;
		one_shift = 5;
		two_shift = 5;
		three_shift = 3 * 5;

		cycled = ((corners & ZERO_CPOS_MASK) | zero_orientation) << zero_shift | ((corners & ONE_CPOS_MASK) | one_orientation) << one_shift
		| ((corners & TWO_CPOS_MASK) | two_orientation) << two_shift | ((corners & THREE_CPOS_MASK) | three_orientation) >> three_shift
		| (corners & ~(ONE_MASK | ZERO_MASK | TWO_MASK | THREE_MASK));
	} else if (direction == 2) {
		zero_shift = 2 * 5;
		one_shift = 2 * 5;
		two_shift = 2 * 5;
		three_shift = 2 * 5;

		cycled = ((corners & ZERO_CPOS_MASK) | zero_orientation) << zero_shift | ((corners & ONE_CPOS_MASK) | one_orientation) << one_shift
		| ((corners & TWO_CPOS_MASK) | two_orientation) >> two_shift | ((corners & THREE_CPOS_MASK) | three_orientation) >> three_shift
		| (corners & ~(ONE_MASK | ZERO_MASK | TWO_MASK | THREE_MASK));
	} else if (direction == -1) {
		zero_shift = 3 * 5;
		one_shift = 5;
		two_shift = 5;
		three_shift = 5;

		// NOTE: This cycle may be different to the cycle in direction == 1 despite looking the same, some << or >> are switched directions
		cycled = ((corners & ZERO_CPOS_MASK) | zero_orientation) << zero_shift | ((corners & ONE_CPOS_MASK) | one_orientation) >> one_shift
		| ((corners & TWO_CPOS_MASK) | two_orientation) >> two_shift | ((corners & THREE_CPOS_MASK) | three_orientation) >> three_shift
		| (corners & ~(ONE_MASK | ZERO_MASK | TWO_MASK | THREE_MASK));
	} else {
		std::cerr << "Invalid direction passed to U_Corner_Cycle: " << direction << "\n";
	}

	return cycled;
}

long D_CORNER_CYCLE(long corners, int direction) {
	// 00111'00110'00101'00100'00011'00010'00001'00000;
        // std::vector<long> D_CORNER_CYCLE {7, 6, 5, 4};
	// Same as U for orientation: 0 -> 0, 1 -> 2, 2 -> 1
	// 00 -> 00, 01 -> 10, 10 -> 01 (Flip the bits)
	
	// Cycle positions

	long bit1;
	long bit2;

	long seven_orientation;
	long six_orientation;
	long five_orientation;
	long four_orientation;
	if (direction != 2) {
		bit1 = corners & 1ULL << 7 * 5 + 4;
		bit2 = corners & 1ULL << 7 * 5 + 3;
		seven_orientation = (bit1 >> 1) | (bit2 << 1);

		bit1 = corners & 1ULL << 6 * 5 + 4;
		bit2 = corners & 1ULL << 6 * 5 + 3;
		six_orientation = (bit1 >> 1) | (bit2 << 1);

		bit1 = corners & 1ULL << 5 * 5 + 4;
		bit2 = corners & 1ULL << 5 * 5 + 3;
		five_orientation = (bit1 >> 1) | (bit2 << 1);

		bit1 = corners & 1ULL << 4 * 5 + 4;
		bit2 = corners & 1ULL << 4 * 5 + 3;
		four_orientation = (bit1 >> 1) | (bit2 << 1);
	 } else {
		// Orientation stays the same for any double move
		seven_orientation = corners & SEVEN_CORI_MASK;
		six_orientation = corners & SIX_CORI_MASK;
		five_orientation = corners & FIVE_CORI_MASK;
		four_orientation = corners & FOUR_CORI_MASK;
	 }

        // std::vector<long> D_CORNER_CYCLE {7, 6, 5, 4};
	int seven_shift;
	int six_shift;
	int five_shift;
	int four_shift;
	long cycled = 2;
	if (direction == 1) {
		seven_shift = 5;
		six_shift = 5;
		five_shift = 5;
		four_shift = 3 * 5;

		cycled = ((corners & SEVEN_CPOS_MASK) | seven_orientation) >> seven_shift | ((corners & SIX_CPOS_MASK) | six_orientation) >> six_shift
		| ((corners & FIVE_CPOS_MASK) | five_orientation) >> five_shift | ((corners & FOUR_CPOS_MASK) | four_orientation) << four_shift
		| (corners & ~(SIX_MASK | SEVEN_MASK | FIVE_MASK | FOUR_MASK));
	} else if (direction == 2) {
		seven_shift = 2 * 5;
		six_shift = 2 * 5;
		five_shift = 2 * 5;
		four_shift = 2 * 5;

		cycled = ((corners & SEVEN_CPOS_MASK) | seven_orientation) >> seven_shift | ((corners & SIX_CPOS_MASK) | six_orientation) >> six_shift
		| ((corners & FIVE_CPOS_MASK) | five_orientation) << five_shift | ((corners & FOUR_CPOS_MASK) | four_orientation) << four_shift
		| (corners & ~(SIX_MASK | SEVEN_MASK | FIVE_MASK | FOUR_MASK));
	} else if (direction == -1) {
		seven_shift = 3 * 5;
		six_shift = 5;
		five_shift = 5;
		four_shift = 5;

		// NOTE: This cycle may be different to the cycle in direction == 1 despite looking the same, some << or >> are switched directions
		cycled = ((corners & SEVEN_CPOS_MASK) | seven_orientation) >> seven_shift | ((corners & SIX_CPOS_MASK) | six_orientation) << six_shift
		| ((corners & FIVE_CPOS_MASK) | five_orientation) << five_shift | ((corners & FOUR_CPOS_MASK) | four_orientation) << four_shift
		| (corners & ~(SIX_MASK | SEVEN_MASK | FIVE_MASK | FOUR_MASK));
	} else {
		std::cerr << "Invalid direction passed to D_Corner_Cycle: " << direction << "\n";
	}

	return cycled;
}

long F_CORNER_CYCLE(long corners, int direction) {
	// 00111'00110'00101'00100'00011'00010'00001'00000;
        // std::vector<long> F_CORNER_CYCLE {3, 2, 6, 7};
	// Same as B for orientation: 0 -> 0, 1 -> 2, 2 -> 1
	// 00 -> 10, 01 -> 01, 10 -> 00 (| + ~ = 1st bit, 2nd bit stays the same)
	
	// Cycle positions

	long bit1;
	long bit2;
	long bit1_pos;

	long three_orientation;
	long two_orientation;
	long six_orientation;
	long seven_orientation;
	if (direction != 2) {
		bit1 = corners & 1ULL << 3 * 5 + 4;
		bit2 = corners & 1ULL << 3 * 5 + 3;
		bit1_pos = 1ULL << 3 * 5 + 4;
		three_orientation = (~(bit1 | (bit2 << 1)) & bit1_pos) | bit2;

		bit1 = corners & 1ULL << 2 * 5 + 4;
		bit2 = corners & 1ULL << 2 * 5 + 3;
		bit1_pos = 1ULL << 2 * 5 + 4;
		two_orientation = (~(bit1 | (bit2 << 1)) & bit1_pos) | bit2;

		bit1 = corners & 1ULL << 6 * 5 + 4;
		bit2 = corners & 1ULL << 6 * 5 + 3;
		bit1_pos = 1ULL << 6 * 5 + 4;
		six_orientation = (~(bit1 | (bit2 << 1)) & bit1_pos) | bit2;

		bit1 = corners & 1ULL << 7 * 5 + 4;
		bit2 = corners & 1ULL << 7 * 5 + 3;
		bit1_pos = 1ULL << 7 * 5 + 4;
		seven_orientation = (~(bit1 | (bit2 << 1)) & bit1_pos) | bit2;
	 } else {
		// Orientation stays the same for any double move
		three_orientation = corners & THREE_CORI_MASK;
		two_orientation = corners & TWO_CORI_MASK;
		six_orientation = corners & SIX_CORI_MASK;
		seven_orientation = corners & SEVEN_CORI_MASK;
	 }

        // std::vector<long> F_CORNER_CYCLE {3, 2, 6, 7};
	int three_shift;
	int two_shift;
	int six_shift;
	int seven_shift;
	long cycled = 2;
	if (direction == 1) {
		three_shift = 5;
		two_shift = 4 * 5;
		six_shift = 5;
		seven_shift = 4 * 5;

		cycled = ((corners & THREE_CPOS_MASK) | three_orientation) >> three_shift | ((corners & TWO_CPOS_MASK) | two_orientation) << two_shift
		| ((corners & SIX_CPOS_MASK) | six_orientation) << six_shift | ((corners & SEVEN_CPOS_MASK) | seven_orientation) >> seven_shift
		| (corners & ~(TWO_MASK | THREE_MASK | SIX_MASK | SEVEN_MASK));
	} else if (direction == 2) {
		three_shift = 3 * 5;
		two_shift = 5 * 5;
		six_shift = 3 * 5;
		seven_shift = 5 * 5;

		cycled = ((corners & THREE_CPOS_MASK) | three_orientation) << three_shift | ((corners & TWO_CPOS_MASK) | two_orientation) << two_shift
		| ((corners & SIX_CPOS_MASK) | six_orientation) >> six_shift | ((corners & SEVEN_CPOS_MASK) | seven_orientation) >> seven_shift
		| (corners & ~(TWO_MASK | THREE_MASK | SIX_MASK | SEVEN_MASK));
	} else if (direction == -1) {
		three_shift = 4 * 5;
		two_shift = 5;
		six_shift = 4 * 5;
		seven_shift = 5;

		// NOTE: This cycle may be different to the cycle in direction == 1 despite looking the same, some << or >> are switched directions
		cycled = ((corners & THREE_CPOS_MASK) | three_orientation) << three_shift | ((corners & TWO_CPOS_MASK) | two_orientation) << two_shift
		| ((corners & SIX_CPOS_MASK) | six_orientation) >> six_shift | ((corners & SEVEN_CPOS_MASK) | seven_orientation) >> seven_shift
		| (corners & ~(TWO_MASK | THREE_MASK | SIX_MASK | SEVEN_MASK));
	} else {
		std::cerr << "Invalid direction passed to F_Corner_Cycle: " << direction << "\n";
	}

	return cycled;
}

long B_CORNER_CYCLE(long corners, int direction) {
	// 00111'00110'00101'00100'00011'00010'00001'00000;
        // std::vector<long> B_CORNER_CYCLE {1, 0, 4, 5};
	// Same as F for orientation: 0 -> 0, 1 -> 2, 2 -> 1
	// 00 -> 10, 01 -> 01, 10 -> 00 (| + ~ = 1st bit, 2nd bit stays the same)
	
	// Cycle positions

	long bit1;
	long bit2;
	long bit1_pos;

	long one_orientation;
	long zero_orientation;
	long four_orientation;
	long five_orientation;
	if (direction != 2) {
		bit1 = corners & 1ULL << 1 * 5 + 4;
		bit2 = corners & 1ULL << 1 * 5 + 3;
		bit1_pos = 1ULL << 1 * 5 + 4;
		one_orientation = (~(bit1 | (bit2 << 1)) & bit1_pos) | bit2;

		bit1 = corners & 1ULL << 0 * 5 + 4;
		bit2 = corners & 1ULL << 0 * 5 + 3;
		bit1_pos = 1ULL << 0 * 5 + 4;
		zero_orientation = (~(bit1 | (bit2 << 1)) & bit1_pos) | bit2;

		bit1 = corners & 1ULL << 4 * 5 + 4;
		bit2 = corners & 1ULL << 4 * 5 + 3;
		bit1_pos = 1ULL << 4 * 5 + 4;
		four_orientation = (~(bit1 | (bit2 << 1)) & bit1_pos) | bit2;

		bit1 = corners & 1ULL << 5 * 5 + 4;
		bit2 = corners & 1ULL << 5 * 5 + 3;
		bit1_pos = 1ULL << 5 * 5 + 4;
		five_orientation = (~(bit1 | (bit2 << 1)) & bit1_pos) | bit2;
	 } else {
		// Orientation stays the same for any double move
		one_orientation = corners & ONE_CORI_MASK;
		zero_orientation = corners & ZERO_CORI_MASK;
		four_orientation = corners & FOUR_CORI_MASK;
		five_orientation = corners & FIVE_CORI_MASK;
	 }

        // std::vector<long> B_CORNER_CYCLE {1, 0, 4, 5};
	int one_shift;
	int zero_shift;
	int four_shift;
	int five_shift;
	long cycled = 2;
	if (direction == 1) {
		one_shift = 5;
		zero_shift = 4 * 5;
		four_shift = 5;
		five_shift = 4 * 5;

		cycled = ((corners & ONE_CPOS_MASK) | one_orientation) >> one_shift | ((corners & ZERO_CPOS_MASK) | zero_orientation) << zero_shift
		| ((corners & FOUR_CPOS_MASK) | four_orientation) << four_shift | ((corners & FIVE_CPOS_MASK) | five_orientation) >> five_shift
		| (corners & ~(ZERO_MASK | ONE_MASK | FOUR_MASK | FIVE_MASK));
	} else if (direction == 2) {
		one_shift = 3 * 5;
		zero_shift = 5 * 5;
		four_shift = 3 * 5;
		five_shift = 5 * 5;

		cycled = ((corners & ONE_CPOS_MASK) | one_orientation) << one_shift | ((corners & ZERO_CPOS_MASK) | zero_orientation) << zero_shift
		| ((corners & FOUR_CPOS_MASK) | four_orientation) >> four_shift | ((corners & FIVE_CPOS_MASK) | five_orientation) >> five_shift
		| (corners & ~(ZERO_MASK | ONE_MASK | FOUR_MASK | FIVE_MASK));
	} else if (direction == -1) {
		one_shift = 4 * 5;
		zero_shift = 5;
		four_shift = 4 * 5;
		five_shift = 5;

		// NOTE: This cycle may be different to the cycle in direction == 1 despite looking the same, some << or >> are switched directions
		cycled = ((corners & ONE_CPOS_MASK) | one_orientation) << one_shift | ((corners & ZERO_CPOS_MASK) | zero_orientation) << zero_shift
		| ((corners & FOUR_CPOS_MASK) | four_orientation) >> four_shift | ((corners & FIVE_CPOS_MASK) | five_orientation) >> five_shift
		| (corners & ~(ZERO_MASK | ONE_MASK | FOUR_MASK | FIVE_MASK));
	} else {
		std::cerr << "Invalid direction passed to B_Corner_Cycle: " << direction << "\n";
	}

	return cycled;
}

long R_EDGE_CYCLE(long edges, int direction) {
	// 00111'00110'00101'00100'00011'00010'00001'00000;
        // std::vector<int> R_EDGE_CYCLE {1, 5, 9, 6};
	
	// Cycle positions

        // std::vector<int> R_EDGE_CYCLE {1, 5, 9, 6};
	int one_shift;
	int five_shift;
	int nine_shift;
	int six_shift;
	long cycled = 2;
	if (direction == 1) {
		one_shift = 4 * 5;
		five_shift = 4 * 5;
		nine_shift = 3 * 5;
		six_shift = 5 * 5;

		cycled = (edges & ONE_MASK) << one_shift | (edges & FIVE_MASK) << five_shift
		| (edges & NINE_MASK) >> nine_shift | (edges & SIX_MASK) >> six_shift
		| (edges & ~(FIVE_MASK | ONE_MASK | NINE_MASK | SIX_MASK));
	} else if (direction == 2) {
		one_shift = 8 * 5;
		five_shift = 1 * 5;
		nine_shift = 8 * 5;
		six_shift = 1 * 5;

		cycled = (edges & ONE_MASK) << one_shift | (edges & FIVE_MASK) << five_shift
		| (edges & NINE_MASK) >> nine_shift | (edges & SIX_MASK) >> six_shift
		| (edges & ~(FIVE_MASK | ONE_MASK | NINE_MASK | SIX_MASK));
	} else if (direction == -1) {
		one_shift = 5 * 5;
		five_shift = 4 * 5;
		nine_shift = 4 * 5;
		six_shift = 3 * 5;

		// NOTE: This cycle may be different to the cycle in direction == 1 despite looking the same, some << or >> are switched directions
		cycled = (edges & ONE_MASK) << one_shift | (edges & FIVE_MASK) >> five_shift
		| (edges & NINE_MASK) >> nine_shift | (edges & SIX_MASK) << six_shift
		| (edges & ~(FIVE_MASK | ONE_MASK | NINE_MASK | SIX_MASK));
	} else {
		std::cerr << "Invalid direction passed to R_Edge_Cycle: " << direction << "\n";
	}

	return cycled;
}

long L_EDGE_CYCLE(long edges, int direction) {
	// 00111'00110'00101'00100'00011'00010'00001'00000;
	// std::vector<int> L_EDGE_CYCLE {3, 7, 11, 4};
	int three_shift;
	int seven_shift;
	int eleven_shift;
	int four_shift;
	long cycled = 2;
	if (direction == 1) {
		three_shift = 4 * 5;
		seven_shift = 4 * 5;
		eleven_shift = 7 * 5;
		four_shift = 1 * 5;

		cycled = (edges & THREE_MASK) << three_shift | (edges & SEVEN_MASK) << seven_shift
		| (edges & ELEVEN_MASK) >> eleven_shift | (edges & FOUR_MASK) >> four_shift
		| (edges & ~(SEVEN_MASK | THREE_MASK | ELEVEN_MASK | FOUR_MASK));
	} else if (direction == 2) {
		three_shift = 8 * 5;
		seven_shift = 3 * 5;
		eleven_shift = 8 * 5;
		four_shift = 3 * 5;

		cycled = (edges & THREE_MASK) << three_shift | (edges & SEVEN_MASK) >> seven_shift
		| (edges & ELEVEN_MASK) >> eleven_shift | (edges & FOUR_MASK) << four_shift
		| (edges & ~(SEVEN_MASK | THREE_MASK | ELEVEN_MASK | FOUR_MASK));
	} else if (direction == -1) {
		three_shift = 1 * 5;
		seven_shift = 4 * 5;
		eleven_shift = 4 * 5;
		four_shift = 7 * 5;

		// NOTE: This cycle may be different to the cycle in direction == 1 despite looking the same, some << or >> are switched directions
		cycled = (edges & THREE_MASK) << three_shift | (edges & SEVEN_MASK) >> seven_shift
		| (edges & ELEVEN_MASK) >> eleven_shift | (edges & FOUR_MASK) << four_shift
		| (edges & ~(SEVEN_MASK | THREE_MASK | ELEVEN_MASK | FOUR_MASK));
	} else {
		std::cerr << "Invalid direction passed to L_Edge_Cycle: " << direction << "\n";
	}

	return cycled;
}

long U_EDGE_CYCLE(long edges, int direction) {
	// 00111'00110'00101'00100'00011'00010'00001'00000;
	// std::vector<int> U_EDGE_CYCLE {0, 1, 2, 3};
	int zero_shift;
	int one_shift;
	int two_shift;
	int three_shift;
	long cycled = 2;
	if (direction == 1) {
		zero_shift = 1 * 5;
		one_shift = 1 * 5;
		two_shift = 1 * 5;
		three_shift = 3 * 5;

		cycled = (edges & ZERO_MASK) << zero_shift | (edges & ONE_MASK) << one_shift
		| (edges & TWO_MASK) << two_shift | (edges & THREE_MASK) >> three_shift
		| (edges & ~(ONE_MASK | ZERO_MASK | TWO_MASK | THREE_MASK));
	} else if (direction == 2) {
		zero_shift = 2 * 5;
		one_shift = 2 * 5;
		two_shift = 2 * 5;
		three_shift = 2 * 5;

		cycled = (edges & ZERO_MASK) << zero_shift | (edges & ONE_MASK) << one_shift
		| (edges & TWO_MASK) >> two_shift | (edges & THREE_MASK) >> three_shift
		| (edges & ~(ONE_MASK | ZERO_MASK | TWO_MASK | THREE_MASK));
	} else if (direction == -1) {
		zero_shift = 3 * 5;
		one_shift = 1 * 5;
		two_shift = 1 * 5;
		three_shift = 1 * 5;

		// NOTE: This cycle may be different to the cycle in direction == 1 despite looking the same, some << or >> are switched directions
		cycled = (edges & ZERO_MASK) << zero_shift | (edges & ONE_MASK) >> one_shift
		| (edges & TWO_MASK) >> two_shift | (edges & THREE_MASK) >> three_shift
		| (edges & ~(ONE_MASK | ZERO_MASK | TWO_MASK | THREE_MASK));
	} else {
		std::cerr << "Invalid direction passed to U_Edge_Cycle: " << direction << "\n";
	}

	return cycled;
}

long D_EDGE_CYCLE(long edges, int direction) {
	// 00111'00110'00101'00100'00011'00010'00001'00000;
	// std::vector<int> D_EDGE_CYCLE {10, 9, 8, 11};
	int ten_shift = 0;
	int nine_shift = 0;
	int eight_shift = 0;
	int eleven_shift = 0;
	long cycled = 2;
	if (direction == 1) {
		ten_shift = 1 * 5;
		nine_shift = 1 * 5;
		eight_shift = 3 * 5;
		eleven_shift = 1 * 5;

		cycled = (edges & TEN_MASK) >> ten_shift | (edges & NINE_MASK) >> nine_shift
		| (edges & EIGHT_MASK) << eight_shift | (edges & ELEVEN_MASK) >> eleven_shift
		| (edges & ~(NINE_MASK | TEN_MASK | EIGHT_MASK | ELEVEN_MASK));
	} else if (direction == 2) {
		ten_shift = 2 * 5;
		nine_shift = 2 * 5;
		eight_shift = 2 * 5;
		eleven_shift = 2 * 5;

		cycled = (edges & TEN_MASK) >> ten_shift | (edges & NINE_MASK) << nine_shift
		| (edges & EIGHT_MASK) << eight_shift | (edges & ELEVEN_MASK) >> eleven_shift
		| (edges & ~(NINE_MASK | TEN_MASK | EIGHT_MASK | ELEVEN_MASK));
	} else if (direction == -1) {
		ten_shift = 1 * 5;
		nine_shift = 1 * 5;
		eight_shift = 1 * 5;
		eleven_shift = 3 * 5;

		// NOTE: This cycle may be different to the cycle in direction == 1 despite looking the same, some << or >> are switched directions
		cycled = (edges & TEN_MASK) << ten_shift | (edges & NINE_MASK) << nine_shift
		| (edges & EIGHT_MASK) << eight_shift | (edges & ELEVEN_MASK) >> eleven_shift
		| (edges & ~(NINE_MASK | TEN_MASK | EIGHT_MASK | ELEVEN_MASK));
	} else {
		std::cerr << "Invalid direction passed to D_Edge_Cycle: " << direction << "\n";
	}

	return cycled;
}

long F_EDGE_CYCLE(long edges, int direction) {
	// 00111'00110'00101'00100'00011'00010'00001'00000;
	// std::vector<int> F_EDGE_CYCLE {2, 6, 10, 7}
	// Cycle positions

	long bit_pos;
	long two_orientation = 0;
	long six_orientation = 0;
	long ten_orientation = 0; 
	long seven_orientation = 0;
	if (direction != 2) {
		bit_pos = 1ULL << 2 * 5 + 4;
		two_orientation = ~(edges & bit_pos) & bit_pos;
		bit_pos = 1ULL << 6 * 5 + 4;
		six_orientation = ~(edges & bit_pos) & bit_pos;
		bit_pos = 1ULL << 10 * 5 + 4;
		ten_orientation = ~(edges & bit_pos) & bit_pos;
		bit_pos = 1ULL << 7 * 5 + 4;
		seven_orientation = ~(edges & bit_pos) & bit_pos;
	}

	int two_shift;
	int six_shift;
	int ten_shift;
	int seven_shift;
	long cycled = 2;
	if (direction == 1) {
		two_shift = 4 * 5;
		six_shift = 4 * 5;
		ten_shift = 3 * 5;
		seven_shift = 5 * 5;

		cycled = ((edges & TWO_EPOS_MASK | two_orientation) << two_shift) | (edges & SIX_EPOS_MASK | six_orientation) << six_shift
		| ((edges & TEN_EPOS_MASK) | ten_orientation) >> ten_shift | ((edges & SEVEN_EPOS_MASK) | seven_orientation) >> seven_shift
		| (edges & ~(SIX_MASK | TWO_MASK | TEN_MASK | SEVEN_MASK));
	} else if (direction == 2) {
		two_shift = 8 * 5;
		six_shift = 1 * 5;
		ten_shift = 8 * 5;
		seven_shift = 1 * 5;

		cycled = (edges & TWO_EPOS_MASK | two_orientation) << two_shift | (edges & SIX_EPOS_MASK | six_orientation) << six_shift
		| (edges & TEN_EPOS_MASK | ten_orientation) >> ten_shift | (edges & SEVEN_EPOS_MASK | seven_orientation) >> seven_shift
		| (edges & ~(SIX_MASK | TWO_MASK | TEN_MASK | SEVEN_MASK));
	} else if (direction == -1) {
		two_shift = 5 * 5;
		six_shift = 4 * 5;
		ten_shift = 4 * 5;
		seven_shift = 3 * 5;

		// NOTE: This cycle may be different to the cycle in direction == 1 despite looking the same, some << or >> are switched directions
		cycled = (edges & TWO_EPOS_MASK | two_orientation) << two_shift | (edges & SIX_EPOS_MASK | six_orientation) >> six_shift
		| (edges & TEN_EPOS_MASK | ten_orientation) >> ten_shift | (edges & SEVEN_EPOS_MASK | seven_orientation) << seven_shift
		| (edges & ~(SIX_MASK | TWO_MASK | TEN_MASK | SEVEN_MASK));
	} else {
		std::cerr << "Invalid direction passed to F_Edge_Cycle: " << direction << "\n";
	}

	return cycled;
}

long B_EDGE_CYCLE(long edges, int direction) {
	// 00111'00110'00101'00100'00011'00010'00001'00000;
	// std::vector<int> B_EDGE_CYCLE {0, 4, 8, 5};
	// Cycle positions

	long bit_pos;
	long zero_orientation = 0;
	long four_orientation = 0;
	long eight_orientation = 0;
	long five_orientation = 0;
	if (direction != 2) {
		bit_pos = 1ULL << 0 * 5 + 4;
		zero_orientation = (~(edges & bit_pos) & bit_pos);
		bit_pos = 1ULL << 4 * 5 + 4;
		four_orientation = (~(edges & bit_pos) & bit_pos);
		bit_pos = 1ULL << 8 * 5 + 4;
		eight_orientation = (~(edges & bit_pos) & bit_pos);
		bit_pos = 1ULL << 5 * 5 + 4;
		five_orientation = (~(edges & bit_pos) & bit_pos);
	}

	int zero_shift;
	int four_shift;
	int eight_shift;
	int five_shift;
	long cycled = 2;
	if (direction == 1) {
		zero_shift = 4 * 5;
		four_shift = 4 * 5;
		eight_shift = 3 * 5;
		five_shift = 5 * 5;

		cycled = (edges & ZERO_EPOS_MASK | zero_orientation) << zero_shift | (edges & FOUR_EPOS_MASK | four_orientation) << four_shift
		| (edges & EIGHT_EPOS_MASK | eight_orientation) >> eight_shift | (edges & FIVE_EPOS_MASK | five_orientation) >> five_shift
		| (edges & ~(FOUR_MASK | ZERO_MASK | EIGHT_MASK | FIVE_MASK));
	} else if (direction == 2) {
		zero_shift = 8 * 5;
		four_shift = 1 * 5;
		eight_shift = 8 * 5;
		five_shift = 1 * 5;

		cycled = (edges & ZERO_MASK) << zero_shift | (edges & FOUR_MASK) << four_shift
		| (edges & EIGHT_MASK) >> eight_shift | (edges & FIVE_MASK) >> five_shift
		| (edges & ~(FOUR_MASK | ZERO_MASK | EIGHT_MASK | FIVE_MASK));
	} else if (direction == -1) {
		zero_shift = 5 * 5;
		four_shift = 4 * 5;
		eight_shift = 4 * 5;
		five_shift = 3 * 5;

		// NOTE: This cycle may be different to the cycle in direction == 1 despite looking the same, some << or >> are switched directions
		cycled = (edges & ZERO_EPOS_MASK | zero_orientation) << zero_shift | (edges & FOUR_EPOS_MASK | four_orientation) >> four_shift
		| (edges & EIGHT_EPOS_MASK | eight_orientation) >> eight_shift | (edges & FIVE_EPOS_MASK | five_orientation) << five_shift
		| (edges & ~(FOUR_MASK | ZERO_MASK | EIGHT_MASK | FIVE_MASK));
	} else {
		std::cerr << "Invalid direction passed to B_Edge_Cycle: " << direction << "\n";
	}

	return cycled;
}

#endif
