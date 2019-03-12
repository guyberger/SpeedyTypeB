#include "TypoMania.h"
using namespace std;
/*
int kbhit(WINDOW* inpw);
char init(GameMode& mode);
char printMenu();
char printDiffc();
void drawLine(int y, int x1, int x2);
void play(unordered_map<string, WINDOW*> windows, GameMode mode);
void addWord(GameMode& mode, set<string>& disp_dict, unordered_map<string, pair<int,int>>& lmap);
void removeWord(set<string>& disp_dict, string& w);
void displayWords(WINDOW* dispw, set<string>& disp_dict, unordered_map<string, pair<int,int>>& lmap, int move);*/
int main(){

	// initialize gmae mode and difficulty
	
	GameMode mode;
	char m_sel = init(mode);
	switch(m_sel){
		case '1':
			mode.init("RickMortyDict.txt");
			break;
		default: 
			cerr << "ERROR" << endl;
			break;
	}
	
	// initialize windows

	unordered_map<string, WINDOW*> windows;
	windows["display"] = newwin(LINES*4/5, COLS, 0, 0);
	windows["input"] = newwin(LINES - LINES*4/5 - 1, COLS/2, LINES*4/5+2, 2);
	windows["menu"] = newwin(LINES - LINES*4/5 - 1, COLS - COLS/2, LINES*4/5+2, COLS/2+1);
	mvprintw(LINES*4/5+2, 0, ">");
	drawLine(LINES*4/5+1, 0, COLS);
	refresh();
	wrefresh(windows["input"]);
	wmove(windows["input"], 0, 0);

	// gameplay
	
	nodelay(windows["input"], true);	// getch wont be a blocking call
	play(windows, mode);
	getch();
	endwin();


}
char init(GameMode& mode){
	initscr();
	cbreak();
	noecho();
	char c = printMenu();
	clear();
	char diffc = printDiffc();
	if(diffc == '1') echo();
	mode.setDiffc(diffc);
	return c;
}

char printMenu(){
	mvprintw(LINES/2-1, (COLS-17)/2, "Choose a mode to play");
	mvprintw(LINES/2, (COLS-17)/2, "[1] Rick and Morty");
	refresh();
	char c = getch();
	while(c < '1' || c > ('0' + MODES)){
		c = getch();
	}
	return c;
}
char printDiffc(){
	mvprintw(LINES/2-1, (COLS-17)/2, "Choose difficulty");
	mvprintw(LINES/2, (COLS-17)/2, "[1] Regular");
	mvprintw(LINES/2+1, (COLS-17)/2, "[2] God mode");
	refresh();
	char c = getch();
	while(c < '1' || c > ('0' + DIFFC)){
		c = getch();
	}
	return c;
}
void drawLine(int y, int x1, int x2){
	while(x1 <= x2){
		mvprintw(y, x1, "-");
		x1++;
	}

}
void play(unordered_map<string, WINDOW*> windows, GameMode& mode){
	WINDOW* inpw = windows["input"];
	WINDOW* dispw = windows["display"];
	WINDOW* menuw = windows["menu"];

	unordered_map<string, pair<int,int>> lmap;	//locations of words
	auto start_time = chrono::high_resolution_clock::now();
	set<string> disp_dict;
	string s("");
	int sec_count = 0, last_sec = 0;
	bool first = true;
	bool move_words = false;
	double div = 1000/mode.getSpeed(), fact = 1;	// counting seconds
	
	while(1){
		auto curr_time = chrono::high_resolution_clock::now();
		auto time_diff = chrono::duration_cast<chrono::milliseconds>(curr_time - start_time);
		int diff = time_diff.count();

		if(1000 <= diff){	// one second
			start_time = curr_time;
			sec_count++;
		}
		if(fact*mode.getSpeed() <= diff){
			fact++;
			if(fact > div) fact = 1;
			move_words = true;
		}
		if(first || (last_sec < sec_count && sec_count % mode.getGap() == 0)){
			addWord(mode, disp_dict, lmap);
			last_sec = sec_count;
			first = false;
		}
		if(move_words){
			int x, y;
			getyx(inpw, y, x);
			displayWords(dispw, disp_dict, lmap, 1);
			wmove(inpw, y ,x);
			move_words = false;
		}
		if(kbhit(inpw)){
			char c = wgetch(inpw);
			if(c == 8){	// backspace
				if(s.length() == 0) continue;
				s.erase(s.length()-1, 1);
				wdelch(inpw); wdelch(inpw);
				continue;
			}
			else if(c < 'A' || c > 'z' || (c < 'a' && c > 'Z')){
				s = "";
				wclear(inpw);
				continue;
			}
			s += c;
			if(disp_dict.count(s) != 0){
				removeWord(disp_dict, s);
				s = "";
				wclear(inpw);
				int x,y;
				getyx(inpw, y, x);
				displayWords(dispw, disp_dict, lmap, 0);
				if(disp_dict.size()==0) wrefresh(dispw);
				wmove(inpw, y, x);
			}
		}
		
		
	}

}
void addWord(GameMode& mode, set<string>& disp_dict, unordered_map<string, pair<int,int>>& lmap){
	if(mode.isEmptyDict()) return;
	string word(mode.extractRandomWord());
	disp_dict.insert(word);
	random_device rd;
	int yloc = (int)(rd()%(LINES*4/5));
	pair<int,int> coords = make_pair(0, yloc);
	lmap[word] = coords;
}
void removeWord(set<string>& disp_dict, string& w){
	disp_dict.erase(w);
}
void displayWords(WINDOW* dispw, set<string>& disp_dict, unordered_map<string, pair<int,int>>& lmap, int move){
	wclear(dispw);
	if(disp_dict.size() == 0) return;
	for(auto w : disp_dict){
		pair<int,int> coords = lmap[w];
		char word[MAX_WORD_SIZE] = {0};
		for(int i=0;i<w.length();i++){
			word[i] = w[i];
		}
		mvwprintw(dispw, coords.second, coords.first+move, word);
		lmap[w] = make_pair(coords.first+move, coords.second);
	}
	wrefresh(dispw);
}
int kbhit(WINDOW* inpw){
	int ch = wgetch(inpw);
	if(ch != ERR){
		ungetch(ch);
		return 1;
	}
	else return 0;
}



