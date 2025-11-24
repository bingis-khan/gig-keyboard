#include <Keyboard.h>
#include <Mouse.h>

// because we do pullup, the shit is actually reversed.
// So, unlike https://www.dribin.org/dave/keyboard/one_html/,
//  *we are powering up ROWS and reading COLUMNS.*

// QMK style defs for style
// ROWS (top - bottom):
#define ROWS { 0, 1, 2, 3 }
// LEFT COLUMNS (left-right):
// #define COLUMNS { 10, 4, 7, 6, 5, 8, 29, 28, 11, 26, 15, 14 }
#define COLUMNS { 14, 15, 26, 11, 28, 29, 8, 5, 6, 7, 4, 10 }
#define CHARS \
  { { KEY_ESC, 'q', 'w', 'e', 'r', 't',         'y', 'u', 'i', 'o', 'p', KEY_BACKSPACE  } \
  , { KEY_LEFT_CTRL, 'a', 's', 'd', 'f', 'g',   'h', 'j', 'k', 'l', ';', KEY_RETURN } \
  , { KEY_LEFT_SHIFT, 'z', 'x', 'c', 'v', 'b',  'n', 'm', ',', '.', '/', KEY_RIGHT_SHIFT } \
  , { KEY_LEFT_ALT, KEY_LEFT_GUI, NO_KEY, 0, ' ', NO_KEY,  ' ', KEY_RIGHT_ALT, '-', '=', '\'', 0 } \
  }

// special keyboard keys.

// num switch
#define NUM_SWITCH_PIN_IDS 3,3
// fn switch
#define FN_SWITCH_PIN_IDS 3,0
// mouse switch
#define JUICE_PIN_IDS 3,11
// where the numeric part begins.
#define NUM_BEGIN_CI 1
#define NUM_BEGIN_RI 0

const int rows[] = ROWS;
const int columns[] = COLUMNS;
#define ROWS_NUM (sizeof(rows)/sizeof(int))
#define COLUMNS_NUM (sizeof(columns)/sizeof(int))
#define NO_KEY ((char)0)


typedef struct {
  char key;
  char num_key;  // when NUM SWITCH is pressed.
  char fn_key;
  char last_pressed_key;  // NO_KEY or an actual key value.
} Key;

Key chars[ROWS_NUM][COLUMNS_NUM];


typedef struct {
  int last_moved;  // both up/down wheel. (a struct, because we might split it into last_moved_up/down)
} MouseWheel;

MouseWheel wheel;


void setup() {
  // set ROWS as OUTPUT
  for (int i = 0; i < ROWS_NUM; i++) {
    pinMode(rows[i], OUTPUT);
  }

  for (int i = 0; i < COLUMNS_NUM; i++) {
    pinMode(columns[i], INPUT_PULLUP);
  }

  // setup keys
  const char char_keys[ROWS_NUM][COLUMNS_NUM] = CHARS;
  for (int ri = 0; ri < ROWS_NUM; ri++) {
    for (int ci = 0; ci < COLUMNS_NUM; ci++) {
      char default_key = char_keys[ri][ci];

      // default case so that I won't have to change = and :
      // JUST LIKE MY HECKING HASKELLINO
      #define CASE false ? NO_KEY
      char num_key = CASE
          // arrows
          // im using vim style arrows now,
          // but my keyboard is on a "grid", so i can just
          // simulate normal arrows.
          //  Also, In this case there should be a toggle (the juice button?)
          : default_key == 'h' ? KEY_LEFT_ARROW
          : default_key == 'j' ? KEY_DOWN_ARROW
          : default_key == 'k' ? KEY_UP_ARROW
          : default_key == 'l' ? KEY_RIGHT_ARROW

          // ctrl -> tab
          // EDIT: this is hard... I use tab, but at the same time I want to use ctrl+arrow keys to move around a command.
          // EDIT: CHANGED LCTRL TO A.
          : default_key == 'a' ? KEY_TAB
          
          // backspace -> delete
          // EDIT: stupid idea - super annoying in practice. I'll leave DELETE unmapped for now.
          // : default_key == KEY_BACKSPACE ? KEY_DELETE

          // misc keys which didn't make the cut
          : default_key == '-' ? '['   // NOTE: not the < > keys, because they have '.' and ',' which are useful when writing decimal points.
          : default_key == '=' ? ']'
          : default_key == KEY_ESC ? '`'
          : default_key == '/' ? '\\'

          : ci >= NUM_BEGIN_CI && ci < (NUM_BEGIN_CI + 10) && ri == NUM_BEGIN_RI
            ? (char)('0' + ((ci - NUM_BEGIN_CI + 1) % 10))
          : default_key;

      char fn_key = CASE
          : default_key == KEY_LEFT_ALT ? NO_KEY  // if both FN and NUM are pressed, release LALT. (BUG: lettuing go of NUM does not repress the LALT.)
                                                  // why?  when LALT is pressed in firefox, PGUP and PGDOWN don't work.

          : default_key == 'j' ? KEY_PAGE_DOWN
          : default_key == 'k' ? KEY_PAGE_UP

          // HACK to enable CTRL + LEFT/RIGHT ARROW in terminal and controls
          : default_key == KEY_LEFT_CTRL ? KEY_LEFT_CTRL

          : ci >= NUM_BEGIN_CI && ci < (NUM_BEGIN_CI + 10) && ri == NUM_BEGIN_RI
            ? ((char)(KEY_F1 + (ci - NUM_BEGIN_CI)))  // NOTE: formula works until F12 (https://docs.arduino.cc/language-reference/en/functions/usb/Keyboard/keyboardModifiers/#escape-and-function-keys)
          : num_key;

      chars[ri][ci] = (Key) {
        .key = default_key,
        .num_key = num_key,
        .fn_key = fn_key,
        .last_pressed_key = NO_KEY,
      };
    }
  }


  // mouse init
  wheel.last_moved = millis();

  Keyboard.begin();
  Mouse.begin();
  Serial.begin(115200);
}

