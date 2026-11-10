#ifndef CONFIG_H
#define CONFIG_H

// ---------------------------------------------------------------------------
// Pinos
// ---------------------------------------------------------------------------
#define PIN_TRIG          5    // HC-SR04 trigger
#define PIN_ECHO          18   // HC-SR04 echo
#define PIN_SERVO         19   // servo que representa a comporta
#define PIN_LED_VERDE     25
#define PIN_LED_AMARELO   26
#define PIN_LED_VERMELHO  27
#define PIN_BOTAO_REDE    4    // botão que simula queda de rede (para o vídeo)
// LCD 20x4 I2C: SDA = GPIO 21, SCL = GPIO 22 (padrão do ESP32)
#define LCD_ENDERECO      0x27
#define LCD_COLUNAS       20
#define LCD_LINHAS        4

// ---------------------------------------------------------------------------
// Instalação do sensor
// O HC-SR04 fica preso acima do arroio, apontado para baixo.
// nivel_da_agua = ALTURA_SENSOR_CM - distancia_ate_a_agua
// ---------------------------------------------------------------------------
#define ALTURA_SENSOR_CM  300.0f

// ---------------------------------------------------------------------------
// Limiares padrão (contrato comum com Python, front e matemática)
// Podem ser trocados remotamente pelos fields 4 e 5 do ThingSpeak.
// ---------------------------------------------------------------------------
#define LIMIAR_ALERTA_PADRAO_CM     150.0f
#define LIMIAR_INUNDACAO_PADRAO_CM  200.0f
#define HISTERESE_CM                10.0f   // evita a comporta ficar abrindo e fechando

// ---------------------------------------------------------------------------
// Tempos (ms)
// ---------------------------------------------------------------------------
#define PERIODO_AMOSTRAGEM_MS   1000UL   // 1 leitura por segundo, período fixo
#define PERIODO_PUBLICACAO_MS   20000UL  // ThingSpeak free aceita no mínimo 15 s
#define PERIODO_DISPLAY_MS      500UL
#define PERIODO_RECONEXAO_MS    5000UL
#define DEBOUNCE_BOTAO_MS       50UL
#define TAMANHO_MEDIA           5        // média móvel das últimas 5 leituras

// ---------------------------------------------------------------------------
// Comporta (servo)
// ---------------------------------------------------------------------------
#define SERVO_ABERTA_GRAUS   0
#define SERVO_FECHADA_GRAUS  90

#endif
