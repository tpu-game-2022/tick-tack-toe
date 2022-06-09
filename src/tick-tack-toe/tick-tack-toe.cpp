#include <memory>
#include <iostream>

class Mass {
public:
	enum status {
		BLANK,
		PLAYER,
		ENEMY,
	};
private:
	status s_ = BLANK;
public:
	void setStatus(status s) { s_ = s; }
	status getStatus() const { return s_; }

	bool put(status s) {
		if (s_ != BLANK) return false;
		s_ = s;
		return true;
	}
};

class Board;

class AI {
public:
	AI() {}
	virtual ~AI() {}

	virtual bool think(Board& b) = 0;

public:
	enum type {
		TYPE_ORDERED = 0,
		TYPE_STANDARD = 1,
		TYPE_MONTECARLO_TREE = 2,
	};

	static AI* createAi(type type);
};

// 順番に打ってみる
class AI_ordered : public AI {
public:
	AI_ordered() {}
	~AI_ordered() {}

	bool think(Board& b);
};

//三目並べの後攻定石+優先度でマスを管理
class AI_standard : public AI {
private:
	int massScore[3][3];

public:
	AI_standard() {}
	~AI_standard() {}

	void calcScore(int* turnCount, Board& b);
	bool think(Board& b);
};

//モンテカルロ木
class AI_montecarlo_tree : public AI {
private:
	static int select_mass(int n, int* a_count, int* a_wins);
	int evaluate(bool all_search, int count, Board& b, Mass::status current, int& best_x, int& best_y);
public :
	AI_montecarlo_tree(){}
	~AI_montecarlo_tree(){}

	bool think(Board& b);
};

AI* AI::createAi(type type)
{
	switch (type) {
		case TYPE_ORDERED:
			return new AI_ordered();
			break;
		case TYPE_STANDARD:
			return new AI_standard();
			break;
		case TYPE_MONTECARLO_TREE:
			return new AI_montecarlo_tree();
			break;
	default:
		return new AI_ordered();
		break;
	}

	return nullptr;
}

class Board
{
	friend class AI_ordered;
	friend class AI_standard;
	friend class AI_montecarlo_tree;

public:
	enum WINNER {
		NOT_FINISED = 0,
		PLAYER,
		ENEMY,
		DRAW,
	};
private:
	enum {
		BOARD_SIZE = 3,
	};
	Mass mass_[BOARD_SIZE][BOARD_SIZE];

public:
	Board() {
		//		mass_[0][0].setStatus(Mass::ENEMY); mass_[0][1].setStatus(Mass::PLAYER); 
	}
	Board::WINNER calc_result() const
	{
		// 縦横斜めに同じキャラが入っているか検索
		// 横
		for (int y = 0; y < BOARD_SIZE; y++) {
			Mass::status winner = mass_[y][0].getStatus();
			if (winner != Mass::PLAYER && winner != Mass::ENEMY) continue;
			int x = 1;
			for (; x < BOARD_SIZE; x++) {
				if (mass_[y][x].getStatus() != winner) break;
			}
			if (x == BOARD_SIZE) { return (Board::WINNER)winner; }
		}
		// 縦
		for (int x = 0; x < BOARD_SIZE; x++) {
			Mass::status winner = mass_[0][x].getStatus();
			if (winner != Mass::PLAYER && winner != Mass::ENEMY) continue;
			int y = 1;
			for (; y < BOARD_SIZE; y++) {
				if (mass_[y][x].getStatus() != winner) break;
			}
			if (y == BOARD_SIZE) { return(Board::WINNER) winner; }
		}
		// 斜め
		{
			Mass::status winner = mass_[0][0].getStatus();
			if (winner == Mass::PLAYER || winner == Mass::ENEMY) {
				int idx = 1;
				for (; idx < BOARD_SIZE; idx++) {
					if (mass_[idx][idx].getStatus() != winner) break;
				}
				if (idx == BOARD_SIZE) { return (Board::WINNER)winner; }
			}
		}
		{
			Mass::status winner = mass_[BOARD_SIZE - 1][0].getStatus();
			if (winner == Mass::PLAYER || winner == Mass::ENEMY) {
				int idx = 1;
				for (; idx < BOARD_SIZE; idx++) {
					if (mass_[BOARD_SIZE - 1 - idx][idx].getStatus() != winner) break;
				}
				if (idx == BOARD_SIZE) { return (Board::WINNER)winner; }
			}
		}
		// 上記勝敗がついておらず、空いているマスがなければ引分け
		for (int y = 0; y < BOARD_SIZE; y++) {
			for (int x = 0; x < BOARD_SIZE; x++) {
				Mass::status fill = mass_[y][x].getStatus();
				if (fill == Mass::BLANK) return NOT_FINISED;
			}
		}
		return DRAW;
	}

