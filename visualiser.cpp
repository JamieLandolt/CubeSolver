#include "solver_core.h"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "rcamera.h"

#include <cstring>
#include <cstdio>
#include <sstream>

// ---------------------------------------------------------------------------
// Cubie model: 27 small cubes at positions (x,y,z) in {-1,0,1}^3. Each cubie
// tracks, for each of the 6 world-axis directions, which sticker colour (if
// any) currently faces that direction. A face is visible/exterior for a
// cubie exactly when its coordinate along that axis equals the extreme value
// for that direction (e.g. +X is visible when x == 1) - this is purely a
// function of current position, and is consistent with the colour data by
// construction (see moveCubies() below).
// ---------------------------------------------------------------------------

enum Dir { PX = 0, NX = 1, PY = 2, NY = 3, PZ = 4, NZ = 5 };
const int DIR_VEC[6][3] = {
	{1, 0, 0}, {-1, 0, 0},
	{0, 1, 0}, {0, -1, 0},
	{0, 0, 1}, {0, 0, -1},
};

int dirIndexOf(int vx, int vy, int vz) {
	for (int i = 0; i < 6; i++) {
		if (DIR_VEC[i][0] == vx && DIR_VEC[i][1] == vy && DIR_VEC[i][2] == vz) return i;
	}
	return -1;
}

// Rotate a vector by a signed quarter turn (sign = -1, +1, or 2 for a half turn)
// about a principal axis (0=X, 1=Y, 2=Z), using the right-hand rule: a positive
// rotation about +axis, viewed from the positive axis looking toward the
// origin, appears counter-clockwise.
void rotateVec(int axis, int sign, int x, int y, int z, int& ox, int& oy, int& oz) {
	if (axis == 0) { // about X
		if (sign == 1) { ox = x; oy = -z; oz = y; }
		else if (sign == -1) { ox = x; oy = z; oz = -y; }
		else { ox = x; oy = -y; oz = -z; } // 180
	} else if (axis == 1) { // about Y
		if (sign == 1) { ox = z; oy = y; oz = -x; }
		else if (sign == -1) { ox = -z; oy = y; oz = x; }
		else { ox = -x; oy = y; oz = -z; }
	} else { // about Z
		if (sign == 1) { ox = -y; oy = x; oz = z; }
		else if (sign == -1) { ox = y; oy = -x; oz = z; }
		else { ox = -x; oy = -y; oz = z; }
	}
}

struct Cubie {
	int x, y, z;
	int faceColor[6]; // -1 = never a real sticker on this cubie/direction
};

// Solved-state sticker colours per world direction.
Color FACE_RENDER_COLOR[6]; // indexed by Dir

struct MoveSpec {
	int axis;      // 0=X, 1=Y, 2=Z
	int sign;      // -1, +1, or 2 (half turn)
	int sliceAxis; // which coordinate selects the slice
	int sliceVal;  // required value of that coordinate (+1 or -1)
};

// Matches solver_core.h's move notation exactly: face letter + optional ' or 2.
bool parseMove(const std::string& mv, MoveSpec& spec) {
	if (mv.empty()) return false;
	char face = mv[0];
	bool prime = mv.size() > 1 && mv[1] == '\'';
	bool half = mv.size() > 1 && mv[1] == '2';

	int axis, baseSign, sliceVal;
	switch (face) {
		case 'R': axis = 0; baseSign = -1; sliceVal = 1; break;
		case 'L': axis = 0; baseSign = 1;  sliceVal = -1; break;
		case 'U': axis = 1; baseSign = -1; sliceVal = 1; break;
		case 'D': axis = 1; baseSign = 1;  sliceVal = -1; break;
		case 'F': axis = 2; baseSign = -1; sliceVal = 1; break;
		case 'B': axis = 2; baseSign = 1;  sliceVal = -1; break;
		default: return false;
	}
	spec.axis = axis;
	spec.sliceAxis = axis;
	spec.sliceVal = sliceVal;
	if (half) spec.sign = 2;
	else if (prime) spec.sign = -baseSign;
	else spec.sign = baseSign;
	return true;
}

