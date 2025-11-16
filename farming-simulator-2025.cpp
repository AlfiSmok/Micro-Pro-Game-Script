#include <Joystick.h>

const uint8_t PIN_KIER_LEWY   = 2;
const uint8_t PIN_KIER_PRAWY  = 3;
const uint8_t PIN_DLUGIE      = 4;

const uint8_t BTN_LEWY   = 0;
const uint8_t BTN_PRAWY  = 1;
const uint8_t BTN_DLUGIE = 2;

const unsigned long DEBOUNCE_MS          = 25;
const unsigned long NEUTRAL_COOLDOWN_MS  = 250;
const unsigned long CHANGE_DIRECTION_DELAY_MS = 50; // <--- NOWE

enum Kierunek { NEUTRAL, LEWY, PRAWY };
Kierunek aktualnyKierunek = NEUTRAL;

unsigned long lastDirectionChange = 0;

struct DebouncePin {
  uint8_t pin;
  bool stableState;
  bool lastRaw;
  unsigned long lastChange;
};

DebouncePin bLewy  = {PIN_KIER_LEWY,  true, true, 0};
DebouncePin bPrawy = {PIN_KIER_PRAWY, true, true, 0};

bool dlugie = false;
unsigned long lastNeutralTapAt = 0;

Joystick_ joystick(
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_JOYSTICK,
  3,
  0,
  false, false, false, false, false, false,
  false, false, false, false, false
);

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_KIER_LEWY,  INPUT_PULLUP);
  pinMode(PIN_KIER_PRAWY, INPUT_PULLUP);
  pinMode(PIN_DLUGIE,     INPUT_PULLUP);
  joystick.begin();
}

bool debounceFallingEdge(DebouncePin &bp, const char* name) {
  bool raw = digitalRead(bp.pin);
  unsigned long now = millis();

  if (raw != bp.lastRaw) {
    bp.lastChange = now;
    bp.lastRaw = raw;
  }

  if ((now - bp.lastChange) > DEBOUNCE_MS && raw != bp.stableState) {
    bp.stableState = raw;
    if (bp.stableState == LOW) return true; 
  }

  return false;
}

void tap(uint8_t btn) {
  joystick.setButton(btn, true);
  delay(20);
  joystick.setButton(btn, false);
}

void loop() {
  unsigned long now = millis();

  bool edgeLewy  = debounceFallingEdge(bLewy,  "Lewy");
  bool edgePrawy = debounceFallingEdge(bPrawy, "Prawy");

  bool neutralStable = (bLewy.stableState == HIGH) && (bPrawy.stableState == HIGH);

  bool allowDirectionChange = (now - lastDirectionChange) > CHANGE_DIRECTION_DELAY_MS;

  if (edgeLewy && allowDirectionChange) {
    tap(BTN_LEWY);
    aktualnyKierunek = LEWY;
    lastDirectionChange = now;
  }

  if (edgePrawy && allowDirectionChange) {
    tap(BTN_PRAWY);
    aktualnyKierunek = PRAWY;
    lastDirectionChange = now;
  }

  if (neutralStable && aktualnyKierunek != NEUTRAL && (now - lastNeutralTapAt) > NEUTRAL_COOLDOWN_MS) {

    if (aktualnyKierunek == LEWY) tap(BTN_LEWY);
    else if (aktualnyKierunek == PRAWY) tap(BTN_PRAWY);

    aktualnyKierunek = NEUTRAL;
    lastNeutralTapAt = now;
    lastDirectionChange = now; // reset żeby nie przełączać od razu
  }

  dlugie = (digitalRead(PIN_DLUGIE) == LOW);
  joystick.setButton(BTN_DLUGIE, dlugie);

  delay(1);
}