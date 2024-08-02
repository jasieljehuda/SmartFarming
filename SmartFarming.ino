#define BLYNK_TEMPLATE_ID "TMPL6Xc3-YXXo"
#define BLYNK_TEMPLATE_NAME "Tugas Akhir"
#define BLYNK_AUTH_TOKEN "BJPecqb5IGVAQJTsE9Lftt25IjSL0oEu"
#define BLYNK_PRINT Serial

#include <Wire.h>
#include <Servo.h>
#include <DHT.h>
#include <RTClib.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>

char auth[] = BLYNK_AUTH_TOKEN;  // Masukkan token autentikasi Blynk Anda
char ssid[] = "hah?";    // Masukkan SSID WiFi Anda
char pass[] = "andasiapa?"; // Masukkan password WiFi Anda

DHT dht(D4,DHT11);
#define echoPin D7  // D7 pada ESP8266
#define trigPin D6  // D6 pada ESP82
int fanPin = D0; // D5 Relay int_2 kuning
#define mistMakerPin D3 //D3 relay int_1 orange
#define lampPin 14 // s2

RTC_DS3231 rtc;
const char* daysOfTheWeek[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


Servo myServo;

// Deklarasi pin virtual untuk Blynk
#define HUMIDITY_PIN V1
#define TEMPERATURE_PIN V0
#define DISTANCE_PIN V2
#define FAN_STATUS_PIN V3
#define LAMP_STATUS_PIN V4
#define MIST_STATUS_PIN V5

// Inisialisasi LCD I2C 20x4 (alamat I2C 0x27, layar 20x4)
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  Serial.begin(9600);
  // Inisialisasi LCD
  Wire.begin(); // SDA adalah D2, SCL adalah D1 untuk ESP8266
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Selamat Datang");

  // Inisialisasi koneksi WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan ke WiFi...");
  }
  Serial.println("Terhubung ke WiFi");

  // Inisialisasi Blynk
  Blynk.begin(auth, ssid, pass);

  // Memulai sensor DHT
  dht.begin();
  if (!rtc.begin()) {
    Serial.println("Tidak bisa menemukan RTC");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Tidak bisa menemukan RTC");
    while (1);
  }

  // Jika RTC kehilangan daya, setel waktu
  if (rtc.lostPower()) {
    Serial.println("RTC kehilangan daya, mengatur waktu!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RTC kehilangan daya");
    ///rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    
  }

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  myServo.attach(D8); // D8 pada ESP8266

  // Set pin perangkat tambahan sebagai output
  pinMode(fanPin, OUTPUT);
  pinMode(lampPin, OUTPUT);
  pinMode(mistMakerPin, OUTPUT);

  delay(1000); // Delay selama 1 detik setelah setup
}

void getUltra() {
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration / 2) / 29.1;

  if (distance <= 5) {
    myServo.write(180);
    Serial.print("Pakan Tersedia! ");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pakan Tersedia!");
    Blynk.virtualWrite(DISTANCE_PIN, distance); // Mengirim nilai jarak ke pin virtual V2 di Blynk
  } else {
    myServo.write(0);
    Serial.print("Waktunya Pengisian Pakan... ");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Waktunya Isi Pakan");
    Blynk.virtualWrite(DISTANCE_PIN, distance); // Mengirim nilai jarak ke pin virtual V2 di Blynk
  }
  delay(100);
}

void getDHT() {
  // Membaca kelembaban sebagai persentase
  float h = dht.readHumidity();
  // Membaca suhu sebagai Celcius (default)
  float t = dht.readTemperature();

  // Periksa apakah pembacaan gagal dan keluar lebih awal (untuk mencoba lagi)
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }


  // Menampilkan nilai ke Serial Monitor
  Serial.print("Kelembaban : ");
  Serial.print(h);
  Serial.print(" %");
  Serial.print("Suhu : ");
  Serial.print(t);
  Serial.print(" *C");
  Serial.println(); // Pastikan newline setelah

  // Mengirim data ke Blynk
  Blynk.virtualWrite(HUMIDITY_PIN, h); // Mengirim nilai kelembaban ke pin virtual V1 di Blynk
  Blynk.virtualWrite(TEMPERATURE_PIN, t); // Mengirim nilai suhu ke pin virtual V0 di Blynk

  // Menampilkan data pada LCD
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(t);
  lcd.print(" C");
  lcd.setCursor(10, 1);
  lcd.print("H:");
  lcd.print(h);
  lcd.print(" %");

  // Logika untuk mengontrol kipas
  if (t > 30.00) { // suhu lebih dari 30 derajat Celsius
    digitalWrite(fanPin, HIGH); // Nyalakan kipas
    Blynk.virtualWrite(FAN_STATUS_PIN, 1); // Kirim status kipas ke pin virtual V3 di Blynk (1 = ON)
  } else {
    digitalWrite(fanPin, LOW); // Matikan kipas
    Blynk.virtualWrite(FAN_STATUS_PIN, 0); // Kirim status kipas ke pin virtual V3 di Blynk (0 = OFF)
  }

  // Logika untuk mengontrol mist maker
  if (h < 50.00) { // Kelembaban kurang dari 50%
    digitalWrite(mistMakerPin, HIGH); // Nyalakan mist maker
    Blynk.virtualWrite(MIST_STATUS_PIN, 1); // Kirim status mist maker ke pin virtual V5 di Blynk (1 = ON)
  } else {
    digitalWrite(mistMakerPin, LOW); // Matikan mist maker
    Blynk.virtualWrite(MIST_STATUS_PIN, 0); // Kirim status mist maker ke pin virtual V5 di Blynk (0 = OFF)
  }
}

void loop() {
  Blynk.run();
  getUltra();
  getDHT();

  DateTime now = rtc.now();
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(", ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  // Menampilkan waktu dan tanggal pada LCD
  lcd.setCursor(0, 2);
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  lcd.setCursor(0, 3);
  lcd.print(now.year(), DEC);
  lcd.print('/');
  lcd.print(now.month(), DEC);
  lcd.print('/');
  lcd.print(now.day(), DEC);
  lcd.print(" ");
  lcd.print(now.hour(), DEC);
  lcd.print(':');
  lcd.print(now.minute(), DEC);
  lcd.print(':');
  lcd.print(now.second(), DEC);

  // Periksa apakah waktu saat ini adalah 02:20:00
// if (now.hour() == 19 && now.minute() == 42 && now.second() == 0) 
//  {
//    // Turn on the relay
//    Serial.println("Lampu Nyala");
//    digitalWrite(lampPin, HIGH);
//    Blynk.virtualWrite(LAMP_STATUS_PIN, 1);
//  }
//
//  // Check if the current time is 21:55:00
//  if (now.hour() == 19 && now.minute() == 43 && now.second() == 0) 
//  {
//    // Turn off the relay
//    Serial.println("Lampu mati");
//    digitalWrite(lampPin, LOW);
//    Blynk.virtualWrite(LAMP_STATUS_PIN, 0);
//  }
  delay(5000);
}
