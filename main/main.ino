/*
        Alarma Sanbo
  %%%%%%%%%%%%%   Autores   %%%%%%%%%%%%%%%
  -Alan Völker
  -Federico Chara Lobo
  -Leandro Ponte Recalde
  -Ezequiel Santangelo

  Arduino NANO check
  Botones N.A x9
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
LiquidCrystal_I2C lcd(0x27, 16, 2);  //poner el address del LCD en 0x27 o 0x03
RTC_DS3231 rtc;
struct alarma {
  int hora;
  int minuto;
  int segundo;
  //porque no bool? porque no se puede guardar un bool en EEPROM.
  char dias[7];  //manejo manual de los dias a repetir.
  char activada;
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
byte flechas_updown[8] = {
  B00100,
  B01110,
  B11111,
  B00000,
  B00000,
  B11111,
  B01110,
  B00100,
};

//Funciones.
void mostrarAlarmas(void);
void ingresarAlarmas(void);
void clearEEPROM(void);           //Adentro para hacer debugging
void overwriteEEPROM(void);       //En caso de que se actualize el struct alarma alarmas.
bool isButtonPressed(void);
void activacion(void);            //Para activar cada una de las alarmas independientemente.
void horaRTC(void);               //En caso de que se quiera cambiar la hora del RTC.
void subeBaja(String, int, int, int*);
void actualizarAlarma(int);       //Luego de seleccionar la alarma
void borrarAlarma(void);
void titilarCuadrado(int, int);   //cuando cambio los dias
void tocarTimbre(void);           //Cuando se cumple la funcion de abajo
int checkHorario(DateTime);       //Revisa si el horario guardado es igual al del RTC.
void bEmergencia(void);           //Hace un override de todo y toca el timbre hasta que suelta. (vuelve al menu principal).


void setup() {
  Serial.begin(9600);
  if (!rtc.begin()) {
    //Serial.println("Couldn't find RTC");
    //Serial.flush(); sin esta linea, aparentemente tira basura rtc.now()
    while (1) delay(10);
  }
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //Una vez
  //Los botones.
  pinMode(LED, OUTPUT);
  pinMode(ALARMA, OUTPUT);
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
  lcd.init();
  lcd.backlight();
  digitalWrite(LED, HIGH);
  lcd.createChar(0, backslash);
  lcd.createChar(1, cuadrado_vacio);
  lcd.createChar(2, cuadrado_lleno);
  lcd.createChar(3, flechas_updown);
  //  clearEEPROM();
  int cprom{};  //IMPORTANTE: Para llevar la cuenta de la memoria EEPROM
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
    alarmas[i].activada = EEPROM.read(cprom);
    cprom++;
  }
}
char diasDeLaSemana[7][2] = { "D", "L", "m", "M", "J", "V", "S" };
char animacion[8][2] = { "/", "-", "\0", "|", "/", "-", "\0", "|" };
int reloj{};
void loop() {
  //DEBE QUEDARSE DENTRO DE VOID LOOP. ----|
  DateTime now = rtc.now();  //  < ---------|
  //Menu inicial
  lcd.setCursor(0, 0);  // columna, fila
  lcd.print("  Timbre Sanbo");
  lcd.setCursor(0, 1);
  if (now.hour() < 10) {
    lcd.print("0");
    lcd.print(now.hour());
  } else
    lcd.print(now.hour());
  lcd.print(':');
  if (now.minute() < 10) {
    lcd.print("0");
    lcd.print(now.minute());
  } else
    lcd.print(now.minute());
  lcd.print(':');
  if (now.second() < 10) {
    lcd.print("0");
    lcd.print(now.second());
  } else lcd.print(now.second());
  lcd.print(" ");
  lcd.print(diasDeLaSemana[now.dayOfTheWeek()][0]);
  lcd.print(" ");
  lcd.print((int) rtc.getTemperature());
  //lcd.print((char)223);
  lcd.print("C ");
  if (reloj == 2 || reloj == 6) lcd.write(byte(0));
  else
    lcd.print(animacion[reloj]);  //No funciona correctamanete en simulacion, en la realidad esta medio LOL.
  reloj++;
  if (reloj == 8) reloj = 0;
  //Comprobacion del timbre
  if (checkHorario(now)) tocarTimbre();
  //Comprobacion del resto de los botones
  for (int i = 0; i < 25; i++) {
    if (digitalRead(BOTON_MENU) == LOW) {
      while (digitalRead(BOTON_MENU) == LOW);
      mostrarAlarmas();  //Menu de alarmas
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
    delay(10);  //Para hacer que el reloj no sea el rayo mcqueen
  }
}
int checkHorario(DateTime now) {
  for (int i = 0; i < max_alarmas; i++) {
    if (now.hour() == alarmas[i].hora && now.minute() == alarmas[i].minuto && now.second() == alarmas[i].segundo) {
      for (int j = 0; j < 7; j++) {
        if (now.dayOfTheWeek() == (alarmas[i].dias[j] + (j - 1)) && alarmas[i].activada == 1) return 1;
      }
    }
  }
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
void activacion(void){
  int alarmaSeleccionada{};
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Alarmas");
  lcd.setCursor(1,1);
  lcd.print("Activadas");
  for(int i=0;i<100;i++){
    if (digitalRead(BOTON_EMERGENCIA) == LOW) {
      bEmergencia();
      return;
    }
    if(digitalRead(BOTON_ADD)== LOW || digitalRead(BOTON_REMOVE)== LOW || digitalRead(BOTON_MENU)== LOW) return;
    delay(10);
  }

  while(1){
    if (digitalRead(BOTON_EMERGENCIA) == LOW) {
      bEmergencia();
      return;
    }
    //Imprimo todas las alarmas con un cuadrado vacio abajo.
    for (int i = 0; i < max_alarmas; i++) {
    lcd.setCursor(i, 0);
    lcd.print(i + 1);
    lcd.setCursor(i, 1);
    lcd.write(byte(1));
    }
    //Imprimo los 8 cuadrados vacios arriba a la derecha.
    for(int i=0;i<max_alarmas;i++){
      lcd.setCursor(i+8,0);
      if(alarmas[i].activada) lcd.write(byte(2));
      else lcd.write(byte(1));
    }
    while (1) {
    
    if (digitalRead(BOTON_RIGHT) == LOW) {
      while (digitalRead(BOTON_RIGHT) == LOW);
      alarmaSeleccionada++;
      break;
    }
    //idem a la izquierda.
    if (digitalRead(BOTON_LEFT) == LOW) {
      while (digitalRead(BOTON_LEFT) == LOW);
      alarmaSeleccionada--;
      break;
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
    if (digitalRead(BOTON_ENTER) == LOW) {
      //se selecciono la alarma a activar/desactivar.
      while(digitalRead(BOTON_ENTER) == LOW);
      alarmas[alarmaSeleccionada].activada = !alarmas[alarmaSeleccionada].activada;

      for(int i=0;i<max_alarmas;i++){
      lcd.setCursor(i+8,0);
      if(alarmas[i].activada) lcd.write(byte(2));
      else lcd.write(byte(1));
      }
    }

    if (digitalRead(BOTON_MENU) == LOW) {
      while (digitalRead(BOTON_MENU) == LOW);
      return;
    }
  }
  }
}
//Funcion que muestra las alarmas.
void mostrarAlarmas(void) {
  while (1) {
    lcd.clear();
    for (int i = 0; i < 125; i++) {  //Corroborar cada 6ms si se pulsa un boton
      if (isButtonPressed()) {
        while (isButtonPressed());
        if (digitalRead(BOTON_EMERGENCIA) == LOW) {
          bEmergencia();
          return;
        }
        return;
      }
      delay(6);  //Para dar ilusion de...carga? 750ms (6*125)
    }

    lcd.setCursor(0, 0);
    lcd.print("Alarmas");
    for (int i = 0; i < 100; i++) {
      if (isButtonPressed()) {
        if (digitalRead(BOTON_EMERGENCIA) == LOW) {
          bEmergencia();
          return;
        }
        while (isButtonPressed());
        return;
      }
      delay(10);
    }
    lcd.clear();
    for (int j = 0; j < max_alarmas; j++) {
      if (digitalRead(BOTON_ADD) == LOW || digitalRead(BOTON_REMOVE) == LOW) return;
      if (digitalRead(BOTON_MENU) == LOW){
        activacion();
        return;
      }
      if (digitalRead(BOTON_EMERGENCIA) == LOW) {
        bEmergencia();
        return;
      }
      if(digitalRead(BOTON_ENTER)==LOW){
        while(digitalRead(BOTON_ENTER)==LOW);
        continue;
      }
      lcd.clear();
      //Mostrar alarma j.
      lcd.setCursor(0, 0);
      lcd.print(j + 1);
      lcd.setCursor(0, 1);
      if (alarmas[j].hora < 10) {
        lcd.print("0");
        lcd.print(alarmas[j].hora);
      } else
        lcd.print(alarmas[j].hora);
      lcd.print(":");
      if (alarmas[j].minuto < 10) {
        lcd.print("0");
        lcd.print(alarmas[j].minuto);
      } else
        lcd.print(alarmas[j].minuto);
      lcd.print(":");
      if (alarmas[j].segundo < 10) {
        lcd.print("0");
        lcd.print(alarmas[j].segundo);
      } else
        lcd.print(alarmas[j].segundo);
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
          return;
        }
        delay(50);
      }  //Tiempo para mostrar la alarma. 5000ms (100*50)
    }
  }
}

void clearEEPROM(void) {
  lcd.clear();
  delay(500);
  lcd.setCursor(0, 0);
  lcd.print("Reseteando alarmas...");
  for (int i = 0; i < 300; i++) EEPROM.write(i, 0);
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
    EEPROM.write(cprom, alarmas[i].activada);
    cprom++;
  }
  return;
}
void subeBaja(String texto, int min, int max, int* modificable){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(texto);
  lcd.setCursor(0, 1);
  lcd.write(byte(3));
  while (1) {
    lcd.setCursor(1, 1);
    if (*modificable < 10) {
      lcd.print("0");
      lcd.print(*modificable);
    } else lcd.print(*modificable);
    if (digitalRead(BOTON_ENTER) == LOW){
      while(digitalRead(BOTON_ENTER) == LOW);
      break;
    } 
    
    while (1) {
      if (digitalRead(BOTON_ENTER) == LOW) break;
      if (digitalRead(BOTON_MENU) == LOW || digitalRead(BOTON_EMERGENCIA) == LOW) return;
      if (digitalRead(BOTON_UP) == LOW) {
        *modificable += 1;
        if (*modificable > max) *modificable = min;
        while (digitalRead(BOTON_UP) == LOW);
        break;
      }
      if (digitalRead(BOTON_DOWN) == LOW) {
        *modificable -= 1;
        if (*modificable < min) *modificable = max;
        while (digitalRead(BOTON_DOWN) == LOW);
        break;
      }
      if (digitalRead(BOTON_LEFT) == LOW) {
        *modificable -= 5;
        if (*modificable < min) *modificable += (max-min);
        while (digitalRead(BOTON_LEFT) == LOW);
        break;
      }
      if (digitalRead(BOTON_RIGHT) == LOW) {
        *modificable += 5;
        if (*modificable >max) *modificable -= (max-min);
        while (digitalRead(BOTON_RIGHT) == LOW);
        break;
      }
    }
  }  
    return;
}
//Para cambiar la hora del RTC mediante software.
void horaRTC(void){
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Cambiar Hora");
  for(int i=0;i<100;i++){
    if(isButtonPressed()) return;
    delay(10);
  }
  DateTime now = rtc.now();
  int hora = now.hour();
  int *pHora = &hora;
  subeBaja("Hora:", 0, 23, pHora);
  if(isButtonPressed()) return;
  //Se asigno la hora
  int minuto = now.minute();
  int *pMinuto = &minuto;
  subeBaja("Minuto:",0,59,pMinuto);
  //Se asignaron los minutos.
  int segundo = now.second();
  int *pSegundo = &segundo;
  subeBaja("Segundo:", 0,59,pSegundo);
  //Se asingaron los segundos
  int ano = now.year();
  int *pAno = &ano;
  subeBaja("Año:",1971,2100,pAno);
  //Se asigno el año.
  if(isButtonPressed()) return;
  //Se asignaron todos los valores
  rtc.adjust(DateTime(now.year(),now.month(),now.day(),hora,minuto,segundo)); //Year, Month, y Day vendrian configurados de antes.
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Hora Ajustada");
  for(int i=0;i<100;i++){
    if(digitalRead(BOTON_EMERGENCIA) == LOW){
      bEmergencia();
      return;
    }
    delay(10);
  }

  return;
}
// Una funcion que devuelve 1 si se presiona uno de los botones significativos...
bool isButtonPressed(void) {
  if (digitalRead(BOTON_MENU) == LOW) return 1;
  if (digitalRead(BOTON_ADD) == LOW) return 1;
  if (digitalRead(BOTON_REMOVE) == LOW) return 1;
  if (digitalRead(BOTON_EMERGENCIA) == LOW) return 1;
  return 0;
}

void borrarAlarmas(void) {
  int alarmaSeleccionada{};
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Borrar alarma");
  for (int i = 0; i < 125; i++) {
    if (isButtonPressed()) {
      if (digitalRead(BOTON_EMERGENCIA) == LOW) {
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
  for (int i = 0; i < max_alarmas; i++) {
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
      overwriteEEPROM();  //En vez de que sea temporal, se guarda permanentemente.
      lcd.print("Alarma Borrada");
      for (int i = 0; i < 100; i++) {
        if (digitalRead(BOTON_EMERGENCIA) == LOW) {
          bEmergencia();
          return;
        }
        if (isButtonPressed()) {
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
  lcd.print("Nueva/editar");
  lcd.setCursor(3, 1);
  lcd.print("Alarma");
  for (int i = 0; i < 125; i++) {
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
  for (int i = 0; i < max_alarmas; i++) {
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
  int parSeleccionado{ 0 };
  int *pHora =  &alarmas[alarma].hora;
  int *pMinuto = &alarmas[alarma].minuto;
  int *pSegundo = &alarmas[alarma].segundo;
  subeBaja("Hora:", 0,23, pHora);
  subeBaja("Minuto:", 0,59, pMinuto);
  subeBaja("Segundo:",0,59, pSegundo);
  if (digitalRead(BOTON_MENU) == LOW) {
    while (digitalRead(BOTON_MENU) == LOW);
    return;
  }
  if (digitalRead(BOTON_EMERGENCIA) == LOW) {
    bEmergencia();
    return;
  }
  //Ahora tiene que elejir los dias cuando se va a repetir la alarma.
  lcd.clear();
  delay(250);
  //Imprimo los dias con el cuadrado vacio abajo
  for (int i = 0; i < max_alarmas - 1; i++) {
    lcd.setCursor(i, 0);
    lcd.print(diasDeLaSemana[i][0]);
    lcd.setCursor(i, 1);
    lcd.write(byte(1));
  }
  //incial
  lcd.setCursor(8, 1);
  for (int j = 0; j < 7; j++) {
    if (alarmas[alarma].dias[j] == 1) {
      lcd.print(diasDeLaSemana[j][0]);
    }
  }
  while (1) {  //punteroDia = alarmas[alarma].dia[0], si hago punteroDia++; paso a dia[1]

    titilarCuadrado(parSeleccionado, alarma);
    if (digitalRead(BOTON_LEFT) == LOW) {
      parSeleccionado -= 1;
      if (parSeleccionado < 0) parSeleccionado = 6;
      while (digitalRead(BOTON_LEFT) == LOW);
      lcd.setCursor(8, 1);
      for(int j = 0; j < 7; j++) lcd.print(" "); //vacio, y despues escribo.
      lcd.setCursor(8, 1);
      for (int j = 0; j < 7; j++) {
        if (alarmas[alarma].dias[j] == 1) {
          lcd.print(diasDeLaSemana[j][0]);
        }
      }
    } else if (digitalRead(BOTON_RIGHT) == LOW) {
      parSeleccionado += 1;
      if (parSeleccionado > 6) parSeleccionado = 0;
      while (digitalRead(BOTON_RIGHT) == LOW);
      lcd.setCursor(8, 1);
      for(int j = 0; j < 7; j++) lcd.print(" "); //vacio, y despues escribo.
      lcd.setCursor(8, 1);
      for (int j = 0; j < 7; j++) {
        if (alarmas[alarma].dias[j] == 1) {
          lcd.print(diasDeLaSemana[j][0]);
        }
      }
    }
    if (digitalRead(BOTON_ADD) == LOW) {
      while (digitalRead(BOTON_ADD) == LOW);
      break;
    }
    if (digitalRead(BOTON_EMERGENCIA) == LOW) {
      bEmergencia();
      return;
    }
  }
  //Por ultimo se tiene que actualizar la EEPROM.
  alarmas[alarma].activada = 1; //Se activa la alarma recien añadida/modificada.
  overwriteEEPROM();
  return;
}
void titilarCuadrado(int parametro, int alarma) {
  while (1) {

    lcd.setCursor(parametro, 1);
    lcd.write(byte(2));  //el dia seleccionado, por el momento

    for (int i = 0; i < 45; i++) {  //450 ms
      if (digitalRead(BOTON_ENTER) == LOW) {
        while (digitalRead(BOTON_ENTER) == LOW);
        if (alarmas[alarma].dias[parametro] == 1) alarmas[alarma].dias[parametro] = 0;
        else
          alarmas[alarma].dias[parametro] = 1;
      }
      if (digitalRead(BOTON_LEFT) == LOW) {
        lcd.setCursor(parametro, 1);
        lcd.write(byte(1));
        return;
      }
      if (digitalRead(BOTON_RIGHT) == LOW) {
        lcd.setCursor(parametro, 1);
        lcd.write(byte(1));
        return;
      }
      if (isButtonPressed()) return;
      delay(10);
    }
    lcd.setCursor(parametro, 1);
    lcd.write(byte(1));
    for (int i = 0; i < 45; i++) {  //450 ms
      if (digitalRead(BOTON_ENTER) == LOW) {
        while (digitalRead(BOTON_ENTER) == LOW);
        if (alarmas[alarma].dias[parametro] == 1) alarmas[alarma].dias[parametro] = 0;
        else
          alarmas[alarma].dias[parametro] = 1;
      }
      if (digitalRead(BOTON_LEFT) == LOW) {
        lcd.setCursor(parametro, 1);
        lcd.write(byte(1));
        return;
      }
      if (digitalRead(BOTON_RIGHT) == LOW) {
        lcd.setCursor(parametro, 1);
        lcd.write(byte(1));
        return;
      }
      if (isButtonPressed()) return;
      delay(10);
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
