import processing.serial.*;

static final String PORT = "/dev/ttyUSB3"; // Serial port
static final char   PROTOCOL_HDR1 = 'M'; // Magic
static final char   PROTOCOL_HDR2 = 'V'; // Value
static final int    COLS = 5; // Columns in the grid
static final int    ROWS = 16; // Rows in the grid
static final int    COLOR_MAX = 3000; // Maximum color value (largest expected data point)
static final int    WIDTH = 200; // Width of the drawing area
static final int    HEIGHT = 640; // Height of the drawing area

int [][] data; // Received data values
Cell[][] grid; // 2D Array of objects

Serial usbPort;  // Create object from Serial class

// convert unsigned byte to int
int unsigned(byte val) {
  return int(val) & 0xff;
}

// Receive a message from serial port
int receive_message() {
  byte [] res = null;

  print("Receiving message... ");
  // find message start
  do {
    while ((res == null) || (res[0] != PROTOCOL_HDR1)) {
      res = usbPort.readBytes(1);
      // FIXME: sometimes serial is just not ready?!
      if (res == null) {
        delay(10);
      }
    }
    res = usbPort.readBytes(1);
  } while ((res == null) || (res[0] != PROTOCOL_HDR2));
  print("found header ...  ");

  // read length
  res = usbPort.readBytes(1);
  if (res == null)
    return 0;

  int len = int(res[0]);
  print(len);
  println(" values");

  if (len > (ROWS * COLS))
    len = ROWS * COLS;

  // read data
  res = usbPort.readBytes(len * 2);
  if (res == null)
    return 0;

  len = res.length / 2;
  for (int idx = 0; idx < len; idx++) {
    // sort into right grid point row by row
    int x = idx % COLS;
    int y = idx / COLS;
    data[x][y] = unsigned(res[(2 * idx)]) + 256 * unsigned(res[(2 * idx) + 1]);

    print(data[x][y]);
    print(' ');
  }
  println();

  return len;
}

int nearest_neighbor(int x, int y) {
  return data[x * COLS / WIDTH][y * ROWS / HEIGHT];
}

int bilinear(int x, int y) {
  int xfull = WIDTH / COLS;
  int yfull = HEIGHT / ROWS;

  int xdiff = (x % xfull);
  int ydiff = (y % yfull);

  int x_idx = x / xfull;
  int y_idx = y / yfull;
  int x_idx_plus_1 = x_idx < (COLS - 1) ? x_idx + 1 : x_idx;
  int y_idx_plus_1 = y_idx < (ROWS - 1) ? y_idx + 1 : y_idx;

  int data_lu = data[x_idx][y_idx];
  int data_ru = data[x_idx_plus_1][y_idx];
  int data_ld = data[x_idx][y_idx_plus_1];
  int data_rd = data[x_idx_plus_1][y_idx_plus_1];

  int data_left  = ((yfull - ydiff) * data_lu   + (ydiff * data_ld))    / yfull;
  int data_right = ((yfull - ydiff) * data_ru   + (ydiff * data_rd))    / yfull;
  int data_mid   = ((xfull - xdiff) * data_left + (xdiff * data_right)) / xfull;

  return data_mid;
}

void setup() {
  // Set up Serial connection
  usbPort = new Serial(this, PORT, 115200);
  // FIXME: wait a little here for reliable connection
//  do {
//    delay(2000);
//  } while (usbPort.readBytes(1) == null);

  // Let's draw the data twice for now...
  size(400, 640); /* cannot use constants?! */

  // Initialize the grid of uniform color cells
  int cell_width = WIDTH / COLS;
  int cell_height = HEIGHT / ROWS;
  grid = new Cell[COLS][ROWS];
  for (int i = 0; i < COLS; i++) {
    for (int j = 0; j < ROWS; j++) {
      // Initialize each object
      grid[i][j] = new Cell(i*cell_width,j*cell_height,cell_width,cell_height);
    }
  }

  // Initialize the data point array
  data = new int[COLS][ROWS];

  // Configure color mode so COLOR_MAX corresponds to 220 degree Hue
  colorMode(HSB, (360 * COLOR_MAX) / 220, 100, 100);
}

void draw() {
  blendMode(BLEND);
  background(0);
  
  receive_message();

  // Draw each cell
  for (int i = 0; i < COLS; i++) {
    for (int j = 0; j < ROWS; j++) {
      grid[i][j].display();
    }
  }

  // Draw finer grid with interpolation
  int colr = 0;
  for (int x = 0; x < WIDTH; x += 10) {
    for (int y = 0; y < HEIGHT; y += 4) {
      colr = bilinear(x, y);
      if (colr > COLOR_MAX)
        colr = COLOR_MAX;

      // Invert so max value is Hue 0 (red) and 0 is blue
      colr = COLOR_MAX - colr;

      // draw the thing
      stroke(colr, 100, 100);
      fill(colr, 100, 100);
      rect(200 + x, y, 10, 4);
    }
  }
}

// A Cell object
class Cell {
  // A cell object knows about its location in the grid
  // as well as its size with the variables x,y,w,h
  int x,y;   // x,y location
  int w,h;   // width and height

  // Cell Constructor
  Cell(int tempX, int tempY, int tempW, int tempH) {
    x = tempX;
    y = tempY;
    w = tempW;
    h = tempH;
  }

  void display() {
    int colr = nearest_neighbor(x, y);
    if (colr > COLOR_MAX)
      colr = COLOR_MAX;

    // Invert color so hue matches 0 (red) for max value
    colr = COLOR_MAX - colr;

    // draw the thing
    stroke(colr, 100, 100);
    fill(colr, 100, 100);
    rect(x,y,w,h);
  }
}