	bool put(int x, int y) {
		if (x < 0 || BOARD_SIZE <= x ||
			y < 0 || BOARD_SIZE <= y) return false;// 盤面外
		return mass_[y][x].put(Mass::PLAYER);
	}

	void show() const {
		std::cout << "　　";
		for (int x = 0; x < BOARD_SIZE; x++) {
			std::cout << " " << x + 1 << "　";
		}
		std::cout << "\n　";
		for (int x = 0; x < BOARD_SIZE; x++) {
			std::cout << "＋－";
		}
		std::cout << "＋\n";
		for (int y = 0; y < BOARD_SIZE; y++) {
			std::cout << " " << char('a' + y);
			for (int x = 0; x < BOARD_SIZE; x++) {
				std::cout << "｜";
				switch (mass_[y][x].getStatus()) {
				case Mass::PLAYER:
					std::cout << "〇";
					break;
				case Mass::ENEMY:
					std::cout << "×";
					break;
				case Mass::BLANK:
					std::cout << "　";
					break;
				default:
//					if (mass_[y][x].isListed(Mass::CLOSE)) std::cout << "＋"; else
//					if (mass_[y][x].isListed(Mass::OPEN) ) std::cout << "＃"; else
					std::cout << "　";
				}
			}
			std::cout << "｜\n";
			std::cout << "　";
			for (int x = 0; x < BOARD_SIZE; x++) {
				std::cout << "＋－";
			}
			std::cout << "＋\n";
		}
	}
};

bool AI_ordered::think(Board& b)
{
	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			if (b.mass_[y][x].put(Mass::ENEMY)) {
				return true;
			}
		}
	}
	return false;
}

