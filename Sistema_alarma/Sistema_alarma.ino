#include <LiquidCrystal.h>
#include <Keypad.h>
#include <SevSeg.h>
#include <SPI.h>
#include <MFRC522.h>
#include "pins.h"
#include "EstadoDesarmado.h"
#include "armado.h"

// ===================== Instancias Globales =====================
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

const byte FILAS = 4;
const byte COLUMNAS = 4;
char teclas[FILAS][COLUMNAS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'#', '0', '*', 'D'}
};

byte pinFilas[FILAS] = {KEYPAD_ROW_1, KEYPAD_ROW_2, KEYPAD_ROW_3, KEYPAD_ROW_4};
byte pinColumnas[COLUMNAS] = {KEYPAD_COL_1, KEYPAD_COL_2, KEYPAD_COL_3, KEYPAD_COL_4};
Keypad teclado = Keypad(makeKeymap(teclas), pinFilas, pinColumnas, FILAS, COLUMNAS);

EstadoDesarmado estadoDesarmado;
Armado estadoArmado;

// ===================== Variables Globales =====================
enum EstadoSistema {
  ESTADO_DESARMADO,
  ESTADO_ARMADO
};

EstadoSistema estadoActual = ESTADO_DESARMADO;
unsigned int tiempoCronometro = 30;
unsigned int tiempoAlarma = 30;

// ===================== Setup =====================
void setup() {
  Serial.begin(9600);
  Serial.println("=== SISTEMA DE ALARMA INICIANDO ===");
  
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Sistema Iniciando");
  
  pinMode(POT_1, INPUT);
  pinMode(POT_2, INPUT);
  pinMode(SWITCH_1, INPUT);
  pinMode(SWITCH_2, INPUT);
  pinMode(SWITCH_3, INPUT);
  pinMode(SWITCH_4, INPUT);
  pinMode(SWITCH_5, INPUT);
  pinMode(SWITCH_6, INPUT);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  
  pinMode(DISPLAY_1_EN, OUTPUT);
  pinMode(DISPLAY_2_EN, OUTPUT);
  pinMode(DATA_BIT_0, OUTPUT);
  pinMode(DATA_BIT_1, OUTPUT);
  pinMode(DATA_BIT_2, OUTPUT);
  pinMode(DATA_BIT_3, OUTPUT);
  
  for (int i = 0; i < 8; i++) {
    int pinLED = 2 + i; 
    pinMode(pinLED, OUTPUT);
    digitalWrite(pinLED, LOW);
  }
  
  estadoActual = ESTADO_DESARMADO;
  // estadoDesarmado.enter(); // Descomentar cuando la clase EstadoDesarmado esté lista
  
  Serial.println("Sistema inicializado");
}

// ===================== Loop Principal =====================
void loop() {
  char teclaPresionada = teclado.getKey();
  bool botonPanicoPresionado = (digitalRead(BUTTON_1) == LOW); 

  // Pánico: Ignora estados y dispara. Solo se activa si la alarma no está ya disparada.
  if (botonPanicoPresionado && !estadoArmado.isAlarmaDisparada()) {
      Serial.println("¡PÁNICO ACTIVADO!");
      estadoActual = ESTADO_ARMADO; 
      estadoArmado.triggerPanico(); 
  }

  // Máquina de Estados
  switch (estadoActual) {
      
    case ESTADO_DESARMADO:
      if (teclaPresionada) {
        // Delega la tecla a la clase Desarmado
        estadoDesarmado.procesarTecla(teclaPresionada);
      }
      estadoDesarmado.update();
      
      if (estadoDesarmado.debeTransicionarArmado()) {
        Serial.println("=== TRANSICIÓN A ESTADO ARMADO ===");
        // estadoDesarmado.exit(); // Descomentar cuando exista
        estadoActual = ESTADO_ARMADO;
        estadoArmado.activarArmado(); 
      }
      break;

    case ESTADO_ARMADO:
      // El motor de vigilancia de sensores
      estadoArmado.update();

      // Transición para volver a DESARMADO (Tu equipo debe inyectar su validación aquí)
      if (teclaPresionada) {
        // NOTA PARA TU EQUIPO: Reemplacen esta lógica falsa con la validación de PIN real
        bool pinCorrecto = false; 
        
        // Simulación: Si presionan '#' asumimos que el PIN fue validado
        if (teclaPresionada == '#') {
            pinCorrecto = true;
        }

        if (pinCorrecto) {
            Serial.println("PIN CORRECTO. DESARMANDO SISTEMA...");
            estadoArmado.desactivarArmado(); 
            estadoActual = ESTADO_DESARMADO;
            // estadoDesarmado.enter(); // Descomentar para reiniciar el LCD
        }
      }
      break;
  }
}