/*
        Alarma Sanbo
%%%%%%%%%%%%%   Autores   %%%%%%%%%%%%%%%
-Alan Völker
-Federico Chara Lobo
-Leandro Ponte Recalde
-Ezequiel Santangelo

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/
#define LED 2
#define BOTON_MENU 3
#define BOTON_ADD 4
#define BOTON_REMOVE 5
#define BOTON_UP 6
#define BOTON_DOWN 7
#define BOTON_RIGHT 8
#define BOTON_LEFT 9
#define BOTON_ENTER 10
#define max_alarmas 5
#include <EEPROM.h>
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); //poner el address del LCD en 0x27 o 0x03
RTC_DS3231 rtc;
//La EEPROM guarda valores de 1 byte... por ende se debe guardar la alarma dividida en 4 valores, segs, mins, hs, dia
int horarios[max_alarmas][/*Valores*/4]= {
  //el dia va de 0 a 6
  //La unica manera de romper el codigo es sobreescribiendo la eeprom con cualquier cosa, es decir, dia 16, hora 99, etc.
  EEPROM.read(0),EEPROM.read(1),EEPROM.read(2),EEPROM.read(3),      //[numero de alarma][hora, minuto, segundo, dia]
  EEPROM.read(4),EEPROM.read(5),EEPROM.read(6),EEPROM.read(7),      // idem
  EEPROM.read(8),EEPROM.read(9),EEPROM.read(10),EEPROM.read(11),    // idem
  EEPROM.read(12),EEPROM.read(13),EEPROM.read(14),EEPROM.read(15),  // idem
  EEPROM.read(16),EEPROM.read(17),EEPROM.read(18),EEPROM.read(19),  // idem
};
byte backslash[8]={
  B00000,
  B10000,
  B01000,
  B00100,
  B00010,
  B00001,
  B00000,
  B00000,
};
byte cuadrado_vacio[8]={
  B11111,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111,
};
byte cuadrado_lleno[8]={
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};
void mostrarAlarmas(void);
void ingresarAlarmas(void);
void clearEEPROM(void);
int isButtonPressed(void);
void setup()
{
  Serial.begin(9600);
  if (! rtc.begin()) {
    //Serial.println("Couldn't find RTC");
    //Serial.flush(); sin esta linea, aparentemente tira basura rtc.now()
    while (1) delay(10);
  }
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //Una vez
  //Los botones.
  pinMode(LED, OUTPUT);
  pinMode(BOTON_MENU, INPUT);
  pinMode(BOTON_ADD, INPUT);
  pinMode(BOTON_REMOVE, INPUT);
  pinMode(BOTON_UP, INPUT);
  pinMode(BOTON_DOWN, INPUT);
  pinMode(BOTON_RIGHT, INPUT);
  pinMode(BOTON_LEFT, INPUT);
  pinMode(BOTON_ENTER, INPUT); 
  pinMode(BOTON_MENU, INPUT_PULLUP);
  pinMode(BOTON_ADD, INPUT_PULLUP);
  pinMode(BOTON_REMOVE, INPUT_PULLUP);
  pinMode(BOTON_UP, INPUT_PULLUP);
  pinMode(BOTON_DOWN, INPUT_PULLUP);
  pinMode(BOTON_RIGHT, INPUT_PULLUP);
  pinMode(BOTON_LEFT, INPUT_PULLUP);
  pinMode(BOTON_ENTER, INPUT_PULLUP); 
  //Para marcar que se encendio.
  digitalWrite(LED, HIGH);
  lcd.createChar(0, backslash);
  lcd.createChar(1, cuadrado_vacio);
  lcd.createChar(2, cuadrado_lleno);
  lcd.init();          
  lcd.backlight();     
  
}

