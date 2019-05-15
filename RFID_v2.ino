/*
 * Yapılacaklar:
 *    RFID Sistemini düzgün olarak çalıştır. +++
 *    Serial ekrandan kontrollü olacak şekilde adım adım kişi ekleme arayüzü oluştur. ++
 *    Oluşturulam kişileri eeproma kayıt et.
 *  Her okumada kişiyi ekrana yazdır. Ekran üzerinde toplantı kodu saat ve tarih olsun. //NOKİA 5110
 *  Toplantı sonrasında eepromdan gerekli bilgileri al ve geri kalan kısmı boşalt.
 *  Olursa excel çıktısı al.
 */

/*Kütüphaneler*/
#include <SPI.h>
#include <EEPROM.h>
#include <RFID.h>

/*Globaller*/
//Objeler
RFID rfid(10,9);

//Değişkenler
byte readedCardId[5] = {0,0,0,0,0}; //Bir önceki sefer okunmuş kart.
byte counter = 0; //Kart benzerlik katsayısı.
bool done = false; //Kart okundu mu sorugusu.


/*
 * Working mode 
 * Her işlem bir şifre gerektirir. Bu şifre eepromda saklı bulunur.
 * Toplamda eepromda sıralı kaç kişi olduğu, eepromun son byte'ına
    -İşlem yapılan son byte ise eeprom'un sondan bir önceki adresine yazılır.
  -Toplam kullanılan byte ise eepromun sondan iki önceki byte'ına yazılır.
 *  - Mode 0: Normal kullanım:
      Gelen kişileri ekrana yazdır, eeproma kaydet.
        Eğer kartı mevcutsa üstüne yazmaz.
  -- Mode 1: Kişi ekleme:
      Kartları eeproma kayıt eder.
        Kartı okut, ismi tanımla, sonucu ekrana yazdır.
  --- Mode 2: Tüm verileri görme:
      EEPROM üzerindeki tüm verileri ekranan yazdırır.
  ---- Mode 3: Katılım hafızası silme:
      Bir önceki katılım verilerini siler.
  ----- Mode 4: Kart silme:
      Okutulan kartı eepromdan siler. Böyle bir kart yoksa da geri dönüşte bulunur.
  ------ Mode 5: Fabrika ayarları:
      Eepromu boşaltır. Bunu yapmadan önce işlem şifresi ve eeprom şifresi sorar.
 */
byte workingMode = 1; 
bool remakeLoop1 = false; //1. working mode için işlem tekrarını kapatır.
void setup()
{
  /*Kurulumlar*/
  Serial.begin(9600); //Seri haberleşmeyi başlat.
  SPI.begin(); //SPI haberleşme protokolünü başlat.
  rfid.init(); //RFID objesinin kurulumu.

  Serial.print("Working Mode: ");
  Serial.println(workingMode);

  if(workingMode == 1)
  {
    // INFO
      Serial.print("Kart ekleme modu başlatıldı. WorkingMode Code: ");
      Serial.println(workingMode);
      Serial.println("/==================================================/");
  }
}

void loop()
{
  
  //Yönetici kullanımı
  if(workingMode != 0/* && Serial.available()*/)
  { 
    
    if(workingMode == 1)
    {
      AddCard();
    }
    else if(workingMode == 2)
    {
      ViewAllContents();
    }
    else if(workingMode == 3)
    {
      DeleteAllContents();
    }
    else if(workingMode == 4)
    {

    }
    else if(workingMode == 5)
    {

    }
  }

  //Normal kullanım.
  else if(workingMode == 0)
  {

  }
}

