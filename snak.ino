#include "LiquidCrystal.h"

byte mySnake[8][8] = 
{
{ B00000,
  B00000,
  B00011,
  B00110,
  B01100,
  B11000,
  B00000,
},
{ B00000,
  B11000,
  B11110,
  B00011,
  B00001,
  B00000,
  B00000,
},
{ B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B01110,
},
{ B00000,
  B00000,
  B00011,
  B01111,
  B11000,
  B00000,
  B00000,
},
{ B00000,
  B11100,
  B11111,
  B00001,
  B00000,
  B00000,
  B00000,
},
{ B00000,
  B00000,
  B00000,
  B11000,
  B01101,
  B00111,
  B00000,
},
{ B00000,
  B00000,
  B01110,
  B11011,
  B11111,
  B01110,
  B00000,
},
{ B00000,
  B00000,
  B00000,
  B01000,
  B10000,
  B01000,
  B00000,
}
};

boolean levelz[5][2][16] = {
{{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false}},

{{true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true},
{true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true}},

{{true,false,false,false,true,false,false,false,false,false,false,true,false,false,false,true},
{true,false,false,false,false,false,false,false,true,false,false,false,false,false,false,true}},

{{true,false,true,false,false,false,false,false,false,true,false,false,false,true,false,false},
{false,false,false,false,true,false,false,true,false,false,false,true,false,false,false,true}}
};

// RS, E, D4, D5, D6, D7
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
unsigned long time, timeNow; 
int gameSpeed;
boolean skip, gameOver, gameStarted;
int olddir;
int selectedLevel,levels;

const int KEY_UP = 12;
const int KEY_DOWN = 11;
const int KEY_LEFT = 13;
const int KEY_RIGHT = 10;

int adc_key_val[5] ={50, 200, 400, 600, 800 };
int NUM_KEYS = 5;
int adc_key_in;
int key=-1;
int oldkey=-1;

boolean x[16][80];
byte myChar[8];
byte nullChar[8] = { 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0 };
boolean special;

struct partdef
{
  int row,column,dir; //0 - up, 1 - down, 2 - right, 3 - left
  struct partdef *next;
}; 
typedef partdef part;

part *head, *tail;
int i,j,collected,score;
long pc,pr;

void drawMatrix()
{
  int cc=0;
  if (!gameOver)
  {
  x[pr][pc] = true;
  //for (i=0;i<8;i++) lcd.createChar(i, nullChar);
  for(int r=0;r<2;r++)
  {
    for(int c=0;c<16;c++)
    {
      special = false;
      for(int i=0;i<8;i++)
      {
        byte b=B00000;
        if (x[r*8+i][c*5+0]) {b+=B10000; special = true;}
        if (x[r*8+i][c*5+1]) {b+=B01000; special = true;}
        if (x[r*8+i][c*5+2]) {b+=B00100; special = true;}
        if (x[r*8+i][c*5+3]) {b+=B00010; special = true;}
        if (x[r*8+i][c*5+4]) {b+=B00001; special = true;}
        myChar[i] = b;
      }
      if (special) 
      {
        lcd.createChar(cc, myChar);
        lcd.setCursor(c,r);
        lcd.write(byte(cc));
	cc++;
      }
      else 
      {
        lcd.setCursor(c,r);
        if (levelz[selectedLevel][r][c]) lcd.write(255);
        else lcd.write(254);
      }
    }
  }
  }
}

void freeList()
{
  part *p,*q;
  p = tail;
  while (p!=NULL)
  {
    q = p;
    p = p->next;
    free(q);
  }
  head = tail = NULL;
}

void gameOverFunction()
{
  delay(1000);
  lcd.clear();
  freeList();
  lcd.setCursor(3,0);
  lcd.print("Game Over!");
  lcd.setCursor(4,1);
  lcd.print("Score: ");
  lcd.print(score);
  delay(1000);
}

void growSnake()
{
  part *p;
  p = (part*)malloc(sizeof(part));
  p->row = tail->row;
  p->column = tail->column;
  p->dir = tail->dir;
  p->next = tail;
  tail = p;
}

void newPoint()
{

  part *p;
  p = tail;
  boolean newp = true;
  while (newp)
  {
    pr = random(16);
    pc = random(80);
    newp = false;
    if (levelz[selectedLevel][pr / 8][pc / 5]) newp=true;
    while (p->next != NULL && !newp) 
    {
      if (p->row == pr && p->column == pc) newp = true;
      p = p->next;
    }
  }
  
  if (collected < 13 && gameStarted) growSnake();
}

void moveHead()
{
  switch(head->dir) // 1 step in direction
  {
    case 0: head->row--; break;
    case 1: head->row++; break;
    case 2: head->column++; break;
    case 3: head->column--; break;
    default : break;
  }
  if (head->column >= 80) head->column = 0;
  if (head->column < 0) head->column = 79;
  if (head->row >= 16) head->row = 0;
  if (head->row < 0) head->row = 15;
  
  if (levelz[selectedLevel][head->row / 8][head->column / 5]) gameOver = true; // wall collision check
  
  part *p;
  p = tail;
  while (p != head && !gameOver) // self collision 
  {
    if (p->row == head->row && p->column == head->column) gameOver = true;
    p = p->next;
  }
  if (gameOver)
    gameOverFunction();
  else
  {
  x[head->row][head->column] = true;
  
  if (head->row == pr && head->column == pc) // point pickup check
  {
    collected++;
    score++;
    if (gameSpeed < 30) gameSpeed+=3;
    newPoint();
  }
  }
}

