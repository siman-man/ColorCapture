#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <queue>
#include <string.h>
#include <sstream>
#include <vector>
#include <cassert>
#include <set>

using namespace std;

typedef long long ll;

const int MAX_HEIGHT = 102;
const int MAX_WIDTH = 102;

const int DY[4] = {-1, 0, 0, 1};
const int DX[4] = { 0,-1, 1, 0};

const char NONE = -1;
const char MY = 0;
const char ENEMY = 1;

int BEAM_WIDTH = 40;

ll g_searchStamp[MAX_HEIGHT][MAX_WIDTH];

unsigned long long xor128(){
  static unsigned long long rx=123456789, ry=362436069, rz=521288629, rw=88675123;
  unsigned long long rt = (rx ^ (rx<<11));
  rx=ry; ry=rz; rz=rw;
  return (rw=(rw^(rw>>19))^(rt^(rt>>8)));
}

int g_turn = 0;
int HEIGHT;
int WIDTH;
int g_stamp;
int g_maxColor;
int g_remainCount;
bool g_warning;
bool g_stopper;
char g_enemyBestColor;
char g_board[MAX_HEIGHT][MAX_WIDTH];
char g_tempBoard[MAX_HEIGHT][MAX_WIDTH];
char g_control[MAX_HEIGHT][MAX_WIDTH];
char g_tempControl[MAX_HEIGHT][MAX_WIDTH];

const char WALL = '@';

struct Coord {
  int y;
  int x;

  Coord(int y = -1, int x = -1) {
    this->y = y;
    this->x = x;
  }
};

struct Node {
  vector<char> colors;
  double score;
  char board[MAX_HEIGHT][MAX_WIDTH];
  char control[MAX_HEIGHT][MAX_WIDTH];

  Node() {
    this->score = 0.0;
  }

  bool operator >(const Node &e) const{
    return score < e.score;
  }    
};

char g_myColor;
char g_enemyColor;

Coord g_enemyEdgeA;
Coord g_enemyEdgeB;

int g_myEdgeY;
int g_myEdgeX;
int g_enemyEdgeY;
int g_enemyEdgeX;

class ColorCapture {
  public:
    int color2int(char c) {
      return (int)(c - 'A');
    }

    char int2color(int c) {
      return 'A' + c;
    }

    void setup(vector<string> &board) {
      HEIGHT = board.size();
      WIDTH = board[0].size();
      memset(g_board, 0, sizeof(g_board));
      memset(g_control, NONE, sizeof(g_control));
      g_stamp = 0;
      g_maxColor = 0;
      g_warning = false;
      g_stopper = false;

      memset(g_searchStamp, -1, sizeof(g_searchStamp));

      g_control[1][1] = MY;
      g_control[HEIGHT][WIDTH] = ENEMY;

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
      g_remainCount = HEIGHT * WIDTH;
      for (int y = 1; y <= HEIGHT; y++) {
        for (int x = 1; x <= WIDTH; x++) {
          g_board[y][x] = board[y-1][x-1];
        }
      }

      g_myColor = g_board[1][1];
      g_enemyColor = g_board[HEIGHT][WIDTH];

      g_enemyEdgeA = Coord(HEIGHT, WIDTH);
      g_enemyEdgeB = Coord(HEIGHT, WIDTH);

      g_myEdgeY = 0;
      g_myEdgeX = 0;
      g_enemyEdgeY = HEIGHT;
      g_enemyEdgeX = WIDTH;

      g_stamp++;
      updateControlField(1, 1, g_myColor, MY);
      g_stamp++;
      updateControlField(HEIGHT, WIDTH, g_enemyColor, ENEMY);

      if (g_myEdgeY > g_enemyEdgeY || g_myEdgeX > g_enemyEdgeX) {
        g_stopper = true;
      }
    }

    void updateControlField(int y, int x, char color, int id) {
      g_control[y][x] = id;
      g_searchStamp[y][x] = g_stamp;
      g_remainCount--;

      if (id == MY) {
        g_myEdgeY = max(g_myEdgeY, y);
        g_myEdgeX = max(g_myEdgeX, x);
      } else {
        g_enemyEdgeY = min(g_enemyEdgeY, y);
        g_enemyEdgeX = min(g_enemyEdgeX, x);
      }

      for (int i = 0; i < 4; i++) {
        int ny = y + DY[i];
        int nx = x + DX[i];

        if (isWall(ny, nx)) continue;
        if (g_searchStamp[ny][nx] == g_stamp) continue;
        if (1 - g_control[ny][nx] == id) continue;

        if (g_board[ny][nx] == color || g_control[ny][nx] == id) {
          updateControlField(ny, nx, color, id);
        } else if (id == ENEMY) {
          if (g_enemyEdgeA.y > ny) {
            g_enemyEdgeA = Coord(ny, nx);
          }
          if (g_enemyEdgeB.x > nx) {
            g_enemyEdgeB = Coord(ny, nx);
          }
        }
      }
    }

