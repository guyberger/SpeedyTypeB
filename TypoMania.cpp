// TypoMania.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
using namespace std;

static bool game_over = false;

int main() {

	// initialize gmae mode and difficulty

	GameMode mode;
	//initialize colors

	//initColors();

	//attron(4);
	char m_sel = init(mode);
	//attroff(4);
	switch (m_sel) {
	case '1':
		mode.init("RickMortyDict.txt");
		break;
	default:
		cerr << "ERROR" << endl;
		break;
	}

	// initialize windows

	unordered_map<string, WINDOW*> windows;
	windows["display"] = newwin(LINES * 4 / 5, COLS, 0, 0);
	windows["input"] = newwin(4, COLS/2, LINES*4/5 + 2, 2);//newwin((int)(LINES - LINES * 4 / 5 - 1), int(COLS / 2), int(LINES * 4 / 5 + 2), 2);
	windows["menu"] = newwin(LINES - LINES * 4 / 5 - 1, COLS - COLS / 2, LINES * 4 / 5 + 2, COLS / 2 + 1);
	clear();
	mvprintw(LINES * 4 / 5 + 2, 0, ">");
	drawLine(LINES * 4 / 5 + 1, 0, COLS);
	refresh();
	wrefresh(windows["input"]);
	wmove(windows["input"], 0, 0);

	// gameplay

	nodelay(windows["input"], true);	// getch wont be a blocking call
	play(windows, mode);
	getch();
	delwin(windows["input"]);
	delwin(windows["menu"]);
	delwin(windows["display"]);
	endwin();
	return 0;
}
/*
void initColors() {
	
	start_color();
	if (has_colors() == false)
	{
		endwin();
		printf("Your terminal does not support color\n");
		exit(1);
	}

	//	modifications to colors. source: https://color.adobe.com/rainbow-color-theme-11664742/edit/?copy=true&base=2&rule=Custom&selected=3&name=Copy%20of%20rainbow&mode=rgb&rgbvalues=0.823529,0.1943244735151891,0.2622443603759071,0.952941,0.443137,0.14902,0.960784,0.870588,0.0196078,0.2,0.623529,0.286275,0,0.572549,0.741176&swatchOrder=0,1,2,3,4
	
	init_color(COLOR_GREEN, 51, 159, 73);
	init_color(COLOR_BLUE, 0, 146, 189);
	init_color(COLOR_YELLOW, 245, 222, 5);
	init_color(COLOR_RED, 210, 50, 67);
	
	init_pair(1, COLOR_GREEN, COLOR_BLACK);	// hits
	init_pair(2, COLOR_RED, COLOR_BLACK);	// misses
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);	// accuracy
	init_pair(4, COLOR_BLUE, COLOR_BLACK);	//typing	
}*/
char init(GameMode& mode) {
	initscr();
	cbreak();
	noecho();
	char c = printMenu();
	clear();
	char diffc = printDiffc();
	mode.setDiffc(diffc);
	return c;
}

