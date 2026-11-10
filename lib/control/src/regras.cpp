#include "regras.h"

EstadoNivel proximo_estado(EstadoNivel atual, float nivel_cm,
                           float limiar_alerta, float limiar_inundacao,
                           float histerese)
{
  // Subida: troca de estado assim que cruza o limiar
  if (nivel_cm >= limiar_inundacao) return INUNDACAO;

  switch (atual) {
    case INUNDACAO:
      // Descida: só sai de INUNDACAO quando baixar o limiar menos a histerese
      if (nivel_cm >= limiar_inundacao - histerese) return INUNDACAO;
      if (nivel_cm >= limiar_alerta - histerese) return ALERTA;
      return NORMAL;

    case ALERTA:
      if (nivel_cm >= limiar_alerta - histerese) return ALERTA;
      return NORMAL;

    case NORMAL:
    default:
      if (nivel_cm >= limiar_alerta) return ALERTA;
      return NORMAL;
  }
}

bool decidir_comporta(EstadoNivel estado, ModoComporta modo)
{
  switch (modo) {
    case MODO_FORCAR_FECHAR:
      return true;
    case MODO_FORCAR_ABRIR:
      return estado == INUNDACAO; // trava: não abre durante inundação
    case MODO_AUTO:
    default:
      return estado == INUNDACAO;
  }
}

bool limiares_validos(float alerta, float inundacao, float altura_sensor)
{
  return alerta > 0 && alerta < inundacao && inundacao <= altura_sensor;
}

const char* nome_estado(EstadoNivel estado)
{
  switch (estado) {
    case ALERTA:    return "ALERTA";
    case INUNDACAO: return "INUNDACAO";
    case NORMAL:
    default:        return "NORMAL";
  }
}
