// Project name: AquaGuard

#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
String device_name = "WaterAlarm";

// UART2 pins for communication with STM32
#define RXD2 16
#define TXD2 17

#define LED_PIN 2 // LED

// PASS
const String PASSWORD = "11111";

// Flag PASS
bool clientAuthorized = false;

void setup() 
{
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);      

  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);  
  while (Serial2.available()) Serial2.read(); // Flush any garbage data from STM32

  SerialBT.begin(device_name);   
  Serial.println("BT started, name: " + device_name);
  Serial.println("Waiting for client and password...");
}

void loop()
{
  // If there is a connected client that is not yet authorized
  if (SerialBT.hasClient() && !clientAuthorized) {
    // Pass
    SerialBT.println("Enter password:");
    Serial.println("Client connected, waiting for password...");
    
    // Waiting for a response
    unsigned long start = millis();
    String response = "";
    while (millis() - start < 100000) {
      if (SerialBT.available()) {
        char c = SerialBT.read();
        if (c == '\n' || c == '\r') continue; 
        response += c;

        if (response.length() >= PASSWORD.length()) {
          break;
        }
      }
      delay(10);
    }
    
    // PASS (TRUE/FALSE)
    response.trim();
    if (response == PASSWORD) {
      clientAuthorized = true;
      SerialBT.println("OK - Access granted");
      Serial.println("Client authorized");
    } else {
      SerialBT.println("Wrong password! Disconnecting...");
      Serial.println("Wrong password, disconnecting client");
      delay(100);
      SerialBT.disconnect();
      clientAuthorized = false;
    }
  }

  // If the client is authorized, normal operation
  if (clientAuthorized && SerialBT.hasClient()) {
    // Receive data from STM32
    if (Serial2.available()) {
      String message = Serial2.readStringUntil('\n');  
      message.trim();

      if (message.length() > 0) {
        // Accept only known messages, ignore noise
        if (message.startsWith("SYSTEM ON")  || message.startsWith("SYSTEM OFF") ||
            message.startsWith("WARNING")    || message.startsWith("GOOD")) {
          // Print to debug console and send via Bluetooth
          Serial.println("STM32 -> BT: " + message);
          SerialBT.println(message);
        }
      }
    }

    // Receive commands from phone via Bluetooth
    if (SerialBT.available()) {
      byte cmd = SerialBT.read();
      Serial.write(cmd);             
      if (cmd == '1') {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("LED ON");
      } else if (cmd == '0') {
        digitalWrite(LED_PIN, LOW);
        Serial.println("LED OFF");
      }
    }
  } else {
    // If the client disconnects, we reset the authorization
    clientAuthorized = false;
  }

  delay(20);
}