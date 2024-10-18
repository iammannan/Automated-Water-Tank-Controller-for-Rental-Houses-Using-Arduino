#include <LiquidCrystal_I2C.h>
//16*2 LCD I2C Display Header file  

#define I2C_ADDR    0x27
#define LCD_COLUMNS 20
#define LCD_LINES   4

// Pin Definitions
const int motorRelayPin = 13;    // Motor relay output
const int valve1Pin = 8;         // Valve 1 (solenoid for Tank 1)
const int valve2Pin = 9;         // Valve 2 (solenoid for Tank 2)
const int valve3Pin = 10;        // Valve 3 (solenoid for Tank 3)

const int switch1Pin = 5;        // Switch 1 (User 1 for Tank 1)
const int switch2Pin = 6;        // Switch 2 (User 2 for Tank 2)
const int switch3Pin = 7;        // Switch 3 (User 3 for Tank 3)

const int floatSwitch1Pin = 2;   // Float switch 1 (Tank 1 full)
const int floatSwitch2Pin = 3;   // Float switch 2 (Tank 2 full)
const int floatSwitch3Pin = 4;   // Float switch 3 (Tank 3 full)

// Queue to track order of requests (0 means no request)
int requestQueue[25] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // Stores the order of requests (1 for User 1, 2 for User 2, 3 for User 3)
int currentRequestIndex = 0;      // Tracks the current request being processed
int currentTank = 0;              // Currently running task

LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);


void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  Serial.println("System initialized");

  // Initialize pins
  pinMode(motorRelayPin, OUTPUT);
  pinMode(valve1Pin, OUTPUT);
  pinMode(valve2Pin, OUTPUT);
  pinMode(valve3Pin, OUTPUT);
  
  pinMode(switch1Pin, INPUT_PULLUP);
  pinMode(switch2Pin, INPUT_PULLUP);
  pinMode(switch3Pin, INPUT_PULLUP);
  
  pinMode(floatSwitch1Pin, INPUT_PULLUP);
  pinMode(floatSwitch2Pin, INPUT_PULLUP);
  pinMode(floatSwitch3Pin, INPUT_PULLUP);
  
  // Ensure all outputs are OFF initially
  digitalWrite(motorRelayPin, LOW);
  digitalWrite(valve1Pin, LOW);
  digitalWrite(valve2Pin, LOW);
  digitalWrite(valve3Pin, LOW);

  Serial.println("Motor and valves initialized to OFF state");

  // Initialize the lcd
  lcd.init(); 
  // Turn on the Backlight
  lcd.backlight(); 


}

void loop() {
  // Read switch states (active low)
  bool switch1Pressed = !digitalRead(switch1Pin);
  bool switch2Pressed = !digitalRead(switch2Pin);
  bool switch3Pressed = !digitalRead(switch3Pin);
  
  // Read float switch states (active low)
  bool floatSwitch1Active = !digitalRead(floatSwitch1Pin); // Tank 1 full
  bool floatSwitch2Active = !digitalRead(floatSwitch2Pin); // Tank 2 full
  bool floatSwitch3Active = !digitalRead(floatSwitch3Pin); // Tank 3 full

  // Enqueue requests based on switch presses
  if (switch1Pressed) {
    enqueueRequest(1);
    Serial.println("User 1 requested to fill Tank 1");
  }
  if (switch2Pressed) {
    enqueueRequest(2);
    Serial.println("User 2 requested to fill Tank 2");
  }
  if (switch3Pressed) {
    enqueueRequest(3);
    Serial.println("User 3 requested to fill Tank 3");
  }

  // Process the request queue
  if (currentRequestIndex < 25 && requestQueue[currentRequestIndex] != 0) {
    currentTank = requestQueue[currentRequestIndex];
    
    if (currentTank == 1 && !floatSwitch1Active) {
      startFilling(1);  // Fill Tank 1
      Serial.println("Filling Tank 1...");
    }
    if (currentTank == 1 && floatSwitch1Active) {
        completeRequest(1);  // Stop filling when Tank 1 is full
        Serial.println("Tank 1 is full, stopping...");
    }
    if (currentTank == 2 && !floatSwitch2Active) {
      startFilling(2);  // Fill Tank 2
      Serial.println("Filling Tank 2...");
    }
    if (currentTank == 2 && floatSwitch2Active) {
        completeRequest(2);  // Stop filling when Tank 2 is full
        Serial.println("Tank 2 is full, stopping...");
    }
    if (currentTank == 3 && !floatSwitch3Active) {
      startFilling(3);  // Fill Tank 3
      Serial.println("Filling Tank 3...");
    }
    if (currentTank == 3 && floatSwitch3Active) {
        completeRequest(3);  // Stop filling when Tank 3 is full
        Serial.println("Tank 3 is full, stopping...");
    }
  }
  
  showTextUsingLCD();
  
  delay(100);
}

