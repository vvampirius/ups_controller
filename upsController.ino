uint32_t get_time_delta(uint32_t since, uint32_t now) {
    uint32_t delta;
    if (since > now) {
      delta = UINT32_MAX - since + now;
    } else {
      delta = now - since;
    }
    return delta;
}

class ContactBounce {
  private:
    bool started = false;
    unsigned long since;
  
  public:
    unsigned long duration = 100;
    void start() {
      started = true;
      since = millis();
    }
    int get_state() {
      if (!started) return 0;
      unsigned long now = millis();
      uint32_t delta = get_time_delta(since, now);
      if (delta < duration) return 1;
      started = false;
      return 2;
    }
};

class Switch{
  private:
    int pin;
    ContactBounce contact_bounce;

  public:
    int state;
    unsigned long since;
    unsigned int change;

    Switch(int p){
      pinMode(p, INPUT);
      state = digitalRead(p);
      since = millis();
      pin = p;      
    }

    void update(){
      int contact_bounce_state = contact_bounce.get_state();
      if (contact_bounce_state == 1) return;
      int new_state = digitalRead(pin);
      if (new_state == state) return;
      if (contact_bounce_state == 0) {
        contact_bounce.start();
        return;
      }
      state = new_state;
      since = millis();
      change++;
    }
};

class LedLightDuration {
  private:
    uint8_t pin;
    bool enabled = false;
    unsigned long enabled_at;
    unsigned long duration;

  public:
    LedLightDuration(uint8_t _pin) {
      pin = _pin;
      pinMode(_pin, OUTPUT);
    }
    void enable(unsigned long _duration) {
      enabled = true;
      duration = _duration;
      enabled_at = millis();
      digitalWrite(pin, HIGH);
    }
    void update() {
      if (!enabled) return;
      if (get_time_delta(enabled_at, millis()) < duration) return;
      enabled = false;
      digitalWrite(pin, LOW);
    }
};

class LedBlink {
  private:
    uint8_t pin;
    bool started = false;
    uint8_t state = LOW;
    unsigned long state_changed_at;
    unsigned long low_state_duration;
    unsigned long high_state_duration;
    void switch_state() {
      switch (state) {
      case LOW:
        state = HIGH;
        state_changed_at = millis();
        digitalWrite(pin, HIGH);
        break;
      case HIGH:
        state = LOW;
        state_changed_at = millis();
        digitalWrite(pin, LOW);
        break;
      }
    }

  public:
    LedBlink(){}
    LedBlink(uint8_t _pin) {
      pin = _pin;
      pinMode(_pin, OUTPUT);
    }
    void start(unsigned long _high_state_duration, unsigned long _low_state_duration) {
      started = true;
      high_state_duration = _high_state_duration;
      low_state_duration = _low_state_duration;
      switch_state();
    }
    void stop() {
      started = false;
      digitalWrite(pin, LOW);
    }
    void update() {
      if (!started) return;
      unsigned long delta = get_time_delta(state_changed_at, millis());
      switch (state) {
        case LOW:
          if (delta < low_state_duration) return;
          break;
        case HIGH:
          if (delta < high_state_duration) return;
          break;
      }
      switch_state();
    }
};

class RouterRelay {
  private:
    uint8_t pin;

  public:
    RouterRelay(uint8_t _pin) {
      pin = _pin;
      pinMode(_pin, OUTPUT);
    }
    void update(int capacity) {
      if (capacity > 15) {
        digitalWrite(pin, HIGH);
      }
      if (capacity < 10) {
        digitalWrite(pin, LOW);
      }
    }
};

class Raspberry {
  private:
    uint8_t relay_pin;
    int shutdown_pin;
    int callbcak_pin;
    uint32_t led_pin;
    bool enabled = false;
    bool automatic = true;
    bool shutdown_started = false;
    unsigned long shutdown_started_at;
    LedBlink led_blink;
  
  public:
    int low_margin = 20;

    Raspberry(uint8_t _relay_pin, uint8_t _shutdown_pin, uint8_t _callback_pin, uint8_t _led_pin) {
      relay_pin = _relay_pin;
      shutdown_pin = _shutdown_pin;
      callbcak_pin = _callback_pin;
      led_pin = _led_pin;
      led_blink = LedBlink(_led_pin);
      pinMode(_relay_pin, OUTPUT);
      pinMode(_shutdown_pin, OUTPUT);
      pinMode(_callback_pin, INPUT);
    }

    void start_shutdown() {
      shutdown_started = true;
      shutdown_started_at = millis();
      led_blink.start(200, 200);
      digitalWrite(shutdown_pin, HIGH);
      //Serial.println("Shutdown started");
      //digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(shutdown_pin, LOW);
    }

    void update(int capacity) {
      led_blink.update();
      if (enabled && shutdown_started) {
        if (get_time_delta(shutdown_started_at, millis()) > 120000 || digitalRead(callbcak_pin) == HIGH) {
          digitalWrite(relay_pin, LOW);
          enabled = false;
          shutdown_started = false;
          led_blink.stop();
          digitalWrite(led_pin, HIGH);
          //Serial.println("Shutdown finished");
          //digitalWrite(13, LOW);
          //delay(2);
        }
        return;
      }
      if (automatic) {
        if (!enabled && capacity >= low_margin + 5) {
          digitalWrite(relay_pin, HIGH);
          digitalWrite(led_pin, LOW);
          enabled = true;
        }
        if (!enabled && capacity < low_margin) {
          digitalWrite(led_pin, HIGH);
        }
        if (enabled && capacity < low_margin) {
          start_shutdown();
        }
      }
    }    

};

const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
const uint8_t rpi_callbcak_pin = 7;
Raspberry raspberrypi = Raspberry(5, 6, 7, 8);
RouterRelay router_relay = RouterRelay(2);
LedLightDuration yellow_led = LedLightDuration(3);
LedLightDuration red_led = LedLightDuration(8);

int get_capacity() {
  int x = analogRead(analogInPin);
  //Serial.print(x);
  //Serial.print('\t');
  int val = map(x, 660, 818, 0, 100);
  if (val < 0) val = 0;
  if (val > 100) val = 100;
  //Serial.println(val);
  return val;  
}

void setup() {
  //Serial.begin(9600);
  yellow_led.enable(150);
  red_led.enable(150);
}

void loop() {
  if (digitalRead(rpi_callbcak_pin) == HIGH) {
    yellow_led.enable(1000);
  }

  int capacity = get_capacity();

  yellow_led.update();
  red_led.update();
  router_relay.update(capacity);
  raspberrypi.update(capacity);

  // wait 2 milliseconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:
  delay(2);
}
