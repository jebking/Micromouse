#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Util.h>
#include <Motors.h>

/* Global Constants */
// For Setting Wall bits in the wall array
#define NORTH 1
#define EAST  2
#define SOUTH 4
#define WEST  8

// Starting and ending position
#define STARTROW 0
#define STARTCOL 0
#define ENDROW 3
#define ENDCOL 3

int debugMode = 0;
int virtualMode = 0;

// Calibration for turns -- needs to change
int RIGHT_TURN_STEPS = 54;
int LEFT_TURN_STEPS = 53;
int MOVE_FORWARD_STEPS = 55;
int STEP_DELAY = 2;

// 0 for North
// 1 for East
// 2 for South 
// 3 for West
int mouseDir = 0;

// maps 0-3 direction to array offset
int offsetMap[4] = {16, 1, -16, -1};

// define distance sensor pins
int leftIRPin = A2;
int rightIRPin = A1;
int forwardIRPin = A0;

// define motor pins
Motors motors (9, 11, 2, 3, 10, 12,
               forwardIRPin, leftIRPin, rightIRPin);

// define push-button pin
int buttonPin = 7;

/* Global Variables */
// Global array to store the cell values
unsigned char cellMap[256];
// Global array to store the wall bits
unsigned char wallMap[256];
// Global array for virtual maze walls
unsigned char virtualWallMap[256];
/*
={0b1100,0b100,0b101,0b110,0b1100,0b110,0b1100,0b101,0b101,0b110,0b1000,0b101,0b101,0b110,0b1000,0b110,
  0b1010,0b1010,0b1110,0b1010,0b1011,0b1010,0b1010,0b1100,0b111,0b1010,0b1010,0b1100,0b110,0b1010,0b1010,0b1010,
  0b1010,0b1010,0b1000,0b1,0b101,0b10,0b1010,0b1010,0b1100,0b11,0b1010,0b1010,0b1010,0b1010,0b1010,0b1010,
  0b1010,0b1010,0b1011,0b1100,0b110,0b1010,0b1010,0b1010,0b1010,0b1100,0b11,0b1010,0b1010,0b1010,0b1010,0b1010,
  0b1010,0b1000,0b0101,0b0010,0b1000,0b0001,0b0001,0b0011,0b1010,0b1011,0b1100,0b0011,0b1010,0b1010,0b1010,0b1010,
  0b1010,0b1010,0b1110,0b1011,0b1010,0b1100,0b101,0b110,0b1001,0b101,0b0,0b101,0b10,0b1010,0b1010,0b1010,
  0b1010,0b1010,0b1000,0b110,0b1010,0b1010,0b1100,0b1,0b101,0b110,0b1010,0b1101,0b10,0b1010,0b1010,0b1010,
  0b1010,0b1010,0b1010,0b1011,0b1010,0b1010,0b1010,0b1100,0b110,0b1010,0b1010,0b1101,0b10,0b1010,0b1010,0b1010,
  0b1010,0b1011,0b1000,0b101,0b1,0b11,0b1010,0b1001,0b10,0b1010,0b1001,0b101,0b11,0b1010,0b1010,0b1010,
  0b1000,0b111,0b1010,0b1100,0b101,0b110,0b1001,0b111,0b1010,0b1000,0b101,0b101,0b101,0b10,0b1010,0b1010,
  0b1010,0b1110,0b1010,0b1010,0b1100,0b1,0b101,0b101,0b11,0b1010,0b1100,0b101,0b101,0b10,0b1010,0b1010,
  0b1010,0b1011,0b1010,0b1010,0b1010,0b1101,0b100,0b100,0b101,0b10,0b1010,0b1100,0b111,0b1010,0b1010,0b1010,
  0b1000,0b101,0b10,0b1010,0b1001,0b111,0b1010,0b1010,0b1110,0b1010,0b1010,0b1000,0b101,0b11,0b1110,0b1010,
  0b1010,0b1110,0b1010,0b1001,0b101,0b101,0b11,0b1010,0b1010,0b1011,0b1010,0b1001,0b101,0b101,0b11,0b1010,
  0b1010,0b1010,0b1000,0b101,0b101,0b101,0b111,0b1010,0b1001,0b101,0b11,0b1010,0b1101,0b101,0b101,0b10,
  0b1001,0b11,0b1001,0b101,0b101,0b101,0b1,0b101,0b101,0b101,0b101,0b1,0b101,0b101,0b101,0b11};*/
