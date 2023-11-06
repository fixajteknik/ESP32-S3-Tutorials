#include "LoRa_E22.h"
#include <HardwareSerial.h>
#define M0 4   //  3in1 PCB mizde pin 7
#define M1 6   //  3in1 PCB mizde pin 6
#define RX 18  //  esp32 s3 de Lora RX e bağlı
#define TX 17  //  esp32 s3 de Lora TX e bağlı
#define LED 42

HardwareSerial fixSerial(1);
LoRa_E22 fixajSerial(TX, RX, &fixSerial, UART_BPS_RATE_9600);

unsigned long kanalBekleme_sure = 0;
int kanalBekleme_bekleme = 3000;

#define Adres 1   //0--65000 arası bir değer girebilirsiniz. Diğer Modüllerden FARKLI olmalı
#define Kanal 20  //Frekans değeri
//E22 için 0--80 arasında bir sayı giriniz. Diğer Modüllerle AYNI olmalı.
//E22 900 için Frekans = 850+kanal değeri.
#define Netid 63  //0--65000 arası bir değer girebilirsiniz. Diğer Modüllerle AYNI olmalı.

#define GonderilecekAdres 2

struct Signal {
  char sifre[15] = "Fixaj.com";
  char konum[15];
  bool btn1;
  byte btn2[10];
  byte btn3[4];
} data;


void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }
  delay(200);

  fixajSerial.begin();

  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(LED, OUTPUT);

  LoraE22Ayarlar();  //LoRa parametre ayarları dinamik şekilde veriliyor.

  digitalWrite(M0, LOW);
  digitalWrite(M1, LOW);
  delay(500);

  Serial.println("başlıyoruz.. Fixaj.com");
}

void loop() {

  data.btn1 = false;
  *(float*)(data.btn2) = 37.199386;
  *(int*)(data.btn3) = 1453;
  strcpy(data.konum, "Teknofest");

  fixajSerial.sendFixedMessage(highByte(GonderilecekAdres), lowByte(GonderilecekAdres), Kanal, &data, sizeof(Signal));
  delay(2000);

  while (fixajSerial.available() > 1) {

    ResponseStructContainer rsc = fixajSerial.receiveMessage(sizeof(Signal));
    struct Signal data = *(Signal*)rsc.data;
    rsc.close();

    if (strcmp(data.sifre, "Fixaj.com") == 0) {
      Serial.println("Sifre doğru");
      Serial.print("Konum: ");
      Serial.println(data.konum);
      Serial.print("buton durumu: ");
      Serial.println(data.btn1);
      Serial.print("Kordinat: ");
      Serial.println(*(float*)(data.btn2), 6);
      Serial.print("Büyük sayı: ");
      Serial.println(*(int*)(data.btn3));

      (*(int*)(data.btn3) % 2) ? digitalWrite(LED, HIGH) :  digitalWrite(LED, LOW); //btn3 değerine göre ledi aç kapa kodu
    }
  }
}

void LoraE22Ayarlar() {

  digitalWrite(M0, LOW);
  digitalWrite(M1, HIGH);

  ResponseStructContainer c;
  c = fixajSerial.getConfiguration();
  Configuration configuration = *(Configuration*)c.data;

  //DEĞİŞEBİLEN AYARLAR
  // Üstte #define kısmında ayarlayınız
  configuration.ADDL = lowByte(Adres);
  configuration.ADDH = highByte(Adres);
  configuration.NETID = Netid;
  configuration.CHAN = Kanal;

  //SEÇENEKLİ AYARLAR
  configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;  //Veri Gönderim Hızı 2,4 varsayılan
  //configuration.SPED.airDataRate = AIR_DATA_RATE_000_03;  //Veri Gönderim Hızı 0,3 En uzak Mesafe
  //configuration.SPED.airDataRate = AIR_DATA_RATE_111_625; //Veri Gönderim Hızı 62,5 En Hızlı

  configuration.OPTION.subPacketSetting = SPS_240_00;  //veri paket büyüklüğü 240 byte Varsayılan
  //configuration.OPTION.subPacketSetting = SPS_064_10; //veri paket büyüklüğü 64 byte
  //configuration.OPTION.subPacketSetting = SPS_032_11;  //veri paket büyüklüğü 32 en hızlısı

  configuration.OPTION.transmissionPower = POWER_22;  //Geönderim Gücü max Varsayılan
  //configuration.OPTION.transmissionPower = POWER_13;
  //configuration.OPTION.transmissionPower = POWER_10;  //Geönderim Gücü min

  //GELİŞMİŞ AYARLAR
  configuration.SPED.uartBaudRate = UART_BPS_9600;
  configuration.SPED.uartParity = MODE_00_8N1;
  configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
  configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;

  //configuration.TRANSMISSION_MODE.WORTransceiverControl = WOR_TRANSMITTER;
  configuration.TRANSMISSION_MODE.WORTransceiverControl = WOR_RECEIVER;

  //configuration.TRANSMISSION_MODE.enableRSSI = RSSI_ENABLED;
  configuration.TRANSMISSION_MODE.enableRSSI = RSSI_DISABLED;

  configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
  //configuration.TRANSMISSION_MODE.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;

  configuration.TRANSMISSION_MODE.enableRepeater = REPEATER_DISABLED;
  //configuration.TRANSMISSION_MODE.enableRepeater = REPEATER_ENABLED;

  configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
  // configuration.TRANSMISSION_MODE.enableLBT = LBT_ENABLED;

  // Ayarları KAYDET ve SAKLA
  fixajSerial.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  c.close();
}