void AI_standard::calcScore(int* turnCount, Board& b)
{
	//各マスごとの優先度
	massScore[0][0] = 50;
	massScore[0][1] = 30;
	massScore[0][2] = 50;
	massScore[1][0] = 30;
	massScore[1][1] = 70;
	massScore[1][2] = 30;
	massScore[2][0] = 50;
	massScore[2][1] = 30;
	massScore[2][2] = 50;

	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		int putCountPlayerX = 0;
		int emptyPosX = -1;
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{
			if (b.mass_[y][x].getStatus() == Mass::status::ENEMY)
			{
				turnCount++;
				massScore[y][x] = -1;
				putCountPlayerX = -100;
			}
			else if (b.mass_[y][x].getStatus() == Mass::status::PLAYER)
			{
				massScore[y][x] = -1;
				putCountPlayerX++;
			}
			else
			{
				emptyPosX = x;
			}
		}

		if (putCountPlayerX == 2)
		{
			massScore[y][emptyPosX] = 100;
		}
		else if (putCountPlayerX == 1)
		{
			//リーチを狙うとき角よりは優先度を下げる
			massScore[y][emptyPosX] = 40;
		}
	}

	for (int x = 0; x < Board::BOARD_SIZE; x++)
	{
		int putCountPlayerY = 0;
		int emptyPosY = -1;
		for (int y = 0; y < Board::BOARD_SIZE; y++)
		{
			if (b.mass_[y][x].getStatus() == Mass::status::ENEMY)
			{
				massScore[y][x] = -1;
				putCountPlayerY = -100;
			}
			else if (b.mass_[y][x].getStatus() == Mass::status::PLAYER)
			{
				massScore[y][x] = -1;
				putCountPlayerY++;
			}
			else
			{
				emptyPosY = y;
			}
		}

		if (putCountPlayerY == 2)
		{
			massScore[emptyPosY][x] = 100;
		}
		else if (putCountPlayerY == 1)
		{
			//リーチを狙うとき角よりは優先度を下げる
			massScore[emptyPosY][x] = 40;
		}
	}

	int putCountPlayerCross = 0;
	int emptyPosCross = -1;
	for (int i = 0; i < Board::BOARD_SIZE; i++)
	{
		if (b.mass_[i][i].getStatus() == Mass::status::ENEMY)
		{
			massScore[i][i] = -1;
			putCountPlayerCross = -100;
		}
		else if (b.mass_[i][i].getStatus() == Mass::status::PLAYER)
		{
			massScore[i][i] = -1;
			putCountPlayerCross++;
		}
		else
		{
			emptyPosCross = i;
		}
	}

	if (putCountPlayerCross == 2)
	{
		massScore[emptyPosCross][emptyPosCross] = 100;
	}
	else if (putCountPlayerCross == 1)
	{
		//リーチを狙うとき角よりは優先度を下げる
		massScore[emptyPosCross][emptyPosCross] = 40;
	}

	putCountPlayerCross = 0;
	emptyPosCross = -1;
	for (int i = 0; i < Board::BOARD_SIZE; i++)
	{
		if (b.mass_[i][Board::BOARD_SIZE - (i + 1)].getStatus() == Mass::status::ENEMY)
		{
			massScore[i][Board::BOARD_SIZE - (i + 1)] = -1;
			putCountPlayerCross = -100;
		}
		else if (b.mass_[i][Board::BOARD_SIZE - (i + 1)].getStatus() == Mass::status::PLAYER)
		{
			massScore[i][Board::BOARD_SIZE - (i + 1)] = -1;
			putCountPlayerCross++;
		}
		else
		{
			emptyPosCross = i;
		}
	}

	if (putCountPlayerCross == 2)
	{
		massScore[emptyPosCross][Board::BOARD_SIZE - (emptyPosCross + 1)] = 100;
	}
	else if (putCountPlayerCross == 1)
	{
		//リーチを狙うとき角よりは優先度を下げる
		massScore[emptyPosCross][Board::BOARD_SIZE - (emptyPosCross + 1)] = 40;
	}

	//後攻初手の定石
	if (*turnCount == 1)
	{
		//真ん中におかれたとき角を最優先にする
		if (b.mass_[Board::BOARD_SIZE / 2][Board::BOARD_SIZE / 2].getStatus() == Mass::status::PLAYER)
		{
			massScore[0][0] = 100;
		}
		//角や辺におかれたとき真ん中を最優先にする
		else
		{
			massScore[Board::BOARD_SIZE / 2][Board::BOARD_SIZE / 2] = 100;
		}
	}
	//二回目置くときに両角取られてて真ん中を取っていれば辺を最優先にする
	else if (*turnCount == 2 && (
		(b.mass_[Board::BOARD_SIZE - 1][Board::BOARD_SIZE - 1].getStatus() == Mass::status::PLAYER && b.mass_[0][0].getStatus() == Mass::status::PLAYER) ||
		(b.mass_[Board::BOARD_SIZE - 1][0].getStatus() == Mass::status::PLAYER && b.mass_[0][Board::BOARD_SIZE - 1].getStatus() == Mass::status::PLAYER)) &&
		b.mass_[Board::BOARD_SIZE / 2][Board::BOARD_SIZE / 2].getStatus() == Mass::status::ENEMY)
	{
		massScore[1][0] = 100;
	}
}

bool AI_standard::think(Board& b)
{
	int turnCount = 1;

	calcScore(&turnCount, b);
	
	//一番スコアが高いマスを選ぶ
	int maxScore = 0;
	int maxScorePosX = -1, maxScorePosY = -1;
	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{
			if (massScore[y][x] > maxScore)
			{
				maxScore = massScore[y][x];
				maxScorePosX = x;
				maxScorePosY = y;
			}		
		}
	}

	if (maxScore != 0)
	{
		b.mass_[maxScorePosY][maxScorePosX].put(Mass::ENEMY);
		return true;
	}
	return false;
}

