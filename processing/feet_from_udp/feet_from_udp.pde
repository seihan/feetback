/**
 * (./) udp.pde - how to use UDP library as unicast connection
 * (cc) 2006, Cousot stephane for The Atelier Hypermedia
 * (->) http://hypermedia.loeil.org/processing/
 *
 */

// import UDP library
import hypermedia.net.*;
import processing.serial.*;

static final int MIN_COLOR_MAX = 4000; // Maximum color value (largest expected data point)
static int COLOR_MAX = MIN_COLOR_MAX; // Maximum color value (largest expected data point)
static final char   PROTOCOL_HDR1 = 'M'; // Magic
static final char   PROTOCOL_HDR2 = 'V'; // Value
static final int COLUMNS = 21;
static final int ROWS = 60;
static final int DRAW_FACTOR = 12;
static final int SCALE_FACTOR = 2;
static final int WIDTH = COLUMNS * DRAW_FACTOR;
static final int HEIGHT = ROWS * DRAW_FACTOR;
static final int SCALE_COLUMNS = COLUMNS * SCALE_FACTOR;
static final int SCALE_ROWS = ROWS * SCALE_FACTOR;

static int received = 0;
static int expected = 0;
static int [] values;
static int [][] grid = new int[COLUMNS][ROWS];
static boolean Redraw = true;

UDP udp;  // define the UDP object
Serial usbPort;  // Create object from Serial class

static int[][] stencil = {
{ -1, -1, -1, -1, -1, -1, -1,  4,  5,  6,  7,  8,  9, 10, -1, -1, -1, -1, -1, -1, -1},
{ -1, -1, -1, -1, -1, -1,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, -1, -1, -1, -1, -1},
{ -1, -1, -1, -1, -1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, -1, -1, -1, -1},
{ -1, -1, -1, -1, -1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, -1, -1, -1, -1},
{ -1, -1, -1, -1,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, -1, -1, -1, -1},
{ -1, -1, -1, -1,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, -1, -1, -1,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, -1, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, -1, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, -1, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, -1, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, -1, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, -1, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, 38, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, 38, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, 38, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, 38, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ -1, 38, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ 39, 38, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ 39, 38, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ 39, 38, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ 39, 38, 37,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, -1, -1, -1},
{ 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, -1, -1, -1},
{ 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, -1, -1},
{ 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, -1, -1},
{ 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, -1, -1},
{ 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1},
{ 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1},
{ 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1},
{ 37, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16},
{ 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16},
{ -1, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16},
{ -1, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16},
{ -1, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16},
{ -1, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16},
{ -1, -1, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16},
{ -1, -1, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1},
{ -1, -1, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1},
{ -1, -1, -1, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1},
{ -1, -1, -1, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1},
{ -1, -1, -1, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, -1},
{ -1, -1, -1, -1, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, -1, -1},
{ -1, -1, -1, -1, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, -1, -1},
{ -1, -1, -1, -1, -1, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, -1, -1},
{ -1, -1, -1, -1, -1, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, -1, -1},
{ -1, -1, -1, -1, -1, -1, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, -1, -1, -1},
{ -1, -1, -1, -1, -1, -1, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, -1, -1, -1},
{ -1, -1, -1, -1, -1, -1, -1, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, -1, -1, -1, -1},
{ -1, -1, -1, -1, -1, -1, -1, -1, 28, 27, 26, 25, 24, 23, 22, 21, 20, -1, -1, -1, -1},
{ -1, -1, -1, -1, -1, -1, -1, -1, -1, 27, 26, 25, 24, 23, 22, 21, -1, -1, -1, -1, -1}
};

int bilinear(int x, int y) {
  int xfull = SCALE_FACTOR;
  int yfull = SCALE_FACTOR;

  int xdiff = (x % xfull);
  int ydiff = (y % yfull);

  int x_idx = x / xfull;
  int y_idx = y / yfull;
  int x_idx_plus_1 = x_idx < (COLUMNS - 1) ? x_idx + 1 : x_idx;
  int y_idx_plus_1 = y_idx < (ROWS - 1) ? y_idx + 1 : y_idx;

  int data_lu = grid[x_idx][y_idx];
  int data_ru = grid[x_idx_plus_1][y_idx];
  int data_ld = grid[x_idx][y_idx_plus_1];
  int data_rd = grid[x_idx_plus_1][y_idx_plus_1];

  int data_left  = ((yfull - ydiff) * data_lu   + (ydiff * data_ld))    / yfull;
  int data_right = ((yfull - ydiff) * data_ru   + (ydiff * data_rd))    / yfull;
  int data_mid   = ((xfull - xdiff) * data_left + (xdiff * data_right)) / xfull;

  return data_mid;
}

int max_value = 0;
int max_value_col = 0;
int max_value_row = 0;

void update_grid() {
  if (Redraw) return;

  int k = 0;
  max_value = 0;
  
  for (int row = 0; row < ROWS; row++) {
    for (int col = 0; col < COLUMNS; col++) {
      int colour = -1;
      //int colour = (col-11) * (row-30) * 18;
      //if ((values != null) && (k < values.length))
      if ((values != null) && (stencil[row][col] != -1) && (k < values.length)) {
        colour = values[k];  // fill values into grid positions
        values[k++] /= 2; // decay value
      }

      if (colour > max_value) {
          max_value = colour;
          max_value_col = col;
          max_value_row = row;
      }

      grid[col][row] = colour;
    }
  }
    
  COLOR_MAX = max(max_value, MIN_COLOR_MAX);
  // Configure color mode so COLOR_MAX corresponds to 220 degree Hue
  colorMode(HSB, (360 * COLOR_MAX) / 220, 100, 100);
  Redraw = true;    // don't draw again until something changes
}

