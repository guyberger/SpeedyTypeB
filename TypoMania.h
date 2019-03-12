#include <ncurses.h>
#include <string>
#include <set>
#include <chrono>
#include <unordered_map>
#include <random>
#include <iostream>
#include <fstream>

using namespace std;

#define MODES 1
#define DIFFC 2
#define MAX_WORD_SIZE 30
#define SLW 500
#define REG 250
#define FST 200

class GameMode{
	set<string> dict;
	int speed;
	int words_gap;
public:
	GameMode(){};
	void init(const char* filename = nullptr){
		if(!filename) return;
		fstream myfile;
		myfile.open(filename, fstream::in);
		try{
			char buffer[MAX_WORD_SIZE] = {0};
			while(myfile.getline(buffer, MAX_WORD_SIZE)){
				string s = "";
				int i = 0;;
				while(buffer[i]) s += buffer[i++];
				dict.insert(s);
			}
		}
		catch(...){};
	}
	~GameMode(){};
	void setDiffc(char diffc){
		switch(diffc){
			case '1':
				speed = SLW;
				words_gap = 3;
				break;
			case '2':
				speed = REG;
				words_gap = 2;
				break;
			case '3':
				speed = FST;
				words_gap = 2;
				break;
			default: break;
		}
	}
	int getSpeed(){
		return speed;
	}
	int getGap(){
		return words_gap;
	}
	bool isEmptyDict(){
		return dict.size() == 0;
	}
	string extractRandomWord(){
		random_device rd;
		set<string>::iterator it(dict.begin());
		int to_add = (int)(rd()%dict.size());
		advance(it, to_add);
		string out(*it);
		dict.erase(*it);
		return out;
	}

};
int kbhit(WINDOW* inpw);
char init(GameMode& mode);
char printMenu();
char printDiffc();
void drawLine(int y, int x1, int x2);
void play(unordered_map<string, WINDOW*> windows, GameMode& mode);
void addWord(GameMode& mode, set<string>& disp_dict, unordered_map<string, pair<int,int>>& lmap);
void removeWord(set<string>& disp_dict, string& w);
void displayWords(WINDOW* dispw, set<string>& disp_dict, unordered_map<string, pair<int,int>>& lmap, int move);
/*
class RickAndMorty : public GameMode{
public:
	RickAndMorty(const char* filename){
		fstream myfile;
		myfile.open
	}	

}*/
/*
class window{
	WINDOW* win;
public:
	window(int height, int length, int starty, int startx){
		win = MakeWin(height, length, starty, startx);
	}
}*/




