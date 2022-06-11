#include "Board.h"
#include "AI.h"

//--- AIを生成する ---//
AI* AI::createAi(type type)
{
	switch (type) {
	case TYPE_ORDERED:
		return new AI_ordered();
	case TYPE_NEGA_MAX:
		return new AI_nega_max();
	case TYPE_ALPHA_BETA:
		return new AI_alpha_beta();
	case TYPE_NEGA_SCOUT:
		return new AI_nega_scout();
	case TYPE_MONTE_CARLO:
		return new AI_monte_carlo();
	case TYPE_MONTE_CARLO_TREE:
		return new AI_monte_carlo_tree();
	default:
		return new AI_ordered();
		break;
	}

	return nullptr;
}


//--- 順番に打ってみる ---//
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


//--- Nega-max戦略AI ---//
int AI_nega_max::evaluate(Board& b, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	// 死活判定
	int r = b.calc_result();
	// 直接評価
	if (r == current) return +10000;   // 呼び出し側の勝ち
	if (r == next) return -10000;      // 呼び出し側の負け
	if (r == Board::DRAW) return 0;    // 引き分け

	int score_max = -10001;   // 打たないのは最悪

	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{
			Mass& m = b.mass_[y][x];
			if (m.getStatus() != Mass::BLANK) continue;

			m.setStatus(current);   // 次の手を打つ
			int dummy;   // 最上位以外は打つわけではないのでダミーを入れてごまかす
			int score = -evaluate(b, next, dummy, dummy);
			m.setStatus(Mass::BLANK);   //手を戻す

			// 最大値の確保と手の記録
			if (score_max < score)
			{
				score_max = score;
				best_x = x;
				best_y = y;
			}
		}
	}
	return score_max;
}

bool AI_nega_max::think(Board& b)
{
	int best_x = -1, best_y;

	evaluate(b, Mass::ENEMY, best_x, best_y);

	if (best_x < 0) return false;   // 打てる手はなかった

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}


//--- alpha-beta法AI ---//
int AI_alpha_beta::evaluate(int alpha, int beta, Board& b, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	// 死活判定
	int r = b.calc_result();
	if (r == current) return +10000;   // 呼び出し側の勝ち
	if (r == next) return -10000;      // 呼び出し側の負け
	if (r == Board::DRAW) return 0;    // 引き分け

	int score_max = -9999;   // 打たないで投了

	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{
			Mass& m = b.mass_[y][x];
			if (m.getStatus() != Mass::BLANK) continue;

			m.setStatus(current);   // 次の手を打つ
			int dummy;
			// 下位ではプレイヤーの種類と最大・最小が入れ替わる。符号を反転して入れ替えると正しく評価可能
			int score = -evaluate(-beta, -alpha, b, next, dummy, dummy);
			m.setStatus(Mass::BLANK);   //手を戻す

			// 上位で最悪の値より悪かったら（相手の評価が良くなったら）、同じ階層はこれ以上探索しない
			if (beta < score)
			{
				return (score_max < score) ? score : score_max;   // 最悪の値より悪い
			}

			// 最大値の確保と手の記録
			if (score_max < score)
			{
				score_max = score;
				alpha = (alpha < score_max) ? score_max : alpha;   // αの値を更新
				best_x = x;
				best_y = y;
			}
		}
	}
	return score_max;
}

bool AI_alpha_beta::think(Board& b)
{
	int best_x, best_y;

	if (evaluate(-10000, 10000, b, Mass::ENEMY, best_x, best_y) <= -9999)
		return false;   // 打てる手はなかった

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}


