#ifndef CONTROL_H
#define CONTROL_H

#include "regras.h"

void init_control(void);

// Avalia o nível medido e atualiza LEDs e comporta. Chamado a cada amostra.
void processar_nivel(float nivel_cm);

// Configuração remota (vinda do MQTT). Retornam false se o valor foi rejeitado.
bool definir_limiar_alerta(float valor);
bool definir_limiar_inundacao(float valor);
bool definir_modo(int modo);

EstadoNivel estado_atual(void);
ModoComporta modo_atual(void);
bool comporta_fechada(void);
float limiar_alerta(void);
float limiar_inundacao(void);

#endif