void moveAll()
{
  part *p;
  p = tail;
  x[p->row][p->column] = false;
  while (p->next != NULL) 
  {
    p->row = p->next->row;
    p->column = p->next->column;
    p->dir = p->next->dir;
    p = p->next;
  }
  moveHead();
}

void createSnake(int n) // n = size of snake
{
  for (i=0;i<16;i++)
    for (j=0;j<80;j++)
      x[i][j] = false;
	  
  part *p, *q;
  tail = (part*)malloc(sizeof(part));
  tail->row = 7;
  tail->column = 39 + n/2;
  tail->dir = 3;
  q = tail;
  x[tail->row][tail->column] = true;
  for (i = 0; i < n-1; i++) // build snake from tail to head
  {
    p = (part*)malloc(sizeof(part));
    p->row = q->row;
    p->column = q->column - 1; //initial snake id placed horizoltally
    x[p->row][p->column] = true;
    p->dir = q->dir;
    q->next = p;
    q = p;
  }
  if (n>1)
  {
    p->next = NULL;
    head  = p;
  }
  else 
  {
    tail->next = NULL;
    head = tail;
  }
}

void newLevel()
{
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Level ");
  
  for(i=0;i<8;i++)
  {
    lcd.createChar(i,mySnake[i]);
    lcd.setCursor(i+4,1);
    lcd.write(byte(i));
  }
  collected = 0;
  gameSpeed = 10;
  createSnake(3);
  time = 0;
}

void startF()
{
  gameOver = false;
  gameStarted = false;
  selectedLevel = 1;
  score = 0;

  newLevel();
}
void setup()
{
  pinMode(KEY_UP, INPUT);	// input buttons
  pinMode(KEY_DOWN, INPUT);
  pinMode(KEY_RIGHT, INPUT);
  pinMode(KEY_LEFT, INPUT);
  
  pinMode(9, OUTPUT);

  Serial.begin(9600);
  randomSeed(analogRead(0));

  levels = 4; //number of lvls
  lcd.begin(16, 2);
  startF();
}

void loop()
{
  if (!gameOver && !gameStarted)
  {
   lcd.setCursor(6,0);
   lcd.print(selectedLevel);
   key = get_key();  // convert into key press
   if (key != oldkey)   // if keypress is detected
   {
     delay(50);  // wait for debounce time
     key = get_key();    // convert into key press
     if (key != oldkey)    
     {  
       oldkey = key;
       if (key >=0)
       {
         olddir = head->dir;

         lcd.clear();
         selectedLevel--;
         newPoint();
         gameStarted = true;
        }
     }
   }
  }
  if (!gameOver && gameStarted)
  {
   Serial.println(collected);
   if(collected == 10) {
     selectedLevel += 2;
     gameStarted = false;
     newLevel();
     goto lab;
   }
   
   skip = false; //skip the second moveAll() function call if the first was made
   
   key = get_key();  // convert into key press
   if (key != oldkey)   // if keypress is detected
   {
     delay(50);  // wait for debounce time
     key = get_key();    // convert into key press
     if (key != oldkey)    
     {  
       oldkey = key;
       if (key >=0)
       {
         olddir = head->dir;
         if (key==0 && head->dir!=3) head->dir = 2;
         if (key==1 && head->dir!=1) head->dir = 0;
         if (key==2 && head->dir!=0) head->dir = 1;
         if (key==3 && head->dir!=2) head->dir = 3;
         
         if (olddir != head->dir)
         {
           skip = true;
           delay(1000/gameSpeed);
           moveAll();
           drawMatrix();
         }
       }
     }
   }
   
   if (!skip)
   {
     timeNow = millis();
     if (timeNow - time > 1000 / gameSpeed)
     {
       moveAll();
       drawMatrix();
       time = millis();
     }
   }
lab:;
  }
  if(gameOver)
  { 
   key = get_key();  // convert into key press
   if (key != oldkey)   // if keypress is detected
   {
     delay(50);  // wait for debounce time
     key = get_key();    // convert into key press
     if (key != oldkey)    
     {  
       oldkey = key;
       if (key >=0)
       {
          startF();
       }
     }
   }
  }
}

int get_key()
{
    //int adc_key_val[5] ={50, 200, 400, 600, 800 };
    int key;

    if (oldkey != 1 && digitalRead(KEY_UP) == HIGH) return 1;
    else if (oldkey != 2 && digitalRead(KEY_DOWN) == HIGH) return 2;
    else if (oldkey != 3 && digitalRead(KEY_LEFT) == HIGH) return 3;
    else if (oldkey != 0 && digitalRead(KEY_RIGHT) == HIGH) return 0;
    else return -1;

    /*int k;
    for (k = 0; k < NUM_KEYS; k++)
    {
      if (input < adc_key_val[k])
      {
        return k;
      }
    }   
    if (k >= NUM_KEYS)k = -1;  // No valid key pressed
    return k;*/
}

/*
void printChar()
{
  Serial.println();
  Serial.println();
  Serial.println();
  for (i=0;i<8;i++)
  {
    Serial.println(myChar[i]);
  }
}

void printMatrix()
{
  Serial.println();
  Serial.println();
  Serial.println();
  for (i=0;i<16;i++)
  {
    for (j=0;j<80;j++)
    {
      Serial.print(x[i][j]);
      Serial.print(" ");
    }
    Serial.println();
  }
}
*/
