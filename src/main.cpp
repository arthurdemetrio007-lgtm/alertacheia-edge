#include <Arduino.h>
#include "config.h"
#include "sensors.h"
#include "control.h"
#include "display.h"
#include "mqtt.h"

// Agenda de tarefas por millis(), sem delay no loop
static unsigned long proxima_amostra = 0;
static unsigned long ultima_publicacao = 0;
static unsigned long ultimo_display = 0;

static float nivel_atual = 0;

// Botão com debounce por polling (INPUT_PULLUP: solto = HIGH)
static int estado_botao = HIGH;
static int leitura_anterior = HIGH;
static unsigned long instante_mudanca = 0;

static void verificar_botao(unsigned long agora)
{
  int leitura = digitalRead(PIN_BOTAO_REDE);

  if (leitura != leitura_anterior) {
    instante_mudanca = agora;
    leitura_anterior = leitura;
  }

  if (agora - instante_mudanca >= DEBOUNCE_BOTAO_MS && leitura != estado_botao) {
    estado_botao = leitura;
    if (estado_botao == LOW) {
      alternar_falha_simulada();
    }
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(PIN_BOTAO_REDE, INPUT_PULLUP);

  init_display();
  init_control();
  init_sensors();
  init_rede();

  unsigned long agora = millis();
  proxima_amostra = agora;
  ultima_publicacao = agora;
  ultimo_display = agora;
}

void loop()
{
  unsigned long agora = millis();

  verificar_botao(agora);

  // Amostragem com período fixo: o próximo instante é calculado a partir do
  // instante previsto, e não do millis() atual. Assim o atraso de um ciclo
  // não se acumula e o intervalo entre amostras fica constante.
  if ((long)(agora - proxima_amostra) >= 0) {
    if (agora - proxima_amostra > PERIODO_AMOSTRAGEM_MS) {
      // Loop ficou travado (ex.: reconexão). Realinha em vez de disparar várias amostras seguidas.
      Serial.println("Amostragem atrasada, realinhando");
      proxima_amostra = agora;
    }
    proxima_amostra += PERIODO_AMOSTRAGEM_MS;

    nivel_atual = ler_nivel_cm();
    processar_nivel(nivel_atual); // decisão na borda: LEDs e comporta
  }

  if (agora - ultima_publicacao >= PERIODO_PUBLICACAO_MS) {
    ultima_publicacao = agora;
    publicar_leitura(nivel_atual, estado_atual(), comporta_fechada());
  }

  if (agora - ultimo_display >= PERIODO_DISPLAY_MS) {
    ultimo_display = agora;
    atualizar_display(nivel_atual, sensor_ok(), estado_atual(),
                      comporta_fechada(), modo_atual(),
                      wifi_ok(), mqtt_ok());
  }

  rede_loop();
}
