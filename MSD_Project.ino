#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

const char* ssid = "409";
const char* password = "409409409";
unsigned long alarmStart = 0;
bool alarmActive = false;

unsigned long lastPrint = 0;

#define PCA_SCL D1
#define PCA_SDA D2

// Front ultrasonic sensor
#define FRONT_TRIG D5
#define FRONT_ECHO D6

// Rear ultrasonic sensor
#define REAR_TRIG D7
#define REAR_ECHO D0


// ================= PCA9685 CHANNELS =================

#define inB1 0
#define inB2 1
#define enB 2

#define inA1 3
#define inA2 4
#define enA 5
  
#define FRONT_LEDS 6

#define REAR_LEDS 8

#define buzzerPin 9

int speed = 4095;


// ================= ADDED - SENSOR VARIABLES =================

// Stores the last gesture received from Python
String currentGesture = "None";

// Sensors initially considered blocked until checked
bool frontBlocked = false;
bool rearBlocked = false;

// Used to check sensors every 100 ms
unsigned long lastSensorCheck = 0;


ESP8266WebServer server(80);

Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver(0x40);


// ================= PCA FUNCTIONS =================

void pcaHIGH(int channel)
{
  pca.setPWM(channel, 4096, 0);
}

void pcaLOW(int channel)
{
  pca.setPWM(channel, 0, 4096);
}


// ================= SETUP =================

void setup()
{
  Serial.begin(115200);

  


  // Ultrasonic sensors
  pinMode(FRONT_TRIG, OUTPUT);
  pinMode(FRONT_ECHO, INPUT);

  pinMode(REAR_TRIG, OUTPUT);
  pinMode(REAR_ECHO, INPUT);


  // PCA9685 initialization
  Wire.begin(PCA_SDA, PCA_SCL);

  pca.begin();

  pca.setPWMFreq(1000);

  delay(10);


  // Wi-Fi connection
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected!");

  Serial.print("ESP IP address: ");
  Serial.println(WiFi.localIP());


  // Initialize PCA outputs
  pcaLOW(inA1);
  pcaLOW(inA2);
  pcaLOW(inB1);
  pcaLOW(inB2);

  pcaLOW(enA);
  pcaLOW(enB);

  pcaLOW(FRONT_LEDS);
  pcaLOW(REAR_LEDS);
  pcaLOW(buzzerPin);


  // ================= TEST =================

  server.on("/test", []()
  {
    Serial.println("TEST SIGNAL RECEIVED");

    server.send(200, "text/plain", "ESP8266 received the test signal");
  });


  // ================= FRONT LIGHTS =================

  server.on("/lights/on", []()
  {
    Serial.println("LIGHT_ON received");

    pcaHIGH(FRONT_LEDS);

    server.send(200, "text/plain", "LIGHTS_ON");
  });


  server.on("/lights/off", []()
  {
    Serial.println("LIGHT_OFF received");

    pcaLOW(FRONT_LEDS);

    server.send(200, "text/plain", "LIGHTS_OFF");
  });


  // ================= SPEED CONTROL =================

  server.on("/low/speed", []()
  {
    Serial.println("low speed");

    speed = 1400;

    server.send(200, "text/plain", "LOW");
  });


  server.on("/high/speed", []()
  {
    Serial.println("high speed");

    speed = 4095;

    server.send(200, "text/plain", "HIGH");
  });


  server.on("/medium/speed", []()
  {
    Serial.println("medium speed");

    speed = 2600;

    server.send(200, "text/plain", "MEDIUM");
  });


  // ================= VOICE STOP =================

  server.on("/stop", []()
  {
    Serial.println("stop");

    // ADDED - clear previous gesture
    currentGesture = "None";

    brake();

    pcaHIGH(REAR_LEDS);

    server.send(200, "text/plain", "STOP");
  });

  server.on("/alarm", []()
  {
    Serial.println("alarm");

    // ADDED - clear previous gestur

    pcaHIGH(buzzerPin);
    alarmStart = millis();
    alarmActive = true;

    server.send(200, "text/plain", "ALARM");
  });


  // ================= GESTURE CONTROL =================

  server.on("/gesture", HTTP_GET, []()
  {
    if (server.hasArg("value"))
    {
      String gesture = server.arg("value");

      // ADDED - remember the last received gesture
      currentGesture = gesture;


      if (gesture == "Open_Palm")
      {
        // CHANGED - use continuously updated sensor state
        if (!frontBlocked)
        {
          go_forward();
        }
        else
        {
          turnOff();
        }
      }


      else if (gesture == "Pointing_Up")
      {
        // CHANGED - use continuously updated sensor state
        if (!rearBlocked)
        {
          go_backwards();
        }
        else
        {
          turnOff();
        }
      }


      else if (gesture == "Thumb_Up")
      {
        go_right();
      }


      else if (gesture == "Thumb_Down")
      {
        go_left();
      }


      else if (gesture == "Closed_Fist")
      {
        brake();
      }


      else if (gesture == "None")
      {
        turnOff();
      }


      Serial.println("Received: " + gesture);
    }

    server.send(200, "text/plain", "Received");
  });


  server.begin();

  Serial.println("Server started");
}