struct CubieModel {
	std::vector<Cubie> cubies;

	void reset() {
		cubies.clear();
		for (int x = -1; x <= 1; x++) {
			for (int y = -1; y <= 1; y++) {
				for (int z = -1; z <= 1; z++) {
					Cubie c;
					c.x = x; c.y = y; c.z = z;
					for (int d = 0; d < 6; d++) c.faceColor[d] = -1;
					if (x == 1) c.faceColor[PX] = PX;
					if (x == -1) c.faceColor[NX] = NX;
					if (y == 1) c.faceColor[PY] = PY;
					if (y == -1) c.faceColor[NY] = NY;
					if (z == 1) c.faceColor[PZ] = PZ;
					if (z == -1) c.faceColor[NZ] = NZ;
					cubies.push_back(c);
				}
			}
		}
	}

	void applyMove(const std::string& mv) {
		MoveSpec spec;
		if (!parseMove(mv, spec)) return;
		for (Cubie& c : cubies) {
			int coord = spec.sliceAxis == 0 ? c.x : (spec.sliceAxis == 1 ? c.y : c.z);
			if (coord != spec.sliceVal) continue;

			int nx, ny, nz;
			rotateVec(spec.axis, spec.sign, c.x, c.y, c.z, nx, ny, nz);
			c.x = nx; c.y = ny; c.z = nz;

			int newFaceColor[6];
			for (int d = 0; d < 6; d++) newFaceColor[d] = -1;
			for (int d = 0; d < 6; d++) {
				if (c.faceColor[d] == -1) continue;
				int rx, ry, rz;
				rotateVec(spec.axis, spec.sign, DIR_VEC[d][0], DIR_VEC[d][1], DIR_VEC[d][2], rx, ry, rz);
				int nd = dirIndexOf(rx, ry, rz);
				newFaceColor[nd] = c.faceColor[d];
			}
			for (int d = 0; d < 6; d++) c.faceColor[d] = newFaceColor[d];
		}
	}

	bool isSolved() const {
		for (const Cubie& c : cubies) {
			for (int d = 0; d < 6; d++) {
				if (c.faceColor[d] != -1 && c.faceColor[d] != d) return false;
			}
		}
		return true;
	}
};

// ---------------------------------------------------------------------------
// Self-test: verify the move table is internally consistent before trusting
// it for rendering. Not exhaustive proof of correctness, but catches sign/
// axis/index errors reliably.
// ---------------------------------------------------------------------------
bool runSelfTest() {
	const char* faces[] = {"R", "L", "U", "D", "F", "B"};
	for (const char* f : faces) {
		CubieModel m;
		m.reset();
		std::string mv(f);
		for (int i = 0; i < 4; i++) m.applyMove(mv);
		if (!m.isSolved()) {
			printf("SELF-TEST FAILED: %s x4 did not return to solved\n", f);
			return false;
		}
	}

	{
		CubieModel m;
		m.reset();
		std::vector<std::string> seq = {"R", "U", "R'", "U'"};
		for (int i = 0; i < 6; i++) {
			for (const std::string& mv : seq) m.applyMove(mv);
		}
		if (!m.isSolved()) {
			printf("SELF-TEST FAILED: (R U R' U') x6 did not return to solved\n");
			return false;
		}
	}

	{
		CubieModel m;
		m.reset();
		std::vector<std::string> scramble = {"R", "U2", "F'", "L", "D", "B2", "R'", "U", "F", "L2"};
		std::unordered_map<std::string, std::string> inverse = {
			{"R", "R'"}, {"R'", "R"}, {"R2", "R2"},
			{"L", "L'"}, {"L'", "L"}, {"L2", "L2"},
			{"U", "U'"}, {"U'", "U"}, {"U2", "U2"},
			{"D", "D'"}, {"D'", "D"}, {"D2", "D2"},
			{"F", "F'"}, {"F'", "F"}, {"F2", "F2"},
			{"B", "B'"}, {"B'", "B"}, {"B2", "B2"},
		};
		for (const std::string& mv : scramble) m.applyMove(mv);
		for (int i = (int)scramble.size() - 1; i >= 0; i--) m.applyMove(inverse[scramble[i]]);
		if (!m.isSolved()) {
			printf("SELF-TEST FAILED: scramble followed by its exact inverse did not return to solved\n");
			return false;
		}
	}

	printf("Self-test passed: move table is internally consistent.\n");
	return true;
}

