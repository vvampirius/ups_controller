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

class Raspberry {
  private:
    uint8_t relay_pin;
    int shutdown_pin;
    int callbcak_pin;
    bool enabled = false;
    bool automatic = true;
    bool shutdown_started = false;
    unsigned long shutdown_started_at;
  
  public:
    int low_margin = 20;

    Raspberry(uint8_t _relay_pin, uint8_t _shutdown_pin, uint8_t _callback_pin) {
      relay_pin = _relay_pin;
      shutdown_pin = _shutdown_pin;
      callbcak_pin = _callback_pin;
      pinMode(_relay_pin, OUTPUT);
      pinMode(_shutdown_pin, OUTPUT);
      pinMode(_callback_pin, INPUT);
    }

    void start_shutdown() {
      shutdown_started = true;
      shutdown_started_at = millis();
      digitalWrite(shutdown_pin, HIGH);
      //Serial.println("Shutdown started");
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(shutdown_pin, LOW);
    }

    void update(int capacity) {
      if (enabled && shutdown_started) {
        if (get_time_delta(shutdown_started_at, millis()) > 120000 || digitalRead(callbcak_pin) == HIGH) {
          digitalWrite(relay_pin, LOW);
          enabled = false;
          shutdown_started = false;
          //Serial.println("Shutdown finished");
          digitalWrite(13, LOW);
          delay(2);
        }
        return;
      }
      if (automatic) {
        if (!enabled && capacity >= low_margin) {
          digitalWrite(relay_pin, HIGH);
          enabled = true;
        }
        if (enabled && capacity < low_margin) {
          start_shutdown();
        }
      }
    }    

};


// These constants won't change. They're used to give names to the pins used:
const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
const int analogOutPin = 13;  // Analog output pin that the LED is attached to

int outputValue = 0;  // value output to the PWM (analog out)

Raspberry raspberrypi = Raspberry(2, 4, 7);

int get_capacity() {
  int val = map(analogRead(analogInPin), 660, 818, 0, 100);
  if (val < 0) val = 0;
  if (val > 100) val = 100;
  return val;  
}

void setup() {
  // initialize serial communications at 9600 bps:
  //Serial.begin(9600);
  pinMode(analogOutPin, OUTPUT);
  digitalWrite(analogOutPin, LOW);
  pinMode(3, OUTPUT);
}

void loop() {
  int capacity = get_capacity();

  if (capacity > 15) digitalWrite(3, HIGH);
  if (capacity < 10) digitalWrite(3, LOW);

  // change the analog out value:
  //analogWrite(analogOutPin, outputValue);

  //Serial.println(capacity);

  raspberrypi.update(capacity);

  // wait 2 milliseconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:
  delay(1000);
}