void loop() {
  // read special modifier keys
  bool num_switch = quick_check(NUM_SWITCH_PIN_IDS);
  bool special_switch = quick_check(FN_SWITCH_PIN_IDS);
  bool mouse_switch = quick_check(JUICE_PIN_IDS);

  if (!mouse_switch) {
    unpress_mouse();
  }


  // read keyboard in general
  {
    for (int ri = 0; ri < ROWS_NUM; ri++) {
      const int r = rows[ri];
      digitalWrite(r, LOW);
      for (int ci = 0; ci < COLUMNS_NUM; ci++) {
        const int c = columns[ci];

        Key *const k = &chars[ri][ci];


        const char key
          = special_switch && num_switch ? k->fn_key
          : num_switch ? k->num_key
          : k->key;

        // TODO: some logic restructure (key release in two places - kinda cringe)
        // ANOTHER TODO: make use of internal keyboard structure and eliminate last_pressed_key (and change it to isPressed? maybe less bugs, but more calls, as we're asking for each key)
        if ((k->last_pressed_key != key || (mouse_switch && ci >= 6)) && k->last_pressed_key != NO_KEY) {
          Keyboard.release(k->last_pressed_key);
        }

        // special mouse escape hatch.
        if (mouse_switch && ci >= 6) {  // NOTE: mouse look hack. because we're using only the right side (except modifiers), we can use the left side as a normal keyboard. this makes it possible to sorta play videogames with mouse look.
          handle_mouse(num_switch, special_switch, c, k);
          continue;
        }

        if (key == NO_KEY) {
          continue;
        }

        // HACK mouse look: don't press lalt when juiced
        if (key == KEY_LEFT_ALT && mouse_switch) {
          continue;
        }

        const bool pressed = digitalRead(c) == LOW;
        if (k->last_pressed_key != key && pressed) Keyboard.press(key);
        if (k->last_pressed_key != NO_KEY && !pressed) Keyboard.release(k->last_pressed_key);
        k->last_pressed_key = pressed ? key : NO_KEY;
      }
      digitalWrite(r, HIGH);
    }
  }

  delay(10);  // this seems to be enough for debouncing.
}


static void handle_mouse(bool slower, bool faster, int c, Key *const k) {
  const bool pressed = digitalRead(c) == LOW;

  int cursor_speed = CASE
    : slower && !faster ? 1
    : faster && !slower ? 12
    : 5;
    // with my experiments, I don't think we need a fourth speed.

  int wheel_delay = CASE
    : slower && !faster ? 200
    : !slower && faster ? 10
    : 40;
  
  switch (k->key) {
    // mouse wheel
    // speed 1 is already plenty fast
    case 'j': {
      int t = millis();
      if (pressed && t - wheel.last_moved >= wheel_delay) {
        Mouse.move(0, 0, -1);
        wheel.last_moved = t;
      }
    } break;
    case 'k': {
      int t = millis();
      if (pressed && t - wheel.last_moved >= wheel_delay) {
        Mouse.move(0, 0, 1);
        wheel.last_moved = t;
      }
    } break;

    // cursor movement
    case '.': {
        if (pressed) Mouse.move(0, -cursor_speed);
    }; break;
    case '=': {
        if (pressed) Mouse.move(0, cursor_speed);
    }; break;
    case '-': {
        if (pressed) Mouse.move(-cursor_speed, 0);
    }; break;
    case '\'': {
        if (pressed) Mouse.move(cursor_speed, 0);
    }; break;

    // cursor buttons
    // TD: maybe just call a function...
    case ',': {
        handle_press(pressed, MOUSE_LEFT);
    } break;
    case 'l': {
        handle_press(pressed, MOUSE_MIDDLE);
    } break;
    case '/': {
        handle_press(pressed, MOUSE_RIGHT);
    } break;
  }
}

static void handle_press(bool pressed, int mb) {
  if (pressed && !Mouse.isPressed(mb)) Mouse.press(mb);
  else if (!pressed && Mouse.isPressed(mb)) Mouse.release(mb);
}

static void unpress_mouse() {
  if (Mouse.isPressed(MOUSE_LEFT))   Mouse.release(MOUSE_LEFT);
  if (Mouse.isPressed(MOUSE_MIDDLE)) Mouse.release(MOUSE_MIDDLE);
  if (Mouse.isPressed(MOUSE_RIGHT))  Mouse.release(MOUSE_RIGHT);
}


// geg
static bool quick_check(int ri, int ci) {
  const int r = rows[ri];
  digitalWrite(r, LOW);

  const int c = columns[ci];
  const char key = chars[ri][ci].key;
  const bool pressed = digitalRead(c) == LOW;

  digitalWrite(r, HIGH);
  return pressed;
}

// meant to work with the "pair" #defines.
static bool is_pair_equal(int r1, int c1, int r2, int c2) {
  return r1 == r2 && c1 == c2;
}
