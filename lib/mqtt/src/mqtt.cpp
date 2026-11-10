#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include "secrets.h"
#include "mqtt.h"
#include "control.h"

// ThingSpeak MQTT
static const char* MQTT_SERVIDOR = "mqtt3.thingspeak.com";
static const int MQTT_PORTA = 1883;

static WiFiClient espClient;
static PubSubClient MQTT(espClient);

// Falha simulada pelo botão, para demonstrar a tela de desconexão no vídeo
enum FalhaSimulada { SEM_FALHA = 0, FALHA_MQTT = 1, FALHA_WIFI = 2 };
static FalhaSimulada falha = SEM_FALHA;

static unsigned long ultima_tentativa_wifi = 0;
static unsigned long ultima_tentativa_mqtt = 0;
static bool mqtt_estava_conectado = false;

static String topico_publicacao(void)
{
  return "channels/" + String(CANAL_ID) + "/publish";
}

static String topico_field(uint8_t field)
{
  return "channels/" + String(CANAL_ID) + "/subscribe/fields/field" + String(field);
}

// ThingSpeak manda payload vazio ou "null" quando a atualização não tem aquele field
static bool eh_numero(const String& s)
{
  if (s.length() == 0) return false;
  bool tem_digito = false;
  for (unsigned int i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c >= '0' && c <= '9') { tem_digito = true; continue; }
    if (c == '.' || (c == '-' && i == 0)) continue;
    return false;
  }
  return tem_digito;
}

// Configuração remota:
//   field4 = limiar de alerta (cm)
//   field5 = limiar de inundação (cm)
//   field6 = modo da comporta (0 AUTO, 1 FORCAR FECHAR, 2 FORCAR ABRIR)
static void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
  String msg((char*)payload, length);
  msg.trim();
  String topico(topic);

  if (!eh_numero(msg)) {
    return; // atualização do canal que não mexeu nesse field
  }

  Serial.print("Mensagem recebida. Topico: ");
  Serial.print(topico);
  Serial.print(" payload: ");
  Serial.println(msg);

  if (topico.endsWith("field4")) {
    definir_limiar_alerta(msg.toFloat());
  } else if (topico.endsWith("field5")) {
    definir_limiar_inundacao(msg.toFloat());
  } else if (topico.endsWith("field6")) {
    definir_modo(msg.toInt());
  }
}

// Uma tentativa só. Se falhar, o rede_loop tenta de novo depois de PERIODO_RECONEXAO_MS.
static void tentar_conectar_mqtt(void)
{
  Serial.print("Conectando MQTT em ");
  Serial.print(MQTT_SERVIDOR);
  Serial.print("... ");

  if (!MQTT.connect(MQTT_CLIENT_ID, MQTT_USUARIO, MQTT_SENHA)) {
    Serial.print("falhou, state=");
    Serial.println(MQTT.state());
    return;
  }
  Serial.println("conectado");

  // QoS 0 no ThingSpeak free
  for (uint8_t field = 4; field <= 6; field++) {
    String t = topico_field(field);
    if (!MQTT.subscribe(t.c_str(), 0)) {
      Serial.print("Falha ao assinar ");
      Serial.println(t);
    }
  }
}

void init_rede(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  ultima_tentativa_wifi = millis();

  MQTT.setServer(MQTT_SERVIDOR, MQTT_PORTA);
  MQTT.setCallback(mqtt_callback);
  MQTT.setSocketTimeout(2);
  // Keepalive maior que o período de publicação: cada publicação já conta como
  // atividade para o broker. Com 15 s (padrão), o Wokwi rodando abaixo de 100%
  // da velocidade real fazia o ping chegar tarde e o ThingSpeak derrubava a conexão.
  MQTT.setKeepAlive(60);
}

void rede_loop(void)
{
  unsigned long agora = millis();

  if (falha == FALHA_WIFI) {
    return; // rádio desligado de propósito, a borda segue funcionando sozinha
  }

  if (WiFi.status() != WL_CONNECTED) {
    if (agora - ultima_tentativa_wifi >= PERIODO_RECONEXAO_MS * 2) {
      Serial.println("WiFi desconectado, tentando reconectar");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_SENHA);
      ultima_tentativa_wifi = agora;
    }
    return;
  }

  if (falha == FALHA_MQTT) {
    return;
  }

  if (!MQTT.connected()) {
    if (mqtt_estava_conectado) {
      mqtt_estava_conectado = false;
      Serial.print("MQTT caiu, state=");
      Serial.println(MQTT.state());
    }
    if (agora - ultima_tentativa_mqtt >= PERIODO_RECONEXAO_MS || ultima_tentativa_mqtt == 0) {
      ultima_tentativa_mqtt = agora;
      tentar_conectar_mqtt();
    }
    return;
  }

  mqtt_estava_conectado = true;
  MQTT.loop();
}

bool wifi_ok(void)
{
  return WiFi.status() == WL_CONNECTED;
}

bool mqtt_ok(void)
{
  return wifi_ok() && MQTT.connected();
}

bool publicar_leitura(float nivel_cm, EstadoNivel estado, bool comporta_fechada)
{
  if (!mqtt_ok()) {
    Serial.println("Sem MQTT, leitura nao publicada");
    return false;
  }

  char payload[80];
  snprintf(payload, sizeof(payload), "field1=%.1f&field2=%d&field3=%d",
           nivel_cm, (int)estado, comporta_fechada ? 1 : 0);

  String topico = topico_publicacao();
  bool ok = MQTT.publish(topico.c_str(), payload);

  Serial.print(ok ? "Publicado: " : "Falha ao publicar: ");
  Serial.println(payload);
  return ok;
}

void alternar_falha_simulada(void)
{
  falha = (FalhaSimulada)((falha + 1) % 3);

  switch (falha) {
    case FALHA_MQTT:
      Serial.println("[DEMO] Simulando queda do MQTT");
      MQTT.disconnect();
      mqtt_estava_conectado = false;
      break;

    case FALHA_WIFI:
      Serial.println("[DEMO] Simulando queda do WiFi");
      MQTT.disconnect();
      mqtt_estava_conectado = false;
      WiFi.disconnect(true); // desliga o rádio
      break;

    case SEM_FALHA:
    default:
      Serial.println("[DEMO] Rede restaurada, reconectando");
      WiFi.mode(WIFI_STA);
      WiFi.begin(WIFI_SSID, WIFI_SENHA);
      ultima_tentativa_wifi = millis();
      ultima_tentativa_mqtt = 0;
      break;
  }
}