// Global ints for current position
int currentRow, currentCol;
// Global counter to set destination cell
int counter = 2;
// For going fast: 1 if the last move was forward
int lastWasForward = 0;
// Code to side-bump
int sideCorrectCode = 0;

/* Function Prototypes */
void initializeMaze();
void floodMaze();
void setBoundaryWalls ();
void setTestWalls();
int chooseNextDir(int currentCell, int _mouseDir);
void makeNextMove();
void turnRight();
void turnLeft();
int moveForward();
void makeNextMove();
void onButtonRelease();
void readCell();
void debugBlink(int times);
void printMaze();

void setup() 
{
  /* * * * * * * * * * * * * * * * * 
   * MOUSE HARDWARE INTITALIZATION *
   * * * * * * * * * * * * * * * * **/
  if (debugMode)
  {
    Serial.begin(9600);
  }
  
  pinMode(13, OUTPUT); // onboard LED
  pinMode(forwardIRPin, INPUT);
  pinMode(leftIRPin, INPUT);
  pinMode(rightIRPin, INPUT);
  
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), onButtonRelease, RISING);
  
  /* * * * * * * * * * * * * * *
   * FLOODFILL INITIALIZATION  *
   * * * * * * * * * * * * * * */
  // Initialize the maze
  initializeMaze ();
  // Set maze boundary walls
  setBoundaryWalls ();

  if (virtualMode)
  {
    for (int i = 5; i > 0; i--) {
      Serial.println(i);
      wait(1000);
    }
  }
}

void loop()
{

  readCell();

  if (debugMode)
  {
    debugBlink(currentRow);
    wait(1000);
    debugBlink(currentCol);
    wait(1000);
    debugBlink(wallMap[16 * currentRow + currentCol]);
    wait(1000);
    debugBlink(mouseDir);
  }

  floodMaze();
  if (virtualMode)
  {
    printMaze();
  }

  makeNextMove();

  if(cellMap[(16*currentRow) + currentCol] == 0)
  {
    counter++;
  }

  if (virtualMode)
  {
    wait(1000);
  }

  if (motors.releaseFlag)
  {
    currentRow = 0;
    currentCol = 0;
    mouseDir = 0;
    counter -= counter % 2;
    motors.releaseFlag = 0;
    if (debugMode)
    {
      debugBlink(2);
    }
  }

  //wait(500);

}

void debugBlink(int times) {
  for (int i = 0; i < times; i++)
  {
    digitalWrite(13, HIGH);
    wait(200);
    digitalWrite(13, LOW);
    wait(200);
  }
}

/* * * * * * * * * * * * * * * *
 *       HARDWARE CODE         *
 * * * * * * * * * * * * * * * */

void readCell()
{
  int irThresholds[4] = {240, 100, 1023, 100};
  
  int readings[4] = {irReading(forwardIRPin),
                     irReading(rightIRPin),
                     0,
                     irReading(leftIRPin)};

  unsigned char currentCell = 16 * currentRow + currentCol;

  wallMap[currentCell] &= 15;

  for (int i = 0; i < 4; i++)
  {
    if (i != 2)
    {
      int dir = (mouseDir + i) % 4;
      
      int oppositeCell = currentCell + offsetMap[dir];
      
      if ((readings[i] > irThresholds[i] && !virtualMode) ||
          (virtualMode && virtualWallMap[currentCell] & 1 << dir))
      {
        // set wall for current cell
        wallMap[currentCell] |= 1 << dir;
  
        // set wall for opposite cell if valid
        if (oppositeCell >= 0 && oppositeCell < 256)
        {
          wallMap[oppositeCell] |= 1 << ((dir + 2) % 4);
        }
      }
      /*else
      {
        wallMap[currentCell] &= ~(1 << dir);
  
        if (oppositeCell >= 0 && oppositeCell < 256)
        {
          wallMap[oppositeCell] &= ~(1 << ((dir + 2) % 4));
        }
      }*/
    }
  }
}

