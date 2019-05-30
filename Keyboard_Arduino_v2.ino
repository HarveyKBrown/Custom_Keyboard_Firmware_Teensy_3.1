//Working Keyboard v.1.2 +Multiple Modifier key support +function layer -mediakeys
#include <String.h>

/* Maps Teensy 3 pins to row and collumn numbers in keyboard matrix
 * Top -> Bottom
 * Left -> Right
 * 
 * Collumn Skips are;
 * 17: OOOOX 
 * 8: OOOXX
 * 9: OOXOO
*/
const byte ROWS = 6;
const byte COLLUMNS = 15;
const byte rowPins[ROWS] = {0, 4, 3, 12, 14, 15};
const byte collumnPins[COLLUMNS] = {23, 22, 21, 20, 19, 18, 17, 16, 0, 6, 7, 8, 9, 10, 11};

/* Map of keys on keyboard
 *  Top row is function layer
 *  Includes modifier keys
 *  EU key codes are given in HEX values
 *  uses HID keycode standard for keyboard input
 */
const int keyMap[6][15] = {
  {0x35, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, 0, 0},
  {KEY_ESC, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE, KEY_DELETE},
  {KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_LEFT_BRACE, KEY_RIGHT_BRACE, 0x64, KEY_PRINTSCREEN},
  {KEY_LEFT_CTRL, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_QUOTE, NULL, KEY_ENTER, KEY_PAGE_UP},
  {MODIFIERKEY_LEFT_SHIFT, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH, NULL, KEY_RIGHT_SHIFT, KEY_UP, KEY_PAGE_DOWN},
  {KEY_LEFT_CTRL, KEY_LEFT_GUI, 0, KEY_RIGHT_ALT, KEY_LEFT_ALT, KEY_SPACE, NULL, 0, 0, 0x32, KEY_RIGHT_CTRL, NULL, KEY_LEFT, KEY_DOWN, KEY_RIGHT} 
}; 

/* Key modifier map
 *  True if keyMap[Position] is KEY_MODIFIER
 */
bool modifierMap[6][15] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
  {1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0}
};


/* Key pressed map
 *  Stores if they key is technically "Pressed"
 */
bool pressedMap[6][15] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

/* Stores current "active" keys in array 
 */
const int KEY_BUFFER_LENGTH = 6;
int activeKeys[KEY_BUFFER_LENGTH] = {0, 0, 0, 0, 0, 0};
char modifierKeys = 0;






/* Sets pin modes
 *  Rows to output
 *  - Skips row 0 (reserved for function layer)
 *  Collumns to pullup resistor inputs
 */
void setup() {
  Serial.begin(9600);
  for (int i = 1; i < ROWS; i++) { pinMode(rowPins[i], OUTPUT); }
  for (int i = 0; i < COLLUMNS; i++) { pinMode(collumnPins[i], INPUT_PULLUP); }
}







/* Ensures drain on all pins
 * Ends by setting drain open on selected pin
 */
void setDrain(int pinIndex) {
  for (int i = 0; i < COLLUMNS; i++) {
    digitalWrite(rowPins[i], HIGH);
  }
  digitalWrite(rowPins[pinIndex], LOW);
}

/* Sets key slots to match activeKeys array
 */
void updateKeys() {
  Keyboard.set_key1(activeKeys[5]);
  Keyboard.set_key2(activeKeys[4]);
  Keyboard.set_key3(activeKeys[3]);
  Keyboard.set_key4(activeKeys[2]);
  Keyboard.set_key5(activeKeys[1]);
  Keyboard.set_key6(activeKeys[0]);
}

/* Gets key, checks ifPressed and activeKeys index
 *  - Paths out modifier keys to be handled seperately
 *  - Switches to function key row when function key is held down
 *  Removed key if not pressed
 *  Attempts to add key if pressed
 */
bool sendKey(int i, int j) {
  bool pressed = (digitalRead(collumnPins[j]) == LOW);
  if (pressed && modifierMap[i][j]) {
    //Keyboard.set_modifier(keyMap[i][j]);
    modifierKeys = modifierKeys + keyMap[i][j];
    return true;
  }
  
  //Function Layer Activator
  if ((
       pressedMap[5][2]
    || pressedMap[5][8]
    || pressedMap[3][0]
    ) && i == 1) { i = 0; }
  
  if (pressed && !pressedMap[i][j]) { keyPress(i, j); return true; }
  if (!pressed && pressedMap[i][j]) { keyRelease(i, j); return true; }
  return false;
}

void keyPress(int i, int j) {
  pressedMap[i][j] = 1;

  //Add key to list
  for (int k = 0; k < KEY_BUFFER_LENGTH; k++) {
    if (activeKeys[k] == NULL) { activeKeys[k] = keyMap[i][j]; break; } 
  }
}

void keyRelease(int i, int j) {
  pressedMap[i][j] = 0;
  //remove key from list
  for (int k = 0; k < KEY_BUFFER_LENGTH; k++) {
    if (activeKeys[k] == keyMap[i][j]) { activeKeys[k] = NULL; break; }
  }
}

void loop() {
  Keyboard.set_modifier(0);
  modifierKeys = 0;
  
  for (int i = 0; i < ROWS; i++) {
    setDrain(i);
    for (int j = 0; j < COLLUMNS; j++) { sendKey(i, j); }
  }

  Keyboard.set_modifier(modifierKeys);
  updateKeys();
  Keyboard.send_now();
  delay(50);
}





