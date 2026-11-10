#include <Arduino.h>
#include <esp_system.h>
#include <esp_task_wdt.h>
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

// ---------------------------------------------------------------------------
// Botão por interrupção + debounce
// A interrupção só marca que houve uma borda de descida. O loop espera
// DEBOUNCE_BOTAO_MS e confirma que o botão continua pressionado. Assim os
// repiques do contato (na pressão e na soltura) não viram cliques falsos, e
// nada de rede roda dentro da interrupção.
// ---------------------------------------------------------------------------
static volatile bool botao_pendente = false;
static volatile unsigned long instante_borda = 0;

static void IRAM_ATTR isr_botao_rede(void)
{
  if (!botao_pendente) {
    botao_pendente = true;
    instante_borda = millis();
  }
}

static void tratar_botao(unsigned long agora)
{
  if (!botao_pendente) return;
  if (agora - instante_borda < DEBOUNCE_BOTAO_MS) return;

  if (digitalRead(PIN_BOTAO_REDE) == LOW) {
    alternar_falha_simulada();
  }
  botao_pendente = false;
}

// ---------------------------------------------------------------------------
// Watchdog: se o loop travar por WATCHDOG_TIMEOUT_MS, o ESP32 reinicia sozinho.
// ---------------------------------------------------------------------------
static void init_watchdog(void)
{
  esp_task_wdt_config_t cfg = {};
  cfg.timeout_ms = WATCHDOG_TIMEOUT_MS;
  cfg.idle_core_mask = (1 << 0);  // também vigia a tarefa ociosa do núcleo 0
  cfg.trigger_panic = true;       // estourou = reinicia

  // O core Arduino já sobe o watchdog de tarefas; aqui só ajustamos o tempo.
  if (esp_task_wdt_reconfigure(&cfg) == ESP_ERR_INVALID_STATE) {
    esp_task_wdt_init(&cfg);
  }
  esp_task_wdt_add(NULL);  // inscreve a tarefa do loop
}

static void informar_motivo_reinicio(void)
{
  esp_reset_reason_t motivo = esp_reset_reason();
  if (motivo == ESP_RST_TASK_WDT || motivo == ESP_RST_INT_WDT || motivo == ESP_RST_WDT) {
    Serial.println("Reiniciado pelo watchdog: o loop tinha travado");
  }
}

static LeituraBorda montar_leitura(void)
{
  LeituraBorda leitura;
  leitura.nivel_cm = nivel_atual;
  leitura.sensor_ok = sensor_ok();
  leitura.estado = estado_atual();
  leitura.comporta_fechada = comporta_fechada();
  leitura.modo = modo_atual();
  return leitura;
}

void setup()
{
  Serial.begin(115200);
  informar_motivo_reinicio();

  pinMode(PIN_BOTAO_REDE, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_BOTAO_REDE), isr_botao_rede, FALLING);

  init_display();
  init_control();
  init_sensors();
  init_rede();
  init_watchdog();

  unsigned long agora = millis();
  proxima_amostra = agora;
  ultima_publicacao = agora;
  ultimo_display = agora;
}

void loop()
{
  unsigned long agora = millis();

  tratar_botao(agora);

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
    publicar_leitura(montar_leitura());
  }

  if (agora - ultimo_display >= PERIODO_DISPLAY_MS) {
    ultimo_display = agora;
    atualizar_display(montar_leitura(), wifi_ok(), mqtt_ok());
  }

  rede_loop();

  esp_task_wdt_reset();  // loop completou uma volta: alimenta o watchdog
}
