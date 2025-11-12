#include <Joystick.h>

const uint8_t PIN_KIER_LEWY   = 2;
const uint8_t PIN_KIER_PRAWY  = 3;
const uint8_t PIN_DLUGIE      = 4;

const uint8_t BTN_LEWY   = 0;
const uint8_t BTN_PRAWY  = 1;
const uint8_t BTN_DLUGIE = 2;

const unsigned long DEBOUNCE_MS          = 25;
const unsigned long NEUTRAL_COOLDOWN_MS  = 250;
const unsigned long TAP_HOLD_MS          = 80;   // czas przytrzymania przycisku

enum Kierunek { NEUTRAL, LEWY, PRAWY };
Kierunek aktualnyKierunek = NEUTRAL;

struct DebouncePin {
  uint8_t pin;
  bool stableState;
  bool lastRaw;
  unsigned long lastChange;
};

DebouncePin bLewy  = {PIN_KIER_LEWY,  true, true, 0};
DebouncePin bPrawy = {PIN_KIER_PRAWY, true, true, 0};

unsigned long lastNeutralTapAt = 0;

Joystick_ joystick(
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_JOYSTICK,
  3, // liczba przycisków
  0, // brak osi
  false, false, false, false, false, false,
  false, false, false, false, false
);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== START DEBUG MODE ===");
  
  pinMode(PIN_KIER_LEWY,  INPUT_PULLUP);
  pinMode(PIN_KIER_PRAWY, INPUT_PULLUP);
  pinMode(PIN_DLUGIE,     INPUT_PULLUP);
  joystick.begin();

  Serial.println("Piny ustawione jako INPUT_PULLUP");
  Serial.println("Oczekiwanie na wejścia...");
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
    Serial.print("[");
    Serial.print(name);
    Serial.print("] Stabilna zmiana: ");
    Serial.println(bp.stableState == LOW ? "LOW (wcisniety)" : "HIGH (puszczony)");
    if (bp.stableState == LOW) return true;
  }

  return false;
}

void tap(uint8_t btn, const char* nazwa) {
  Serial.print("  -> TAP ");
  Serial.print(nazwa);
  Serial.print(" przez ");
  Serial.print(TAP_HOLD_MS);
  Serial.println(" ms");

  joystick.setButton(btn, true);
  delay(TAP_HOLD_MS);
  joystick.setButton(btn, false);
}

void loop() {
  unsigned long now = millis();

  bool edgeLewy  = debounceFallingEdge(bLewy,  "Lewy");
  bool edgePrawy = debounceFallingEdge(bPrawy, "Prawy");

  if (edgeLewy) {
    tap(BTN_LEWY, "LEWY");
    aktualnyKierunek = LEWY;
    Serial.println("Ustawiono kierunek: LEWY");
  }
o
  if (edgePrawy) {
    tap(BTN_PRAWY, "PRAWY");
    aktualnyKierunek = PRAWY;
    Serial.println("Ustawiono kierunek: PRAWY");
  }

  bool neutralStable = (bLewy.stableState == HIGH) && (bPrawy.stableState == HIGH);

  if (neutralStable && aktualnyKierunek != NEUTRAL && (now - lastNeutralTapAt) > NEUTRAL_COOLDOWN_MS) {
    Serial.println("Powrot do NEUTRAL → wysylam tap OFF");
    if (aktualnyKierunek == LEWY) tap(BTN_LEWY, "LEWY OFF");
    else if (aktualnyKierunek == PRAWY) tap(BTN_PRAWY, "PRAWY OFF");
    aktualnyKierunek = NEUTRAL;
    lastNeutralTapAt = now;
}

static bool lastDlugieState = false;
bool nowDlugieState = (digitalRead(PIN_DLUGIE) == LOW);

if (nowDlugieState != lastDlugieState) {
    lastDlugieState = nowDlugieState;

    if (!nowDlugieState) {
        joystick.setButton(BTN_DLUGIE, true);
        delay(TAP_HOLD_MS);
        joystick.setButton(BTN_DLUGIE, false);
        Serial.println("Światła długie: automatyczny TAP przy puszczeniu");
    } else {
        joystick.setButton(BTN_DLUGIE, true);
        delay(TAP_HOLD_MS);
        joystick.setButton(BTN_DLUGIE, false);
        Serial.println("Światła długie: TAP przy wciśnięciu");
    }
}

  static unsigned long lastStatePrint = 0;
  if (now - lastStatePrint > 500) {
    Serial.print("[STANY] Lewy=");
    Serial.print(digitalRead(PIN_KIER_LEWY));
    Serial.print("  Prawy=");
    Serial.print(digitalRead(PIN_KIER_PRAWY));
    Serial.print("  Aktualny=");
    if (aktualnyKierunek == LEWY) Serial.print("LEWY");
    else if (aktualnyKierunek == PRAWY) Serial.print("PRAWY");
    else Serial.print("NEUTRAL");
    Serial.print("  Długie=");
    Serial.println(nowDlugieState ? "ON" : "OFF");
    lastStatePrint = now;
  }

  delay(1);
}