//Eeprom kurulum değişkenleri.
byte lastByte; //En son işlem yapılan eeprom byte'ı.
byte byteCounter; //Toplamda kaç byte yer kullanıldığını tutan sayaç.
byte lastestBytes = 3; //Sondan önceki özel byte'lar
void AddCard()
{
  if(!remakeLoop1)
  {
    Serial.println("\n/==================================================/");
    //EEPROM
    Serial.println("EEPROM'a ulaşıldı.");
    Serial.print("En son kullanılan EEPROM adresi: ");
    Serial.println(lastByte);
    Serial.print("Toplam kullanılan adres: ");
    Serial.println(byteCounter);


    Serial.println("Kartınızı okutunuz...");
    remakeLoop1 = true;
  } 
  if(remakeLoop1)
  {
    if(rfid.isCard())
    {      
      if(rfid.readCardSerial())
      {       
        //Kart verisini içinde tutan array'i gez
        for(int i = 0; i < 5; i++)
        {
          //Eğer kartın şuanki id'si bir önceki loop üzerindeki id ile aynı ise bu loop'u es geç.
            //Eğer serinin tüm elemanları aynı ise kartlar aynı demektir.
          if(readedCardId[i] == rfid.serNum[i]) counter++;
          else counter--; 
      }

      if(counter != 5)
      {
        Serial.println("Kart okundu.");
        byte printCounter; //Kaç byte yazıldığını sayar.
        //Diziyi ekrana yazdır ve okunan veriyi eski veriye eşitle.
        Serial.print("Kart id numarası: ");
        for (int i = 0; i < 5; i++)
        {
          //Veriyi ekrana yazdır.
          Serial.print(rfid.serNum[i]);   
          readedCardId[i] = rfid.serNum[i];

          if(isCardExist(readedCardId))
          //Veriyi eeproma yaz.
          lastByte++;
          EEPROM.write(lastByte, readedCardId[i]);
          printCounter = i + 1;
          //
        }


        byteCounter += printCounter; //Kullanılan toplam byte'ı ekrana yazdır.
        //Serial.println("/==================================================/");
        Serial.print("\nEEPROM ");
        Serial.print(lastByte);
        Serial.print(" - ");
        Serial.print(lastByte + printCounter);
        Serial.print(" adresleri arasına ");
        Serial.print(printCounter);
        Serial.println(" byte veri yazıldı!");
        Serial.println("  ");
        Serial.print("Toplamda ");
        Serial.print(lastByte + lastestBytes);
        Serial.println(" byte alan kullanıldı.");
        Serial.print("Geriye ");
        Serial.print(1024 - lastByte - lastestBytes); //Son iki byte'ı rfid programı kendisi kullanıyor.
        Serial.println(" byte alan kaldı.");
        Serial.println("/==================================================/");
        remakeLoop1 = false;
      }
      counter = 0; //Sayacı sıfırla.
      }
      rfid.halt();
    }
  }
}
//Hafıza kontrol fonksiyonu

byte sameCard; //Eğer kart mevcutsa mevcut kartın EEPROM adresi bu değişkene yazılır.
/*
 * İçine kontrol edilcek kartın id'si yazılır.
 * Kart hafızada varsa true döndürür.
 */
bool isCardExist(byte[] card)
{
  //Aynı id'ye sahip bir kart varsa true döndür.
  bool _isCardExist = false;
  /*
   * Arduino uno 1024byte EEPROM hafızasına sahiptir.
   * Dolayısıyla for saymayı EEPROMUN Son byte'ında kayıtlı olan lastByte değişkenine kadar yapar.
   * Bir kart 5 byte'lık bir diziden oluştuğu için, ilk for 5'er 5'er sayar.
   */
  for(int i = 0; i < 1023; i += 5)
  {
    for(int j = 0; j < 5; j++)
    {
      if(card[i] == EEPROM.read(i+j))
      {
        _isCardExist = true;
      }
    }
    //Eğer kart mevcutsa 
    if(_isCardExist)
    {
      //Eğer kart mevcutsa adresini kaydet
      sameCard = j;
      break;
    }
    else sameCard = null; //Kart aynı değilse değişkeni boşalt.
  }
}

void ViewAllContents()
{
  for(int i = 0; i < 1024;i++)
  {
    Serial.println(EEPROM.read(i));
  }
}

void DeleteAllContents()
{
  for(int i = 0; i < 1020; i++)
  {
    EEPROM.write(i,0);
  }
}