char diasDeLaSemana[7][2] = {"D", "L", "m", "M", "J", "V", "S"};
char animacion[8][2] = {"/", "-", "\0","|","/","-","\0","|"};
int reloj{};
void loop()
{
  //DEBE QUEDARSE DENTRO DE VOID LOOP.
  DateTime now = rtc.now(); 
  //Menu inicial
  lcd.setCursor(0, 0); // columna, fila       
  lcd.print("  Timbre Sanbo");
  lcd.setCursor(0, 1);
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
    lcd.print(diasDeLaSemana[now.dayOfTheWeek()][0]);
    lcd.print(" ");
    lcd.print((int)rtc.getTemperature());
    lcd.print("C ");
    if (reloj==2 || reloj==6){
      lcd.write(byte(0));
    }
    else lcd.print(animacion[reloj]); //No funciona correctamanete en simulacion.
    reloj++;
    if(reloj==8){
      reloj=0;
    }
    delay(250); //Para hacer que el reloj no sea el rayo mcqueen
  //Menu de alarmas
  if(digitalRead(BOTON_MENU)==LOW){
    while(digitalRead(BOTON_MENU)==LOW);
    mostrarAlarmas();
  }

}
    /*
    Restante: 
    
    Si queres agregar, tocar el boton "+", y de ahi usas las flechitas para moverte (para
    concretar el horario, y que sea similar a como pones la hora en la BIOS), una vez que se termina, se 
    deberia de tocar el "ENTER", y carga el menu de nuevo, ahora haciendo display de las alarmas actualizadas.

    Si toca el "-", sale un menu con cuadrados vacios y llenos, y con el teclado 
    seleccionan entre la alarma 1 y max_alarmas, se borra/n la/s alarma/s seleccionada/s, y se vuelve a cargar el menu.
    
    Por criterio de diseño, las alarmas deberian estar limitadas a cierto numero, aun por determinar (quizas 8,
    para darle algo de 'formateo' al muestreo...o bien por el simple hecho de no scrollear nunca, jode las cosas el scroll de pantalla).
    
    */
//Funcion que muestra las alarmas.
void mostrarAlarmas(void){
  while(1){
    lcd.clear();
    for(int i=0;i<125;i++){ //Corroborar cada 6ms si se pulsa un boton
      if(isButtonPressed()){
      while(isButtonPressed());
      return; 
      } 
      delay(6); //Para dar ilusion de...carga? 750ms (6*125)
    }
  
    lcd.setCursor(0, 0);
    lcd.print("Alarmas");
    for (int i = 0; i < 100; i++)
    {
      if(isButtonPressed()){
      while(isButtonPressed());
      return; 
      } 
      delay(10);
    }
    lcd.clear();
    for(int j=0; j<max_alarmas;j++){
      if(isButtonPressed()) return; 
      lcd.clear();
      //Mostrar alarma j.
      lcd.setCursor(0, 0);
      lcd.print(j+1);
      lcd.setCursor(0, 1);
        for (int i = 0; i < 4; i++){
          if (i==3){
            lcd.print(" ");
            lcd.print(diasDeLaSemana[horarios[j][i]][0]); //dudoso, me fijo luego... horarios[j][4] tiene un numero del 0 al 6... si eso lo pongo en el dia e la semana...
            continue;
      }
          if(horarios[j][i]<10){
            lcd.print("0");
            lcd.print(horarios[j][i]);
      } else lcd.print(horarios[j][i]);    
      if(i<2){
        lcd.print(":");
      }
      
  }
      for(int i=0; i<100;i++){
        if(isButtonPressed()){
        while(isButtonPressed());
        return; 
      } 
        delay(50);
      } //Tiempo para mostrar la alarma. 5000ms (100*50)
  }
}
/* Para borrar la memoria EEPROM, veo si poner un boton...creo que generaria fallos...a no ser que genere una pantalla que dure x cant de segundos
 * y que diga memorias reseteadas
 */
void clearEEPROM(void){ 
  lcd.clear();
  delay(500);
  lcd.setCursor(0,0);
  lcd.print("Reseteando alarmas...");
  for(int i=0;i<20;i++) EEPROM.write(i, 0); 
  delay(2000);
  lcd.clear();
}
// Una funcion que devuelve 1 si se presiona uno de los botones...
int isButtonPressed(void){
  if(digitalRead(BOTON_MENU)==LOW) return 1;
  if(digitalRead(BOTON_ADD)==LOW) return 1;
  if(digitalRead(BOTON_REMOVE)==LOW) return 1;
  return 0;
}
/*  Funcion para ingresar las alarmas, de manera que muestre todo 0, (y el dia 0, que es LUNES) si no hay nada escrito. Y 
 *  si hay, que muestre lo que hay.
 *  ahora, para ingresar una nueva alarma, tiene que titilar el parametro a modificar, y con las flechas ARRIBA y ABAJO se cambia de a 5.
 *  con las flechas DERECHA e IZQUIERDA se cambia el parametro a modificar.
 */ 
void ingresarAlarmas(void){
  
}
