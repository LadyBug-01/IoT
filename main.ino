#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL2cXv4RuSN"
#define BLYNK_TEMPLATE_NAME "ControleAccesRFID"
#define BLYNK_AUTH_TOKEN "l0XM9hu7wx9Asn8bfuVtloKEEfBl5cGA"

#include <WiFi.h>
#include <BlynkSimpleEsp32_SSL.h> 
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ================= WIFI (POUR WOKWI) =================
char ssid[] = "Wokwi-GUEST"; 
char pass[] = "";

// ================= PINS =================
#define BUZZER_PIN   14
#define GREEN_LED    26
#define RELAY_PIN    12
#define SERVO_PIN    13
#define PIR_PIN      27

// ================= OBJECTS =================
Servo myServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================= VARIABLES =================
String receivedID = "";
bool motionDetected = false;
String authorizedID = "77 77 77 77";

int failedAttempts = 0;
const int MAX_ATTEMPTS = 4;
unsigned long lockUntil = 0;
const unsigned long LOCK_TIME = 90000; 

bool waitingFor2FA = false;
unsigned long twoFATimeout = 0;
const unsigned long TWOFA_TIMEOUT = 30000; 

void showLCD(String line1, String line2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RELAY_PIN, LOW);

  myServo.attach(SERVO_PIN);
  myServo.write(0);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  showLCD("System Ready", "Blynk 2FA");
  Serial.println("System Ready - En attente de scan");

  Serial.print("Connecting to Wokwi WiFi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // MODIFICATION ICI : Connexion via le port SSL sécurisé (443) pour Wokwi
  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 443);
  Blynk.connect();
  
  if(Blynk.connected()){
      Serial.println("Blynk Connecté de manière sécurisée !");
      Blynk.virtualWrite(V4, "System Online");
  } else {
      Serial.println(" Blynk Hors ligne");
      Blynk.virtualWrite(V4, "WiFi Error");
  }
}

bool isSystemLocked() {
  return (millis() < lockUntil);
}

void openDoor(String reason) {
  Serial.println("Door Opening - " + reason);
  showLCD("Access Granted", "Door Opening");
  Blynk.virtualWrite(V4, "Door Opening");
  
  Blynk.logEvent("rfid_scan", " PORTE OUVERTE ! Accès autorisé via " + reason);

  myServo.write(90);
  digitalWrite(GREEN_LED, HIGH);
  delay(4000); 

  myServo.write(0);
  digitalWrite(GREEN_LED, LOW);

  showLCD("Door Closed", "");
  Blynk.virtualWrite(V4, "Door Closed");
}

BLYNK_WRITE(V1) {
  if (param.asInt() == 1) {
    if (isSystemLocked()) {
      Blynk.virtualWrite(V4, "System is LOCKED!");
      showLCD("System Locked", "Wait...");
      return;
    }
    openDoor("Manual Blynk");
    Blynk.virtualWrite(V1, 0); 
  }
}

BLYNK_WRITE(V0) {
  receivedID = param.asString();
  receivedID.trim();
  
  Serial.println("=== RFID DEBUG ===");
  Serial.print("Reçu: ["); Serial.print(receivedID); Serial.println("]");

  if (isSystemLocked()) {
    showLCD("System Locked", "Wait 90s");
    Blynk.virtualWrite(V4, "LOCKED - Wait 90s");
    return;
  }

  if (waitingFor2FA) {
    Blynk.virtualWrite(V4, "Wait for 2FA...");
    return;
  }

  if (receivedID == authorizedID) {
    failedAttempts = 0;
    start2FA(); 
  } else {
    accessDenied(); 
  }
}

void start2FA() {
  waitingFor2FA = true;
  twoFATimeout = millis() + TWOFA_TIMEOUT;

  showLCD("RFID OK", "Check Phone!");
  Blynk.virtualWrite(V4, "Check Phone Notification!");
  
  // Envoi de l'événement vers Blynk
  Blynk.logEvent("rfid_scan", " ALERTE : Le code légitime (" + authorizedID + ") a été saisi. Appuyez sur V2 pour ouvrir !");
  
  Serial.println("2FA started - Notification Push envoyée.");
}

BLYNK_WRITE(V2) {
  if (param.asInt() == 1 && waitingFor2FA) {
    waitingFor2FA = false;
    openDoor("Blynk Button 2FA");
    Blynk.virtualWrite(V2, 0); 
  }
}

void accessDenied() {
  failedAttempts++;
  Serial.println("ACCESS DENIED - Tentative " + String(failedAttempts));

  digitalWrite(BUZZER_PIN, HIGH);
  showLCD("Access Denied", "Invalid Card");
  Blynk.virtualWrite(V4, "Access Denied");

  if (failedAttempts >= MAX_ATTEMPTS) {
    lockUntil = millis() + LOCK_TIME;
    showLCD("SYSTEM LOCKED", "90 seconds");
    Blynk.virtualWrite(V4, "LOCKED 90s - Alert!");
    
    Blynk.logEvent("rfid_scan", " ALERTE CRITIQUE : Système verrouillé après " + String(failedAttempts) + " faux codes !");
    Serial.println("!!! BRUTEFORCE DETECTED !!!");
  } else {
    Blynk.logEvent("rfid_scan", " ATTENTION : Un code erroné a été tapé : [" + receivedID + "]");
  }

  delay(3000);
  digitalWrite(BUZZER_PIN, LOW);
  
  if (!isSystemLocked()) {
    showLCD("Try Again", "Scan RFID");
    Blynk.virtualWrite(V4, "Scan RFID Again");
  }
}

void loop() {
  if (Blynk.connected()) {
    Blynk.run();
  } else {
    Blynk.connect();
  }

  if (waitingFor2FA && millis() > twoFATimeout) {
    waitingFor2FA = false;
    showLCD("2FA Timeout", "Access Denied");
    Blynk.virtualWrite(V4, "2FA Timeout - Denied");
    Blynk.logEvent("rfid_scan", " Temps écoulé : Le bouton 2FA n'a pas été pressé à temps.");
    Serial.println("2FA Timeout - Access denied");
  }

  int pirState = digitalRead(PIR_PIN);
  if (pirState == HIGH && !motionDetected) {
    motionDetected = true; 
    digitalWrite(GREEN_LED, HIGH);
    showLCD("Motion Detected", "Scan RFID");
    
    if(Blynk.connected()) {
      Blynk.virtualWrite(V4, "Motion Detected - Scan RFID");
    }
  }
  
  if (pirState == LOW && motionDetected) {
    motionDetected = false; 
    digitalWrite(GREEN_LED, LOW);
  }
}
