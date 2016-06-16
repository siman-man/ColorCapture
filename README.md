Problem Statement
問題文
Color Capture is a two-player game played on a square board of D x D pixels. Each pixel of the board is assigned a color. 
ColorCaptureはDxDの広さで行われる2人プレイのボードゲームです。 各ピクセルのセルには色が設定されています。
Two players start with one pixel each, player 0 in upper left corner of the board, player 1 in lower right corner of 
各プレイヤーはいずれかのピクセルから始まります。プレイヤー0は左上から始まり、プレイヤー1は右下から始まります。
the board. Each player controls their starting pixel and all pixels of the same color connected to their starting pixel 
各プレイヤーは開始セルと接続されている同じセルの操作を行います。

with a path of vertically or horizontally adjacent pixels of the same color. 
上下右左で同じ色で接続されている部分は経路としてみなされます。

Players take turns changing color of the area of the board they control; on their turn the player changes color of all
各プレイヤーはボード上の色を変化させることが出来ます。                             彼らは全ての色を変えることが出来ます。
pixels they control, and thus possibly takes new areas under control (areas of new color which were adjacent to areas of 
                     全てのボードを制御出来る。
old color controlled by the player). The player can't change the color of his area to its current color or the 
                                      プレイヤーは相手の色のピクセルは変化させることが出来ません。
opponent's area current color. The pixels which are entirely surrounded with pixels controlled by one of the players 
                                ピクセルの周りが自分の色で囲われた時に、そのピクセルは制御下されます。
are considered to be controlled by that player (and change their color immediately), since the other player will 
そうなると、他のプレイヤーはそのピクセルを制御できません。
never be able to reach them. 

The game is over when each pixel is controlled by one of the players, or when a total of D2 turns have been made. 
プレイヤーが全てのセルを制御下におくか、D2 turn経過するとゲーム終了です。

You will play this game against an AI opponent. AI will pick new color randomly from the colors of pixels which are 
あなたは敵AIと対戦することが出来ます。 AIはランダムで色を選択します。
adjacent to the area it controls. If no such color provides a valid move (for example, when the area controlled by 
もしそれが、
the AI is surrounded by pixels of opponent's current color), AI will pick a random valid color.

Implementation
Your code must implement one method makeTurn(vector <string> board, int timeLeftMs). board describes current state 
of the board: board[i][j] gives the color of pixel in row i and column j denoted by uppercase letters of English alphabet.
timeLeftMs gives the amount of time left of the initial timeout, based on tester measurements. 
The method must return an integer index of the color you want to switch the color of your area to 
(without letter encoding: return 0 to switch to color 'A', 1 to switch to 'B' etc.) Your code will play for player 0, 
and thus will do the first turn. 

For example, in example 0 valid return values during your first move are 0 and 2 (for colors 'A' and 'C'). 
Colors 1 ('B') and 3 ('D') are invalid moves, because 'B' is your current color, and 'D' is your opponent's current color.

Scoring
The score for a test case will be calculated as follows. If your code returned an invalid color or exceeded time limit, 
your score for that test case will be 0. Otherwise, your score will be the number of pixels controlled by your player 
in the end of the game, divided by the total number of pixels on the board (D2). Your total score will be the arithmetic
average of scores for each test case, multiplied by 106.

Tools
An offline tester is available here. You can use it to test/debug your solution locally. You can also check its 
source code for exact implementation of test case generation, game simulation and score calculation. 
That page also contains links to useful information and sample solutions in several languages.
 
Definition
      
Class:  ColorCapture
Method: makeTurn
Parameters: vector <string>, int
Returns:  int
Method signature: int makeTurn(vector <string> board, int timeLeftMs)
(be sure your method is public)
    
 
Notes
- The time limit is 10 seconds per test case (this includes only the time spent in your code). The memory limit is 1024 megabytes.
- There is no explicit code size limit. The implicit source code size limit is around 1 MB (it is not advisable to submit codes of size close to that or larger). Once your code is compiled, the binary size should not exceed 1 MB.
- The compilation time limit is 30 seconds. You can find information about compilers that we use and compilation options here.
- There are 5 example test cases and 20 full submission (provisional) test cases.
- The match is rated.



# 与えられているもの

盤面のサイズ、色

# 戦略考察

* ひたすら領地が大きくなる色を取得