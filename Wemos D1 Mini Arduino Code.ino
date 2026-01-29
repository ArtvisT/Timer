#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);

bool start = LOW;

bool oncekiButonDurum = HIGH;
unsigned long butonZamani1 = 0;
#define buton D7
#define dt D5
#define clk D6

#define dokunmatikButon D3
#define led D0
unsigned long dokunmatikButonZamani = 0;
bool dokunmatikButonOncekiDurum = HIGH;


volatile int yeniEncoded = 0;
volatile long encoderDeger = 0;
volatile int eskiEncoded = 0;

int oncekiEncoderDeger = 0;
unsigned long encoderZamani = 0;

int mod = 1;

long kalanSaniye = 14400;
int saniye = 0;
int dakika = 0;
int saat = 0;
unsigned long zaman = 0;
unsigned long oncekiZaman = 0;
//unsigned long toplamSaniye = 0;
//unsigned long oncekiToplamSaniye = 0;
char zamanBuffer[16];

void ICACHE_RAM_ATTR updateEncoder() {
  if (mod == 2) {
    encoderZamani = zaman;
    int clk_durum = digitalRead(clk);
    int dt_durum = digitalRead(dt);


    int yeniEncoded = (clk_durum << 1) | dt_durum;

    int anahtar = (eskiEncoded << 2) | yeniEncoded;

    if (anahtar == 0b0010 || anahtar == 0b1011 || anahtar == 0b1101 || anahtar == 0b0100) encoderDeger++;
    if (anahtar == 0b1110 || anahtar == 0b0111 || anahtar == 0b0001 || anahtar == 0b1000) encoderDeger--;


    eskiEncoded = yeniEncoded;
  }
}

void setup() {
  //Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(19, 8);
  display.println("Timer");
  display.setCursor(46, 38);
  display.println("V1");
  display.display();
  delay(3400);


  int zamanlayiciDakika = (kalanSaniye / 60) % 60;
  int zamanlayiciSaat = (kalanSaniye / 3600) % 24;
  sprintf(zamanBuffer, "%02d:%02d", zamanlayiciSaat, zamanlayiciDakika);
  ekraniGuncelle();


  pinMode(buton, INPUT_PULLUP);
  pinMode(dt, INPUT_PULLUP);
  pinMode(clk, INPUT_PULLUP);
  pinMode(led, OUTPUT);
  pinMode(dokunmatikButon, INPUT_PULLUP);
  digitalWrite(led, LOW);


  attachInterrupt(digitalPinToInterrupt(dt), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(clk), updateEncoder, CHANGE);
}

void loop() {
  zaman = millis();
  bool butonDurum = digitalRead(buton);
  bool dokunmatikButonDurum = digitalRead(dokunmatikButon);

  noInterrupts();
  long gosterilecekDeger = encoderDeger / 2;
  interrupts();
  /*
  toplamSaniye = zaman / 1000;
  saniye = toplamSaniye % 60;
  dakika = (toplamSaniye / 60) % 60;
  saat = (toplamSaniye / 3600) % 24;
  */

  if (!start && butonDurum == LOW && butonDurum != oncekiButonDurum && zaman - butonZamani1 >= 200) {
    butonZamani1 = zaman;


    mod++;
    if (mod > 2) mod = 1;
  }
  oncekiButonDurum = butonDurum;

  if (butonDurum == HIGH && zaman - butonZamani1 >= 10000 && zaman - encoderZamani >= 10000) {
    mod = 1;
  }

  if (kalanSaniye <= 0) {
    kalanSaniye = 0;
    start = false;
    analogWrite(led, 0);
  }



  if (dokunmatikButonDurum == LOW && dokunmatikButonDurum != dokunmatikButonOncekiDurum && zaman - dokunmatikButonZamani >= 200) {
    dokunmatikButonZamani = zaman;
    start = !start;
    mod = 1;
    if (kalanSaniye > 0 && start) {
      analogWrite(led, 300);
    } else {
      analogWrite(led, 0);
    }
  }
  dokunmatikButonOncekiDurum = dokunmatikButonDurum;

  switch (mod) {
    case 1:
      if (start) {
        if (zaman - oncekiZaman >= 1000) {
          oncekiZaman = zaman;

          if (kalanSaniye > 0) {
            kalanSaniye--;
          }

          int zamanlayiciDakika = (kalanSaniye / 60) % 60;
          int zamanlayiciSaat = (kalanSaniye / 3600) % 24;
          sprintf(zamanBuffer, "%02d:%02d", zamanlayiciSaat, zamanlayiciDakika);
          ekraniGuncelle();
        }
      }
      break;

    case 2:
      if (gosterilecekDeger != oncekiEncoderDeger) {

        long degisim = gosterilecekDeger - oncekiEncoderDeger;
        kalanSaniye = kalanSaniye - (kalanSaniye % 60);

        kalanSaniye = kalanSaniye + (degisim * 60);
        if (kalanSaniye < 0) kalanSaniye = 0;

        int zamanlayiciDakika = (kalanSaniye / 60) % 60;
        int zamanlayiciSaat = (kalanSaniye / 3600) % 24;
        sprintf(zamanBuffer, "%02d:%02d", zamanlayiciSaat, zamanlayiciDakika);
        ekraniGuncelle();
        oncekiEncoderDeger = gosterilecekDeger;
      }
      break;
  }
}


void ekraniGuncelle() {
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(4, 16);
  display.print(zamanBuffer);
  display.display();
}
