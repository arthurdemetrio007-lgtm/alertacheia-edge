#ifndef DISPLAY_H
#define DISPLAY_H

#include "regras.h"

void init_display(void);

// Tela local (LCD 20x4):
//   linha 0: nível atual
//   linha 1: estado (NORMAL / ALERTA / INUNDACAO)
//   linha 2: comporta e modo (AUTO ou REM = remoto)
//   linha 3: status WiFi e MQTT
void atualizar_display(float nivel_cm, bool sensor_ok, EstadoNivel estado,
                       bool comporta_fechada, ModoComporta modo,
                       bool wifi_ok, bool mqtt_ok);

#endif
