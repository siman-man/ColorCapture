#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string.h>
#include <sstream>
#include <vector>
#include <set>

using namespace std;

typedef long long ll;

const int MAX_HEIGHT = 102;
const int MAX_WIDTH = 102;

const int DY[4] = {-1, 0, 0, 1};
const int DX[4] = { 0,-1, 1, 0};

const int NONE = -1;
const int MY = 0;
const int ENEMY = 1;

bool g_canGetPixel[MAX_HEIGHT][MAX_WIDTH];
int g_control[MAX_HEIGHT][MAX_WIDTH];
ll g_searchStamp[MAX_HEIGHT][MAX_WIDTH];

int g_turn = 0;
int leftTime;
int HEIGHT;
int WIDTH;
int g_stamp;
int g_maxColor;
vector< vector<char> > g_board;
map<char, int> g_getCount;

const char WALL = 'W';

struct Coord {
  int y;
  int x;

  Coord(int y = -1, int x = -1) {
    this->y = y;
    this->x = x;
  }
};

char g_myColor;
char g_enemyColor;

vector<Coord> g_myPixelList;

class ColorCapture {
  public:
    ColorCapture() {
      srand(12345);
    }

    int color2int(char c) {
      return (int)(c - 'A');
    }

    char int2color(int c) {
      return 'A' + c;
    }

    void setup(vector<string> &board) {
      HEIGHT = board.size();
      WIDTH = board[0].size();
      g_board = vector< vector<char> >(HEIGHT+2, vector<char>(WIDTH+2));
      g_stamp = 0;
      g_maxColor = 0;

      memset(g_control, NONE, sizeof(g_control));
      memset(g_searchStamp, -1, sizeof(g_searchStamp));

      g_control[1][1] = MY;
      g_control[HEIGHT][WIDTH] = ENEMY;

      g_myPixelList.push_back(Coord(1, 1));

      for (int y = 0; y <= HEIGHT+1; y++) {
        for (int x = 0; x <= WIDTH+1; x++) {
          if (y == 0 || y == HEIGHT+1 || x == 0 || x == WIDTH+1) {
            g_board[y][x] = WALL;
          } else {
            g_board[y][x] = board[y-1][x-1];
            g_maxColor = max(g_maxColor, color2int(board[y-1][x-1]));
          }
        }
      }

      fprintf(stderr,"D = %d, C = %d\n", HEIGHT, g_maxColor);
    }

    void update(vector<string> &board) {
      memset(g_canGetPixel, false, sizeof(g_canGetPixel));

      for (int y = 1; y <= HEIGHT; y++) {
        for (int x = 1; x <= WIDTH; x++) {
          g_board[y][x] = board[y-1][x-1];
        }
      }

      g_myColor = g_board[1][1];
      g_enemyColor = g_board[HEIGHT][WIDTH];

      updateCanGetPixelField();
      updateGetCountPixel();

      g_stamp++;
      updateControlField(1, 1, g_myColor, MY);
      g_stamp++;
      updateControlField(HEIGHT, WIDTH, g_enemyColor, ENEMY);
    }

    void updateCanGetPixelField() {
      int mpsize = g_myPixelList.size();
      for (int i = 0; i < mpsize; i++) {
        Coord *coord = &g_myPixelList[i];

        for (int j = 0; j < 4; j++) {
          int ny = coord->y + DY[j];
          int nx = coord->x + DX[j];

          if (isWall(ny, nx)) continue;
          if (g_control[ny][nx] == NONE) {
            g_canGetPixel[ny][nx] = true;
          }
        }
      }
    }

    void updateControlField(int y, int x, int color, int id) {
      g_control[y][x] = id;
      g_searchStamp[y][x] = g_stamp;

      for (int i = 0; i < 4; i++) {
        int ny = y + DY[i];
        int nx = x + DX[i];

        if (isWall(ny, nx)) continue;
        if (g_board[ny][nx] == color && g_searchStamp[ny][nx] != g_stamp) {
          updateControlField(ny, nx, color, id);
        }
      }
    }

    void updateGetCountPixel() {
      g_stamp++;
      g_getCount.clear();

      for (int y = 1; y <= HEIGHT; y++) {
        for (int x = 1; x <= WIDTH; x++) {
          if (g_canGetPixel[y][x] && g_searchStamp[y][x] != g_stamp) {
            search(y, x);
          }
        }
      }
    }

    void changeColor(char color) {
      g_stamp++;

      for (int y = 1; y <= HEIGHT; y++) {
        for (int x = 1; x <= WIDTH; x++) {
          if (g_canGetPixel[y][x] && g_board[y][x] == color) {
            g_control[y][x] = MY;
            g_myPixelList.push_back(Coord(y, x));
          }
        }
      }
    }

    void search(int y, int x) {
      char color = g_board[y][x];
      g_getCount[color]++;
      g_canGetPixel[y][x] = true;
      g_searchStamp[y][x] = g_stamp;

      for (int i = 0; i < 4; i++) {
        int ny = y + DY[i];
        int nx = x + DX[i];

        if (isWall(ny, nx)) continue;
        if (g_board[ny][nx] == color && g_searchStamp[ny][nx] != g_stamp) {
          search(ny, nx);
        }
      }
    }

    int makeTurn(vector<string> board, int timeLeftMs) {
      g_turn++;
      showField();

      if (g_turn == 1) {
        setup(board);
      }

      update(board);
      int bestColor = selectBestColor();
      changeColor(int2color(bestColor));

      return bestColor;
    }

    int selectBestColor() {
      int maxCount = -1;
      int bestColor = -1;

      for (int i = 0; i <= g_maxColor; i++) {
        char color = int2color(i);
        int cnt = g_getCount[color];

        if (canChange(color) && maxCount < cnt) {
          maxCount = cnt;
          bestColor = i;
        }
      }
      fprintf(stderr,"best color = %d, count = %d\n", bestColor, maxCount);

      return bestColor;
    }

    void showField() {
      for (int y = 1; y <= HEIGHT; y++) {
        for (int x = 1; x <= WIDTH; x++) {
          if (g_control[y][x] == NONE) {
            fprintf(stderr,"x");
          } else {
            fprintf(stderr,"%d",g_control[y][x]);
          }
        }
        fprintf(stderr,"\n");
      }
    }

    inline bool canChange(char color) {
      return (color != g_myColor && color != g_enemyColor);
    }

    inline bool isWall(int y, int x) {
      return g_board[y][x] == WALL;
    }
};

// -------8<------- end of solution submitted to the website -------8<-------
template<class T> void getVector(vector<T>& v) {
  for (int i = 0; i < v.size(); ++i)
    cin >> v[i];
}

int main() {
  ColorCapture cc;
  int N, timeLeftMs;
  while (true) {cin >> N;
    vector<string> board(N);
    getVector(board);
    cin >> timeLeftMs;
    int ret = cc.makeTurn(board, timeLeftMs);
    cout << ret << endl;
    cout.flush();}
}
