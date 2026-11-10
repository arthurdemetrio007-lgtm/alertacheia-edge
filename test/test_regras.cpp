// Teste das regras de decisão da borda, roda no PC (sem ESP32):
//   g++ -I lib/control/src test/test_regras.cpp lib/control/src/regras.cpp -o test_regras && ./test_regras
#include <cstdio>
#include "regras.h"

static int falhas = 0;

#define CHECA(cond, msg) do { \
  if (cond) { printf("ok    %s\n", msg); } \
  else { printf("FALHA %s\n", msg); falhas++; } \
} while (0)

int main()
{
  const float A = 150, I = 200, H = 10;

  // Subida
  CHECA(proximo_estado(NORMAL, 149.9f, A, I, H) == NORMAL, "abaixo do alerta continua NORMAL");
  CHECA(proximo_estado(NORMAL, 150.0f, A, I, H) == ALERTA, "cruzou alerta vira ALERTA");
  CHECA(proximo_estado(NORMAL, 230.0f, A, I, H) == INUNDACAO, "subida brusca vai direto para INUNDACAO");
  CHECA(proximo_estado(ALERTA, 200.0f, A, I, H) == INUNDACAO, "cruzou inundacao vira INUNDACAO");

  // Descida com histerese
  CHECA(proximo_estado(ALERTA, 145.0f, A, I, H) == ALERTA, "ALERTA segura ate alerta - histerese");
  CHECA(proximo_estado(ALERTA, 139.9f, A, I, H) == NORMAL, "ALERTA volta a NORMAL abaixo de 140");
  CHECA(proximo_estado(INUNDACAO, 195.0f, A, I, H) == INUNDACAO, "INUNDACAO segura ate inundacao - histerese");
  CHECA(proximo_estado(INUNDACAO, 189.9f, A, I, H) == ALERTA, "INUNDACAO desce para ALERTA");
  CHECA(proximo_estado(INUNDACAO, 100.0f, A, I, H) == NORMAL, "queda brusca vai direto para NORMAL");

  // Oscilação em volta do limiar não fica trocando de estado
  EstadoNivel e = NORMAL;
  int trocas = 0;
  float serie[] = {148, 151, 149, 152, 147, 150, 146};
  for (float n : serie) {
    EstadoNivel novo = proximo_estado(e, n, A, I, H);
    if (novo != e) trocas++;
    e = novo;
  }
  CHECA(trocas == 1, "ruido em volta de 150 gera uma troca so");

  // Comporta
  CHECA(decidir_comporta(NORMAL, MODO_AUTO) == false, "AUTO + NORMAL: aberta");
  CHECA(decidir_comporta(ALERTA, MODO_AUTO) == false, "AUTO + ALERTA: aberta");
  CHECA(decidir_comporta(INUNDACAO, MODO_AUTO) == true, "AUTO + INUNDACAO: fechada");
  CHECA(decidir_comporta(NORMAL, MODO_FORCAR_FECHAR) == true, "remoto fechar: fechada");
  CHECA(decidir_comporta(ALERTA, MODO_FORCAR_ABRIR) == false, "remoto abrir em ALERTA: aberta");
  CHECA(decidir_comporta(INUNDACAO, MODO_FORCAR_ABRIR) == true, "trava: remoto abrir em INUNDACAO continua fechada");

  // Validação de configuração remota
  CHECA(limiares_validos(150, 200, 300), "limiares normais aceitos");
  CHECA(!limiares_validos(0, 200, 300), "alerta zero rejeitado");
  CHECA(!limiares_validos(210, 200, 300), "alerta acima da inundacao rejeitado");
  CHECA(!limiares_validos(150, 350, 300), "inundacao acima da altura do sensor rejeitada");

  printf("\n%s (%d falha%s)\n", falhas ? "REPROVADO" : "APROVADO", falhas, falhas == 1 ? "" : "s");
  return falhas ? 1 : 0;
}
