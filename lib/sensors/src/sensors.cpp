#include <Arduino.h>
#include "config.h"
#include "sensors.h"

// Buffer circular da média móvel
static float amostras[TAMANHO_MEDIA];
static uint8_t indice = 0;
static uint8_t preenchidas = 0;
static float ultima_media = 0.0f;
static bool ultima_leitura_ok = false;

// Mesmo procedimento da aula do ultrassônico: pulso de 10 us no trigger
static void trigger_hc(void)
{
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(5);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
}

static float tempo_para_distancia(unsigned long tempo_us)
{
  return (float)tempo_us / 58.0f;
}

void init_sensors(void)
{
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);
}

float ler_distancia_cm(void)
{
  trigger_hc();
  // Timeout de 30 ms (~5 m). Sem timeout, o pulseIn pode travar o loop por 1 s.
  unsigned long tempo = pulseIn(PIN_ECHO, HIGH, 30000UL);
  if (tempo == 0) {
    return -1.0f;
  }
  return tempo_para_distancia(tempo);
}

float ler_nivel_cm(void)
{
  float distancia = ler_distancia_cm();

  if (distancia < 0) {
    // Sem eco: mantém a última média para não gerar alerta falso
    ultima_leitura_ok = false;
    Serial.println("Sensor sem eco, mantendo ultima media");
    return ultima_media;
  }
  ultima_leitura_ok = true;

  float nivel = ALTURA_SENSOR_CM - distancia;
  if (nivel < 0) nivel = 0;
  if (nivel > ALTURA_SENSOR_CM) nivel = ALTURA_SENSOR_CM;

  amostras[indice] = nivel;
  indice = (indice + 1) % TAMANHO_MEDIA;
  if (preenchidas < TAMANHO_MEDIA) preenchidas++;

  float soma = 0;
  for (uint8_t i = 0; i < preenchidas; i++) {
    soma += amostras[i];
  }
  ultima_media = soma / preenchidas;
  return ultima_media;
}

bool sensor_ok(void)
{
  return ultima_leitura_ok;
}