// Function to add a request to the queue
void enqueueRequest(int user) {
  for (int i = 0; i < 25; i++) {
    if (requestQueue[i] == 0) {
      requestQueue[i] = user;  // Add the user request to the next available slot
      Serial.print("Request for Tank ");
      Serial.print(user);
      Serial.println(" added to queue");
      break;
    }
  }
}

// Function to start filling a specific tank
void startFilling(int tank) {
  digitalWrite(motorRelayPin, HIGH); // Turn on motor
  Serial.println("Motor turned ON");

  if (tank == 1) {
    digitalWrite(valve1Pin, HIGH);   // Open valve 1
    digitalWrite(valve2Pin, LOW);    // Ensure other valves are closed
    digitalWrite(valve3Pin, LOW);
    Serial.println("Valve 1 opened, others closed");
  } else if (tank == 2) {
    digitalWrite(valve2Pin, HIGH);   // Open valve 2
    digitalWrite(valve1Pin, LOW);    // Ensure other valves are closed
    digitalWrite(valve3Pin, LOW);
    Serial.println("Valve 2 opened, others closed");
  } else if (tank == 3) {
    digitalWrite(valve3Pin, HIGH);   // Open valve 3
    digitalWrite(valve1Pin, LOW);    // Ensure other valves are closed
    digitalWrite(valve2Pin, LOW);
    Serial.println("Valve 3 opened, others closed");
  }
}

// Function to stop filling a specific tank and dequeue the request
void completeRequest(int tank) {
  // Close the corresponding valve
  if (tank == 1) digitalWrite(valve1Pin, LOW);
  if (tank == 2) digitalWrite(valve2Pin, LOW);
  if (tank == 3) digitalWrite(valve3Pin, LOW);

  Serial.print("Valve ");
  Serial.print(tank);
  Serial.println(" closed");

  // Move to the next request in the queue
  currentRequestIndex++;
  
  // If there are no more requests, turn off the motor
  if (currentRequestIndex >= 25 || requestQueue[currentRequestIndex] == 0) {
    digitalWrite(motorRelayPin, LOW);  // Turn off motor
    Serial.println("No more requests, motor turned OFF");
    currentRequestIndex = 0;           // Reset to start of queue
    resetQueue();                      // Clear the queue
  }
}

// Function to reset the request queue
void resetQueue() {
  for (int i = 0; i < 25; i++) {
    requestQueue[i] = 0;  // Clear all requests
  }
  Serial.println("Request queue reset");
}

void showTextUsingLCD() {
  // Clear the display buffer
    lcd.clear(); 
    if(currentRequestIndex == 0 && requestQueue[0] == 0){
      lcd.setCursor(2, 0);
      lcd.print("Press SWITCH");
      lcd.setCursor(4,1);
      lcd.print("to START");
    }else{
      lcd.setCursor(0, 0);
      lcd.print("Filling House ");
      lcd.setCursor(14, 0);
      lcd.print(currentTank);
      lcd.setCursor(0,1);
      lcd.print("Waiting :");
      lcd.setCursor(9,1);
      lcd.print(requestQueue[currentRequestIndex+1]);
      lcd.setCursor(10,1);
      lcd.print(",");
      lcd.setCursor(11,1);
      lcd.print(requestQueue[currentRequestIndex+2]);
      lcd.setCursor(12,1);
      lcd.print(",");
      lcd.setCursor(13,1);
      lcd.print(requestQueue[currentRequestIndex+3]);
      lcd.setCursor(14,1);
      lcd.print(",");
      lcd.setCursor(15,1);
      lcd.print(requestQueue[currentRequestIndex+4]);
    }
}
