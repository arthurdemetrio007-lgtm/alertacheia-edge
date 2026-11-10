#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "display.h"

static LiquidCrystal_I2C lcd(LCD_ENDERECO, LCD_COLUNAS, LCD_LINHAS);

// Escreve a linha inteira com espaços no fim, para não sobrar caractere antigo
static void escrever_linha(uint8_t linha, const char* texto)
{
  char buf[LCD_COLUNAS + 1];
  snprintf(buf, sizeof(buf), "%-20s", texto);
  lcd.setCursor(0, linha);
  lcd.print(buf);
}

void init_display(void)
{
  lcd.init();
  lcd.backlight();
  escrever_linha(0, "AlertaCheia");
  escrever_linha(1, "Monitor de nivel");
  escrever_linha(2, "Iniciando...");
  escrever_linha(3, "");
}

void atualizar_display(float nivel_cm, bool sensor_ok, EstadoNivel estado,
                       bool comporta_fechada, ModoComporta modo,
                       bool wifi_ok, bool mqtt_ok)
{
  char texto[LCD_COLUNAS + 1];

  if (sensor_ok) {
    snprintf(texto, sizeof(texto), "Nivel: %.1f cm", nivel_cm);
  } else {
    snprintf(texto, sizeof(texto), "Nivel: %.1f cm (!)", nivel_cm);
  }
  escrever_linha(0, texto);

  snprintf(texto, sizeof(texto), "Estado: %s", nome_estado(estado));
  escrever_linha(1, texto);

  snprintf(texto, sizeof(texto), "Comp: %s (%s)",
           comporta_fechada ? "FECHADA" : "ABERTA",
           modo == MODO_AUTO ? "AUTO" : "REM");
  escrever_linha(2, texto);

  snprintf(texto, sizeof(texto), "WiFi:%-3s  MQTT:%-3s",
           wifi_ok ? "OK" : "OFF",
           mqtt_ok ? "OK" : "OFF");
  escrever_linha(3, texto);
}
