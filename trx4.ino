byte light = LOW;
byte gear = LOW;
unsigned long gear_last_activated = 0;
unsigned long gear_last_deactivated = 0;
unsigned long differential_last_active = 0;
byte differential_front_state = UNLOCKED;
byte differential_back_state = UNLOCKED;
bool light_toggle_lock = false;
bool differential_front_lock = false;
bool differential_back_lock = false;


void light_on() {
  Serial.println("Light on");
  digitalWrite(LIGHT_PIN, HIGH);
  light = HIGH;
}


void light_off() {
  Serial.println("Light off");
  digitalWrite(LIGHT_PIN, LOW);
  light = LOW;
}

void toogle_light() {
  if (light == LOW)
    light_on();
  else {
    light_off();
  }
}

void set_high_gear() {
  Serial.println("HIGH gear");
  gear_servo.writeMicroseconds(GEAR_HIGH);
  gear = HIGH;
}

void set_low_gear() {
  Serial.println("LOW gear");
  gear_servo.writeMicroseconds(GEAR_LOW);
  gear = LOW;
}

void toggle_gear() {
  if (gear == LOW) {
    set_high_gear();
  } else {
    set_low_gear();
  }
}

void set_front(byte state) {
  int uS;
  if (state == LOCKED) {
    uS = DIFFERENTIAL_FRONT_LOCK;
  } else if (state == UNLOCKED) {
    uS = DIFFERENTIAL_FRONT_UNLOCK;
  } else {
    Serial.println("Warning got unkonwn state for front differential!");
    return;
  }
  differential_front_state = state;
  Serial.println("Front differential " + String(state == LOCKED ? "locked" : "unlocked") );
  differential_front_servo.writeMicroseconds(uS);
}

void set_back(byte state) {
  int uS;
  if (state == LOCKED) {
    uS = DIFFERENTIAL_BACK_LOCK;
  } else if (state == UNLOCKED) {
    uS = DIFFERENTIAL_BACK_UNLOCK;
  } else {
    Serial.println("Warning got unkonwn state for back differential!");
    return;
  }
  differential_back_state = state;
  Serial.println("Back differential " + String(state == LOCKED ? "locked" : "unlocked") );
  differential_back_servo.writeMicroseconds(uS);
}

void toggle_front() {
  set_front(!differential_front_state);
}

void toggle_back() {
  set_back(!differential_back_state);
}

void differential(float pos) {
  if (pos > DIFFERENTIAL_FRONT_BOUND && !differential_front_lock) {
    toggle_front();
    differential_front_lock = true;
  } else if (pos < DIFFERENTIAL_BACK_BOUND && !differential_front_lock) {
    toggle_back();
    differential_front_lock = true;
  } else if (pos < DIFFERENTIAL_FRONT_BOUND && pos > DIFFERENTIAL_BACK_BOUND) {
    differential_front_lock = false;
  }
}

void gear_light(float pos) {
  now = millis();
  unsigned long period = now - gear_last_activated;
  if (pos > GEAR_LIGHT_UPPER_BOUND && !gear_last_activated) {
    gear_last_activated = now;
    log(DEBUG, "pos above upper bound and not last active");
  } else if (pos > GEAR_LIGHT_UPPER_BOUND && gear_last_activated) {
    log(DEBUG, "pos above upper bound and last active");
    if (period > LIGHT_TIME && !light_toggle_lock) {
      log(DEBUG, "period longer then light and not locked");
      toogle_light();
      light_toggle_lock = true;
    }
  }
  else if (pos < GEAR_LIGHT_LOWER_BOUND && gear_last_activated) {
    log(DEBUG, "pos below lower bound and last active");
    unsigned long pause = now - gear_last_deactivated;
    if (pause < DOUBLEE_CLICK_TIME) {
      reset_state();
    } else if (period > GEAR_TIME && period < LIGHT_TIME) {
      toggle_gear();
    }
    gear_last_activated = 0;
    gear_last_deactivated = now;
    light_toggle_lock = false;
  }
}

void winch(float pos) {
  if (pos > 0.5) {
    digitalWrite(WINCH_IN_PIN, LOW);
    digitalWrite(WINCH_OUT_PIN, HIGH);
    Serial.println("winch OUT");
  } else if (pos < -0.5) {
    digitalWrite(WINCH_IN_PIN, HIGH);
    digitalWrite(WINCH_OUT_PIN, LOW);
    Serial.println("winch IN");
  } else {
    digitalWrite(WINCH_IN_PIN, LOW);
    digitalWrite(WINCH_OUT_PIN, LOW);
  }
}

void evaluate(int CH, float pos) {
  log(DEBUG, "Channel: " + String(CH) + " | " + String(pos));
  if (CH == DIFFERENTIAL_CH) {
    log(DEBUG, "Channel: " + String(CH) + " Differential");
    differential(pos);
  } else if (CH == GEAR_LIGHT_CH) {
    log(DEBUG, "Channel: " + String(CH) + " gear & light");
    gear_light(pos);
  } else if (CH == WINCH_CH) {
    log(DEBUG, "Channel: " + String(CH) + " winch");
    winch(pos);
  }
}
