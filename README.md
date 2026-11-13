# AlertaCheia: sensor de nível de arroio (Edge Computing & Computer Systems)

Global Solution 2026.2 · CitizenTech · Engenharia de Software 1º ano · FIAP Porto Alegre

## Integrantes

- Arthur Veiga Demétrio, RM: 570056

## Contexto

Nas cheias de maio de 2024, muita gente em Porto Alegre e na região metropolitana soube que a água estava subindo só quando ela já estava na rua. O AlertaCheia mede o nível de um arroio ou canal em tempo real, decide sozinho no próprio dispositivo (na borda) quando acender o alerta e fechar a comporta, e manda os dados para a nuvem. A partir dali, o sistema avisa os moradores do bairro.

Esta parte do projeto é o dispositivo IoT: um ESP32 simulado no Wokwi.

## Vídeo de demonstração

[`video/alertacheia_edge.mp4`](video/alertacheia_edge.mp4) (4 min 33 s, sem narração, com legendas). No GitHub, abra o arquivo e clique em **View raw** para assistir ou baixar.

O vídeo mostra, na mesma tela, o circuito no Wokwi, o LCD ampliado, o monitor serial e o painel web:

1. Partida: o ESP32 conecta ao WiFi e ao MQTT e publica a primeira leitura.
2. Mudança de nível: NORMAL, ALERTA e INUNDACAO, com LEDs, LCD, comporta e painel web atualizando.
3. Queda de rede: o botão derruba o MQTT e depois o WiFi; o LCD mostra `OFF`, a decisão continua na borda e tudo reconecta sozinho.
4. Configuração remota: o limiar de alerta trocado pelo field4 chega por MQTT, muda o estado no LCD e aparece no painel.
5. Trava de segurança: o comando remoto de abrir (field6 = 2) é ignorado durante a inundação.

## O que o dispositivo faz

| Requisito | Como foi feito |
|---|---|
| Leitura de sensor com amostragem constante | HC-SR04 (ultrassônico) lido a cada 1 s. O próximo instante de leitura é calculado a partir do instante previsto, não do `millis()` atual, então o atraso de um ciclo não se acumula. Média móvel das últimas 5 leituras para filtrar ruído. |
| Atuação com lógica na borda | Servo representando a comporta. A decisão de fechar é tomada no ESP32, mesmo sem internet. |
| Interface local | LCD 20x4 I2C com nível, estado, comporta e status de WiFi/MQTT. LEDs verde, amarelo e vermelho indicando o estado. |
| Envio por MQTT | Publica nível, estado e comporta no ThingSpeak a cada 20 s. |
| Persistência | Histórico salvo no canal do ThingSpeak, lido depois pelo painel web e pelo código Python do projeto. |
| Recepção de configuração e acionamento remoto | Recebe limiar de alerta, limiar de inundação e comando da comporta pelos fields 4, 5 e 6. |

### Regras de decisão na borda

O nível é calculado como `nivel = 300 cm - distância até a água`, porque o sensor fica preso 300 cm acima do fundo do arroio.

| Estado | Condição de entrada | LED | Comporta (modo AUTO) |
|---|---|---|---|
| NORMAL | nível abaixo do limiar de alerta | verde | aberta |
| ALERTA | nível a partir do limiar de alerta (padrão 150 cm) | amarelo | aberta |
| INUNDACAO | nível a partir do limiar de inundação (padrão 200 cm) | vermelho | fechada |

- **Histerese de 10 cm na descida:** com os limiares padrão, para sair de ALERTA o nível precisa ficar abaixo de 140 cm, e para sair de INUNDACAO precisa ficar abaixo de 190 cm. Sem isso, a água oscilando em volta do limiar faria a comporta abrir e fechar sem parar.
- **Trava de segurança:** o comando remoto de abrir a comporta é ignorado enquanto o estado for INUNDACAO. O monitor serial registra `Trava de seguranca: comando de abrir ignorado durante inundacao`.
- **Configuração validada:** um limiar recebido só é aceito se `0 < alerta < inundação <= 300`.

## Padrão de qualidade do firmware

Segue o padrão pedido nos checkpoints do 2º semestre:

- **Arquivos e funções:** cada responsabilidade em uma biblioteca própria (`sensors`, `control`, `display`, `mqtt`), e as regras de decisão em `regras.cpp`, sem hardware, testáveis no PC.
- **Enums e structs:** `EstadoNivel` e `ModoComporta` para estados e modos; `LeituraBorda` agrupa nível, estado, comporta e modo, e é o que a tela mostra e o MQTT publica.
- **Watchdog:** o watchdog de tarefas do ESP32 vigia o loop. Se ele ficar 10 s sem completar uma volta, o ESP32 reinicia sozinho e avisa no monitor serial que o reinício veio do watchdog.
- **Sem delay:** o loop é uma agenda de tarefas por `millis()`. A reconexão de WiFi e MQTT também não trava o loop.
- **Interrupção e debounce no botão:** o botão gera uma interrupção na borda de descida. A interrupção só marca o evento; o loop espera 50 ms e confirma que o botão continua pressionado antes de agir, descartando os repiques do contato.

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

**Para mudar o nível no simulador:** clique no HC-SR04 e mova o controle de distância. Distância menor significa água mais alta (100 cm de distância = 200 cm de nível = INUNDACAO). O valor medido fica alguns centímetros diferente da conta (no vídeo, 250 cm de distância deram 46,5 cm de nível): é a diferença entre o tempo de eco que o Wokwi gera e a conversão `tempo / 58` do código. A média móvel também leva 5 leituras para acompanhar uma mudança.

## Plataforma IoT: ThingSpeak

Canal usado no projeto: **3567645**, com 6 fields:

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

Para mandar uma configuração ou um comando remoto, basta escrever no field pela API de escrita do canal. Exemplo para forçar o fechamento da comporta:

```
https://api.thingspeak.com/update?api_key=<WRITE_API_KEY>&field6=1
```

Voltar ao automático: `field6=0`. Trocar o limiar de alerta para 170 cm: `field4=170`. Se a resposta da API for `0`, o ThingSpeak recusou a escrita por ter vindo menos de 15 s depois da anterior; basta repetir.

O dashboard do dispositivo fica na entrega de Front-end Design (painel web do AlertaCheia, que aparece no vídeo). O próprio canal do ThingSpeak também mostra os gráficos de cada field.

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
video/                    vídeo de demonstração
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

Código e vídeo desenvolvidos com apoio de IA (Claude). O que foi pedido, o que a IA fez e o que eu revisei estão em `prompts.md`.