// ---------------------------------------------------------------------------
// Simple immediate-mode UI helpers (no external GUI library dependency).
// ---------------------------------------------------------------------------
bool Button(Rectangle r, const char* label, bool enabled = true) {
	Vector2 mouse = GetMousePosition();
	bool hover = enabled && CheckCollisionPointRec(mouse, r);
	Color bg = !enabled ? Color{60, 60, 60, 255} : (hover ? Color{90, 130, 200, 255} : Color{70, 100, 160, 255});
	DrawRectangleRec(r, bg);
	DrawRectangleLinesEx(r, 1, Color{20, 20, 20, 255});
	int textW = MeasureText(label, 18);
	DrawText(label, (int)(r.x + (r.width - textW) / 2), (int)(r.y + (r.height - 18) / 2), 18, WHITE);
	return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void Slider(Rectangle r, float& value, float minV, float maxV, const char* label) {
	DrawText(label, (int)r.x, (int)r.y - 20, 16, LIGHTGRAY);
	DrawRectangleRec(r, Color{50, 50, 50, 255});
	float t = (value - minV) / (maxV - minV);
	float knobX = r.x + t * r.width;
	Rectangle knob = {knobX - 6, r.y - 4, 12, r.height + 8};
	DrawRectangleRec(knob, Color{200, 200, 60, 255});

	Vector2 mouse = GetMousePosition();
	if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, {r.x - 8, r.y - 8, r.width + 16, r.height + 16})) {
		float nt = (mouse.x - r.x) / r.width;
		nt = Clamp(nt, 0.0f, 1.0f);
		value = minV + nt * (maxV - minV);
	}
}

// ---------------------------------------------------------------------------
// Move animation queue.
// ---------------------------------------------------------------------------
struct AnimatedMove {
	std::string move;
};