void makeNextMove ()
{ 
  unsigned char currentCell = 16 * currentRow + currentCol;
  int nextDir = chooseNextDir(currentCell, mouseDir);
  
  if (debugMode)
  {
    Serial.println(wallMap[currentCell]);
  }

  if (virtualMode)
  {
    switch (nextDir)
    {
      case 0:
        Serial.println("NORTH");
        break;
      case 1:
        Serial.println("EAST");
        break;
      case 2:
        Serial.println("SOUTH");
        break;
      case 3:
        Serial.println("WEST");
        break;
    }
  }
  else
  {
    if (wallMap[currentCell] & 1 << mouseDir)
    {
      motors.wallOrientateFwd();
    }
    if (sideCorrectCode % 2 == 1 &&
        (!(wallMap[currentCell] & 1 << (mouseDir + 1) % 4) ||
         (nextDir + 4 - mouseDir) % 4 == 3))
    {
      sideCorrectCode = 0;
    }
    else if (sideCorrectCode % 2 == 0 &&
             (!(wallMap[currentCell] & 1 << (mouseDir + 3) % 4) ||
              (nextDir + 4 - mouseDir) % 4 == 1))
    {
      sideCorrectCode = 0;
    }
    switch (sideCorrectCode)
    {
      case 1:
        motors.turnLeft();
        motors.wallOrientateBkwd();
        motors.turnRight();
        break;
      case 2:
        motors.turnRight();
        motors.wallOrientateBkwd();
        motors.turnLeft();
        break;
      case 3:
        motors.turnRight();
        motors.wallOrientateFwd();
        motors.turnLeft();
        break;
      case 4:
        motors.turnLeft();
        motors.wallOrientateFwd();
        motors.turnRight();
        break;
    }
    makeTurn(nextDir);
    mouseDir = nextDir;
    lastWasForward &= sideCorrectCode == 0;
    if (wallMap[currentCell] & 1 << (mouseDir + 2) % 4)
    {
      motors.wallOrientateBkwd();
    }
    sideCorrectCode = moveForward();
  }
  
  currentRow += offsetMap[nextDir] / 16;
  currentCol += offsetMap[nextDir] % 16;
  mouseDir = nextDir;
}

void makeTurn(int nextDir)
{
  int angle = (4 + nextDir - mouseDir) % 4;
  if (angle == 0)
  {
    lastWasForward = 1;
    return;
  }
  else
  {
    lastWasForward = 0;
    wait(400);
  }
  switch (angle)
  {
    case 1:
      motors.turnRight();
      break;
    case 2:
      if (irReading(leftIRPin) < irReading(rightIRPin))
      {
        motors.turnAroundLeft();
      }
      else
      {
        motors.turnAroundRight();
      }
      break;
    case 3:
      motors.turnLeft();
      break;
  }
}

int chooseNextDir(int currentCell, int _mouseDir)
{
  // Define a default, very high step value
  unsigned char lowest = 255;

  int nextDir = 0;

  // Compare through all the neighbors
  for (int i = 0; i < 4; i++)
  {
    int curDir = (_mouseDir + i) % 4;
    if (cellMap[currentCell + offsetMap[curDir]] < lowest && !(wallMap[currentCell] & 1 << curDir))
    {
      lowest = cellMap[currentCell + offsetMap[curDir]];
      nextDir = curDir;
    }
  }

  return nextDir;
}


int moveForward()
{
  if (counter >= 2)
  {
    int nextCell = 16*currentRow + currentCol + offsetMap[mouseDir];
    int forwardIsNext = wallMap[nextCell] < 240 &&
                        chooseNextDir(nextCell, mouseDir) == mouseDir &&
                        cellMap[nextCell] != 0;
    int start_pwm = 60;
    if (lastWasForward)
    {
      start_pwm = motors.pwmRecord;
    }
    return motors.accForward(start_pwm, 255, 284, 1, !forwardIsNext);
  }
  else
  {
    return motors.accForward(60, 90, 284, 1, 1);
  }
}

void onButtonRelease()
{
  motors.releaseFlag = 1;
  if (debugMode)
  {
    Serial.println("button");
  }
}

/* * * * * * * * * * * * * * * *
 *          LOGIC CODE         * 
 * * * * * * * * * * * * * * * */