PFont f;                           // STEP 1 Declare PFont variable
int fps = 0;
int start = 0;

/**
 * init
 */
void setup() {
  size(504,720);
  background(0);
  // Configure color mode so COLOR_MAX corresponds to 220 degree Hue
  colorMode(HSB, (360 * COLOR_MAX) / 220, 100, 100);
  f = createFont("Arial",16,true); // STEP 2 Create Font
  textFont(f,DRAW_FACTOR-1);                  // STEP 3 Specify font to be used
  
  // create a new datagram connection on port 6000
  // and wait for incomming message
  udp = new UDP( this, 5005 );
  //udp.log( true );     // <-- printout the connection activity
  udp.listen( true );
  
  values = new int[COLUMNS * ROWS];
  update_grid();
}
  

//process events
void draw() {
  if (!Redraw) return;  // don't bother redrawing the same thing

  noStroke();           // dots don't have an outline
  for (int col = 0; col < COLUMNS; col++)
    for (int row = 0; row < ROWS; row++) {
      int colour = grid[col][row];

      // Invert so max value is Hue 0 (red) and 0 is blue
      if (colour >= 0)
        fill(COLOR_MAX - colour, 100, 100);        // fill with this color
      else
        fill(0, 0, 30);
      ellipse(WIDTH - col * DRAW_FACTOR - DRAW_FACTOR/2,
              HEIGHT - row * DRAW_FACTOR - DRAW_FACTOR/2,
              1.2 * DRAW_FACTOR, 1.2 * DRAW_FACTOR);  // draw this dot
    }

  for (int col = 0; col < SCALE_COLUMNS; col++)
    for (int row = 0; row < SCALE_ROWS; row++) {
      int colour = bilinear(col, row);

      // Invert so max value is Hue 0 (red) and 0 is blue
      if (colour >= 0)
        fill(COLOR_MAX - colour, 100, 100);        // fill with this color
      else
        fill(0, 0, 30);
      ellipse(WIDTH + WIDTH - col * DRAW_FACTOR / SCALE_FACTOR - DRAW_FACTOR / SCALE_FACTOR/2,
              HEIGHT - row * DRAW_FACTOR / SCALE_FACTOR - DRAW_FACTOR / SCALE_FACTOR/2,
              DRAW_FACTOR / SCALE_FACTOR, DRAW_FACTOR / SCALE_FACTOR);  // draw this dot
    }

  for (int i = 1; i < ROWS; i++) {
    if (i == max_value_row)
      fill(0, 100, 100); // red
    else
      fill(0, 0, 0); // black
    
    text(i, WIDTH - DRAW_FACTOR + DRAW_FACTOR/4, HEIGHT - i * DRAW_FACTOR - 1);   // STEP 5 Display Text
  }
  for (int i = 0; i < 10; i++) {
    if (i == max_value_col)
      fill(0, 100, 100); // red
    else
      fill(0, 0, 0); // black

    text(i%10,(COLUMNS - i - 1) * DRAW_FACTOR + DRAW_FACTOR/4, HEIGHT- 1);   // STEP 5 Display Text
  }
  for (int i = 10; i < COLUMNS; i++) {
    if (i == max_value_col)
      fill(0, 100, 100); // red
    else
      fill(0, 0, 0); // black

    text(i%10, (COLUMNS - i - 1) * DRAW_FACTOR + DRAW_FACTOR/4, HEIGHT - 1);   // STEP 5 Display Text
    text(i/10, (COLUMNS - i - 1) * DRAW_FACTOR + DRAW_FACTOR/4, HEIGHT - DRAW_FACTOR - 1);   // STEP 5 Display Text
  }

  fill(0, 100, 100); // red
  textFont(f,DRAW_FACTOR+4);                  // STEP 3 Specify font to be used
  text(max_value, DRAW_FACTOR/2, 1.5*DRAW_FACTOR);   // STEP 5 Display Text
  text(fps * 1000 / (millis() - start), 40*DRAW_FACTOR, 1.5*DRAW_FACTOR);   // STEP 5 Display Text
  textFont(f,DRAW_FACTOR-1);                  // STEP 3 Specify font to be used

  fps++;
  Redraw = false;    // don't draw again until something changes
}

/**
 * To perform any action on datagram reception, you need to implement this 
 * handler in your code. This method will be automatically called by the UDP 
 * object each time he receive a nonnull message.
 * By default, this method have just one argument (the received message as 
 * byte[] array), but in addition, two arguments (representing in order the 
 * sender IP address and his port) can be set like below.
 */
void receive( byte[] data ) {       // <-- default handler
  if (data[0] == PROTOCOL_HDR1 && data[1] == PROTOCOL_HDR2) {
    if (received != expected) {
      //print("WARNING: Discarding incomplete message - packet loss?");
      print("x");
    }
    //println();
    //print("============ New message received! ==============");

    expected = int(data[2]) + (256 * int(data[3]));
    //print("Expecting ");
    //print(expected);
    //println();
    received = 0;
    data = subset(data, 4);
  }

  if ((data.length / 4) > (expected - received)) {
      print("!");
      //print("WARNING: Discarding overfull message - packet loss?");
      //println();
      received = 0;
      expected = 0;
      return;
  }

  for (int idx = 0; idx < data.length; idx+=4, received++) {
    int offset = int(data[idx]) + (256 * int(data[idx + 1]));
    values[offset] = int(data[idx+2]) + (256 * int(data[idx + 3]));
    //values[received] = int(data[received]);
    //print(values[received]);
    //print("\t");
  }
  
  if (received == expected) {
    // Full message received!
    //print(".");
    if (start == 0) {
      start = millis();
    }
    update_grid();
  }
}