//--- Nega-Scout法AI ---//
int AI_nega_scout::evaluate(int limit, int alpha, int beta, Board& board, Mass::status current, int& best_x, int& best_y)
{
	if (limit-- == 0) return 0;   // 深さ制限に達した。引き分けにしておく

	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	// 死活判定
	int r = board.calc_result();
	if (r == current) return +10000;   // 呼び出し側の勝ち
	if (r == next) return -10000;      // 呼び出し側の負け
	if (r == Board::DRAW) return 0;    // 引き分け

	int a = alpha, b = beta;

	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{
			Mass& m = board.mass_[y][x];
			if (m.getStatus() != Mass::BLANK) continue;

			m.setStatus(current);   // 次の手を打つ
			int dummy;
			int score = -evaluate(limit, -b, -a, board, next, dummy, dummy);   // 長男は普通に探索
			if (a < score && score < beta && !(x == 0 && y == 0) && limit <= 2)
			{
				a = -evaluate(limit, -beta, -score, board, next, dummy, dummy);
			}
			m.setStatus(Mass::BLANK);   //手を戻す。　　　次男以降は評価が良ければきちんと探索

			if (a < score)
			{
				a = score;
				best_x = x;
				best_y = y;
			}

			// β刈り
			if (beta <= a)
			{
				return a;
			}

			b = a + 1;   // ヌルウィンドウの更新
		}
	}
	return a;
}

bool AI_nega_scout::think(Board& b)
{
	int best_x, best_y;

	if (evaluate(5, -10000, 10000, b, Mass::ENEMY, best_x, best_y) <= -9999)
		return false;   // 打てる手はなかった

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}


//--- モンテカルロ法AI ---//
int AI_monte_carlo::evaluate(bool fiest_time, Board& board, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	///末端
	// 死活判定
	int r = board.calc_result();
	if (r == current) return +10000;   // 呼び出し側の勝ち
	if (r == next) return -10000;      // 呼び出し側の負け
	if (r == Board::DRAW) return 0;    // 引き分け

	/// 空いているマスを調べる
	char x_table[Board::BOARD_SIZE * Board::BOARD_SIZE];
	char y_table[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int wins[Board::BOARD_SIZE * Board::BOARD_SIZE];   // 勝利数
	int loses[Board::BOARD_SIZE * Board::BOARD_SIZE];  // 敗北数
	int blank_mass_num = 0;
	// 空いているマスを数え配列として位置を確保
	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{
			Mass& m = board.mass_[y][x];
			if (m.getStatus() == Mass::BLANK)
			{
				x_table[blank_mass_num] = x;
				y_table[blank_mass_num] = y;
				wins[blank_mass_num] = loses[blank_mass_num] = 0;
				blank_mass_num++;
			}
		}
	}

	if (fiest_time)
	{
		/// ランダムに空きマスを選び、勝利数を数える
		// 一番上の階層でランダムに指すのを繰り返す
		for (int i = 0; i < 10000; i++)
		{
			int idx = rand() % blank_mass_num;
			Mass& m = board.mass_[y_table[idx]][x_table[idx]];

			m.setStatus(current);   // 次の手を打つ
			int dummy;
			int score = -evaluate(false, board, next, dummy, dummy);
			m.setStatus(Mass::BLANK);   //手を戻す

			if (0 <= score)
			{
				wins[idx]++;
			}
			else
			{
				loses[idx]++;
			}
		}
		/// 勝利数の最も多いマスを選択
		int score_max = -9999;
		for (int idx = 0; idx < blank_mass_num; idx++)
		{
			int score = wins[idx] + loses[idx];
			if (0 != score)
			{
				score = 100 * wins[idx] / score;   // 勝率
			}
			if (score_max < score)
			{
				score_max = score;
				best_x = x_table[idx];
				best_y = y_table[idx];
			}
			std::cout << x_table[idx] + 1 << (char)('a' + y_table[idx]) << " " << score
				<< "% （win：" << wins[idx] << ", lose：" << loses[idx] << "）" << std::endl;
		}

		return score_max;
	}

	/// 最上位でなければ最後までランダムに選択
	// 上位でない層はどんどん適当に打っていく
	int idx = rand() % blank_mass_num;
	Mass& m = board.mass_[y_table[idx]][x_table[idx]];
	m.setStatus(current);   // 次の手を打つ
	int dummy;
	int score = -evaluate(false, board, next, dummy, dummy);
	m.setStatus(Mass::BLANK);   // 手を戻す

	return score;
}

bool AI_monte_carlo::think(Board& b)
{
	int best_x = -1, best_y;

	if (evaluate(true, b, Mass::ENEMY, best_x, best_y) <= -9999)
		return false;   // 打てる手はなかった

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}