// ================= LOOP =================

void loop()
{
  server.handleClient();


  // ADDED - check ultrasonic sensors every 100 ms
  if (millis() - lastSensorCheck >= 100)
  {
    lastSensorCheck = millis();

    long distance1 = getValue1();
    long distance2 = getValue2();


    // ADDED - save previous sensor states
    bool previousFrontBlocked = frontBlocked;
    bool previousRearBlocked = rearBlocked;


    // ADDED - determine whether an obstacle is detected
    // 9999 means no valid echo was received
    frontBlocked = (distance1 <= 10);
    rearBlocked = (distance2 <= 10);


    // ================= FRONT SENSOR =================

    // ADDED - automatic obstacle stopping while moving forward
    if (currentGesture == "Open_Palm")
    {
      if (frontBlocked && !previousFrontBlocked)
      {
        turnOff();

        Serial.println("FRONT OBSTACLE - STOP");
      }

      else if (!frontBlocked && previousFrontBlocked)
      {
        go_forward();

        Serial.println("FRONT CLEAR - FORWARD");
      }
    }


    // ================= REAR SENSOR =================

    // ADDED - automatic obstacle stopping while reversing
    if (currentGesture == "Pointing_Up")
    {
      if (rearBlocked && !previousRearBlocked)
      {
        turnOff();

        Serial.println("REAR OBSTACLE - STOP");
      }

      else if (!rearBlocked && previousRearBlocked)
      {
        go_backwards();

        Serial.println("REAR CLEAR - BACKWARD");
      }
    }


    // ADDED - print sensor distances once every second
    // without delaying the rest of the program
    if (millis() - lastPrint >= 1000)
    {
      lastPrint = millis();

      Serial.print("Front distance: ");
      Serial.print(distance1);
      Serial.print(" cm | Rear distance: ");
      Serial.print(distance2);
      Serial.println(" cm");
    }
  }
  if (alarmActive && millis() - alarmStart >= 2000)
  {
    pcaLOW(buzzerPin);
    alarmActive = false;
  }
}


// ================= MOTOR FUNCTIONS =================

void go_forward()
{
  pca.setPWM(enA, 0, speed);
  pca.setPWM(enB, 0, speed);

  pcaHIGH(inA1);
  pcaLOW(inA2);

  pcaHIGH(inB1);
  pcaLOW(inB2);

  pcaLOW(REAR_LEDS);
}


void go_backwards()
{
  pca.setPWM(enA, 0, speed);
  pca.setPWM(enB, 0, speed);

  pcaLOW(inA1);
  pcaHIGH(inA2);

  pcaLOW(inB1);
  pcaHIGH(inB2);

  pcaLOW(REAR_LEDS);
}


void go_right()
{
  pca.setPWM(enA, 0, speed);
  pca.setPWM(enB, 0, speed);

  pcaLOW(inB1);
  pcaLOW(inB2);

  pcaHIGH(inA1);
  pcaLOW(inA2);

  pcaLOW(REAR_LEDS);
}


void go_left()
{
  pca.setPWM(enA, 0, speed);
  pca.setPWM(enB, 0, speed);

  pcaLOW(inA1);
  pcaLOW(inA2);

  pcaHIGH(inB1);
  pcaLOW(inB2);

  pcaLOW(REAR_LEDS);
}


void turnOff()
{
  pcaLOW(inA1);
  pcaLOW(inA2);

  pcaLOW(inB1);
  pcaLOW(inB2);

  pcaLOW(enA);
  pcaLOW(enB);
}


void brake()
{
  pcaHIGH(inA1);
  pcaHIGH(inA2);

  pcaHIGH(inB1);
  pcaHIGH(inB2);

  pcaHIGH(REAR_LEDS);
}


// ================= FRONT ULTRASONIC =================

long getValue1()
{
  digitalWrite(FRONT_TRIG, LOW);
  delayMicroseconds(10);

  digitalWrite(FRONT_TRIG, HIGH);
  delayMicroseconds(10);

  digitalWrite(FRONT_TRIG, LOW);

  long value = pulseIn(FRONT_ECHO, HIGH, 30000);

  if (value == 0)
    return 9999;
  else
    return (value / 2) / 29.1;
}


// ================= REAR ULTRASONIC =================

long getValue2()
{
  digitalWrite(REAR_TRIG, LOW);
  delayMicroseconds(10);

  digitalWrite(REAR_TRIG, HIGH);
  delayMicroseconds(10);

  digitalWrite(REAR_TRIG, LOW);

  long value = pulseIn(REAR_ECHO, HIGH, 30000);

  if (value == 0)
    return 9999;
  else
    return (value / 2) / 29.1;
}
