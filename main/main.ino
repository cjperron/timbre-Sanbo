/*
        Alarma Sanbo
  %%%%%%%%%%%%%   Autores   %%%%%%%%%%%%%%%
  -Alan Völker
  -Federico Chara Lobo
  -Leandro Ponte Recalde
  -Ezequiel Santangelo

  Arduino NANO check
  Botones N.A x8
  tira de 15 pines x2
  cables hembra macho
  placa virgen 100x100
  clavos


  while (millis() % cant_ms == 0) { //codigo }
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/
#define TIEMPO_SONANDO_MS 7000
#define LED 2
#define ALARMA 12
#define BOTON_MENU 3
#define BOTON_ADD 4
#define BOTON_REMOVE 5
#define BOTON_UP 6
#define BOTON_DOWN 7
#define BOTON_RIGHT 8
#define BOTON_LEFT 9
#define BOTON_ENTER 10
#define BOTON_EMERGENCIA 11
#define max_alarmas 8
#include <EEPROM.h>
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); //poner el address del LCD en 0x27 o 0x03
RTC_DS3231 rtc;
struct alarma {
  int hora;
  int minuto;
  int segundo;
  //porque no bool? porque no se puede guardar un bool en EEPROM.
  char dias[7]; //manejo manual de los dias a repetir.
};
struct alarma alarmas[max_alarmas + 1];

byte backslash[8] = {
  B00000,
  B10000,
  B01000,
  B00100,
  B00010,
  B00001,
  B00000,
  B00000,
};
byte cuadrado_vacio[8] = {
  B11111,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111,
};
byte cuadrado_lleno[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};
//Funciones.
void mostrarAlarmas(void);
void ingresarAlarmas(void);
void clearEEPROM(void); //Adentro para hacer debugging
void overwriteEEPROM(void); //En caso de que se actualize el struct alarma alarmas.
int isButtonPressed(void);
void actualizarAlarma(int); //Luego de seleccionar la alarma
void borrarAlarma(void);
void titilar(int, int*); //cuando cambio la hora
void titilarCuadrado(int, char*); //cuando cambio los dias
void tocarTimbre(void); //Cuando se cumple la funcion de abajo
int checkHorario(DateTime); //Revisa si el horario guardado es igual al del RTC.
void bEmergencia(void); //Hace un override de todo y toca el timbre hasta que suelta. (vuelve al menu principal).
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
  pinMode(ALARMA, OUTPUT);
  pinMode(BOTON_MENU, INPUT);
  pinMode(BOTON_ADD, INPUT);
  pinMode(BOTON_REMOVE, INPUT);
  pinMode(BOTON_UP, INPUT);
  pinMode(BOTON_DOWN, INPUT);
  pinMode(BOTON_RIGHT, INPUT);
  pinMode(BOTON_LEFT, INPUT);
  pinMode(BOTON_ENTER, INPUT);
  pinMode(BOTON_EMERGENCIA, INPUT);
  pinMode(BOTON_MENU, INPUT_PULLUP);
  pinMode(BOTON_ADD, INPUT_PULLUP);
  pinMode(BOTON_REMOVE, INPUT_PULLUP);
  pinMode(BOTON_UP, INPUT_PULLUP);
  pinMode(BOTON_DOWN, INPUT_PULLUP);
  pinMode(BOTON_RIGHT, INPUT_PULLUP);
  pinMode(BOTON_LEFT, INPUT_PULLUP);
  pinMode(BOTON_ENTER, INPUT_PULLUP);
  pinMode(BOTON_EMERGENCIA, INPUT_PULLUP);
  //Para marcar que se encendio.
  digitalWrite(LED, HIGH);
  lcd.createChar(0, backslash);
  lcd.createChar(1, cuadrado_vacio);
  lcd.createChar(2, cuadrado_lleno);
  lcd.init();
  lcd.backlight();
  //  clearEEPROM();
  int cprom = 0; //IMPORTANTE: Para llevar la cuenta de la memoria EEPROM
  //Lleno el array de structs
  for (int i = 0; i < max_alarmas; i++) {
    alarmas[i].hora = EEPROM.read(cprom);
    cprom++;
    alarmas[i].minuto = EEPROM.read(cprom);
    cprom++;
    alarmas[i].segundo = EEPROM.read(cprom);
    cprom++;
    for (int j = 0; j < 7; j++) {
      alarmas[i].dias[j] = EEPROM.read(cprom);
      cprom++;
    }
  }
}
char diasDeLaSemana[7][2] = {"D", "L", "m", "M", "J", "V", "S"};
char animacion[8][2] = {"/", "-", "\0", "|", "/", "-", "\0", "|"};
int reloj{};
void loop()
{
  //DEBE QUEDARSE DENTRO DE VOID LOOP. ----|
  DateTime now = rtc.now(); //  < ---------|
  //Menu inicial
  lcd.setCursor(0, 0); // columna, fila
  lcd.print("  Timbre Sanbo");
  lcd.setCursor(0, 1);
  if (now.hour() < 10 ) {
    lcd.print("0");
    lcd.print(now.hour());
  } else lcd.print(now.hour());
  lcd.print(':');
  if (now.minute() < 10 ) {
    lcd.print("0");
    lcd.print(now.minute());
  } else lcd.print(now.minute());
  lcd.print(':');
  if (now.second() < 10 ) {
    lcd.print("0");
    lcd.print(now.second());
  } else lcd.print(now.second());
  lcd.print(" ");
  lcd.print(diasDeLaSemana[now.dayOfTheWeek()][0]);
  lcd.print(" ");
  lcd.print((int)rtc.getTemperature());
  //lcd.print((char)223);
  lcd.print("C ");
  if (reloj == 2 || reloj == 6) lcd.write(byte(0));
  else lcd.print(animacion[reloj]); //No funciona correctamanete en simulacion, en la realidad esta medio LOL.
  reloj++;
  if (reloj == 8) reloj = 0;
  //Comprobacion del timbre
  if (checkHorario(now)) tocarTimbre();
  //Comprobacion del resto de los botones
  for (int i = 0; i < 25; i++) {
    if (digitalRead(BOTON_MENU) == LOW) {
      while (digitalRead(BOTON_MENU) == LOW);
      mostrarAlarmas();//Menu de alarmas
    }
    if (digitalRead(BOTON_ADD) == LOW) {
      while (digitalRead(BOTON_ADD) == LOW);
      ingresarAlarmas();
    }
    if (digitalRead(BOTON_REMOVE) == LOW) {
      while (digitalRead(BOTON_REMOVE) == LOW);
      borrarAlarmas();
    }
    if (digitalRead(BOTON_EMERGENCIA) == LOW) {
      bEmergencia();
    }
    delay(10); //Para hacer que el reloj no sea el rayo mcqueen
  }

}
int checkHorario(DateTime now) {
  for (int i = 0; i < max_alarmas; i++) if (now.hour() == alarmas[i].hora && now.minute() == alarmas[i].minuto && now.second() == alarmas[i].segundo) return 1;
  return 0;
}
void tocarTimbre(void) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tocando timbre");
  digitalWrite(ALARMA, HIGH);
  delay(TIEMPO_SONANDO_MS);
  digitalWrite(ALARMA, LOW);
  return;
}
//Funcion que muestra las alarmas.
void mostrarAlarmas(void) {
  while (1) {
    lcd.clear();
    for (int i = 0; i < 125; i++) { //Corroborar cada 6ms si se pulsa un boton
      if (isButtonPressed()) {
        while (isButtonPressed());
        if (digitalRead(BOTON_EMERGENCIA) == LOW) {
          bEmergencia();
          return;
        }
        return;
      }
      delay(6); //Para dar ilusion de...carga? 750ms (6*125)
    }

    lcd.setCursor(0, 0);
    lcd.print("Alarmas");
    for (int i = 0; i < 100; i++)
    {
      if (isButtonPressed()) {
        while (isButtonPressed());
        if (digitalRead(BOTON_EMERGENCIA) == LOW) {
          bEmergencia();
          return;
        }
        return;
      }
      delay(10);
    }
    lcd.clear();
    for (int j = 0; j < max_alarmas; j++) {
      if (isButtonPressed()) return;
      if (digitalRead(BOTON_EMERGENCIA) == LOW) {
        bEmergencia();
        return;
      }
      lcd.clear();
      //Mostrar alarma j.
      lcd.setCursor(0, 0);
      lcd.print(j + 1);
      lcd.setCursor(0, 1);
      if (alarmas[j].hora < 10) {
        lcd.print("0");
        lcd.print(alarmas[j].hora);
      } else lcd.print(alarmas[j].hora);
      lcd.print(":");
      if (alarmas[j].minuto < 10) {
        lcd.print("0");
        lcd.print(alarmas[j].minuto);
      } else lcd.print(alarmas[j].minuto);
      lcd.print(":");
      if (alarmas[j].segundo < 10) {
        lcd.print("0");
        lcd.print(alarmas[j].segundo);
      } else lcd.print(alarmas[j].segundo);
      lcd.print(" ");
      //Para mostrar los dias seleccionados en la alarma
      for (int i = 0; i < 7; i++) {
        if (alarmas[j].dias[i] == 1) {
          lcd.print(diasDeLaSemana[i][0]);
        }
      }
      for (int i = 0; i < 100; i++) {
        if (isButtonPressed()) {
          while (isButtonPressed());
          if (digitalRead(BOTON_EMERGENCIA) == LOW) {
            bEmergencia();
            return;
          }
          return;
        }
        delay(50);
      } //Tiempo para mostrar la alarma. 5000ms (100*50)
    }
  }
}
/* Para borrar la memoria EEPROM, veo si poner un boton...creo que generaria fallos...a no ser que genere una pantalla que dure x cant de segundos
  y que diga memorias reseteadas
*/
void clearEEPROM(void) {
  lcd.clear();
  delay(500);
  lcd.setCursor(0, 0);
  lcd.print("Reseteando alarmas...");
  for (int i = 0; i < 100; i++) EEPROM.write(i, 0);
  delay(2000);
  lcd.clear();
}
void overwriteEEPROM(void) {
  int cprom = 0;
  for (int i = 0; i < max_alarmas; i++) {
    EEPROM.write(cprom, alarmas[i].hora);
    cprom++;
    EEPROM.write(cprom, alarmas[i].minuto);
    cprom++;
    EEPROM.write(cprom, alarmas[i].segundo);
    cprom++;
    for (int j = 0; j < 7; j++) {
      EEPROM.write(cprom, alarmas[i].dias[j]);
      cprom++;
    }
  }
}

