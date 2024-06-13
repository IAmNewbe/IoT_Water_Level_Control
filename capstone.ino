
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <Wire.h>

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPL6t2Mbr3M4"
#define BLYNK_TEMPLATE_NAME         "Water Level"
#define BLYNK_AUTH_TOKEN            "AYpRfdmrBYz1p1rBuilMdZwWhpMX78Td"

// #include <SPI.h>
// #include <Ethernet.h>
// #include <BlynkSimpleEthernet.h>
#include <BlynkSimpleEsp32.h>

LiquidCrystal_I2C lcd(0x27,16,2);

byte customChar[] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

// START DEFINE GPIO
int pump1 = 5;
int pump2 = 17;
int pump3 = 16;
int pump4 = 4;

int pot_1 = 32;
int pot_2 = 33;

const int trig1 = 18;
const int echo1 = 19;

const int trig2 = 23;
const int echo2 = 25;

const int slideButton = 34;

// END DEFINE GPIO

// START MQTT PREQUISITE
unsigned long lastSend;
unsigned long lastOn;

const char* ssid = "crustea";
const char* password = "crustea1234";
const char* mqtt_server = "18.140.254.213";
const int mqtt_port = 1883;
const char* topic = "v1/devices/me/telemetry";

#define TOKEN "TEST"

WiFiClient wifiClient;
PubSubClient client(wifiClient);
// END MQTT PREQUISITE


// START HCSR PREQUISITE
//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;
// END HCSR PREQUISITE

// START VARIABLE 
  float Main_Level, Reservoir_Level;
  int PotValue_1, PotValue_2;
  int Adjusted_Main_Level, Adjusted_Reservoir_Level;
  int dump, water;
  int Threshold;
  bool state;
  float Soil;
  char status_koneksi[8] = "Offline";
// END  VARIABLE

int Water_Level(int trigPin, int echoPin) {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  
  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;
  
  // Prints the distance in the Serial Monitor
  // Serial.print("Distance (cm): ");
  // Serial.println(distanceCm);
  lcd.setCursor(0,0);
  lcd.print("Distance : ");
  lcd.setCursor(0,1);
  lcd.print(distanceCm);
  delay(1000);
  lcd.clear();

  return distanceCm;
}

void Read_Inputs(){

  Main_Level = Water_Level(trig1, echo2);
  Main_Level = 17 - Main_Level;
  Reservoir_Level = Water_Level(trig2, echo2);
  Reservoir_Level = 17 -  Reservoir_Level;

  PotValue_1 = analogRead(pot_1);
  PotValue_2 = analogRead(pot_2);

  state = analogRead(slideButton);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    strcpy(status_koneksi, "Offline");
    Production();
    // Serial.println("offline mode");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  strcpy(status_koneksi, "Online");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(1000);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266 Device", TOKEN, NULL)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void Test_Pump(){
  if (Adjusted_Main_Level >= 7) {
    pinMode(pump1, HIGH);
    pinMode(pump3, HIGH);
    pinMode(pump2, LOW);
  } else if (Adjusted_Main_Level <= 7) {
    pinMode(pump2, HIGH);
    pinMode(pump1, LOW);
    pinMode(pump3, LOW);
  }
}

void Adjust_Pump() {
  if (water == 1) {
    pinMode(pump2, HIGH);
  } else {
    pinMode(pump2, LOW);
  }
  if (Main_Level <= Adjusted_Reservoir_Level) {
    if (Reservoir_Level >= 3) {
      pinMode(pump1, HIGH);
    } else {
      pinMode(pump1, LOW);
    }
  }else if (Main_Level >= Adjusted_Main_Level) {
    if (Soil >= Threshold || Reservoir_Level <= 14) {
      pinMode(pump4, HIGH);
      pinMode(pump1, LOW);
      pinMode(pump2, LOW);
      // pinMode(pump3, LOW);
    }
    
  }
}

void Production() {
  Read_Inputs();
 
  Adjusted_Main_Level = map(PotValue_1, 0, 4095, 0, 15);
  Adjusted_Reservoir_Level = map(PotValue_2, 0, 4095, 0, 15);
  Serial.println("Offline Mode");
}

