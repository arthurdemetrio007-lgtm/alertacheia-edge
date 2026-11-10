#ifndef MQTT_H
#define MQTT_H

#include "regras.h"

// Inicia WiFi e MQTT sem travar o loop.
void init_rede(void);

// Chamar a todo ciclo do loop: reconecta WiFi/MQTT sem bloquear e processa mensagens.
void rede_loop(void);

bool wifi_ok(void);
bool mqtt_ok(void);

// Publica nível, estado e comporta no canal (fields 1, 2 e 3).
bool publicar_leitura(float nivel_cm, EstadoNivel estado, bool comporta_fechada);

// Botão de demonstração: alterna NORMAL -> sem MQTT -> sem WiFi -> NORMAL.
void alternar_falha_simulada(void);

#endif
