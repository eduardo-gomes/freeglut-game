#include <chrono>

#include "scenes.hpp"
#include "random.hpp"

namespace scene {
constexpr unsigned int MAP_HEIGHT = 15, MAP_WIDTH = 8;
class Tetris::TetrisMap{
	Tetris* game;
	bool ocuped[MAP_HEIGHT][MAP_WIDTH];
	vec4f color[MAP_HEIGHT][MAP_WIDTH];
	public:
	inline bool isOcuped(unsigned int x, unsigned int y){return ocuped[y][x];}
	inline void Ocupe(unsigned int x, unsigned int y, const vec4f &cor){
		ocuped[y][x] = 1;
		color[y][x] = cor;
	}
	void render(){
		for (unsigned int x = 0; x < MAP_WIDTH; ++x)
			for (unsigned int y = 0; y < MAP_HEIGHT; ++y)
				if (ocuped[y][x]) renderer->DrawnQuad({(float)x, (float)y, 0.0f}, color[y][x], {0.975f, 0.975f});
	}
	TetrisMap(Tetris* g): game(g){
		for(unsigned int y = 0; y < MAP_HEIGHT; ++y)
			memset(ocuped[y], 0, sizeof(ocuped[0]));
	}
	void update(){
		for (unsigned int y = 0; y < MAP_HEIGHT; ++y){
			bool isFull = 1;
			for(unsigned int x = 0; x < MAP_WIDTH; ++x)
				isFull = isFull && ocuped[y][x];
			if(isFull){
				for (unsigned int y2 = y+1; y2 < MAP_HEIGHT; ++y2) {
					memcpy(&ocuped[y2 - 1], &ocuped[y2], sizeof(ocuped[y2]));
					memcpy(&color[y2 - 1], &color[y2], sizeof(color[y2]));
				}
				game->points += MAP_WIDTH * 10;
				if(y > 0) y--;
			}
		}
	}
};


void Tetris::update(double) {
}

class Tetris::peca {
	Tetris* game;
   protected:
	const vec4f cor;
	const float (&positions)[4][4][2];
	int rotation = 0;
	int xpos, ypos;
	peca(const float (&arr)[4][4][2], Tetris* t) : game(t), cor({Random::getf(), Random::getf(), Random::getf(), 1.0f}), positions(arr), xpos(3), ypos(13){}

