#include <ncurses.h>
#include <string>
#include <vector>
#include <set>
#include <chrono>
#include <unordered_map>
#include <random>
#include <iostream>
#include <fstream>
using namespace std;
#define MODES 1
#define DIFF 2
#define MAX_WORD_SIZE 100

WINDOW * MakeWin(int, int, int, int, bool);
void play(WINDOW* inp_win, WINDOW* disp_win, set<string>& dict);
void DisplayWords(WINDOW* disp_win, set<string>& disp_dict, unordered_map<string, pair<int,int>>& wmap);
void AddWord(set<string>& dict, set<string>& disp_dict, unordered_map<string, pair<int,int>>& wmap);
void RemoveWord(set<string>& disp_dict, string& w);
int kbhit(WINDOW*);
char printMenu();
char printDiff();
int gameplay(char, char);
int main(){

	initscr();
	cbreak();
	noecho();
	char c = printMenu();
	clear();
	char diff = printDiff();	// 1 regular, 2 god mode
	gameplay(c, diff);

	return 0;
}
char printMenu(){

	mvprintw(LINES/2-1, (COLS-17)/2, "Choose a mode to play");
	mvprintw(LINES/2, (COLS-17)/2, "1. Rick and Morty");
	refresh();
	char c = getch();
	while(c < '1' || c > ('0'+MODES)){	//must choose a valid option
		c = getch();
	}
	return c;
}
char printDiff(){

	mvprintw(LINES/2-1, (COLS-17)/2, "Choose difficulty");
	mvprintw(LINES/2, (COLS-17)/2, "1. Regular");
	mvprintw(LINES/2+1, (COLS-17)/2, "2. God Mode");
	refresh();
	char c = getch();
	while(c < '1' || c > ('0'+DIFF)){	//must choose a valid option
		c = getch();
	}
	return c;
}

int gameplay(char mode, char difficulty){

	//initscr();
	//cbreak();

	// Define windows and variales
	
	//string mydict[] = {"box", "cow", "words", "leaves"};
	set<string> dict;
	fstream myfile;
	switch(mode){
		case '1':
			myfile.open("RickMortyDict.txt", fstream::in);
			break;
		default:
			break;
	}
	try{
		char buffer[MAX_WORD_SIZE] = {0};
		while(myfile.getline(buffer, MAX_WORD_SIZE)){
			string s = "";
			int i=0;
			while(buffer[i]) 
				s += buffer[i++];
			dict.insert(s);
		}
	}
	catch(...){
		
	};
	switch(difficulty){
		case '1':
			echo();
			break;
		default: break;
	}

	int startx = 0, starty = LINES*4/5;

	WINDOW * inp_win_box = MakeWin(LINES - starty, COLS, starty, startx, 1);
	WINDOW * inp_win = MakeWin(LINES - starty - 2, COLS - 4, starty+1, startx + 3, 0);
	WINDOW * disp_win_box = MakeWin(starty, COLS, 0, 0, 1);
	WINDOW * disp_win = MakeWin(starty-2, COLS-2, 1, 1, 0);
	mvwprintw(inp_win_box, 1, 1, ">");
	
	wrefresh(inp_win_box);
	wmove(inp_win, 0 ,0);

		
	// Actual Gameplay

	nodelay(inp_win, true);

	play(inp_win, disp_win, dict);
	getch();	
	endwin();

	return 0;
}

WINDOW * MakeWin(int height, int weight, int start_y, int start_x, bool make_box){
	WINDOW* out = newwin(height, weight, start_y, start_x);
	refresh();
	if(make_box) box(out, 0, 0);
	wrefresh(out);
	return out;
}

void play(WINDOW* inp_win, WINDOW* disp_win, set<string>& dict){
	
	unordered_map<string, pair<int, int>> wmap;
	auto start_time = std::chrono::high_resolution_clock::now();
	set<string> disp_dict = {};

	string s("");
	int last = 0; //to know when half a second has passed
	int last_sec = 0;
	double sec_counter = 0;
	bool first = true;

	while(1){	// main loop to play game...
		auto curr_time = std::chrono::high_resolution_clock::now();
		auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(curr_time - start_time);
		int diff = time_diff.count();	// diff is the seconds elapsed from the start
		int half_sec_passed = 0;
		if(last+500 < diff){
			half_sec_passed = 1;
			last = diff;
			sec_counter += 0.5;
		}

		if((first)||(last_sec < (int)sec_counter && (int)sec_counter % 4 == 0)){	// add a word every 4 seconds
			
			//mvwprintw(disp_win, 5, 5, "HRERE");
			//wrefresh(disp_win);
			AddWord(dict, disp_dict, wmap);
			last_sec = (int)sec_counter;
			first = false;
		}
		if(half_sec_passed){
			int x,y;
			getyx(inp_win, y, x);
		       	DisplayWords(disp_win, disp_dict, wmap);
			wmove(inp_win, y, x);
		}
		if(kbhit(inp_win)){	//a key was pressed
			char c = wgetch(inp_win);	// keep checking for a win. winning == user enetred 'sup'
			if(c == 8){
				if(s.length() == 0) continue;
				s.erase(s.length()-1,1);
				wdelch(inp_win);
				wdelch(inp_win);
				continue;
			}
			
			else if(c < 'A' || c > 'z' || (c < 'a' && c > 'Z')) {
				s = "";
				wclear(inp_win);
				continue;
			}
			s += c;
			if(disp_dict.count(s) != 0){
				RemoveWord(disp_dict, s);
				s = "";
				wclear(inp_win);
				//mvwprintw(disp_win, 0, 0, "You win!");
			}	
		}	
	
	}
}
void AddWord(set<string>& dict, set<string>& disp_dict, unordered_map<string, pair<int,int>>& wmap){
	if(dict.size() == 0) return;
	string word(*(dict.begin()));
	dict.erase(word);
	disp_dict.insert(word);
	random_device rd;
	int yloc = (int)(rd()%(LINES*4/5-2)+1);	
	pair<int,int> coords = make_pair(0,yloc);	// (x,y)
	wmap[word] = coords;

}
void RemoveWord(set<string>& disp_dict, string& w){
	disp_dict.erase(w);

}
void DisplayWords(WINDOW* disp_win, set<string>& disp_dict, unordered_map<string, pair<int,int>>& wmap){
	wclear(disp_win);
	for(auto w : disp_dict){
		pair<int,int> coords = wmap[w];
		char word[MAX_WORD_SIZE] = {0};
		for(int i=0;i<w.length();i++){
			word[i] = w[i];
		}
		mvwprintw(disp_win, coords.second, coords.first+1, word);	//advance word to the right
		wmap[w] = make_pair(coords.first+1, coords.second);
	
	}
	wrefresh(disp_win);

}
int kbhit(WINDOW* inp_win)
{
	int ch = wgetch(inp_win);
	if (ch != ERR) {
		ungetch(ch);
        	return 1;
	} else {
		return 0;
	}
}