//--- モンテカルロ木探索AI ---//
// 勝率が高いほど多く割り当てる
int AI_monte_carlo_tree::select_mass(int n, int* a_count, int* a_wins)
{
	int total = 0;
	for (int i = 0; i < n; i++)
	{
		total += 10000 * (a_wins[i] + 1) / (a_count[i] + 1);   // 0の時にも確率が上がるように+1する
	}
	if (total <= 0) return -1;

	int r = rand() % total;
	for (int i = 0; i < n; i++)
	{
		r -= 10000 * (a_wins[i] + 1) / (a_count[i] + 1);
		if (r < 0)
		{
			return i;
		}
	}
	return -1;
}

int AI_monte_carlo_tree::evaluate(bool all_search, int sim_count, Board& board, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	///末端
	// 死活判定
	int r = board.calc_result();
	if (r == current) return +100;   // 呼び出し側の勝ち
	if (r == next) return -100;      // 呼び出し側の負け
	if (r == Board::DRAW) return 0;    // 引き分け

	/// 空いているマスを調べる
	char x_table[Board::BOARD_SIZE * Board::BOARD_SIZE];
	char y_table[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int wins[Board::BOARD_SIZE * Board::BOARD_SIZE];    // 勝利数
	int count[Board::BOARD_SIZE * Board::BOARD_SIZE];   // 敗北数
	int scores[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int blank_mass_num = 0;
	// 空いているマスを数え配列として位置を確保
	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{
			Mass& m = board.mass_[y][x];
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
			int idx = select_mass(blank_mass_num, count, wins);   // 勝率に応じたマスの選択
			if (idx < 0) break;
			Mass& m = board.mass_[y_table[idx]][x_table[idx]];

			m.setStatus(current);   // 次の手を打つ
			int dummy;
			int score = -evaluate(false, 0, board, next, dummy, dummy);
			m.setStatus(Mass::BLANK);   //手を戻す

			if (0 < score)
			{
				wins[idx]++;
				count[idx]++;
			}
			else
			{
				count[idx]++;
			}
			/// 試行回数を超えた際に木を成長（ここでは閾値はマジックナンバー）
			// 閾値を超えれば、木を成長させる
			if (sim_count / 10 < count[idx]   // 閾値は10%以上の探索回数
				&& 10 < sim_count)            // 回数が少ない時はランダムの精度が下がるので、成長させない
			{
				m.setStatus(current);   // 次の手を打つ
				scores[idx] = 100 - evaluate(true, (int)sqrt(sim_count), board, next, dummy, dummy);
				m.setStatus(Mass::BLANK);   //手を戻す
				wins[idx] = -1;   // この枝は乱数で呼ばれないようにする
			}
		}
		/// 勝利数の最も多いマスを選択
		int score_max = -9999;
		for (int idx = 0; idx < blank_mass_num; idx++)
		{
			int score;
			if (-1 == wins[idx])
			{
				score = scores[idx];   // 枝分かれした。成長させた場合はその評価値を採用。
			}
			else if (0 == count[idx])
			{
				score = 0;   // 一度も通らなかった
			}
			else
			{
				double c = 1. * sqrt(2 * log(sim_count) / count[idx]);
				score = 100 * wins[idx] / count[idx] + (int)(c);   // 勝率
			}

			if (score_max < score)
			{
				score_max = score;
				best_x = x_table[idx];
				best_y = y_table[idx];
			}
			std::cout << x_table[idx] + 1 << (char)('a' + y_table[idx]) << " " << score
				<< "% （win：" << wins[idx] << ", count：" << count[idx] << "）" << std::endl;
		}

		return score_max;
	}

	/// 最上位でなければ最後までランダムに選択
	// 上位でない層はどんどん適当に打っていく
	int idx = rand() % blank_mass_num;
	Mass& m = board.mass_[y_table[idx]][x_table[idx]];
	m.setStatus(current);   // 次の手を打つ
	int dummy;
	int score = -evaluate(false, 0, board, next, dummy, dummy);
	m.setStatus(Mass::BLANK);   // 手を戻す

	return score;
}

bool AI_monte_carlo_tree::think(Board& b)
{
	int best_x = -1, best_y;

	evaluate(true, 10000, b, Mass::ENEMY, best_x, best_y);

	if (best_x < 0) return false;   // 打てる手はなかった

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}