void initializeMaze ()
{
  // Initialize misc variables
  int stepValue = 0;

  currentRow = STARTROW;
  currentCol = STARTCOL;
  
  for (int i = 0; i < 255; i++)
  {
    cellMap[i] = 255;
    wallMap[i] = 240;
  }
}

void setBoundaryWalls ()
{
  // NORTH
  for (int i = 0; i < 16; i++)
  {
    wallMap[i] |= SOUTH;
  }
  // EAST
  for (int i = 15; i < 256; i += 16)
  {
    wallMap[i] |= EAST;
  }
  // SOUTH
  for (int i = 240; i < 256; i++)
  {
    wallMap[i] |= NORTH;
  }
  // WEST
  for (int i = 0; i < 241; i += 16)
  {
    wallMap[i] |= WEST;
  }
}

void floodMaze ()
{
  // reset the array of values
  for (int i = 0; i < 256; i++)
  {
    cellMap[i] = 255;
  }
  
  // char to store current distance from end
  unsigned char stepValue = 0;
  // array to act as stack
  unsigned char cellStack[256];
  // array to act as temporary storage
  unsigned char nextCellStack[256];
  // int to serve as a pointer to the top of the stack
  // 0 means the stack is empty
  int stackPointer, nextStackPointer;
  
    // Initialize pointers to the top of each stack
  stackPointer = 0;
  nextStackPointer = 0;

if(counter % 2 == 0) { 
  //initial path to centre
    stackPointer = 4;
    cellStack[0] = (16 * ENDROW) + ENDCOL;
    cellStack[1] = (16 * (ENDROW + 1)) + ENDCOL;
    cellStack[2] = (16 * ENDROW) + (ENDCOL + 1);
    cellStack[3] = (16 * (ENDROW + 1)) + (ENDCOL + 1);
  }
  else {
  //path back to start
    stackPointer = 1;
    cellStack[0] = (16 * STARTROW) + STARTCOL;
   }
  
  while (stackPointer > 0)
  {
    
    // Stop flooding if our cell has a value
    if (cellMap[16 * currentRow + currentCol] != 255)
    {
      break;
    }

    // Pop the cell off the stack
    unsigned char curCell = cellStack[stackPointer - 1];
    stackPointer--;
    
    if (cellMap[curCell] == 255)
    {
      // Set the current cell value to the step path value
      cellMap[curCell] = stepValue;
      
      // Serial.print ("Flood Cell: %d\n", curCell);
  
      // Add all unvisited, available uneighbors to the stack for the next step
      for (int i = 0; i < 4; i++)
      {
        unsigned char adjCell = curCell + offsetMap[i];
        if (adjCell >= 0 && adjCell < 256 && cellMap[adjCell] == 255 && (wallMap[curCell] & 1 << i) == 0)
        {
          nextCellStack[nextStackPointer] = adjCell;
          nextStackPointer++;
        }
      }
    }
    
    // if the stack is empty, move on to the next step
    if (stackPointer == 0)
    {
      // move the next stack to the main stack
      for (int i = 0; i < nextStackPointer; i++)
      {
        cellStack[i] = nextCellStack[i];
      }

      stackPointer = nextStackPointer;
      
      // empty next stack
      nextStackPointer = 0;

      stepValue++;
    }
    
    // Print stack for debug
    /*Serial.print ("Current Stack\n");
    for (int i = 0; i < stackPointer; i++)
    {
    Serial.print ("Stack Member: %d\n", cellStack[i]);
    }
    */

    if (debugMode == 2)
    {
      printMaze();
    }
  }
}

// Print the maze
void printMaze()
{
  for (int i = 0; i < 16; i++)
  {
    Serial.print ("---\t");
  }
  for (int i = 15; i >= 0; i--)
  {
    Serial.print ("\n");
    for (int j = 0; j < 16; j++)
    {
      if (currentRow == i && currentCol == j)
      {
        Serial.print("@");
      }
      Serial.print(cellMap[16 * i + j]);
      Serial.print("\t");
    }
    Serial.print("\n");
  }
  for (int i = 0; i < 16; i++)
  {
    Serial.print ("---\t");
  }
  Serial.print ("\n");
}
