#ifndef PINS_H
#define PINS_H

// ==================== LCD 16x2 (Modo Paralelo 8-bit) ====================
#define LCD_RS          22  // Register Select
#define LCD_EN          23  // Enable
#define LCD_D0          24  // Data bit 0
#define LCD_D1          25  // Data bit 1
#define LCD_D2          26  // Data bit 2
#define LCD_D3          27  // Data bit 3
#define LCD_D4          28  // Data bit 4
#define LCD_D5          29  // Data bit 5
#define LCD_D6          30  // Data bit 6
#define LCD_D7          31  // Data bit 7
// RW está conectado a tierra (escritura solamente)

// ==================== Displays de 7 Segmentos ====================
#define DISPLAY_1_EN    32  // Enable Display 1
#define DISPLAY_2_EN    33  // Enable Display 2
#define DATA_BIT_0      34  // Bit 0 para dato binario
#define DATA_BIT_1      35  // Bit 1 para dato binario
#define DATA_BIT_2      36  // Bit 2 para dato binario
#define DATA_BIT_3      37  // Bit 3 para dato binario

// ==================== Teclado Matricial 4x4 ====================
#define KEYPAD_ROW_1    38  // Fila 1
#define KEYPAD_ROW_2    39  // Fila 2
#define KEYPAD_ROW_3    40  // Fila 3
#define KEYPAD_ROW_4    41  // Fila 4
#define KEYPAD_COL_1    42  // Columna 1
#define KEYPAD_COL_2    43  // Columna 2
#define KEYPAD_COL_3    44  // Columna 3
#define KEYPAD_COL_4    45  // Columna 4

// ==================== Botones ====================
#define BUTTON_1        46  // Botón 1
#define BUTTON_2        47  // Botón 2

// ==================== Entradas Analógicas ====================
#define POT_1           A0  // Potenciómetro 1
#define POT_2           A1  // Potenciómetro 2
#define SWITCH_1        A2  // Switch 1
#define SWITCH_2        A3  // Switch 2
#define SWITCH_3        A4  // Switch 3
#define SWITCH_4        A5  // Switch 4
#define SWITCH_5        A6  // Switch 5
#define SWITCH_6        A7  // Switch 6
#define RELAY_PIN       A8  // Relay

// ==================== LEDs (Pines 2-9) ====================
#define LED_1           2   // LED 1
#define LED_2           3   // LED 2
#define LED_3           4   // LED 3
#define LED_4           5   // LED 4
#define LED_5           6   // LED 5
#define LED_6           7   // LED 6
#define LED_7           8   // LED 7
#define LED_8           9   // LED 8

#endif // PINS_H
