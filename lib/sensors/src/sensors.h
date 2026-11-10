#ifndef SENSORS_H
#define SENSORS_H

void init_sensors(void);

// Distância do sensor até a água, em cm. Retorna -1 se não houve eco.
float ler_distancia_cm(void);

// Nível da água em cm, já com média móvel das últimas TAMANHO_MEDIA leituras.
float ler_nivel_cm(void);

// false se a última leitura falhou (sem eco).
bool sensor_ok(void);

#endif