// Una funcion que devuelve 1 si se presiona uno de los botones significativos...
int isButtonPressed(void) {
  if (digitalRead(BOTON_MENU) == LOW) return 1;
  if (digitalRead(BOTON_ADD) == LOW) return 1;
  if (digitalRead(BOTON_REMOVE) == LOW) return 1;
  if (digitalRead(BOTON_EMERGENCIA) == LOW) return 1;
  return 0;
}
/*  Funcion para ingresar las alarmas, de manera que muestre todo 0, (y el dia 0, que es LUNES) si no hay nada escrito. Y
    si hay, que muestre lo que hay.
    ahora, para ingresar una nueva alarma, tiene que titilar el parametro a modificar, y con las flechas ARRIBA y ABAJO se cambia de a 5.
    con las flechas DERECHA e IZQUIERDA se cambia el parametro a modificar.
*/
void borrarAlarmas(void) {
  int alarmaSeleccionada{};
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Borrar alarma");
  for (int i = 0; i < 125; i++)
  {
    if (isButtonPressed()) {
      if(digitalRead(BOTON_EMERGENCIA) == LOW) {
        bEmergencia();
        return;
      }
      while (isButtonPressed());
      return;
    }
    delay(6);
  }
  lcd.clear();
  //Imprimo todas las alarmas con un cuadrado vacio abajo.
  for (int i = 0; i < max_alarmas; i++)
  {
    lcd.setCursor(i, 0);
    lcd.print(i + 1);
    lcd.setCursor(i, 1);
    lcd.write(byte(1));
  }
  //Ahora le hago highlight a la alarma seleccionada.
  while (1) {
    //Si la flecha derecha se presiona, se mueve un cuadrado a la derecha.
    if (digitalRead(BOTON_RIGHT) == LOW) {
      while (digitalRead(BOTON_RIGHT) == LOW);
      alarmaSeleccionada++;
    }
    //idem a la izquierda.
    if (digitalRead(BOTON_LEFT) == LOW) {
      while (digitalRead(BOTON_LEFT) == LOW);
      alarmaSeleccionada--;
    }
    if (digitalRead(BOTON_EMERGENCIA) == LOW) {
      bEmergencia();
      return;
    }
    //Si se sobrepasan los umbrales, se da la vuelta.
    if (alarmaSeleccionada > max_alarmas - 1) alarmaSeleccionada = 0;
    if (alarmaSeleccionada < 0) alarmaSeleccionada = max_alarmas - 1;
    //Pongo un cuadrado lleno donde deberia ir.
    for (int i = 0; i < max_alarmas; i++) {
      lcd.setCursor(i, 1);
      if (i == alarmaSeleccionada) continue;
      lcd.write(byte(1));
    }
    lcd.setCursor(alarmaSeleccionada, 1);
    lcd.write(byte(2));
    //Si apreta ENTER, selecciona la alarma, + y - no deberian de hacer nada.
    if (digitalRead(BOTON_ENTER) == LOW) {
      //se selecciono la alarma
      alarmas[alarmaSeleccionada].hora = 0;
      alarmas[alarmaSeleccionada].minuto = 0;
      alarmas[alarmaSeleccionada].segundo = 0;
      for (int i = 0; i < 7; i++) {
        alarmas[alarmaSeleccionada].dias[i] = 0;
      }
      lcd.clear();
      lcd.setCursor(0, 0);
      overwriteEEPROM(); //En vez de que sea temporal, se guarda permanentemente.
      lcd.print("Alarma Borrada");
      for(int i=0;i<100;i++){
      if (digitalRead(BOTON_EMERGENCIA) == LOW) {
        bEmergencia();
        return;
      }
      if (isButtonPressed()){
        while (isButtonPressed());
        return;
      }
        delay(6);
      }
      return;
    }
  }
}
void ingresarAlarmas(void) {
  int alarmaSeleccionada{};
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ingresar alarmas");
  for (int i = 0; i < 125; i++)
  {
    if (isButtonPressed()) {
      while (isButtonPressed());
      if (digitalRead(BOTON_EMERGENCIA) == LOW) {
        bEmergencia();
        return;
      }
      return;
    }
    delay(6);
  }
  lcd.clear();
  //Imprimo todas las alarmas con un cuadrado vacio abajo.
  for (int i = 0; i < max_alarmas; i++)
  {
    lcd.setCursor(i, 0);
    lcd.print(i + 1);
    lcd.setCursor(i, 1);
    lcd.write(byte(1));
  }
  //Ahora le hago highlight a la alarma seleccionada.
  while (1) {
    //Si la flecha derecha se presiona, se mueve un cuadrado a la derecha.
    if (digitalRead(BOTON_RIGHT) == LOW) {
      while (digitalRead(BOTON_RIGHT) == LOW);
      alarmaSeleccionada++;
    }
    //idem a la izquierda.
    if (digitalRead(BOTON_LEFT) == LOW) {
      while (digitalRead(BOTON_LEFT) == LOW);
      alarmaSeleccionada--;
    }
    if (digitalRead(BOTON_EMERGENCIA) == LOW) {
      bEmergencia();
      return;
    }
    //Si se sobrepasan los umbrales, se da la vuelta.
    if (alarmaSeleccionada > max_alarmas - 1) alarmaSeleccionada = 0;
    if (alarmaSeleccionada < 0) alarmaSeleccionada = max_alarmas - 1;
    for (int i = 0; i < max_alarmas; i++) {
      lcd.setCursor(i, 1);
      if (i == alarmaSeleccionada) continue;
      lcd.write(byte(1));
    }
    lcd.setCursor(alarmaSeleccionada, 1);
    lcd.write(byte(2));
    //Si apreta ENTER, selecciona la alarma, + y - no deberian de hacer nada.
    if (digitalRead(BOTON_ENTER) == LOW) {
      //se selecciono la alarma
      actualizarAlarma(alarmaSeleccionada);
      return;
    }
    if (digitalRead(BOTON_MENU) == LOW) {
      while (digitalRead(BOTON_MENU) == LOW);
      return;
    }
  }
  if (digitalRead(BOTON_EMERGENCIA) == LOW) {
    bEmergencia();
    return;
  }
  return;
}
void actualizarAlarma(int alarma) {
  //Para saber la posicion de la EEPROM en la cual arrancar... se sabe que son 10 espacios lo que ocupa cada espacio..se arranca en alarma*10.
  int hora = alarmas[alarma].hora;
  int minuto = alarmas[alarma].minuto;
  int segundo = alarmas[alarma].segundo;
  int *punteroHora = &hora, *punteroMinuto = &minuto, *punteroSegundo = &segundo;
  char *punteroDia = alarmas[alarma].dias;
  char diasSeleccionados[7];
  int parSeleccionado {};
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(alarma + 1);
  lcd.setCursor(0, 1);
  if (hora < 10) {
    lcd.print("0");
    lcd.print(hora);
  } else lcd.print(hora);
  lcd.print(":");
  if (minuto < 10) {
    lcd.print("0");
    lcd.print(minuto);
  } else lcd.print(minuto);
  lcd.print(":");
  if (segundo < 10) {
    lcd.print("0");
    lcd.print(segundo);
  } else lcd.print(segundo);
  //Se imprimio la hora guardada, ahora tiene que titilar en el primer parametro para modificar (se modifican los valores copia, para evitar errores)...
  while (1) {
    if (parSeleccionado == 0) titilar(parSeleccionado, punteroHora);
    if (parSeleccionado == 1) titilar(parSeleccionado, punteroMinuto);
    if (parSeleccionado == 2) titilar(parSeleccionado, punteroSegundo);

    if (digitalRead(BOTON_ENTER) == LOW) {
      while (digitalRead(BOTON_ENTER) == LOW);
      break;
    }
    if (digitalRead(BOTON_MENU) == LOW) {
      while (digitalRead(BOTON_MENU) == LOW);
      return;
    }
    if (digitalRead(BOTON_EMERGENCIA) == LOW) {
      bEmergencia();
      return;
    }
    if (digitalRead(BOTON_LEFT) == LOW) {
      while (digitalRead(BOTON_LEFT) == LOW);
      parSeleccionado--;
    }
    if (digitalRead(BOTON_RIGHT) == LOW) {
      while (digitalRead(BOTON_RIGHT) == LOW);
      parSeleccionado++;
    }

    if (parSeleccionado > 2) parSeleccionado = 0;
    if (parSeleccionado < 0) parSeleccionado = 2;

  }
  //Ahora tiene que elejir los dias cuando se va a repetir la alarma.
  lcd.clear();
  delay(250);
  //Imprimo los dias con el cuadrado vacio abajo
  for (int i = 0; i < max_alarmas; i++)
  {
    lcd.setCursor(i, 0);
    lcd.print(diasDeLaSemana[i][0]);
    lcd.setCursor(i, 1);
    lcd.write(byte(1));
  }
  //Se actualiza el struct con los valores copia
  alarmas[alarma].hora = hora;
  alarmas[alarma].minuto = minuto;
  alarmas[alarma].segundo = segundo;

  parSeleccionado = 0;
  while (1) { //punteroDia = alarmas[alarma].dia[0], si hago punteroDia++; paso a dia[1]
  /* 
  idea:
    hago que la func titilarCuadrado solo cambie el estado y que una vez que se lea el enter (o no), haga el rellenado de cuadrados..
  */
    titilarCuadrado(parSeleccionado, punteroDia);
    if (digitalRead(BOTON_LEFT) == LOW) {
      while (digitalRead(BOTON_LEFT) == LOW);
      parSeleccionado--;
    }
    if (digitalRead(BOTON_RIGHT) == LOW) {
      while (digitalRead(BOTON_RIGHT) == LOW);
      parSeleccionado++;
    }
    if (digitalRead(BOTON_ADD) == LOW) break;
    if (digitalRead(BOTON_EMERGENCIA) == LOW) {
      bEmergencia();
      return;
    }
  }
  //Los dias se actualizaron junto con el puntero, asi que no hay necesidad de pasar los valores.
  //Por ultimo se tiene que actualizar la EEPROM.
  overwriteEEPROM();
  return;
}
void titilarCuadrado(int parametro, char* punteroDia) {
  lcd.setCursor(parametro, 1);
  if (*punteroDia == 0) {
    lcd.write(byte(2));
    for (int i = 0; i < 25; i++) { //450ms
      if (isButtonPressed()){
        if(*punteroDia) lcd.write(byte(2));
        else lcd.write(byte(1));
        return;
      }
      if (digitalRead(BOTON_ENTER) == LOW) {
        while(digitalRead(BOTON_ENTER)==LOW);
        *punteroDia = !(*punteroDia); //nego el estado anterior (marco el cuadrado)
      }
      delay(18);
    }
    lcd.write(byte(1));
    if(*punteroDia) lcd.write(byte(2));
  }
  else if (*punteroDia == 1) {
    lcd.write(byte(1));
    for (int i = 0; i < 25; i++) { //450ms
      if (isButtonPressed()){
        if(*punteroDia) lcd.write(byte(2));
        else lcd.write(byte(1));
        return;
      }
      if (digitalRead(BOTON_ENTER) == LOW) {
        while(digitalRead(BOTON_ENTER)==LOW);
        *punteroDia = !(*punteroDia); //nego el estado anterior (marco el cuadrado)
      }
      delay(18);
    }
    lcd.write(byte(2));
    if(!*punteroDia) lcd.write(byte(1));
  }
}

