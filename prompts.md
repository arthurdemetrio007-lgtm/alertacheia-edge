# Registro de uso de IA (Edge Computing & Computer Systems)

Ferramenta: Claude (Anthropic).

## Prompts

1. Análise do enunciado da Global Solution e das entregas de cada disciplina, com o tema de enchentes.
2. "o sensor é da materia de edge?" e pedido para começar o dispositivo a partir do projeto `ThingSpeak_ESP32` usado em aula.
3. Ajuda para criar o canal e o device MQTT no ThingSpeak e configurar o `secrets.h`.
4. Envio do erro `MissingPackageManifestError` no primeiro build do PlatformIO.
5. Envio dos prints do monitor serial durante os testes no Wokwi, mostrando o MQTT reconectando depois de cada publicação.
6. "o que ficou para trás? o que falta fazer?" para revisar as pendências da entrega.
7. Envio do recado do professor com o padrão de qualidade do checkpoint 4 (arquivos, funções, structs, enums, watchdog, sem delay, debounce e interrupção).

## O que a IA fez

- Propôs a arquitetura do dispositivo seguindo a estrutura do projeto de aula (libs `sensors`, `control`, `mqtt`, mais `display`).
- Escreveu a primeira versão do firmware, do `diagram.json` e do teste das regras.
- Compilou o firmware para ESP32 e rodou o teste das regras no PC.
- Explicou que o erro do build era um pacote baixado pela metade e passou os comandos para limpar e baixar de novo.
- Leu o código e apontou a causa da reconexão: keepalive padrão de 15 s do PubSubClient com o Wokwi rodando abaixo da velocidade real, fazendo o ThingSpeak derrubar a conexão.
- Aplicou a correção (`setKeepAlive(60)`), um log de queda de conexão e um ajuste para o aviso da trava de segurança aparecer só uma vez.
- Fixou a versão da plataforma no `platformio.ini` e espaçou os LEDs no diagrama.
- Conferiu o código contra o padrão do checkpoint 4 e adicionou o que faltava: watchdog de tarefas, botão por interrupção com debounce e a struct `LeituraBorda`.

## O que foi revisado por mim

- Configurei o canal e o device MQTT no ThingSpeak e as credenciais no `secrets.h`, que fica fora do Git.
- Compilei no meu computador e rodei a simulação no Wokwi pelo VS Code.
- Testei os três estados mexendo na distância do HC-SR04: NORMAL, ALERTA (140 cm) e INUNDACAO (84 cm), com a comporta fechando sozinha.
- Conferi as publicações chegando no ThingSpeak.
- Testei o comando remoto pelo field 6: forçar fechar (1), forçar abrir (2) e voltar para automático (0).
- Confirmei a trava de segurança: com o nível em inundação, o comando de abrir foi ignorado e a comporta continuou fechada.
- Testei o botão de queda de rede: MQTT cai, WiFi cai, LEDs e comporta continuam funcionando, depois tudo reconecta.
- Percebi nos testes que o MQTT reconectava a cada envio e que a mesma mensagem remota chegava duas vezes. Depois da correção, rodei 2 minutos e conferi uma conexão só, com publicações seguidas sem queda.
