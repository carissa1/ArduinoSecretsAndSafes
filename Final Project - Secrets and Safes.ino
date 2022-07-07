#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 8, 9, 10, 13);

#include <pitches.h>

// TIMER
byte byteDigits[10] = { // byte form of each digit
  B11111100,  // = 0
  B01100000,  // = 1
  B11011010,  // = 2
  B11110010,  // = 3
  B01100110,  // = 4
  B10110110,  // = 5
  B10111110,  // = 6
  B11100000,  // = 7
  B11111110,  // = 8
  B11100110   // = 9
};
int num = 9999; // number that timer is at
bool startTimer = false; // start timer or not
bool timerGoing = false; // is the timer going
int timerLength = 0; // amount of time for timer
int currentDigit = 0; // current digit to print (4 digits)
int decreaseTimer = 5; // number of loops before decrease

// LCD
int hearts = 3; // number of lives for lcd game
int waitTime = 200; // time before next obstacle
int numBlocks = 1; // number of blocks in obstacle
int jumpNum = 2.5; // jump time of character 
bool charJumping = false; // is character jumping
bool obstacleGoing = false; // obstacle on screen or not
int charPos[2] = {0, 1}; // x and y of character
int blockPos[2] = {15, 1}; // x and y of leftmost obstacle
int currentX = 0;
int currentY = 1;
const int switchPin = A0;
const int yPin = A1;
const int xPin = A2;
bool playingLcdGame = false; // playing lcd game or not
bool invincible = false; // if already failed jump only lose one heart
int lcdIndex = 0; // index in games and completed

// BUTTONS
const int btnsPin = A3; // pin for buttons
bool playingComboLock = false; // playing combination lock
int btnPressed; // which btn pressed
int combination[] = {1, 3, 4, 4, 2, 3, 1, 2, 2, 4}; // combination
int comboLength = 10; // length of combination
int comboIndex = 0; // index in combination
int comboBtn = combination[comboIndex]; // button that is next in sequence
int btnVal; // analog value from button
int attempt = 1; // attempt number (3 tries)
int combinationIndex = 1; // index in games and completed

// CODE WORD
char *words[] = {"Arduino", "Pineapple", "Coding", "Chess", "Llave", "Llama", "Keystone", "Futbol", "Cincuenta", "Seventy-two"};
// char codeWord[] = "4B 65 79 73 74 6F 6E 65";
int wordsLength = 10; // length list of words
int codeWordIndex = 6; // code word index
char codeWord = "Keystone"; // code word
int wordIndex = 0; // index that is on lcd
bool playingCodeWord = false; // playing or not
int codeIndex = 2; // index in games and completed

// START/OVERALL STUFF
int order[3] = {"Avoid", "Combination", "Code word"}; // order to complete tasks (maybe extra)
char *games[] = {"Avoid", "Combination", "Code word"}; // games to play
bool completed[] = {false, false, false};
int nameLengths[] = {5, 11, 9};
int gameIndex = 0; // index of games
int points = 0; // points
int fails = 0; // failed
int choice = 0; // game chosen
uint8_t checkmark[8] = {0x0,0x1,0x3,0x16,0x1c,0x8,0x0}; // checkmark
bool gameOver = false; // lost
bool unlocked = false; // won
bool lock = false; // lock box

// Pins for timer
const int dataPin = 2;
const int clockPin = 3;
const int digitPins[4] = {4, 5, 6, 7};

// Buzzer 
const int buzzerPin = A4;

// Motor
const int motorPin = A5;

void setup() {
  // Initialize pins for timer
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  for(int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
  }
  Serial.begin(9600); // start Serial monitor
  // Get random order to complete tasks
  for (size_t i = 0; i < 2; i++) {
    size_t j = random(0, 2 - i);
    int t = order[i];
    order[i] = order[j];
    order[j] = t;
  }

  // Initialize buttons
  pinMode(btnsPin, INPUT);

  // Initialize buzzer
  pinMode(buzzerPin, OUTPUT);

  // Initialize lcd
  lcd.begin(16, 2);
  lcd.createChar(1, checkmark);
  
  // Timer1 module overflow interrupt configuration
  TCCR1A = 0;
  TCCR1B = 1;  // enable Timer1 with prescaler = 1 ( 16 ticks each 1 µs)
  TCNT1  = 0;  // set Timer1 preload value to 0 (reset)
  TIMSK1 = 1;  // enable Timer1 overflow interrupt
}