int AI_montecarlo_tree::evaluate(bool all_search, int sim_count, Board& b, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;

	int r = b.calc_result();
	if (r == current) return +100;
	if (r == next)return -100;
	if (r == Board::DRAW)return 0;

	char x_table[Board::BOARD_SIZE * Board::BOARD_SIZE];
	char y_table[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int wins[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int count[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int scores[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int blank_mass_num = 0;

	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{
			Mass& m = b.mass_[y][x];
			if (m.getStatus() == Mass::BLANK)
			{
				x_table[blank_mass_num] = x;
				y_table[blank_mass_num] = y;
				wins[blank_mass_num] = count[blank_mass_num] = 0;
				scores[blank_mass_num] = -1;
				blank_mass_num++;
			}
		}
	}

	if (all_search) 
	{
		for (int i = 0; i < sim_count; i++)
		{
			int idx = select_mass(blank_mass_num, count, wins);
			if (idx < 0) break;
			Mass& m = b.mass_[y_table[idx]][x_table[idx]];

			m.setStatus(current);
			int dummy;
			int score = -evaluate(false, 0, b, next, dummy, dummy);
			m.setStatus(Mass::BLANK);

			if (0 < score)
			{
				wins[idx]++;
				count[idx]++;
			}
			else
			{
				count[idx]++;
			}

			if (sim_count / 10 < count[idx] && 10 < sim_count)
			{
				m.setStatus(current);
				scores[idx] = 100 - evaluate(true, (int)sqrt(sim_count), b, next, dummy, dummy);
				m.setStatus(Mass::BLANK);
				wins[idx] = -1;
			}
		}

		int score_max = -9999;
		for (int idx = 0; idx < blank_mass_num; idx++)
		{
			int score;
			if (-1 == wins[idx])
			{
				score = scores[idx];
			}
			else if (0 == count[idx])
			{
				score = 0;
			}
			else
			{
				double c = 1 * sqrt(2 * log(sim_count) / count[idx]);
				score = 100 * wins[idx] / count[idx] + (int)(c);
			}

			if (score_max < score)
			{
				score_max = score;
				best_x = x_table[idx];
				best_y = y_table[idx];
			}
			std::cout << x_table[idx] + 1 << (char)('a' + y_table[idx]) << " " << score << "win:"<< std::endl;
		}
		return score_max;
	}

	int idx = rand() % blank_mass_num;
	Mass& m = b.mass_[y_table[idx]][x_table[idx]];
	m.setStatus(current);
	int dummy;
	int score = -evaluate(false, 0, b, next, dummy, dummy);
	m.setStatus(Mass::BLANK);

	return score;
}

int AI_montecarlo_tree::select_mass(int n, int* a_count, int* a_wins)
{
	int total = 0;
	for (int i = 0; i < n; i++)
	{
		total += 10000 * (a_wins[i] + 1) / (a_count[i] + 1);
	}
	if (total <= 0)return -1;

	int r = rand() % total;
	for (int i = 0; i < n; i++)
	{
		r -= 10000 * (a_wins[i] + 1) / (a_count[i] + 1);
		if (r < 0) return i;
	}
	return -1;
}
bool AI_montecarlo_tree::think(Board& b)
{
	int best_x = -1, best_y;
	evaluate(true, 10000, b, Mass::ENEMY, best_x, best_y);

	if (best_x < 0)return false;

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

class Game
{
private:
	const AI::type ai_type = AI::TYPE_STANDARD;

	Board board_;
	Board::WINNER winner_ = Board::NOT_FINISED;
	AI* pAI_ = nullptr;

public:
	Game() {
		pAI_ = AI::createAi(ai_type);
	}
	~Game() {
		delete pAI_;
	}

	bool put(int x, int y) {
		bool success = board_.put(x, y);
		if (success) winner_ = board_.calc_result();

		return success;
	}

	bool think() {
		bool success = pAI_->think(board_);
		if (success) winner_ = board_.calc_result();
		return success;
	}

	Board::WINNER is_finised() {
		return winner_;
	}

	void show() {
		board_.show();
	}
};

void show_start_message()
{
	std::cout << "========================" << std::endl;
	std::cout << "       GAME START       " << std::endl;
	std::cout << std::endl;
	std::cout << "input position likes 1 a" << std::endl;
	std::cout << "========================" << std::endl;
}

void show_end_message(Board::WINNER winner)
{
	if (winner == Board::PLAYER) {
		std::cout << "You win!" << std::endl;
	}
	else if (winner == Board::ENEMY)
	{
		std::cout << "You lose..." << std::endl;
	}
	else {
		std::cout << "Draw" << std::endl;
	}
	std::cout << std::endl;
}

int main()
{
	for (;;) {// 無限ループ
		show_start_message();

		// initialize
		unsigned int turn = 0;
		std::shared_ptr<Game> game(new Game());

		while (1) {
			game->show();// 盤面表示

			// 勝利判定
			Board::WINNER winner = game->is_finised();
			if (winner) {
				show_end_message(winner);
				break;
			}

			if (0 == turn) {
				// user input
				char col[1], row[1];
				do {
					std::cout << "? ";
					std::cin >> row >> col;
				} while (!game->put(row[0] - '1', col[0] - 'a'));
			}
			else {
				// AI
				if (!game->think()) {
					show_end_message(Board::WINNER::PLAYER);// 投了
				}
				std::cout << std::endl;
			}
			// プレイヤーとAIの切り替え
			turn = 1 - turn;
		}
	}

	return 0;
}