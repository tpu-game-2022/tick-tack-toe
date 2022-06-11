#pragma once

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

class Board
{
	friend class AI_ordered;
	friend class AI_nega_max;
	friend class AI_alpha_beta;
	friend class AI_nega_scout;
	friend class AI_monte_carlo;
	friend class AI_monte_carlo_tree;

public:
	enum WINNER {
		NOT_FINISED = 0,
		PLAYER,
		ENEMY,
		DRAW,
	};
private:
	enum {
		//BOARD_SIZE = 3,
		BOARD_SIZE = 5,
	};
	Mass mass_[BOARD_SIZE][BOARD_SIZE];

public:
	Board() {
		//		mass_[0][0].setStatus(Mass::ENEMY); mass_[0][1].setStatus(Mass::PLAYER); 
	}
	Board::WINNER calc_result() const
	{
		// c‰¡Î‚ß‚É“¯‚¶ƒLƒƒƒ‰‚ª“ü‚Á‚Ä‚¢‚é‚©ŒŸõ
		// ‰¡
		for (int y = 0; y < BOARD_SIZE; y++) {
			Mass::status winner = mass_[y][0].getStatus();
			if (winner != Mass::PLAYER && winner != Mass::ENEMY) continue;
			int x = 1;
			for (; x < BOARD_SIZE; x++) {
				if (mass_[y][x].getStatus() != winner) break;
			}
			if (x == BOARD_SIZE) { return (Board::WINNER)winner; }
		}
		// c
		for (int x = 0; x < BOARD_SIZE; x++) {
			Mass::status winner = mass_[0][x].getStatus();
			if (winner != Mass::PLAYER && winner != Mass::ENEMY) continue;
			int y = 1;
			for (; y < BOARD_SIZE; y++) {
				if (mass_[y][x].getStatus() != winner) break;
			}
			if (y == BOARD_SIZE) { return(Board::WINNER)winner; }
		}
		// Î‚ß
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
		// ã‹LŸ”s‚ª‚Â‚¢‚Ä‚¨‚ç‚¸A‹ó‚¢‚Ä‚¢‚éƒ}ƒX‚ª‚È‚¯‚ê‚Îˆø•ª‚¯
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
			y < 0 || BOARD_SIZE <= y) return false;// ”Õ–ÊŠO
		return mass_[y][x].put(Mass::PLAYER);
	}

	void show() const {
		std::cout << "@@";
		for (int x = 0; x < BOARD_SIZE; x++) {
			std::cout << " " << x + 1 << "@";
		}
		std::cout << "\n@";
		for (int x = 0; x < BOARD_SIZE; x++) {
			std::cout << "{|";
		}
		std::cout << "{\n";
		for (int y = 0; y < BOARD_SIZE; y++) {
			std::cout << " " << char('a' + y);
			for (int x = 0; x < BOARD_SIZE; x++) {
				std::cout << "b";
				switch (mass_[y][x].getStatus()) {
				case Mass::PLAYER:
					std::cout << "Z";
					break;
				case Mass::ENEMY:
					std::cout << "~";
					break;
				case Mass::BLANK:
					std::cout << "@";
					break;
				default:
					//					if (mass_[y][x].isListed(Mass::CLOSE)) std::cout << "{"; else
					//					if (mass_[y][x].isListed(Mass::OPEN) ) std::cout << "”"; else
					std::cout << "@";
				}
			}
			std::cout << "b\n";
			std::cout << "@";
			for (int x = 0; x < BOARD_SIZE; x++) {
				std::cout << "{|";
			}
			std::cout << "{\n";
		}
	}
};