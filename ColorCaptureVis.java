import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.io.*;
import java.lang.Math.*;
import java.security.*;
import java.util.*;
import javax.swing.*;

class ColorCaptureAI {
    SecureRandom r1;
    int player;
    public ColorCaptureAI(int s, int p) {
        player = p;
    try {
        r1 = SecureRandom.getInstance("SHA1PRNG");
        r1.setSeed(s);
    }
    catch (NoSuchAlgorithmException e) { }
    }
    int getColor(char c) {
        return (int)(c - 'A');
    }
    int makeTurn(String[] board, int timeLeftMs) throws NoSuchAlgorithmException {
        int D = board.length;
        // get list of all colors present on the board adjacent to area controlled by player
        // bfs starting with corner cell already controlled by player
        int[] r = new int[D * D];
        int[] c = new int[D * D];
        boolean[][] vis = new boolean[D][D];
        int n = 0;

        // starting point depends on the player
        int r0, c0;
        r0 = c0 = (player == 0 ? 0 : D - 1);
        char playerColor = board[r0].charAt(c0);

        vis[r0][c0] = true;
        r[n] = r0;
        c[n] = c0;
        ++n;

        HashSet<Integer> colors = new HashSet<>();
        int maxC = 4;
        final int[] dr = {0, 1, 0, -1};
        final int[] dc = {-1, 0, 1, 0};
        for (int ind = 0; ind < n; ++ind)
        for (int d = 0; d < 4; ++d) {
            int newr = r[ind] + dr[d];
            int newc = c[ind] + dc[d];
            if (newr < 0 || newc < 0 || newr >= D || newc >= D)
                continue;
            if (!vis[newr][newc]) {
                if (board[newr].charAt(newc) == playerColor) {
                    vis[newr][newc] = true;
                    r[n] = newr;
                    c[n] = newc;
                    ++n;
                } else {
                    // adjacent to one of player's cells but of different color - keep
                    int col = getColor(board[newr].charAt(newc));
                    colors.add(new Integer(col));
                    if (col > maxC)
                        maxC = col;
                }
            }
        }

        // can't switch to colors currently used by players
        int col0 = getColor(board[0].charAt(0));
        int col1 = getColor(board[D - 1].charAt(D - 1));
        colors.remove(new Integer(col0));
        colors.remove(new Integer(col1));

        // if there are any other colors available, switch to one of them at random
        if (!colors.isEmpty()) {
            Integer[] cols = colors.toArray(new Integer[0]);
            return cols[r1.nextInt(cols.length)];
        }
        // if there are no good moves, do something
        int cret = r1.nextInt(maxC - 1);
        if (cret >= Math.min(col0, col1))
            ++cret;
        if (cret >= Math.max(col0, col1))
            ++cret;
        return cret;
    }
}

public class ColorCaptureVis {
    static int maxSize = 100, minSize = 10;
    static int maxColors = 16, minColors = 4;

    final int[] dr = {0, 1, 0, -1};
    final int[] dc = {-1, 0, 1, 0};

    int D;              // dimension of the board in pixels
    int C;              // number of colors in the game
    int[][] map;        // which color each region is
    int[][] control;    // which player controls each region (if any)
    int[] area;         // area controlled by players 0 and 1, respectively

    SecureRandom r1;

