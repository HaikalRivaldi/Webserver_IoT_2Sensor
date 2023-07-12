
#include <WiFi.h>
#include <MQ135.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

// Replace with your network credentials
const char* ssid = "HAIKAL GANTENG";
const char* password = "888888888";

// Set web server port number to 80
AsyncWebServer server(80);

// Initiating pin analog for MQ135 sensor
#define Anlgpin 34
MQ135 gasSensor = MQ135(Anlgpin);

//Buzzer Pin
#define Buzzerpin 18 // GPIO 18

// RGB Pin
#define PIN_RED    19 // GPIO 19
#define PIN_GREEN  5  // GPIO 5
#define PIN_BLUE   17 // GPIO 17

// Ultrasonic Pin
#define echoPin 27 // Arduino pin connected to Echo pin of HC-SR04
#define trigPin 26 // Arduino pin connected to Trig pin of HC-SR04

//motor Pin
#define IN1_m 32
#define IN2_m 25

//Variable yang digunakan untuk membaca 
long duration; // Variable to store the travel time of the signal
int distance;  // Variable to store the measured distance
float ppm; // Variable to store the measured ppm

//#==========Declaration of all the functions to be used================#
// fungsi untuk menyalakan LED RGB
void setColor(bool R, bool G, bool B) {
  digitalWrite(PIN_RED,   R);
  digitalWrite(PIN_GREEN, G);
  digitalWrite(PIN_BLUE,  B);
}

//fungsi pembacaan nilai Ultrasonic
String ReadUltrasonic(){
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;
    String distanceString = String(distance) + " Cm";
    return distanceString;
  }

//fungsi pembacaan nilai MQ135
String MQ135_read(){
  //digunakan hanya pada saat kalibrasi sensor
//  float rzero = gasSensor.getRZero();
//  Serial.print(rzero);
//  Serial.print("\n");
//  delay(1000);
  
  ppm = gasSensor.getPPM();
//  Serial.print("CO2 concentration: ");
//  Serial.print(ppm);
//  Serial.println(" ppm");
  String ppmString = String(ppm) + " PPM";
  return ppmString;
  }

// fungsi status LED untuk merubah kondisi yang ada di dalam webserver
String LedStatus(int distance, float ppm){
  String ledstatus = "OFF";

  if (distance < 20 && ppm < 10){
    ledstatus = "ON (Green)";
    }
  else if (distance > 20 && ppm > 10){
    ledstatus = "ON (Red)";
    }
  else if (distance > 20 && ppm < 10){
    ledstatus = "ON (blue)";
    }
  else{
    ledstatus = "ON, CEK TANGKI !!!!";
    }
  return ledstatus;
  }

// fungsi status Buzzer untuk merubah kondisi yang ada di dalam webserver
String BuzzerStatus(int distance, float ppm){
  String buzzer = "";

  if (distance < 20 && ppm < 10){
    buzzer = "OFF";
    }
  else if (distance > 20 && ppm > 10){ //buzzer on menandakan buka valve
    buzzer = "ON";
    }
  else if (distance > 20 && ppm < 10){
    buzzer = "OFF";
    }
  else{
    buzzer = "ON, Cek Tangki !!!!";
    }
  return buzzer;
  }

// fungsi status Motor untuk merubah kondisi yang ada di dalam webserver
String MotorStatus(int distance, float ppm){
  String motorstatus = "OFF";

  if (distance < 20 && ppm < 10){
    motorstatus = "ON";
    }
  else if (distance > 20 && ppm > 10){
    motorstatus = "OFF";
    }
  else if (distance > 20 && ppm < 10){
    motorstatus = "OFF";
    }
  else{
    motorstatus = "OFF";
    }
  return motorstatus;
  }

//merupakan fungsi untuk mengganti nilai yang ada pada HTML di webserver agar mengikuti
// pembacaan yang ada pada fungsi ultrasonik dan MQ135
String processor(const String& var) {
  if (var == "DISTANCE") {
    return ReadUltrasonic();
  }
  else if (var == "PPM"){
    return MQ135_read();
    }
  else if (var == "LEDSTATUS"){
    return LedStatus(distance, ppm);
    }
  else if (var == "BUZZER"){
    return BuzzerStatus(distance, ppm);
    }
  else if (var == "MOTOR"){
    return MotorStatus(distance, ppm);
    }
  return String();
}