void loop() {
  if (!gameOver && !unlocked && !lock) {
    if (!playingLcdGame && !playingComboLock && !playingCodeWord) {
      choice = getChoice();
      if (choice == 0) {
        setUpLcdGame();
        lcdGame();
      }
      else if (choice == 1) {
        setUpComboLock();
        comboLock();
      }
      else if (choice == 2) {
        setUpCodeWords();
        codeWords();
      }
    }
    if (playingLcdGame) {
      lcdGame();
    }
    if (playingComboLock) {
      comboLock();
    }
    if (playingCodeWord) {
      codeWords();
    }
    if (timerGoing) {
      decreaseTimer -= 1;
      // Serial.println(decreaseTimer);
      if (decreaseTimer == 0) {
        num--;
        // Serial.println(num);
        decreaseTimer = 5;
      }
    } 
    if (fails >= 3) {
      gameOver = true;
      intruder();
    }
    if (points == 3) {
      unlocked = true;
      unlock();
    }
  }
  else {
    if ((unlocked && readBtns() == 0) || lock) {
      unlocked = false;
      lock = true;
      lockSafe();
    }
    if (unlocked) {
      unlock();
    }
    if (gameOver) {
      intruder();
    }
  }
  delay(50);
}

// OVERALL FUNCTIONS
void shuffleList(char* arr[]) {
  const int arrLength = sizeof(arr) / sizeof(arr[0]);
  for (int i=0; i < arrLength; i++) {
    int n = random(0, arrLength);  // Integer from 0 to arrLength-1
    int temp = arr[n];
    arr[n] = arr[i];
    arr[i] = temp;
  }
}

int readBtns() {
  btnVal = analogRead(btnsPin); //Read analog value from A3 pin
	
	//For 1st (select) button
	if (btnVal >= 0 && btnVal <= 90){
		return 0;
	}
	//For 2nd button:
	else if (btnVal >= 450 && btnVal <= 550){
		return 1;
	}
	//For 3rd button:
	else if (btnVal >= 551  && btnVal <= 650){
		return 2;
	}
	//For 4th button:
	else if (btnVal >= 651  && btnVal <= 740){
		return 3;
	}
  //For 5th button:
	else if (btnVal >= 741  && btnVal <= 800){
		return 4;
	}
	//No button pressed
	else{
		return 5;
  }
}

// START SCREEN
int getChoice() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(games[gameIndex]);
  if (completed[gameIndex]) {
    lcd.setCursor(nameLengths[gameIndex], 0);
    lcd.print(" ");
    lcd.setCursor(nameLengths[gameIndex]+1, 0);
    lcd.print(char(1));
  }
  lcd.setCursor(0, 1);
  lcd.print(points);
  lcd.setCursor(1, 1);
  lcd.print("/3DONE");
  lcd.setCursor(8, 1);
  lcd.print(fails);
  lcd.setCursor(9, 1);
  lcd.print("/3FAIL");
  currentX = analogRead(xPin);
  currentY = analogRead(yPin);
  if (currentX < 400 || currentY < 400) {
    if ((gameIndex - 1) < 0) {
      gameIndex = 2;
    }
    else {
      gameIndex--;
    }
  }
  if (currentX > 600 || currentY > 600) {
    gameIndex = (gameIndex + 1) % 3;
  }
  delay(200);
  if (readBtns() == 0) {
    return gameIndex;
  }
  else {
    return 3;
  }
}

// UNLOCK
void unlock() {
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("ACCESS");
  lcd.setCursor(4, 1);
  lcd.print("GRANTED!");
  tone(buzzerPin, NOTE_C4, 20);
  analogWrite(motorPin, 255);
  delay(200);
}

// LOCK
void lockSafe() {
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("LOCKED");
  analogWrite(motorPin, 0);
  delay(200);
}

// INTRUDER
void intruder() {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("INTRUDER!");
  lcd.setCursor(1, 1);
  lcd.print("ALARM ALARM");
  tone(buzzerPin, NOTE_D5, 20);
  delay(200);
}

// TIMER
ISR(TIMER1_OVF_vect)   // Timer1 interrupt service routine (ISR)
{
  if (startTimer) {
    if (!timerGoing) {
      timerGoing = true;
      num = timerLength;
    }
    screenOff();
    writeNumber();
    if (num == 0) {
      stopTimer();
    }
  }
}

void writeNumber() {
  int digits[4] = {0, 0, 0, 0};
  int copyNum = num;
  for (int i = 0; i < 4; i++) {
    digits[i] = copyNum % 10;
    copyNum = copyNum / 10;
  }

  // for (int i = 0; i < 4; i++) {
  digitalWrite(clockPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, byteDigits[digits[currentDigit]]);
  digitalWrite(digitPins[currentDigit], LOW);
  digitalWrite(clockPin, HIGH);
  // }
  currentDigit = (currentDigit + 1) % 4;
}

void screenOff() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(digitPins[i], HIGH);
  }
}

void setTimer(int time) {
  timerLength = time;
  startTimer = true;
}

void stopTimer() {
  startTimer = false;
  timerGoing = false;
}

// LCD GAME
void displayAll() {
  lcd.clear();
  lcd.setCursor(15, 0);
  lcd.print(hearts);
  lcd.setCursor(charPos[0],charPos[1]);
  lcd.print("X");
  if (obstacleGoing) {
    for (int i = 0; i < numBlocks; i++) {
      if (blockPos[0]+i < 16) {
        lcd.setCursor(blockPos[0]+i, blockPos[1]);
        lcd.print("O");
      }
    }
  }
}