   public:
	virtual ~peca(){}
	inline bool rotate(bool clockwise) {
		int oldRotation = rotation;
		rotation = (rotation + (clockwise ? 1 : 3)) % 4;
		if (!isInsideMap(xpos, ypos) || colide(xpos, ypos)) {
			rotation = oldRotation;
			return 0;
		}
		return 1;
	}
	inline bool colide(int x, int y){
		if(isInsideMap(x, y)){
			for(int i = 0; i < 4; ++i){
				unsigned int quady = positions[rotation % 4][i][1] + y, quadx = positions[rotation % 4][i][0] + x;
				if(game->Map->isOcuped(quadx, quady))
					return true;
			}
		}else return true;
		return false;
	}
	inline bool isInsideMap(int x, int y){return isInsideMapH(x) && isInsideMapV(y);}
	inline bool isInsideMapV(int y) {
		if ((positions[rotation % 4][0][1] + y) >= MAP_HEIGHT || (positions[rotation % 4][0][1] + y) < 0 ||
			(positions[rotation % 4][1][1] + y) >= MAP_HEIGHT || (positions[rotation % 4][1][1] + y) < 0 ||
			(positions[rotation % 4][2][1] + y) >= MAP_HEIGHT || (positions[rotation % 4][2][1] + y) < 0 ||
			(positions[rotation % 4][3][1] + y) >= MAP_HEIGHT || (positions[rotation % 4][3][1] + y) < 0)
			return false;
		return true;
	}
	inline bool isInsideMapH(int x) {
		if ((positions[rotation % 4][0][0] + x) >= MAP_WIDTH || (positions[rotation % 4][0][0] + x) < 0 ||
			(positions[rotation % 4][1][0] + x) >= MAP_WIDTH || (positions[rotation % 4][1][0] + x) < 0 ||
			(positions[rotation % 4][2][0] + x) >= MAP_WIDTH || (positions[rotation % 4][2][0] + x) < 0 ||
			(positions[rotation % 4][3][0] + x) >= MAP_WIDTH || (positions[rotation % 4][3][0] + x) < 0)
			return false;
		return true;
	}
	int move(int x, int y) {
		if (!isInsideMapH(xpos + x)) return -1;
		if (colide(xpos + x, ypos + y)) return 0;
		xpos += x;
		ypos += y;
		return 1;
	}
	inline bool cantMove(int x, int y) {
		return colide(xpos + x, ypos + y);
	}
	inline void die(){
		for (int i = 0; i < 4; ++i) {
			game->Map->Ocupe(positions[rotation % 4][i][0] + xpos, positions[rotation % 4][i][1] + ypos, cor);
		}
	}
	void render() {
		for (int i = 0; i < 4; ++i)
			renderer->DrawnQuad({positions[rotation % 4][i][0] + xpos, positions[rotation % 4][i][1] + ypos, 0.0f}, cor, {0.975f, 0.975f});
	}
};

class Tetris::bagulhin : public Tetris::peca {
   public:
	const float positions[4][4][2] = {
		//[rotation] [piece] [cord]
		{{0.0f, 0.0f}, {1.0f, 0.0f}, {2.0f, 0.0f}, {1.0f, 1.0f}},	 //up
		{{0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 2.0f}, {1.0f, 1.0f}},	 //right
		{{0.0f, 1.0f}, {1.0f, 1.0f}, {2.0f, 1.0f}, {1.0f, 0.0f}},	 //down
		{{1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 2.0f}, {0.0f, 1.0f}},	 //left
	};
	bagulhin(Tetris*t) : peca(positions, t) {
	}
};

void Tetris::Render() {
	for (unsigned int h = 0; h < MAP_HEIGHT; ++h)
		for (unsigned int w = 0; w < MAP_WIDTH; ++w) {
			renderer->DrawnQuad({(float)w, (float)h, 0.0f}, {0.0f, 0.35f, 1.0f, 0.10f}, {0.95f, 0.95f});
		}
	static std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();
	if (tp <= std::chrono::steady_clock::now()) {
		tp += std::chrono::seconds(1);
		atual->move(0, -1);
		if (keyboard::w) atual->rotate(1);
	}
	static std::chrono::steady_clock::time_point tp2 = std::chrono::steady_clock::now();
	if (tp2 <= std::chrono::steady_clock::now()) {
		tp2 += std::chrono::milliseconds(30);
		if (keyboard::d) atual->move( 1, 0);
		if (keyboard::a) atual->move(-1, 0);
	}
	if(atual->cantMove(0, -1)){
		//to do when die: copy blocks and color to a matrix
		atual->die();
		delete atual;
		atual = new bagulhin(this);
	}
	Map->update();
	Map->render();
	atual->render();
	renderer->Drawn();
}
void Tetris::RenderGUI() {
	ImGui::Begin("Tetris");
	ImGui::Text("Points: %u", this->points);
	static bool Info = 1;
	ImGui::Checkbox("Info", &Info);
	if (ImGui::Button("Next"))
		new renderertestrotate;
	ImGui::End();
	if (Info) renderer->DispInfo();
}

Tetris::Tetris() {
	Map = new TetrisMap(this);
	Renderer::LookAt(4.0f, 7.5f, 20.0f, 4.0f, 7.5f, 0.0f, 0.0f, 1.0f, 0.0f);
	atual = new bagulhin(this);
}
Tetris::~Tetris() {
	delete Map;
	delete atual;
}

}  // namespace scene