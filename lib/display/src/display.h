#ifndef DISPLAY_H
#define DISPLAY_H

#include "regras.h"

void init_display(void);

// Tela local (LCD 20x4):
//   linha 0: nível atual
//   linha 1: estado (NORMAL / ALERTA / INUNDACAO)
//   linha 2: comporta e modo (AUTO ou REM = remoto)
//   linha 3: status WiFi e MQTT
void atualizar_display(const LeituraBorda& leitura, bool wifi_ok, bool mqtt_ok);

#endif