void titilar(int parametro, int* contenido) {
  if (parametro == 0) { //Hora
    lcd.setCursor(0, 1);
    lcd.write(byte(2));
    lcd.setCursor(1, 1);
    lcd.write(byte(2));
    for (int i = 0; i < 25; i++) { //Para aumentar la hora (450ms)
      if (digitalRead(BOTON_UP) == LOW) {
        while (digitalRead(BOTON_UP) == LOW);
        *contenido+=1;
      }
      if (digitalRead(BOTON_DOWN) == LOW) {
        while (digitalRead(BOTON_DOWN) == LOW);
        *contenido-=1;
      }
      if (*contenido < 0) *contenido = 23;
      if (*contenido > 23) *contenido = 0;
      if (digitalRead(BOTON_EMERGENCIA) == LOW) {
        bEmergencia();
        return;
      }
      if (isButtonPressed()) return;
      delay(18);
    }
    lcd.setCursor(0, 1); //Para volver a escribir la hora.
    for (int i = 0; i < 25; i++) { //Para aumentar la hora (450ms)
      if (*contenido < 10) {
      lcd.print("0");
      lcd.print(*contenido);
    } else lcd.print(*contenido);
      if (digitalRead(BOTON_UP) == LOW) {
        while (digitalRead(BOTON_UP) == LOW);
        *contenido++;
      }
      if (digitalRead(BOTON_DOWN) == LOW) {
        while (digitalRead(BOTON_DOWN) == LOW);
        *contenido--;
      }
      if (*contenido < 0) *contenido = 23;
      if (*contenido > 23) *contenido = 0;
      if (digitalRead(BOTON_EMERGENCIA) == LOW) {
        bEmergencia();
        return;
      }
      if(isButtonPressed()) return;
      delay(18);
    }
  }
  if (parametro == 1) { //Minutos
    lcd.setCursor(3, 1);
    lcd.write(byte(2));
    lcd.setCursor(4, 1);
    lcd.write(byte(2));
    for (int i = 0; i < 25; i++) { //Para aumentar los minutos (450ms)
      if (digitalRead(BOTON_UP) == LOW) {
        while (digitalRead(BOTON_UP) == LOW);
        *contenido+=1;
      }
      if (digitalRead(BOTON_DOWN) == LOW) {
        while (digitalRead(BOTON_DOWN) == LOW);
        *contenido-=1;
      }
      if (*contenido < 0) *contenido = 23;
      if (*contenido > 23) *contenido = 0;
      if (digitalRead(BOTON_EMERGENCIA) == LOW) {
        bEmergencia();
        return;
      }
      if (isButtonPressed()) return;
      delay(18);
    }
    lcd.setCursor(3, 1); //Para volver a escribir los minutos.
    for (int i = 0; i < 25; i++) { //Para aumentar los minutos (450ms)
      if (*contenido < 10) {
      lcd.print("0");
      lcd.print(*contenido);
    } else lcd.print(*contenido);
      if (digitalRead(BOTON_UP) == LOW) {
        while (digitalRead(BOTON_UP) == LOW);
        *contenido++;
      }
      if (digitalRead(BOTON_DOWN) == LOW) {
        while (digitalRead(BOTON_DOWN) == LOW);
        *contenido--;
      }
      if (*contenido < 0) *contenido = 23;
      if (*contenido > 23) *contenido = 0;
      if (digitalRead(BOTON_EMERGENCIA) == LOW) {
        bEmergencia();
        return;
      }
      if(isButtonPressed()) return;
      delay(18);
    }
  }
  if (parametro == 2) { //Segundos
    lcd.setCursor(6, 1);
    lcd.write(byte(2));
    lcd.setCursor(7, 1);
    lcd.write(byte(2));
    for (int i = 0; i < 25; i++) { //Para aumentar los segundos (450ms)
      if (digitalRead(BOTON_UP) == LOW) {
        while (digitalRead(BOTON_UP) == LOW);
        *contenido+=1;
      }
      if (digitalRead(BOTON_DOWN) == LOW) {
        while (digitalRead(BOTON_DOWN) == LOW);
        *contenido-=1;
      }
      if (*contenido < 0) *contenido = 23;
      if (*contenido > 23) *contenido = 0;
      if (digitalRead(BOTON_EMERGENCIA) == LOW) {
        bEmergencia();
        return;
      }
      if (isButtonPressed()) return;
      delay(18);
    }
    lcd.setCursor(6, 1); //Para volver a escribir los segundos.
    for (int i = 0; i < 25; i++) { //Para aumentar los segundos (450ms)
      if (*contenido < 10) {
      lcd.print("0");
      lcd.print(*contenido);
    } else lcd.print(*contenido);
      if (digitalRead(BOTON_UP) == LOW) {
        while (digitalRead(BOTON_UP) == LOW);
        *contenido+=5;
      }
      if (digitalRead(BOTON_DOWN) == LOW) {
        while (digitalRead(BOTON_DOWN) == LOW);
        *contenido-=5;
      }
      if (*contenido < 0) *contenido = 55;
      if (*contenido > 55) *contenido = 0;
      if (digitalRead(BOTON_EMERGENCIA) == LOW) {
        bEmergencia();
        return;
      }
      if(isButtonPressed()) return;
      delay(18);
    }
  }
  return;
}
void bEmergencia(void) {
  if (digitalRead(BOTON_EMERGENCIA) == LOW) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sonando...");
    while (digitalRead(BOTON_EMERGENCIA) == LOW) {
      digitalWrite(ALARMA, HIGH);
    }
    delay(500);
    digitalWrite(ALARMA, LOW);
    lcd.clear();
  }
  return;
}