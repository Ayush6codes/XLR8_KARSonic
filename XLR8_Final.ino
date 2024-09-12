#include <WiFi.h>
#include <typeinfo>
#include <string.h>
#include <stdio.h>

// SSID and password for the access point
const char* ssid = "KARSonic"; // Team name as the SSID 
const char* password = "Bando@69"; // Password

// Define a structure to hold IMU (Inertial Measurement Unit) data
typedef struct {
  float gx, gy, gz;
} IMUData;

IMUData myMessage; // Create a variable to store received IMU data
int cmd = 0;       // Initialize motor control command variable
int spd = 0;       // Initialize motor speed variable

// Function to update motor control based on received IMU data
void updateMotorControl() {
  float gx = myMessage.gx;
  float gy = myMessage.gy;
  float gz = myMessage.gz;

  // Motor control logic based on IMU data
  if ((gz != 0) && (gx != 0) && (abs(gy) < 2)) {
    spd = constrain(abs(map((atan2(gx, gz) * 180 / PI), 0, 90, 0, 255)), 0, 255);
    cmd = (gx > 0) ? 1 : 2; // Forward or backward
  } else if (((gz != 0) && (gy != 0) && (abs(gx) < 2)) && (atan2(gy, gz) * 180 / PI < 70)) {
    spd = constrain(abs(map((atan2(gy, gz) * 180 / PI), 0, 70, 200, 0)), 0, 200); //For small angle turns
    cmd = (gy > 0) ? 3 : 4; // For small angle Right or left
  } else if (((gz != 0) && (gy != 0) && (abs(gx) < 2)) && (atan2(gy, gz) * 180 / PI > 70)) {
    spd = constrain(abs(map((atan2(gy, gz) * 180 / PI), 70, 90, 0, 255)), 0, 255); //For large angle turns
    cmd = (gy > 0) ? 5 : 6; // For large angle Right or left
  } else {
    cmd = 0; // Stop
    spd = 0;
  }

  // Adjust motor speed thresholds
  if (((gz != 0) && (gx != 0) && (abs(gy) < 2)) && (spd > 60 && spd < 150) && (((gz != 0) && (gy != 0) && (abs(gx) < 2)) && (atan2(gy, gz) * 180 / PI > 70)))  {
    spd = 150;
  }
  if (((gz != 0) && (gx != 0) && (abs(gy) < 2)) && (spd > 150 && spd < 255) && (((gz != 0) && (gy != 0) && (abs(gx) < 2)) && (atan2(gy, gz) * 180 / PI > 70))) {
    spd = 255;
  }

  // Display motor control information
  Serial.print("cmd: ");
  Serial.print(cmd);   // Display motor command
  Serial.print(", speed: ");
  Serial.println(spd); // Display motor speed
}

// Pin assignments for motor control
const int ENA = 8;
const int ENB = 9;

const int IN1 = 10;
const int IN2 = 11;
const int IN3 = 12;
const int IN4 = 13;

// Create a WiFiServer object for the TCP server
WiFiServer server(80);

void setup() {
  // Start Serial for debugging
  Serial.begin(115200);
    // Configure motor control pins as outputs
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Set up the access point
  Serial.println("Setting up WiFi AP...");
  WiFi.softAP(ssid, password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  // Start the server
  server.begin();

  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client is connected");
  }

}

void applyMotorControl() {
  float gx = myMessage.gx;
  float gy = myMessage.gy;
  float gz = myMessage.gz;
  switch (cmd) {
    case 1:  // Forward
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      analogWrite(ENA, spd);
      analogWrite(ENB, spd);
      break;
    case 2:  // Backward
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      analogWrite(ENA, spd);
      analogWrite(ENB, spd);
      break;
    case 3:  // Small Angle Right
      if (((gz != 0) && (gy != 0) && (abs(gx) < 2)) && (atan2(gy, gz) * 180 / PI > 30)) {
      digitalWrite(IN1, HIGH); 
      digitalWrite(IN2, LOW); 
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW); 
      analogWrite(ENA, 255);
      analogWrite(ENB, 30);
      }
      break;
    case 4:  // Small Angle Left
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      analogWrite(ENA, 30);
      analogWrite(ENB, 255);
      break;
    case 5:  // Large Angle Right
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      analogWrite(ENA, 230);
      analogWrite(ENB, 230);
      break;
    case 6:  // Large Angle Left
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      analogWrite(ENA, 230);
      analogWrite(ENB, 230);
      break;
    default:  // Stop
      digitalWrite(IN1, LOW); 
      digitalWrite(IN2, LOW); 
      digitalWrite(IN3, LOW); 
      digitalWrite(IN4, LOW); 
      spd = 0;
      analogWrite(ENA, spd);
      analogWrite(ENB, spd);
      break;
  }
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  //if (client) {
    //while (client.connected()) {
      if (client.available()) {
        digitalWrite(LED_BUILTIN, HIGH);
        String request = client.readStringUntil('\r');
        Serial.print("Received data: ");
        //Serial.println(request);
        char chararr[50];
        request.toCharArray(chararr, sizeof(chararr));

        Serial.println(chararr);
        char *res;
        float arr[3];
        int i = 0;

        res = strtok(chararr , " ");
        while(res != NULL){
          Serial.println(res);
          String strr = String(res);
          float val = strr.toFloat();
          Serial.println(val);
          res = strtok(NULL, " ");
          arr[i]= val;
          i++;
        }
        myMessage.gx = arr[0];
        myMessage.gy = arr[1];
        myMessage.gz = arr[2];
        updateMotorControl();
        applyMotorControl();
        digitalWrite(LED_BUILTIN, LOW);}
      digitalWrite(LED_BUILTIN, LOW);
   // Continuously update and apply motor control
  delay(100); // Delay to control loop speed
}
