#ifndef REGRAS_H
#define REGRAS_H

// Regras de decisão da borda, sem nada de hardware.
// Ficam separadas para poderem ser testadas no PC (pasta test/).

enum EstadoNivel {
  NORMAL = 0,
  ALERTA = 1,
  INUNDACAO = 2
};

enum ModoComporta {
  MODO_AUTO = 0,          // a borda decide sozinha
  MODO_FORCAR_FECHAR = 1, // comando remoto
  MODO_FORCAR_ABRIR = 2   // comando remoto
};

// Retrato da borda num instante: o que a tela mostra e o que vai para a nuvem.
struct LeituraBorda {
  float nivel_cm;
  bool sensor_ok;
  EstadoNivel estado;
  bool comporta_fechada;
  ModoComporta modo;
};

// Próximo estado a partir do atual, com histerese na descida.
EstadoNivel proximo_estado(EstadoNivel atual, float nivel_cm,
                           float limiar_alerta, float limiar_inundacao,
                           float histerese);

// true = comporta fechada.
// Trava de segurança: em INUNDACAO a comporta fica fechada mesmo com comando remoto de abrir.
bool decidir_comporta(EstadoNivel estado, ModoComporta modo);

// Validação de configuração recebida remotamente.
bool limiares_validos(float alerta, float inundacao, float altura_sensor);

const char* nome_estado(EstadoNivel estado);

#endif