    int turnN;
    // -----------------------------------------
    char getChar(int color) {
        return (char)('A' + color);
    }
    // -----------------------------------------
    String[] getBoard() {
        String[] board = new String[D];
        for (int i = 0; i < D; ++i) {
            StringBuffer sb = new StringBuffer();
            for (int j = 0; j < D; ++j)
                sb.append(getChar(map[i][j]));
            board[i] = sb.toString();
        }
        return board;
    }
    // -----------------------------------------
    void updateArea(int player) {
        // step 1. update area controlled by the player by virtue of being connected to his color
        // (or previously being under his control)
        int playercolor = player == 0 ? map[0][0] : map[D - 1][D - 1];
        area[player] = 0;
        // bfs starting with cells already controlled by that player
        int[] r = new int[D * D];
        int[] c = new int[D * D];
        boolean[][] vis = new boolean[D][D];
        int n = 0;
        for (int i = 0; i < D; ++i)
        for (int j = 0; j < D; ++j)
            if (control[i][j] == player) {
                vis[i][j] = true;
                r[n] = i;
                c[n] = j;
                ++n;
                ++area[player];
            }
        for (int ind = 0; ind < n; ++ind)
            for (int d = 0; d < 4; ++d) {
                int newr = r[ind] + dr[d];
                int newc = c[ind] + dc[d];
                if (newr < 0 || newc < 0 || newr >= D || newc >= D)
                    continue;
                if (!vis[newr][newc] && map[newr][newc] == playercolor) {
                    control[newr][newc] = player;
                    vis[newr][newc] = true;
                    r[n] = newr;
                    c[n] = newc;
                    ++n;
                    ++area[player];
                }
            }

        // step 2. find areas not reachable by the other player and assign them to controlled by this player
        // based on controlled areas without respect to current colors
        vis = new boolean[D][D];
        n = 0;
        for (int i = 0; i < D; ++i)
        for (int j = 0; j < D; ++j)
            if (control[i][j] == 1 - player) {
                vis[i][j] = true;
                r[n] = i;
                c[n] = j;
                ++n;
            }
        // bfs on uncontrolled cells
        for (int ind = 0; ind < n; ++ind)
            for (int d = 0; d < 4; ++d) {
                int newr = r[ind] + dr[d];
                int newc = c[ind] + dc[d];
                if (newr < 0 || newc < 0 || newr >= D || newc >= D)
                    continue;
                if (!vis[newr][newc] && control[newr][newc] == -1) {
                    vis[newr][newc] = true;
                    r[n] = newr;
                    c[n] = newc;
                    ++n;
                }
            }
        // mark all unowned unreachable cells as controlled by this player (and update their color as well)
        for (int i = 0; i < D; ++i)
        for (int j = 0; j < D; ++j)
            if (!vis[i][j] && control[i][j] == -1) {
                control[i][j] = player;
                map[i][j] = playercolor;
                ++area[player];
            }
    }
    // -----------------------------------------
    double getScore(int player) {
        // at any moment score is percentage of the board controlled by player
        return area[player] * 1.0 / (D * D);
    }
    // -----------------------------------------
    boolean endGame() {
        // * all area is controlled by one of the players
        // * D^2 turns have been done (safety net)
        return area[0] + area[1] == D * D || turnN == D * D + 1;
    }
    // -----------------------------------------
    void generate(String seedStr) {
    try {
        // generate test case
        r1 = SecureRandom.getInstance("SHA1PRNG");
        long seed = Long.parseLong(seedStr);
        r1.setSeed(seed);
        D = r1.nextInt(maxSize - minSize + 1) + minSize;
        C = r1.nextInt(maxColors - minColors + 1) + minColors;
        if (seed <= 3) {
            D = minSize * (int)seed;
            C = minColors + 2 * (int)(seed - 1);
        }
        if (seed == 4) {
            D = maxSize;
            C = maxColors;
        }

        // generate the map of cell colors
        map = new int[D][D];
        for (int i = 0; i < D; ++i)
        for (int j = 0; j < D; ++j) {
            map[i][j] = r1.nextInt(C);
        }
        // make sure opponents' colors are different initially
        while (map[D - 1][D - 1] == map[0][0]) {
            map[0][0] = r1.nextInt(C);
        }

        // set starting region controls
        control = new int[D][D];
        for (int i = 0; i < D; ++i) {
            Arrays.fill(control[i], -1);
        }
        control[0][0] = 0;
        control[D - 1][D - 1] = 1;

        // update areas controlled by players (can be greater than 1 from the start)
        area = new int[2];
        updateArea(0);
        updateArea(1);

        if (debug) {
            System.out.println("D = " + D);
            System.out.println("C = " + C);
            System.out.println("Starting board: ");
            String[] b = getBoard();
            for (String st : b) {
                System.out.println(st);
            }
            System.out.println("Player 0 area = " + area[0]);
            System.out.println("Player 1 area = " + area[1]);
        }

        if (vis) {
            W = D * SZ + 40 + SZ * 4;
            H = D * SZ + 40;
        }
    }
    catch (Exception e) {
        System.err.println("An exception occurred while generating test case.");
        e.printStackTrace();
    }
    }
    // -----------------------------------------
    public double runTest(String seed) {
    try {
        generate(seed);

        if (vis) {
            jf.setSize(W, H);
            jf.setVisible(true);
            draw();
        }

        ColorCaptureAI ai = new ColorCaptureAI(r1.nextInt(), 1);
        turnN = 1;
        int timeLeft = TL * 1000;          // keep track only of human player time, and only outside of manual mode
        while (!endGame()) {
            String[] board = getBoard();
            // decide which method to call - human or AI - based on turn #
            int player = (turnN - 1) % 2;
            long startTime = System.currentTimeMillis();
            int color = player == 0 ? makeTurn(board, timeLeft) : ai.makeTurn(board, timeLeft);
            if (player == 0 && !manual) {
                timeLeft -= (int)(System.currentTimeMillis() - startTime);
                if (timeLeft < 0) {
                    addFatalError("Turn #" + turnN + ": time limit exceeded.");
                    return 0.0;
                }
            }

            // check the returned color for validity
            if (color < 0 || color >= C) {
                addFatalError("Turn #" + turnN + ": return is not a valid color: " + color + ".");
                return 0.0;
            }
            // check that it's not equal to previous color or opponent's current color
            if (color == map[0][0] || color == map[D - 1][D - 1]) {
                addFatalError("Turn #" + turnN + ": return is the same as your previous color or opponent's color.");
                return 0.0;
            }

            // perform color switch and update controlled cells accordingly
            for (int i = 0; i < D; ++i)
            for (int j = 0; j < D; ++j)
                if (control[i][j] == player) {
                    map[i][j] = color;
                }
            updateArea(player);

            if (vis) {
                draw();
            }
            if (debug) {
                System.out.println("Turn #" + turnN + ": player 0 = " + area[0] + ", player 1 = " + area[1]);
            }
            ++turnN;
        }

        System.out.println((turnN - 1) + " turns done.");
        System.out.println("Time left: " + timeLeft + " ms.");
        return getScore(0);
    }
    catch (Exception e) {
        System.err.println("An exception occurred while trying to get your program's results.");
        e.printStackTrace();
        return 0;
    }
    }
// ------------- visualization part ------------
    JFrame jf;
    Vis v;
    static String exec;
    static boolean vis, manual, debug;
    static Process proc;
    static int del;
    InputStream is;
    OutputStream os;
    BufferedReader br;
    static int SZ, W, H, TL;
    volatile boolean manualReady;
    volatile int manualMove;
    // -----------------------------------------
    int makeTurn(String[] board, int timeLeftMs) throws IOException {
        if (!manual && proc != null) {
            StringBuffer sb = new StringBuffer();
            sb.append(board.length).append("\n");
            for (String b : board) {
                sb.append(b).append("\n");
            }
            sb.append(timeLeftMs).append("\n");
            os.write(sb.toString().getBytes());
            os.flush();
        }

        // and get the return value
        int ret = -1;
        if (manual) {
            manualReady = false;
            // get player move
            while (!manualReady)
            {   try { Thread.sleep(50);}
                catch (Exception e) { e.printStackTrace(); }
            }
            ret = manualMove;
        }
        else if (proc != null) {
            ret = Integer.parseInt(br.readLine());
        }
        return ret;
    }
    // -----------------------------------------
    void draw() {
        if (!vis) return;
        v.repaint();
        try { Thread.sleep(del); }
        catch (Exception e) { };
    }
    // -----------------------------------------
    BufferedImage cache;
    Color[] colors;
    int[] cs = {
        0xFFE300,
        0x0000FF,
        0xFF6800,
        0xA6BDD7,
        0xFF0000,
        0xCEA262,
        0x817066,
        0x007D34,
        0xF6869E,
        0x00538A,
        0x7F180D,
        0x93AA00,
        0xFF00FF,
        0x702E65,
        0xFFFFFF,
        0x000000};
    HashMap<Integer, Integer> colorMap;
    void GeneratePalette() {
        colors = new Color[C];
        colorMap = new HashMap<>();
        for (int i = 0; i < C; ++i) {
            colors[i] = new Color(cs[i]);
            colorMap.put(cs[i], i);
        }
    }
    // -----------------------------------------
    public class Vis extends JPanel implements MouseListener, WindowListener {
        public void paint(Graphics g) {
            if (colors == null) {
                GeneratePalette();
            }
            cache = new BufferedImage(W, H, BufferedImage.TYPE_INT_RGB);
            Graphics2D g2 = (Graphics2D)cache.getGraphics();
            // background
            g2.setColor(new Color(0xDDDDDD));
            g2.fillRect(0, 0, W, H);

            // current colors of the cells of the board (draw every cell)
            for (int i = 0; i < D; ++i)
                for (int j = 0; j < D; ++j) {
                    g2.setColor(colors[map[i][j]]);
                    g2.fillRect(j * SZ, i * SZ, SZ, SZ);
                }

            // palette of colors on the side to switch to in manual mode (in case there are no options available on the field)
            // in a 5 x 5 block of size SZ
            if (manual)
                for (int i = 0; i < C; ++i) {
                    g2.setColor(colors[i]);
                    g2.fillRect((D + 1 + i / 4) * SZ, (i % 4) * SZ, SZ, SZ);
                }
            g.drawImage(cache,0,0,W,H,null);
        }
        // -------------------------------------
        public Vis() {
            addMouseListener(this);
            jf.addWindowListener(this);
        }
        // -------------------------------------
        // WindowListener
        public void windowClosing(WindowEvent e){
            if (proc != null)
                try { proc.destroy(); }
                catch (Exception ex) { ex.printStackTrace(); }
            System.exit(0);
        }
        public void windowActivated(WindowEvent e) { }
        public void windowDeactivated(WindowEvent e) { }
        public void windowOpened(WindowEvent e) { }
        public void windowClosed(WindowEvent e) { }
        public void windowIconified(WindowEvent e) { }
        public void windowDeiconified(WindowEvent e) { }
        // -------------------------------------
        // MouseListener
        public void mouseClicked(MouseEvent e) {
            // for manual play
            if (!manual || manualReady) return;

            // click either on palette or on board to select move color

            int x = e.getX(), y = e.getY();
            int c = cache.getRGB(x, y) & 0xFFFFFF;
            if (colorMap.get(c) != null) {
                manualMove = colorMap.get(c);
                manualReady = true;
            }
        }
        public void mousePressed(MouseEvent e) { }
        public void mouseReleased(MouseEvent e) { }
        public void mouseEntered(MouseEvent e) { }
        public void mouseExited(MouseEvent e) { }
    }
    // -----------------------------------------
    public ColorCaptureVis(String seed) {
      try {
        if (vis)
        {   jf = new JFrame();
            v = new Vis();
            jf.getContentPane().add(v);
        }
        if (exec != null) {
            try {
                Runtime rt = Runtime.getRuntime();
                proc = rt.exec(exec);
                os = proc.getOutputStream();
                is = proc.getInputStream();
                br = new BufferedReader(new InputStreamReader(is));
                new ErrorReader(proc.getErrorStream()).start();
            } catch (Exception e) { e.printStackTrace(); }
        }
        System.out.println("Score = " + runTest(seed));
        if (proc != null)
            try { proc.destroy(); }
            catch (Exception e) { e.printStackTrace(); }
      }
      catch (Exception e) { e.printStackTrace(); }
    }
    // -----------------------------------------
    public static void main(String[] args) {
        String seed = "1";
        vis = true;
        manual = false;
        del = 100;
        SZ = 10;
        TL = 10;
        for (int i = 0; i<args.length; i++)
        {   if (args[i].equals("-seed"))
                seed = args[++i];
            if (args[i].equals("-exec"))
                exec = args[++i];
            if (args[i].equals("-delay"))
                del = Integer.parseInt(args[++i]);
            if (args[i].equals("-novis"))
                vis = false;
            if (args[i].equals("-manual"))
                manual = true;
            if (args[i].equals("-size"))
                SZ = Integer.parseInt(args[++i]);
            if (args[i].equals("-debug"))
                debug = true;
            if (args[i].equals("-timelimit"))
                TL = Integer.parseInt(args[++i]);
        }
        if (seed.equals("1") || seed.equals("2") || seed.equals("3"))
            SZ = 15;
        if (exec == null)
            manual = true;
        if (manual)
            vis = true;
        ColorCaptureVis f = new ColorCaptureVis(seed);
    }
    // -----------------------------------------
    void addFatalError(String message) {
        System.out.println(message);
    }
}

class ErrorReader extends Thread{
    InputStream error;
    public ErrorReader(InputStream is) {
        error = is;
    }
    public void run() {
        try {
            byte[] ch = new byte[50000];
            int read;
            while ((read = error.read(ch)) > 0)
            {   String s = new String(ch,0,read);
                System.out.print(s);
                System.out.flush();
            }
        } catch(Exception e) { }
    }
}
