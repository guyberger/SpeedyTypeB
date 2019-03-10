#include <ncurses.h>
#include <string>
#include <vector>
#include <set>
#include <chrono>
#include <unordered_map>
using namespace std;

#define MAX_WORD_SIZE 100

WINDOW * MakeWin(int, int, int, int, bool);
void play(WINDOW* inp_win, WINDOW* disp_win, set<string>& dict);
void DisplayWords(WINDOW* disp_win, set<string>& disp_dict, unordered_map<string, pair<int,int>> wmap, int diff);
void AddWord(set<string>& dict, set<string>& disp_dict, unordered_map<string, pair<int,int>>& wmap);
void RemoveWord(set<string>& disp_dict, string& w);
int kbhit(WINDOW*);
int main(){

	initscr();
	cbreak();
	nodelay(stdscr, TRUE);

	// Define windows and variales
	

	int startx = 0, starty = LINES - 15;
	string mydict[] = {"box"};
	set<string> dict(mydict, mydict + sizeof(mydict)/sizeof(string));

	WINDOW * inp_win_box = MakeWin(LINES - starty, COLS, starty, startx, 1);
	WINDOW * inp_win = MakeWin(LINES - starty - 2, COLS - 4, starty+1, startx + 3, 0);
	WINDOW * disp_win_box = MakeWin(starty, COLS, 0, 0, 1);
	WINDOW * disp_win = MakeWin(starty-2, COLS-2, 1, 1, 0);
	mvwprintw(inp_win_box, 1, 1, ">");
	
	wrefresh(inp_win_box);

	// Actual Gameplay

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
	int count_sec = 0;

	string s("");


	while(1){	// main loop to play game...
		auto curr_time = std::chrono::high_resolution_clock::now();
		auto time_diff = std::chrono::duration_cast<std::chrono::seconds>(curr_time - start_time);
		int diff = time_diff.count();
		//mvwprintw(disp_win, 5, 5, "TIME IS %d", diff);
		//wrefresh(disp_win);
		//return;
		if(diff >= count_sec && diff % 4 == 0){
			count_sec++;
			//mvwprintw(disp_win, 5, 5, "HRERE");
			//wrefresh(disp_win);
			AddWord(dict, disp_dict, wmap);
		}
		DisplayWords(disp_win, disp_dict, wmap, count_sec);
		if(kbhit(inp_win)){	//a key was pressed
			char c = wgetch(inp_win);	// keep checking for a win. winning == user enetred 'sup'
			if(c == 8){
				if(s.length() == 0) continue;
				s.erase(s.length()-1,1);
				wdelch(inp_win);
				wdelch(inp_win);
				continue;
			}
			s += c;
			if(disp_dict.count(s) != 0){
				RemoveWord(disp_dict, s);
				s = "";
				wclear(inp_win);
				//mvwprintw(disp_win, 0, 0, "You win!");
				wrefresh(disp_win);
			}	
		}	
	
	}
}
void AddWord(set<string>& dict, set<string>& disp_dict, unordered_map<string, pair<int,int>>& wmap){
	if(dict.size() == 0) return;
	string word(*(dict.begin()));
	dict.erase(word);
	disp_dict.insert(word);
	pair<int,int> coords = make_pair(0,0);	// (x,y)
	wmap[word] = coords;

}
void RemoveWord(set<string>& disp_dict, string& w){
	disp_dict.erase(w);

}
void DisplayWords(WINDOW* disp_win, set<string>& disp_dict, unordered_map<string, pair<int,int>> wmap, int diff){
	wclear(disp_win);
	diff = diff%2;
	for(auto w : disp_dict){
		pair<int,int> coords = wmap[w];
		char word[MAX_WORD_SIZE] = {0};
		for(int i=0;i<w.length();i++){
			word[i] = w[i];
		}
		mvwprintw(disp_win, coords.second+diff, coords.first, word);	//advance word to the right
		wmap[w] = make_pair(coords.first, coords.second+diff);
	
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