//merupakan fungsi kondisi yang akan dibaca secara berulang pada perangkat ESP32
void Condition(int distance, float ppm){
  
  if (distance < 20 && ppm < 10){
    setColor(LOW, HIGH, LOW); //Green
    digitalWrite(Buzzerpin, LOW);
    digitalWrite(IN1_m, HIGH);
    digitalWrite(IN2_m, LOW);
    }
  else if (distance > 20 && ppm > 10){
    setColor(HIGH, LOW, LOW); //Red 
    digitalWrite(Buzzerpin, HIGH);
    digitalWrite(IN1_m, LOW);
    digitalWrite(IN2_m, LOW);
    }
  else if (distance > 20 && ppm < 10){
    setColor(LOW, LOW, HIGH); //Blue
    digitalWrite(Buzzerpin, LOW);
    digitalWrite(IN1_m, LOW);
    digitalWrite(IN2_m, LOW);
    }
  else{
    setColor(HIGH, LOW, LOW); //Blue
    delay(1000);
    setColor(LOW, HIGH, LOW);
    delay(1000);
    setColor(LOW, LOW, HIGH);
    delay(1000);
    
    digitalWrite(Buzzerpin, HIGH);
    delay(1000);
    digitalWrite(Buzzerpin, LOW);
    delay(1000);

    digitalWrite(IN1_m, LOW);
    digitalWrite(IN2_m, LOW);
    }
  }
  
void setup(){
  Serial.begin(115200);
  pinMode(PIN_RED,OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(Buzzerpin, OUTPUT);
  pinMode(IN1_m, OUTPUT);
  pinMode(IN2_m, OUTPUT);
  
  // Connect to Wi-Fi, proses ini merupakan sebuah iterasi untuk melakukan koneksi ke dalam jaringan yang dipilih
  // sebelum jaringan terkoneksi perulangan akan selalu mengirimkan keluaran "Connecting to WiFi.." hingga terkoneksi
  // waktu yang dibutuhkan untuk terkoneksi di set dalam 1 detik, digunakannya interval ini untuk memberikan ruang waktu proses pada Chip ESP32 untuk terkoneksi ke jaringan
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  // ketika berhasil terkoneksi ke sebuah jaringan, maka akan langsung mengeluarkan IP Address Local milik Chip ESP32
  Serial.println(WiFi.localIP());

  // Inisiasi library SPIFFS agar dapat membaca file HTML yang telah diupload
  // pembacaan ini ditunjukan apakah file dapat terbaca atau tidak
  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS");
    return;
  }

  //merupakan proses yang digunakan untuk memanggil file HTML yang akan digunakan sebagai webserver 
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  
  //inisiasi pengiriman data distance yang dibaca oleh ultrasonic
  server.on("/distance", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", ReadUltrasonic().c_str());
  });

  //inisiasi pengiriman data ppm yang dibaca oleh MQ135
  server.on("/ppm", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", MQ135_read().c_str());
  });

  //inisiasi pengiriman data kondisi LED
  server.on("/ledstatus", HTTP_GET, [](AsyncWebServerRequest *request){
    String ledStatusValue = LedStatus(distance, ppm);
    request->send(200, "text/plain", ledStatusValue);
  });

  //inisiasi pengiriman data kondisi Buzzer
  server.on("/buzzerstatus", HTTP_GET, [](AsyncWebServerRequest *request){
    String buzzerStatusValue = BuzzerStatus(distance, ppm);
    request->send(200, "text/plain", buzzerStatusValue);
  });

    //inisiasi pengiriman data kondisi Motor
  server.on("/motorstatus", HTTP_GET, [](AsyncWebServerRequest *request){
    String motorStatusValue = MotorStatus(distance, ppm);
    request->send(200, "text/plain", motorStatusValue);
  });

  // Start server
  server.begin();
}

void loop(){
  //merupakan fungsi kondisi yang sudah dibuat sebelumnya lalu dipanggil agar dapat melakukan looping dan akan terus membaca kondisi secara berulang
  Condition(distance, ppm);
}
