#include "relay.h"
#include "raspberry.h"
#include "getTimeDelta.h"
#include "EEPROM.h"

Battery battery = Battery(A0);
Relay routerRelay = Relay(2, 5, 1000, 10000, 0);
Raspberry raspberryRelay = Raspberry(6, 7, 3, 4, 10, 11, 1050, 60000, 10000, 1);
Relay thirdRelay = Relay(8, 9, 1050, 10000, 2);
long lastPrintAt = 0;

void demo() {
  routerRelay.on();
  delay(250);
  raspberryRelay.on();
  delay(250);
  thirdRelay.on();
  delay(250);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(2000);
  routerRelay.off();
  delay(250);
  raspberryRelay.off();
  delay(250);
  thirdRelay.off();
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("System initialized");
  delay(100);
  //demo();
  if (!routerRelay.isOffDueToVoltage) routerRelay.on();
  if (raspberryRelay.powerMode == 0) raspberryRelay.on();
  if (!thirdRelay.isOffDueToVoltage) thirdRelay.on();
}

void loop() {
  battery.read();
  if (getTimeDelta(lastPrintAt) >= 1000 || lastPrintAt == 0) {
    Serial.print("raw: ");
    Serial.print(battery.rawValue);
    Serial.print(", voltage: ");
    Serial.print(battery.voltage);
    if (battery.isOnline) {
      Serial.print(", ONLINE duration: ");
      Serial.print(getTimeDelta(battery.onlineSince));
    }
    Serial.println();
    lastPrintAt = millis();
  }
  routerRelay.checkBattery(&battery);
  raspberryRelay.checkBattery(&battery);
  raspberryRelay.checkShutdown();
  raspberryRelay.checkPingAge();
  raspberryRelay.updateLed();
  raspberryRelay.checkButton();
  thirdRelay.checkBattery(&battery);
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    if (command.startsWith("SHUTDOWN")) {
      if (raspberryRelay.powerMode == 0) {
        Serial.println("got shutdown signal");
        raspberryRelay.setPowerMode(1);
        raspberryRelay.shutdownStartedAt = millis();
      }
    } else if (command.startsWith("REBOOT")) {
      Serial.println("got reboot signal");
      raspberryRelay.powerMode = 2;
      raspberryRelay.shutdownStartedAt = millis();
    } else if (command.startsWith("PING")) {
      Serial.println("PONG");
      raspberryRelay.pingCheckpointAt = millis();
      raspberryRelay.raspberryLed->light(500);
    }
  }
  delay(10);
}
