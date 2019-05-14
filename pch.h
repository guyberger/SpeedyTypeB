// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file


#ifndef PCH_H
#define PCH_H
//#define _AFXDLL 1
#include "C:\temp\PDCurses-3.8\curses.h"
#include <string>
#include <set>
#include <chrono>
#include <unordered_map>
#include <random>
#include <iostream>
#include <fstream>
#define HAVE_STRUCT_TIMESPEC
//#include <pthread.h>
#include <atomic>
#include <Windows.h>
#include <thread>
//#include <afxwin.h>
using namespace std;

#define MODES 1
#define DIFFC 3
#define MAX_WORD_SIZE 20
#define SLW 500
#define REG 250
#define FST 200

class GameMode {
	set<string> dict;
	int speed;
	int words_gap;
public:
	GameMode() {};
	void init(const char* filename = nullptr) {
		if (!filename) return;
		fstream myfile;
		myfile.open(filename, fstream::in);
		try {
			char buffer[MAX_WORD_SIZE] = { 0 };
			while (myfile.getline(buffer, MAX_WORD_SIZE)) {
				string s = "";
				int i = 0;;
				while (buffer[i]) s += buffer[i++];
				dict.insert(s);
			}
		}
		catch (...) {};
	}
	~GameMode() {};
	void setDiffc(char diffc) {
		switch (diffc) {
		case '1':
			speed = SLW;
			words_gap = 3;
			echo();
			break;
		case '2':
			speed = REG;
			words_gap = 2;
			echo();
			break;
		case '3':
			speed = FST;	// + invisible typing
			words_gap = 2;
			break;
		default: break;
		}
	}
	int getSpeed() {
		return speed;
	}
	int getGap() {
		return words_gap;
	}
	bool isEmptyDict() {
		return dict.size() == 0;
	}
	string extractRandomWord() {
		random_device rd;
		set<string>::iterator it(dict.begin());
		int to_add = (int)(rd() % dict.size());
		advance(it, to_add);
		string out(*it);
		dict.erase(*it);
		return out;
	}

};

class SpinLock
{
public:
	void lock()
	{
		while (lck.test_and_set(std::memory_order_acquire))
		{
		}
	}

	void unlock()
	{
		lck.clear(std::memory_order_release);
	}

private:
	std::atomic_flag lck = ATOMIC_FLAG_INIT;
};
class Stats {
	SpinLock* lock;
public:
	Stats(){
		lock = new SpinLock;
		hit = miss = 0;
	}
	~Stats() {
		delete lock;
	}
	SpinLock* getLockRef() {
		return lock;
	}
	int hit;
	int miss;
	double acc;	//accurcy in %
	void updateAcc() {
		int tot = hit + miss;
		acc = (tot == 0) ? 100 : 100 * ((double)((double)hit / tot));
	}
	void addHit() {
		hit++;
		updateAcc();
	}
	void addMiss() {
		miss++;
		updateAcc();
	}
};
void initColors();
int updateScore(void* args);
int kbhit(WINDOW* inpw);
char init(GameMode& mode);
char printMenu();
char printDiffc();
void drawLine(int y, int x1, int x2);
void play(unordered_map<string, WINDOW*>& windows, GameMode& mode);
void addWord(GameMode& mode, set<string>& disp_dict, unordered_map<string, pair<int, int>>& lmap);
void removeWord(set<string>& disp_dict, string& w);
void displayWords(WINDOW* dispw, set<string>& disp_dict, unordered_map<string, pair<int, int>>& lmap, int move, Stats* stats);
// TODO: add headers that you want to pre-compile here

#endif //PCH_H