void moveChar() {
  currentX = analogRead(xPin);
  currentY = analogRead(yPin);
  if (!charJumping && ((currentX < 400 || currentX > 600) || (currentY < 400 || currentY > 600))) {
    // Serial.println("JUMPING");
    charPos[1] = 0;
    displayAll();
    jumpNum = 2*numBlocks;
    charJumping = true;
  }
  if (charJumping) {
    jumpNum -= 1;
  }
  if (jumpNum <= 0) {
    charPos[1] = 1;
    displayAll();
    charJumping = false;
  }
  if (charPos[0] == blockPos[0] && charPos[1] == blockPos[1] && !invincible) {
    hearts--;
    invincible = true;
  }
}

void moveBlocks() {
  if (obstacleGoing) {
    // Serial.println("BLOCK");
    blockPos[0]--;
  }
  if (blockPos[0] < 0) {
    if (numBlocks == 2) {
      numBlocks = random(1, 5);
      blockPos[0] = 15;
      obstacleGoing = false;
      invincible = false;
    }
    else if (numBlocks > 1) {
      blockPos[0] = 0;
      numBlocks -= 1;
    }
    else {
      numBlocks = random(1, 5);
      blockPos[0] = 15;
      obstacleGoing = false;
      invincible = false;
    }
  }
}

void setUpLcdGame() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Avoid the blocks");
  lcd.setCursor(0, 1);
  lcd.print("Move joystick");
  delay(1000);

  hearts = 3;
  invincible = false;
  playingLcdGame = true;

  setTimer(20);
}

void lcdGame() {
  if (num > 0 && hearts > 0) {
    if (random(1000) < 400 && !obstacleGoing) {
      if (random(1000) < 500) {
        blockPos[1] = 0;
      }
      else {
        blockPos[1] = 1;
      }
      obstacleGoing = true;
    }
    moveChar();
    moveBlocks();
    displayAll();
    delay(200);
  }
  if (num <= 0 && hearts > 0) {
    if (!completed[lcdIndex]) {
      points += 1;
    }
    playingLcdGame = false;
    completed[lcdIndex] = true;
    stopTimer();
  }
  if (hearts <= 0) {
    fails++;
    playingLcdGame = false;
    stopTimer();
  }
}

// COMBINATION LOCK
void setUpComboLock() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter the");
  lcd.setCursor(0, 1);
  lcd.print("combination");
  delay(1000);

  attempt = 0;
  comboIndex = 0;
  comboBtn = combination[comboIndex];
  playingComboLock = true;
  setTimer(60);
}

void comboLock() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Number: ");
  lcd.setCursor(7, 0);
  lcd.print(comboIndex);
  lcd.setCursor(0, 1);
  lcd.print("Attempt: ");
  lcd.setCursor(9, 1);
  lcd.print(attempt);

  btnPressed = readBtns();

  if (btnPressed == comboBtn) {
    comboIndex++;
    if (comboIndex == comboLength) {
      if (!completed[combinationIndex]) {
        points++;
      }
      stopTimer();
      playingComboLock = false;
      completed[combinationIndex] = true;
    }
    else {
      comboBtn = combination[comboIndex];
    }
  }
  else if (btnPressed != 5 && btnPressed != 0) {
    attempt++;
    if (attempt == 3) {
      fails++;
      stopTimer();
      playingComboLock = false;
    }
  }

  if (num <= 0) {
    fails++;
    stopTimer();
    playingComboLock = false;
  }

  delay(150);
}

// CODE WORD
void setUpCodeWords() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Select the");
  lcd.setCursor(0,1);
  lcd.print("code word");
  delay(1000);

  playingCodeWord = true;
  wordIndex = random(wordsLength);

  // for (size_t i = 0; i < wordsLength; i++) {
  //   size_t j = random(0, wordsLength - i);
  //   int t = words[i];
  //   words[i] = words[j];
  //   words[j] = t;
  // }
  // Serial.println(words[wordIndex]);
  // for (int i=0; i < wordsLength; i++) {
  //   if (codeWord = words[i]) {
  //     codeWordIndex = i;
  //     break;
  //   }
  // }
  // Serial.println(codeWordIndex);
  // Serial.println(words[codeWordIndex]);
  setTimer(20);
}

void codeWords() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(words[wordIndex]);

  currentX = analogRead(xPin);
  currentY = analogRead(yPin);
  if (currentX < 400 || currentY < 400) {
    if ((wordIndex - 1) < 0) {
      wordIndex = wordsLength - 1;
    }
    else {
      wordIndex--;
    }
  }
  if (currentX > 600 || currentY > 600) {
    wordIndex = (wordIndex + 1) % wordsLength;
  }
  if (num <= 0) {
    fails++;
    playingCodeWord = false;
    stopTimer();
  }
  else if (readBtns() == 0) {
    if (wordIndex == codeWordIndex) {
      if (!completed[codeIndex]) {
        points += 1;
      }
      completed[codeIndex] = true;
    }
    else {
      fails++;
    }
    playingCodeWord = false;
    stopTimer();
  }

  delay(200);
}
