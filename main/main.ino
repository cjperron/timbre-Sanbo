/*
        Alarma Sanbo
%%%%%%%%%%%%%   Autores   %%%%%%%%%%%%%%%
-Alan Völker
-Federico Chara Lobo
-Leandro Ponte Recalde
-Ezequiel Santangelo

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 or 0x03
RTC_DS1307 rtc;
byte reloj_12[8]={
  B01110,
  B10101,
  B10101,
  B10101,
  B10101,
  B10001,
  B10001,
  B01110,
};
byte reloj_1_30[8]={
  B01110,
  B10011,
  B10011,
  B10101,
  B10101,
  B10001,
  B10001,
  B01110,
};
byte reloj_3[8]={
  B01110,
  B10001,
  B10001,
  B10001,
  B10111,
  B10001,
  B10001,
  B01110,
};
byte reloj_4_30[8]={
  B01110,
  B10001,
  B10001,
  B10001,
  B10101,
  B10101,
  B10011,
  B01110,
};
byte reloj_6[8]={
  B01110,
  B10001,
  B10001,
  B10001,
  B10101,
  B10101,
  B10101,
  B01110,
};
byte reloj_7_30[8]={
  B01110,
  B10001,
  B10001,
  B10001,
  B10101,
  B10101,
  B11001,
  B01110,
};
byte reloj_9[8]={
  B01110,
  B10001,
  B10001,
  B10001,
  B11101,
  B10001,
  B10001,
  B01110,
};
byte reloj_10_30[8]={
  B01110,
  B11001,
  B10101,
  B10101,
  B10101,
  B10001,
  B10001,
  B01110,
};
void setup()
{
  Serial.begin(9600);
  if (! rtc.begin()) {
    //Serial.println("Couldn't find RTC");
    //Serial.flush(); sin esta linea, aparentemente tira basura rtc.now()
    while (1) delay(10);
  }
  //Todos estos caracteres son la animacion del reloj
  lcd.createChar(0, reloj_12);
  lcd.createChar(1, reloj_1_30);
  lcd.createChar(2, reloj_3);
  lcd.createChar(3, reloj_4_30);
  lcd.createChar(4, reloj_6);
  lcd.createChar(5, reloj_7_30);
  lcd.createChar(6, reloj_9);
  lcd.createChar(7, reloj_10_30);
  lcd.init();          
  lcd.backlight();     
  lcd.setCursor(0, 0); // columna, fila       
  lcd.print("   Alarma Sanbo");
}

char diasDeLaSemana[7][12] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"}; //a implementar.
int i{0}, j{0};
void loop()
{
  DateTime now = rtc.now(); //DEBE QUEDARSE DENTRO DE VOID LOOP.
  lcd.setCursor(0, 1);/* A IMPLEMENTAR SCROLLEANDO  
    lcd.print(now.year());
    lcd.print('/');
    lcd.print(now.month());
    lcd.print('/');
    lcd.print(now.day());
    lcd.print(" (diaDeLaSemana");
    lcd.print(") ");
    */
    if(now.hour()<10 ){
      lcd.print("0");
      lcd.print(now.hour());
    } else lcd.print(now.hour());
    lcd.print(':');
    if(now.minute()<10 ){
      lcd.print("0");
      lcd.print(now.minute());
    } else lcd.print(now.minute());
    lcd.print(':');
    if(now.second()<10 ){
      lcd.print("0");
      lcd.print(now.second());
    } else lcd.print(now.second());
    lcd.print(" ");
    lcd.write(byte(i)); //No funciona correctamanete en simulacion.
    i++;
    if(i==8){
      i=0;
    }
    delay(100); //Para hacer que el reloj no sea el rayo mcqueen
    //delay(1000); SOLO CUANDO NO ANDE BIEN A FRECUENCIA DE CLOCK 16Mhz

    /*
    Restante: 

    Definir los botones de la alarma. { "MENU", "+", "-", "ENTER" "FLECHA-ARRIBA", "FLECHA-ABAJO", "FLECHA-DERECHA", "FLECHA-IZQUIERDA" }

    Hacer el menu que salte con tocar el boton "MENU", y que te diga cuantas alarmas hay, y que las
    figure scrolleando. 
    
    Si queres agregar, tocar el boton "+", y de ahi usas las flechitas para moverte (para
    concretar el horario, y que sea similar a como pones la hora en la BIOS), una vez que se termina, se 
    deberia de tocar el "ENTER", y carga el menu de nuevo, ahora haciendo display de las alarmas actualizadas.

    Si toca el "-", sale un menu con cuadrados vacios y llenos, y con el teclado 
    seleccionan entre la alarma 1/maximo, se borra la alarma, y se vuelve a cargar el menu.
    
    Por criterio de diseño, las alarmas deberian estar limitadas a cierto numero, aun por determinar (quizas 8,
    para darle algo de formateo al muestreo...).
    
    */
}


//inserte Funcion que hace que el texto (en input) se desplace de izquierda a derecha en la linea especificada en el input.