int main(int argc, char** argv) {
	if (!runSelfTest()) {
		printf("Refusing to start: cube move table failed self-test.\n");
		return 1;
	}

	FACE_RENDER_COLOR[PX] = RED;      // Right
	FACE_RENDER_COLOR[NX] = ORANGE;   // Left
	FACE_RENDER_COLOR[PY] = WHITE;    // Up
	FACE_RENDER_COLOR[NY] = YELLOW;   // Down
	FACE_RENDER_COLOR[PZ] = GREEN;    // Front
	FACE_RENDER_COLOR[NZ] = BLUE;     // Back

	const int screenW = 1280;
	const int screenH = 800;
	InitWindow(screenW, screenH, "Rubik's Cube Visualiser");
	SetTargetFPS(60);

	CubieModel model;
	model.reset();

	Cube realCube;
	std::pair<long, long> realState = realCube.get_solved_state();

	// Solver holds an expensive precomputed lookup table built once at startup
	// (generate_solution_lookup/generate_orientations) - construct it once and
	// reuse it for every solve, rather than rebuilding it per solve attempt.
	// This can take a noticeable amount of time, so render one frame telling
	// the user before blocking on it.
	BeginDrawing();
	ClearBackground(Color{25, 25, 30, 255});
	DrawText("Building solver lookup tables, please wait...", 40, screenH / 2 - 10, 22, WHITE);
	EndDrawing();
	printf("Building solver lookup tables (this may take a while)...\n");
	fflush(stdout);
	Solver solver(realCube);
	printf("Ready.\n");
	fflush(stdout);

	std::vector<std::string> lastScramble;
	std::pair<std::list<std::string>, std::list<std::string>> lastSolution;
	bool haveSolution = false;
	bool solving = false;

	std::vector<AnimatedMove> animQueue;
	std::string currentAnimMove;
	float animProgress = 0.0f; // 0..1
	bool animating = false;
	float moveDurationSec = 0.4f;
	std::vector<std::string>* animSourceList = nullptr; // which list currentAnimMove came from, for highlighting
	int animSourceIndex = -1;

	// For highlighting: flattened views of the currently displayed scramble/solution
	std::vector<std::string> displayedScrambleFlat;
	std::vector<std::string> displayedSolutionFlat;
	int highlightScrambleIdx = -1;
	int highlightSolutionIdx = -1;

	char scrambleInput[256] = "";
	int scrambleInputLen = 0;
	bool inputFocused = false;

	Camera3D camera = {0};
	camera.position = Vector3{6.0f, 6.0f, 8.0f};
	camera.target = Vector3{0.0f, 0.0f, 0.0f};
	camera.up = Vector3{0.0f, 1.0f, 0.0f};
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	const float spacing = 1.05f;
	const float cubieSize = 0.95f;
	const float stickerInset = 0.06f;

	while (!WindowShouldClose()) {
		// --- Update animation ---
		if (!animating && !animQueue.empty()) {
			currentAnimMove = animQueue.front().move;
			animQueue.erase(animQueue.begin());
			animProgress = 0.0f;
			animating = true;

			if (animSourceList == &displayedScrambleFlat) {
				highlightScrambleIdx = ++animSourceIndex;
			} else if (animSourceList == &displayedSolutionFlat) {
				highlightSolutionIdx = ++animSourceIndex;
			}
		}

		if (animating) {
			animProgress += GetFrameTime() / moveDurationSec;
			if (animProgress >= 1.0f) {
				animProgress = 1.0f;
				model.applyMove(currentAnimMove);
				realState = realCube.move(currentAnimMove, realState.first, realState.second);
				animating = false;

				if (animQueue.empty()) {
					animSourceList = nullptr;
					animSourceIndex = -1;
					highlightScrambleIdx = -1;
					highlightSolutionIdx = -1;
				}
			}
		}

		// --- Camera orbit with right mouse drag ---
		if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
			Vector2 delta = GetMouseDelta();
			CameraYaw(&camera, -delta.x * 0.01f, true);
			CameraPitch(&camera, -delta.y * 0.01f, true, true, false);
		}

		// --- Text input for manual scramble entry ---
		Rectangle inputBox = {20, 20, 400, 32};
		Vector2 mouse = GetMousePosition();
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			inputFocused = CheckCollisionPointRec(mouse, inputBox);
		}
		if (inputFocused) {
			int key = GetCharPressed();
			while (key > 0) {
				if (scrambleInputLen < 255 && ((key >= 'A' && key <= 'Z') || key == ' ' || key == '\'' || (key >= '0' && key <= '9'))) {
					scrambleInput[scrambleInputLen++] = (char)key;
					scrambleInput[scrambleInputLen] = '\0';
				}
				key = GetCharPressed();
			}
			if (IsKeyPressed(KEY_BACKSPACE) && scrambleInputLen > 0) {
				scrambleInput[--scrambleInputLen] = '\0';
			}
		}

		// --- Buttons ---
		bool clickedRandom = Button({440, 20, 160, 32}, "Random Scramble", !animating);
		bool clickedExecScramble = Button({610, 20, 160, 32}, "Execute Scramble", !animating && !lastScramble.empty());
		bool clickedSolve = Button({780, 20, 100, 32}, "Solve", !animating && !lastScramble.empty() && !solving);
		bool clickedExecSolution = Button({890, 20, 160, 32}, "Execute Solution", !animating && haveSolution);
		bool clickedApplyInput = Button({1060, 20, 100, 32}, "Apply", !animating && scrambleInputLen > 0);

		if (clickedRandom) {
			std::pair<std::vector<std::string>, std::pair<long, long>> p = realCube.random_scramble(12, 6);
			lastScramble = p.first;
			haveSolution = false;
			displayedScrambleFlat = lastScramble;
			scrambleInputLen = 0;
			scrambleInput[0] = '\0';
			for (const std::string& mv : lastScramble) {
				if (scrambleInputLen + mv.size() + 1 < 255) {
					strcpy(scrambleInput + scrambleInputLen, mv.c_str());
					scrambleInputLen += mv.size();
					scrambleInput[scrambleInputLen++] = ' ';
					scrambleInput[scrambleInputLen] = '\0';
				}
			}
		}

		if (clickedApplyInput) {
			std::vector<std::string> parsed;
			std::istringstream iss(scrambleInput);
			std::string tok;
			bool allValid = true;
			while (iss >> tok) {
				MoveSpec spec;
				if (!parseMove(tok, spec)) { allValid = false; break; }
				parsed.push_back(tok);
			}
			if (allValid && !parsed.empty()) {
				lastScramble = parsed;
				displayedScrambleFlat = lastScramble;
				haveSolution = false;
			}
		}

		if (clickedExecScramble) {
			model.reset();
			realState = realCube.get_solved_state();
			animQueue.clear();
			for (const std::string& mv : lastScramble) animQueue.push_back({mv});
			animSourceList = &displayedScrambleFlat;
			animSourceIndex = -1;
		}

		if (clickedSolve && !lastScramble.empty()) {
			solving = true;
			solver.reset_full();
			solver.dfs(lastScramble);
			lastSolution = solver.get_solution();
			haveSolution = lastSolution.second.size() > 0;
			displayedSolutionFlat.clear();
			for (const std::string& mv : lastSolution.first) displayedSolutionFlat.push_back(mv);
			for (const std::string& mv : lastSolution.second) displayedSolutionFlat.push_back(mv);
			solving = false;
		}

		if (clickedExecSolution) {
			animQueue.clear();
			for (const std::string& mv : displayedSolutionFlat) animQueue.push_back({mv});
			animSourceList = &displayedSolutionFlat;
			animSourceIndex = -1;
		}

		// --- Draw ---
		BeginDrawing();
		ClearBackground(Color{25, 25, 30, 255});

		BeginMode3D(camera);

		for (const Cubie& c : model.cubies) {
			Vector3 pos = {c.x * spacing, c.y * spacing, c.z * spacing};

			bool isAnimating = animating;
			bool partOfMove = false;
			if (isAnimating) {
				MoveSpec spec;
				if (parseMove(currentAnimMove, spec)) {
					int cc = spec.sliceAxis == 0 ? c.x : (spec.sliceAxis == 1 ? c.y : c.z);
					partOfMove = (cc == spec.sliceVal);
				}
			}

			rlPushMatrix();
			rlTranslatef(pos.x, pos.y, pos.z);

			if (partOfMove) {
				MoveSpec spec;
				parseMove(currentAnimMove, spec);
				float targetDeg = (spec.sign == 2) ? 180.0f : 90.0f * spec.sign;
				float angle = targetDeg * animProgress;
				if (spec.axis == 0) rlRotatef(angle, 1, 0, 0);
				else if (spec.axis == 1) rlRotatef(angle, 0, 1, 0);
				else rlRotatef(angle, 0, 0, 1);
			}

			DrawCube(Vector3{0, 0, 0}, cubieSize, cubieSize, cubieSize, Color{15, 15, 15, 255});

			for (int d = 0; d < 6; d++) {
				if (c.faceColor[d] == -1) continue;
				Color col = FACE_RENDER_COLOR[c.faceColor[d]];
				float half = cubieSize / 2.0f + 0.01f;
				float s = cubieSize - stickerInset * 2;
				Vector3 n = {(float)DIR_VEC[d][0], (float)DIR_VEC[d][1], (float)DIR_VEC[d][2]};
				Vector3 center = Vector3Scale(n, half);

				Vector3 u, v;
				if (fabsf(n.x) > 0.5f) { u = {0, 1, 0}; v = {0, 0, 1}; }
				else if (fabsf(n.y) > 0.5f) { u = {1, 0, 0}; v = {0, 0, 1}; }
				else { u = {1, 0, 0}; v = {0, 1, 0}; }
				u = Vector3Scale(u, s / 2.0f);
				v = Vector3Scale(v, s / 2.0f);

				Vector3 p1 = Vector3Add(center, Vector3Add(Vector3Negate(u), Vector3Negate(v)));
				Vector3 p2 = Vector3Add(center, Vector3Add(u, Vector3Negate(v)));
				Vector3 p3 = Vector3Add(center, Vector3Add(u, v));
				Vector3 p4 = Vector3Add(center, Vector3Add(Vector3Negate(u), v));

				rlBegin(RL_QUADS);
				rlColor4ub(col.r, col.g, col.b, col.a);
				rlNormal3f(n.x, n.y, n.z);
				rlVertex3f(p1.x, p1.y, p1.z);
				rlVertex3f(p2.x, p2.y, p2.z);
				rlVertex3f(p3.x, p3.y, p3.z);
				rlVertex3f(p4.x, p4.y, p4.z);
				rlEnd();
			}

			rlPopMatrix();
		}

		EndMode3D();

		// --- UI overlay ---
		DrawRectangleRec(inputBox, Color{40, 40, 40, 255});
		DrawRectangleLinesEx(inputBox, 1, inputFocused ? Color{200, 200, 60, 255} : GRAY);
		DrawText(scrambleInput, (int)inputBox.x + 6, (int)inputBox.y + 7, 18, WHITE);

		DrawText("Scramble:", 20, 65, 16, LIGHTGRAY);
		{
			int x = 20, y = 85;
			for (size_t i = 0; i < displayedScrambleFlat.size(); i++) {
				bool hl = (int)i == highlightScrambleIdx;
				Color c = hl ? YELLOW : LIGHTGRAY;
				const char* txt = displayedScrambleFlat[i].c_str();
				DrawText(txt, x, y, 18, c);
				x += MeasureText(txt, 18) + 12;
				if (x > screenW - 100) { x = 20; y += 24; }
			}
		}

		DrawText(haveSolution ? "Solution:" : (solving ? "Solving..." : "Solution: (press Solve)"), 20, 130, 16, LIGHTGRAY);
		{
			int x = 20, y = 150;
			for (size_t i = 0; i < displayedSolutionFlat.size(); i++) {
				bool hl = (int)i == highlightSolutionIdx;
				Color c = hl ? YELLOW : SKYBLUE;
				const char* txt = displayedSolutionFlat[i].c_str();
				DrawText(txt, x, y, 18, c);
				x += MeasureText(txt, 18) + 12;
				if (x > screenW - 100) { x = 20; y += 24; }
			}
		}

		Slider({20, screenH - 40, 300, 8}, moveDurationSec, 0.05f, 1.5f, "Playback speed (seconds per move)");
		char speedLabel[64];
		snprintf(speedLabel, sizeof(speedLabel), "%.2fs/move", moveDurationSec);
		DrawText(speedLabel, 340, screenH - 60, 16, LIGHTGRAY);

		DrawText("Right-drag to orbit camera. Type a scramble (e.g. R U R' U') and press Apply, or use Random Scramble.",
			20, screenH - 90, 14, GRAY);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}
