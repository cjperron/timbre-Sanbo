/*
        Alarma Sanbo
%%%%%%%%%%%%%   Autores   %%%%%%%%%%%%%%%
-Alan Völker
-Federico Chara Lobo
-Leandro Ponte Recalde
-Ezequiel Santangelo
%%%%%%%%%%%%%   Materiales   %%%%%%%%%%%%%%%
##
*/
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 or 0x03
RTC_DS1307 rtc;
char diasDeLaSemana[7][12] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};
void setup()
{
  Serial.begin(9600);
  lcd.init();          // Inicializar el lcd
  lcd.backlight();     // prende la backlight
  lcd.setCursor(0, 0); // pone el cursor en la fila 0, columna 1.
  // lcd.print("Hola mundo!");           // Print "Hola mundo" en el Display
  if (!rtc.begin())
  {
    // Serial.println("No se encontro el RTC");
    // Serial.flush();
    while (1)
      delay(10);
  }

  if (!rtc.isrunning())
  {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}
void loop()
{
  DateTime now = rtc.now();
  if (now.hour() >= 0 && now.hour() < 10)
  {
    lcd.print("0");
    lcd.print(now.hour());
  }
  else
  {
    lcd.print(now.hour());
  }
  lcd.print(':');
  if (now.minute() >= 0 && now.minute() < 10)
  {
    lcd.print("0");
    lcd.print(now.minute());
  }
  else
  {
    lcd.print(now.minute());
  }
  lcd.print(':');
  if (now.second() >= 0 && now.second() < 10)
  {
    lcd.print("0");
    lcd.print(now.second());
  }
  else
  {
    lcd.print(now.second());
  }
  delay(1000);
  lcd.clear();
  /*
  lcd.print(now.year(), DEC);
  lcd.print('/');
  lcd.print(now.month(), DEC);
  lcd.print('/');
  lcd.print(now.day(), DEC);
  lcd.print(" (");
  lcd.print(diasDeLaSemana[now.dayOfTheWeek()]);
  lcd.print(") ");
  */
}

