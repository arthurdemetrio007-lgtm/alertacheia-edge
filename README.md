# AlertaCheia: sensor de nível de arroio (Edge Computing & Computer Systems)

Global Solution 2026.2 · CitizenTech · Engenharia de Software 1º ano · FIAP Porto Alegre

## Integrantes

- Arthur Veiga Demétrio, RM: 570056

## Contexto

Nas cheias de maio de 2024, muita gente em Porto Alegre e na região metropolitana soube que a água estava subindo só quando ela já estava na rua. O AlertaCheia mede o nível de um arroio ou canal em tempo real, decide sozinho no próprio dispositivo (na borda) quando acender o alerta e fechar a comporta, e manda os dados para a nuvem. A partir dali, o sistema avisa os moradores do bairro.

Esta parte do projeto é o dispositivo IoT: um ESP32 simulado no Wokwi.

## O que o dispositivo faz

| Requisito | Como foi feito |
|---|---|
| Leitura de sensor com amostragem constante | HC-SR04 (ultrassônico) lido a cada 1 s. O próximo instante de leitura é calculado a partir do instante previsto, não do `millis()` atual, então o atraso de um ciclo não se acumula. Média móvel das últimas 5 leituras para filtrar ruído. |
| Atuação com lógica na borda | Servo representando a comporta. A decisão de fechar é tomada no ESP32, mesmo sem internet. |
| Interface local | LCD 20x4 I2C com nível, estado, comporta e status de WiFi/MQTT. LEDs verde, amarelo e vermelho indicando o estado. |
| Envio por MQTT | Publica nível, estado e comporta no ThingSpeak a cada 20 s. |
| Persistência | Histórico salvo no canal do ThingSpeak, lido depois pelo código Python do projeto. |
| Recepção de configuração e acionamento remoto | Recebe limiar de alerta, limiar de inundação e comando da comporta pelos fields 4, 5 e 6. |

### Regras de decisão na borda

O nível é calculado como `nivel = 300 cm - distância até a água`, porque o sensor fica preso 300 cm acima do fundo do arroio.

| Estado | Condição de entrada | LED | Comporta (modo AUTO) |
|---|---|---|---|
| NORMAL | nível abaixo do limiar de alerta | verde | aberta |
| ALERTA | nível a partir de 150 cm | amarelo | aberta |
| INUNDACAO | nível a partir de 200 cm | vermelho | fechada |

- **Histerese de 10 cm na descida:** para sair de ALERTA o nível precisa baixar de 140 cm, e para sair de INUNDACAO precisa baixar de 190 cm. Sem isso, a água oscilando em volta do limiar faria a comporta abrir e fechar sem parar.
- **Trava de segurança:** comando remoto de abrir a comporta é ignorado enquanto o estado for INUNDACAO.
- **Configuração validada:** limiar recebido só é aceito se `0 < alerta < inundação <= 300`.

## Tela local (LCD 20x4)

```
Nivel: 152.3 cm          <- nível atual (aparece "(!)" se o sensor não respondeu)
Estado: ALERTA           <- NORMAL / ALERTA / INUNDACAO
Comp: ABERTA (AUTO)      <- comporta e modo: AUTO = borda decide, REM = comando remoto
WiFi:OK   MQTT:OK        <- status da conexão
```

**Botão azul (Simular queda de rede):** cada toque avança um passo, para demonstrar a desconexão no vídeo.

1. 1º toque: derruba o MQTT. A tela mostra `WiFi:OK   MQTT:OFF`.
2. 2º toque: desliga o WiFi. A tela mostra `WiFi:OFF  MQTT:OFF`.
3. 3º toque: volta a rede. O dispositivo reconecta sozinho.

Durante a queda, o sensor continua lendo, os LEDs e a comporta continuam reagindo, e a tela continua atualizando. A reconexão não trava o loop.

**Para mudar o nível no simulador:** clique no HC-SR04 e mova o controle de distância. Distância menor significa água mais alta (100 cm de distância = 200 cm de nível = INUNDACAO).

## Plataforma IoT: ThingSpeak

Um canal com 6 fields:

| Field | Nome | Quem escreve | Valores |
|---|---|---|---|
| field1 | nivel_cm | dispositivo | nível da água em cm |
| field2 | estado | dispositivo | 0 = NORMAL, 1 = ALERTA, 2 = INUNDACAO |
| field3 | comporta | dispositivo | 0 = aberta, 1 = fechada |
| field4 | limiar_alerta_cm | painel / operador | ex.: 150 |
| field5 | limiar_inundacao_cm | painel / operador | ex.: 200 |
| field6 | modo_comporta | painel / operador | 0 = AUTO, 1 = forçar fechar, 2 = forçar abrir |

Tópicos MQTT (broker `mqtt3.thingspeak.com:1883`):

- Publicação: `channels/<CANAL_ID>/publish` com payload `field1=152.3&field2=1&field3=0`
- Assinatura: `channels/<CANAL_ID>/subscribe/fields/field4`, `field5` e `field6`

Para mandar uma configuração ou comando remoto, basta escrever no field pela API de escrita do canal. Exemplo, forçar fechar a comporta:

```
https://api.thingspeak.com/update?api_key=<WRITE_API_KEY>&field6=1
```

Voltar ao automático: `field6=0`. Trocar o limiar de alerta para 170 cm: `field4=170`.

O dashboard do dispositivo fica na entrega de Front-end Design. O próprio canal do ThingSpeak também mostra os gráficos de cada field.

## Como rodar

Mesmo ambiente das aulas: VS Code + extensão PlatformIO IDE + extensão Wokwi Simulator (com licença ativada).

1. Criar o canal no ThingSpeak com os 6 fields acima.
2. Em ThingSpeak > Devices > MQTT, criar um device com permissão de **Publish e Subscribe** nesse canal e anotar Client ID, Username e Password.
3. Copiar `include/secrets.example.h` para `include/secrets.h` e preencher com essas credenciais e o ID do canal.
4. Abrir esta pasta no VS Code e compilar (botão Build do PlatformIO ou `pio run`).
5. Abrir `diagram.json` e rodar `F1 > Wokwi: Start Simulator`.

O monitor serial mostra cada leitura, troca de estado, publicação e mensagem recebida.

### Teste das regras de decisão

As regras ficam em `lib/control/src/regras.cpp`, separadas do hardware, e podem ser testadas no PC:

```
g++ -I lib/control/src test/test_regras.cpp lib/control/src/regras.cpp -o test_regras
./test_regras
```

## Estrutura

```
include/config.h          pinos, limiares padrão e tempos
include/secrets.h         credenciais (não vai para o GitHub)
lib/sensors/              leitura do HC-SR04 e média móvel
lib/control/              regras de decisão, LEDs e comporta
lib/display/              tela LCD 20x4 I2C
lib/mqtt/                 WiFi e MQTT sem bloqueio, configuração remota
src/main.cpp              agenda de tarefas por millis()
diagram.json              circuito do Wokwi
test/test_regras.cpp      teste da lógica de borda
```

## Circuito

| Componente | Pino do ESP32 |
|---|---|
| HC-SR04 TRIG / ECHO | GPIO 5 / GPIO 18 |
| LCD 20x4 I2C SDA / SCL | GPIO 21 / GPIO 22 |
| Servo (comporta) | GPIO 19 |
| LED verde / amarelo / vermelho | GPIO 25 / 26 / 27 (com resistor de 330 Ω) |
| Botão de simular queda de rede | GPIO 4 (INPUT_PULLUP) |

## Uso de IA

Código desenvolvido com apoio de IA para revisão e organização. Os prompts estão em `prompts.md`.
