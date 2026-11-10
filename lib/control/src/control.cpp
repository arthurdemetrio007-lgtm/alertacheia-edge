#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"
#include "control.h"

static Servo comporta;

static EstadoNivel estado = NORMAL;
static ModoComporta modo = MODO_AUTO;
static bool fechada = false;
static float alerta_cm = LIMIAR_ALERTA_PADRAO_CM;
static float inundacao_cm = LIMIAR_INUNDACAO_PADRAO_CM;
static float ultimo_nivel = 0;

// Aplica estado e modo nas saídas físicas
static void aplicar_saidas(void)
{
  digitalWrite(PIN_LED_VERDE,    estado == NORMAL    ? HIGH : LOW);
  digitalWrite(PIN_LED_AMARELO,  estado == ALERTA    ? HIGH : LOW);
  digitalWrite(PIN_LED_VERMELHO, estado == INUNDACAO ? HIGH : LOW);

  bool deve_fechar = decidir_comporta(estado, modo);

  // Avisa só quando a trava entra em ação, não a cada amostra
  static bool trava_ativa = false;
  bool trava = (modo == MODO_FORCAR_ABRIR && deve_fechar);
  if (trava && !trava_ativa) {
    Serial.println("Trava de seguranca: comando de abrir ignorado durante inundacao");
  }
  trava_ativa = trava;

  if (deve_fechar != fechada) {
    fechada = deve_fechar;
    comporta.write(fechada ? SERVO_FECHADA_GRAUS : SERVO_ABERTA_GRAUS);
    Serial.print("Comporta ");
    Serial.println(fechada ? "FECHADA" : "ABERTA");
  }
}

void init_control(void)
{
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_AMARELO, OUTPUT);
  pinMode(PIN_LED_VERMELHO, OUTPUT);

  comporta.setPeriodHertz(50);
  comporta.attach(PIN_SERVO, 500, 2400);
  comporta.write(SERVO_ABERTA_GRAUS);

  aplicar_saidas();
}

void processar_nivel(float nivel_cm)
{
  ultimo_nivel = nivel_cm;
  EstadoNivel novo = proximo_estado(estado, nivel_cm, alerta_cm, inundacao_cm, HISTERESE_CM);

  if (novo != estado) {
    Serial.print("Estado: ");
    Serial.print(nome_estado(estado));
    Serial.print(" -> ");
    Serial.println(nome_estado(novo));
    estado = novo;
  }
  aplicar_saidas();
}

bool definir_limiar_alerta(float valor)
{
  if (!limiares_validos(valor, inundacao_cm, ALTURA_SENSOR_CM)) {
    Serial.print("Limiar de alerta rejeitado: ");
    Serial.println(valor);
    return false;
  }
  alerta_cm = valor;
  Serial.print("Novo limiar de alerta: ");
  Serial.println(alerta_cm);
  processar_nivel(ultimo_nivel); // reavalia na hora, sem esperar a próxima amostra
  return true;
}

bool definir_limiar_inundacao(float valor)
{
  if (!limiares_validos(alerta_cm, valor, ALTURA_SENSOR_CM)) {
    Serial.print("Limiar de inundacao rejeitado: ");
    Serial.println(valor);
    return false;
  }
  inundacao_cm = valor;
  Serial.print("Novo limiar de inundacao: ");
  Serial.println(inundacao_cm);
  processar_nivel(ultimo_nivel);
  return true;
}

bool definir_modo(int novo_modo)
{
  if (novo_modo < MODO_AUTO || novo_modo > MODO_FORCAR_ABRIR) {
    Serial.print("Modo rejeitado: ");
    Serial.println(novo_modo);
    return false;
  }
  modo = (ModoComporta)novo_modo;
  Serial.print("Modo da comporta: ");
  Serial.println(modo == MODO_AUTO ? "AUTO" : (modo == MODO_FORCAR_FECHAR ? "FORCAR FECHAR" : "FORCAR ABRIR"));
  aplicar_saidas();
  return true;
}

EstadoNivel estado_atual(void)  { return estado; }
ModoComporta modo_atual(void)   { return modo; }
bool comporta_fechada(void)     { return fechada; }
float limiar_alerta(void)       { return alerta_cm; }
float limiar_inundacao(void)    { return inundacao_cm; }