    int makeTurn(vector<string> board, int timeLeftMs) {
      if (timeLeftMs < 1500 && !g_warning) {
        fprintf(stderr,"turn %d: Warning\n", g_turn);
        g_warning = true;
      }
      g_turn++;

      if (g_turn == 1) {
        setup(board);
      }

      update(board);

      int bestColor = selectBeamColor();

      //fprintf(stderr,"turn %d: my color = %c, enemy color = %c, best color = %d\n", g_turn, g_myColor, g_enemyColor, bestColor);
      cerr.flush();

      return bestColor;
    }

    int selectBeamColor() {
      queue<Node> que;
      Node root;
      memcpy(root.board, g_board, sizeof(g_board));
      memcpy(root.control, g_control, sizeof(g_control));
      que.push(root);
      save();

      int depthLimit = (g_warning)? 1 : 4;

      if (!g_warning && g_maxColor > 4) {
        depthLimit += g_maxColor/6;
      }
      if (!g_warning && g_maxColor <= 4) {
        depthLimit = 2;
      }

      for (int depth = 0; depth < depthLimit; depth++) {
        priority_queue<Node, vector<Node>, greater<Node> > pque;

        while (!que.empty()) {
          Node node = que.front(); que.pop();

          for (int c = 0; c <= g_maxColor; c++) {
            char color = int2color(c);
            if (depth == 0 && !canChangeColor(color)) continue;
            if (depth > 0 && node.colors[depth-1] == color) continue;

            memcpy(g_board, node.board, sizeof(node.board));
            memcpy(g_control, node.control, sizeof(node.control));

            g_stamp++;
            double score = erosion(1, 1, color);
            Node cand;
            cand.score = node.score + score;
            cand.colors = node.colors;
            cand.colors.push_back(color);
            memcpy(cand.board, g_board, sizeof(g_board));
            memcpy(cand.control, g_control, sizeof(g_control));

            pque.push(cand);
          }
        }

        for (int j = 0; j < BEAM_WIDTH && !pque.empty(); j++) {
          Node node = pque.top(); pque.pop();
          que.push(node);
        }
      }

      rollback();

      if (que.size() > 0) {
        Node bestNode = que.front();
        return color2int(bestNode.colors[0]);
      } else {
        return randomColor();
      }
    }

    int randomColor() {
      for (int i = 0; i <= g_maxColor; i++) {
        char color = int2color(i);
        if (canChangeColor(color)) return i;
      }

      return 0;
    }

    void save() {
      memcpy(g_tempBoard, g_board, sizeof(g_board));
      memcpy(g_tempControl, g_control, sizeof(g_control));
    }

    void rollback() {
      memcpy(g_board, g_tempBoard, sizeof(g_tempBoard));
      memcpy(g_control, g_tempControl, sizeof(g_tempControl));
    }

    int erosion(int y, int x, char color) {
      int cnt = 0;

      if (g_control[y][x] == NONE && g_board[y][x] == color) {
        int dA = calcDist(g_enemyEdgeA.y, g_enemyEdgeA.x, y, x);
        int dB = calcDist(g_enemyEdgeB.y, g_enemyEdgeB.x, y, x);

        if (g_stopper) {
        //if (g_turn < HEIGHT*1.1) {
          cnt = max(y,x) + (HEIGHT-min(dA, dB));
        } else {
          cnt = 100 * min(y,x);
        }
      }

      g_searchStamp[y][x] = g_stamp;
      g_control[y][x] = MY;
      g_board[y][x] = color;

      for (int i = 0; i < 4; i++) {
        int ny = y + DY[i];
        int nx = x + DX[i];
        if (isWall(ny, nx)) continue;
        if (g_searchStamp[ny][nx] == g_stamp) continue;
        if (g_control[ny][nx] == ENEMY) continue;

        if (g_control[ny][nx] == MY || g_board[ny][nx] == color) {
          if (g_maxColor <= 5) {
            cnt += erosion(ny, nx, color);
          } else {
            cnt = max(cnt, erosion(ny, nx, color));
          }
        }
      }

      return cnt;
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

    inline bool canChangeColor(char color) {
      return (color != g_myColor && color != g_enemyColor);
    }

    inline bool isWall(int y, int x) {
      return g_board[y][x] == WALL;
    }

    int calcDist(int y1, int x1, int y2, int x2) {
      return abs(y1 - y2) + abs(x1 - x2);
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