char printMenu() {
	mvprintw(LINES / 2 - 1, (COLS - 17) / 2, "Choose a mode to play");
	mvprintw(LINES / 2, (COLS - 17) / 2, "[1] Rick and Morty");
	refresh();
	char c = getch();
	while (c < '1' || c >('0' + MODES)) {
		c = getch();
	}
	return c;
}
char printDiffc() {
	mvprintw(LINES / 2 - 1, (COLS - 17) / 2, "Choose difficulty");
	mvprintw(LINES / 2, (COLS - 17) / 2, "[1] Easy");
	mvprintw(LINES / 2 + 1, (COLS - 17) / 2, "[2] Regular");
	mvprintw(LINES / 2 + 2, (COLS - 17) / 2, "[3] God mode");
	refresh();
	char c = getch();
	while (c < '1' || c >('0' + DIFFC)) {
		c = getch();
	}
	return c;
}
void drawLine(int y, int x1, int x2) {
	while (x1 <= x2) {
		mvprintw(y, x1, "-");
		x1++;
	}

}
void play(unordered_map<string, WINDOW*>& windows, GameMode& mode) {
	WINDOW* inpw = windows["input"];
	WINDOW* dispw = windows["display"];
	WINDOW* menuw = windows["menu"];	//useless for now

	Stats stats;
	thread th(updateScore, (void*)&stats);
	//int rc;
	//pthread_t stats_thread;
	//rc = pthread_create(&stats_thread, NULL, updateScore, (void*)&stats);	//this thread will update the score
	unordered_map<string, pair<int, int>> lmap;	//locations of words
	auto start_time = chrono::high_resolution_clock::now();
	set<string> disp_dict;
	string s("");
	int sec_count = 0, last_sec = 0;
	bool first = true;
	bool move_words = true;
	double div = 1000 / mode.getSpeed(), fact = 1;	// counting seconds

	while (1) {
		if (mode.isEmptyDict() && disp_dict.size() == 0) {	// winner!
			wclear(dispw);
			wrefresh(inpw);
			this_thread::sleep_for(std::chrono::milliseconds(300));
			const char* s1 = "Finished Dictionary!";
			const char* s2 = "<Press any key to exit>";
			mvwprintw(dispw, LINES / 4, COLS / 2 - 10, s1);
			mvwprintw(dispw, LINES / 4 + 1, COLS / 2 - 12, s2);
			wrefresh(dispw);
			game_over = true;
			th.join();
			return;
		}
		auto curr_time = chrono::high_resolution_clock::now();
		auto time_diff = chrono::duration_cast<chrono::milliseconds>(curr_time - start_time);
		int diff = time_diff.count();

		if (1000 <= diff) {	// one second
			start_time = curr_time;
			sec_count++;
		}
		if (fact*mode.getSpeed() <= diff) {
			fact++;
			if (fact > div) fact = 1;
			move_words = true;
		}
		if (first || (last_sec < sec_count && sec_count % mode.getGap() == 0)) {
			addWord(mode, disp_dict, lmap);
			last_sec = sec_count;
			first = false;
		}
		if (move_words) {
			int x, y;
			getyx(inpw, y, x);
			displayWords(dispw, disp_dict, lmap, 1, &stats);
			wmove(inpw, y, x);
			move_words = false;
		}
		if (kbhit(inpw)) {
			char c = wgetch(inpw);
			if (c == 8) {	// backspace
				if (s.length() == 0) continue;
				s.erase(s.length() - 1, 1);
				wdelch(inpw); wdelch(inpw);
				continue;
			}
			else if (c < 'A' || c > 'z' || (c < 'a' && c > 'Z')) {
				s = "";
				wclear(inpw);
				continue;
			}
			s += c;
			if (s.length() > MAX_WORD_SIZE) {
				s.erase(s.length() - 1, 1);
				int x, y;
				getyx(inpw, y, x);
				wmove(inpw, y, x - 1);
				continue;
			}
			if (disp_dict.count(s) != 0) {	// "hit"
				stats.getLockRef()->lock();
				stats.addHit();
				stats.getLockRef()->unlock();
				removeWord(disp_dict, s);
				s = "";
				wclear(inpw);
				int x, y;
				getyx(inpw, y, x);
				displayWords(dispw, disp_dict, lmap, 0, &stats);
				if (disp_dict.size() == 0) wrefresh(dispw);
				wmove(inpw, y, x);
			}
		}


	}

}
void addWord(GameMode& mode, set<string>& disp_dict, unordered_map<string, pair<int, int>>& lmap) {
	if (mode.isEmptyDict()) return;
	string word(mode.extractRandomWord());
	disp_dict.insert(word);
	random_device rd;
	int yloc = (int)(rd() % (LINES * 4 / 5));
	pair<int, int> coords = make_pair(0, yloc);
	lmap[word] = coords;
}
void removeWord(set<string>& disp_dict, string& w) {
	disp_dict.erase(w);
}
void displayWords(WINDOW* dispw, set<string>& disp_dict, unordered_map<string, pair<int, int>>& lmap, int move, Stats* stats) {
	wclear(dispw);
	if (disp_dict.size() == 0) return;
	set<string> toRemove;
	for (auto w : disp_dict) {
		pair<int, int> coords = lmap[w];
		char word[MAX_WORD_SIZE] = { 0 };
		for (int i = 0;(unsigned int)i < w.length();i++) {
			word[i] = w[i];
		}
		if ((unsigned int)coords.first + (unsigned int)move + w.length() > (unsigned int)COLS) {	//"miss"
			stats->getLockRef()->lock();
			stats->addMiss();
			stats->getLockRef()->unlock();
			toRemove.insert(w);
			continue;
		}
		mvwprintw(dispw, coords.second, coords.first + move, word);
		lmap[w] = make_pair(coords.first + move, coords.second);
	}
	for (auto w : toRemove) {
		removeWord(disp_dict, w);
	}
	wrefresh(dispw);
}
int kbhit(WINDOW* inpw) {
	int ch = wgetch(inpw);
	if (ch != ERR) {
		ungetch(ch);
		return 1;
	}
	else return 0;
}

int updateScore(void* args) {
	if (game_over) return 0;
	WINDOW* score = newwin(4, COLS - COLS / 2 - 4, LINES * 4 / 5 + 2, COLS / 2 + 3);
	Stats* stats = (Stats*)args;
	stats->getLockRef()->lock();
	int last_hit = stats->hit, last_miss = stats->miss;
	mvwprintw(score, 0, 0, "Hit: %d    Miss: %d    Accuracy: %.2f%%", stats->hit, stats->miss, stats->acc);
	stats->getLockRef()->unlock();
	wrefresh(score);
	while (1) {
		if (game_over) return 0;
		stats->getLockRef()->lock();
		if (last_hit != stats->hit || last_miss != stats->miss) {
			mvwprintw(score, 0, 0, "Hit: %d    Miss: %d    Accuracy: %.2f%%", stats->hit, stats->miss, stats->acc);
			wrefresh(score);
			last_hit = stats->hit;
			last_miss = stats->miss;
		}
		stats->getLockRef()->unlock();
		this_thread::sleep_for(std::chrono::milliseconds(300));
	}
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