void Display_Level() {
  if (state == true || WiFi.status() != WL_CONNECTED) {
    for (int i = 0; i <= Adjusted_Main_Level; i++){
    lcd.setCursor(0, i);
    lcd.write(byte(0));
      if (i > Adjusted_Main_Level) {
        lcd.clear();
      }
    }

    for (int i = 0; i <= Adjusted_Reservoir_Level; i++){
      lcd.setCursor(1, i);
      lcd.write(byte(0));
        if (i > Adjusted_Reservoir_Level) {
          lcd.clear();
        }
    }

  } else {
    int Int_Main_Level = round(Main_Level);
    int Int_Reservoir_Level = round(Reservoir_Level); 
    for (int i = 0; i <= Int_Main_Level; i++){
    lcd.setCursor(0, i);
    lcd.write(byte(0));
      if (i > Int_Main_Level) {
        lcd.clear();
      }
    }

    for (int i = 0; i <= Int_Reservoir_Level; i++){
      lcd.setCursor(1, i);
      lcd.write(byte(0));
        if (i > Int_Reservoir_Level) {
          lcd.clear();
        }
    }
  }
  
}

void sendData(){
  Blynk.virtualWrite(V3, Reservoir_Level);
}

BLYNK_WRITE(V4)
{
  Adjusted_Main_Level = param.asInt(); // assigning incoming value from pin V1 to a variable
  // You can also use:
  // String i = param.asStr();
  // double d = param.asDouble();
  Serial.print("Adjusted_Main_Level Slider value is: ");
  Serial.println(Adjusted_Main_Level);
}

BLYNK_WRITE(V0)
{
  water = param.asInt(); // assigning incoming value from pin V1 to a variable
  // You can also use:
  // String i = param.asStr();
  // double d = param.asDouble();
  if (water == 1) {
    pinMode(pump2, HIGH);
  } else {
    pinMode(pump2, LOW);
  }
  Serial.print("water button value is: ");
  Serial.println(water);
}

BLYNK_WRITE(V1)
{
  dump = param.asInt(); // assigning incoming value from pin V1 to a variable
  // You can also use:
  // String i = param.asStr();
  // double d = param.asDouble();
  if (dump == 1) {
    pinMode(pump3, HIGH);
  } else {
    pinMode(pump3, LOW);
  }
  Serial.print("dump button value is: ");
  Serial.println(dump);
}

BLYNK_WRITE(V2)
{
  Soil = param.asInt(); // assigning incoming value from pin V1 to a variable
  // You can also use:
  // String i = param.asStr();
  // double d = param.asDouble();
  Serial.print("soil value is: ");
  Serial.println(Soil);
}

void setup() {
  Serial.begin(115200); // Starts the serial communication

  pinMode(trig1, OUTPUT); // Sets the trigPin as an Output
  pinMode(echo1, INPUT); // Sets the echoPin as an Input
  pinMode(trig2, OUTPUT); // Sets the trigPin as an Output
  pinMode(echo2, INPUT); // Sets the echoPin as an Input

  pinMode(slideButton, INPUT);
  pinMode(pot_1, INPUT);
  pinMode(pot_2, INPUT);

  Serial.begin(115200);
  lcd.init();                      // initialize the lcd
  lcd.createChar(0, customChar); 
  lcd.clear();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(2,0);
  lcd.print("CAPSTONE");
  lcd.setCursor(1,1);
  lcd.print("WATER LEVEL");
  delay(3500);
  setup_wifi();
  
  // client.setServer(mqtt_server, 1883);
  // client.setCallback(callback);
  // Blynk.begin(auth, ssid, pass);
  Blynk.begin( BLYNK_AUTH_TOKEN, ssid, password);
  lcd.clear();
}

void loop() {

  state = false;

  if (state == true || WiFi.status() != WL_CONNECTED) {
    Production();
    Blynk.disconnect();
    strcpy(status_koneksi, "Offline");
  }else {
    Serial.println("Run Blynk");
    Read_Inputs();
    Blynk.run();
    strcpy(status_koneksi, "Online");
  }
  Adjust_Pump();
  Display_Level();

  // if (!client.connected()) {
  //   reconnect();
  // } else {
  //   strcpy(status_koneksi, "Online");
  // }
  
}
