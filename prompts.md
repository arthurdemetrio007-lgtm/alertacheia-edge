# Registro de uso de IA – Edge Computing & Computer Systems

**Ferramenta utilizada:** Claude (Anthropic).

## 1. Utilização da IA durante o desenvolvimento

Ao longo do desenvolvimento do projeto da Global Solution, utilizei o Claude como ferramenta de apoio para esclarecer dúvidas técnicas, analisar erros, revisar implementações e auxiliar na organização do firmware.

As principais interações foram:

1. Análise do enunciado da Global Solution, identificação dos requisitos e organização das entregas das disciplinas relacionadas ao tema de enchentes.
2. Estudo da estrutura do projeto `ThingSpeak_ESP32`, utilizado em aula, para compreender como adaptar sua organização ao dispositivo desenvolvido.
3. Orientações sobre a configuração do ThingSpeak, criação do canal, configuração MQTT e utilização das credenciais no arquivo `secrets.h`.
4. Análise de erros encontrados durante a compilação no PlatformIO.
5. Investigação do comportamento da conexão MQTT durante as simulações no Wokwi.
6. Revisão das funcionalidades implementadas e identificação de pontos que precisavam de ajustes.
7. Conferência dos requisitos técnicos apresentados pelo professor para o checkpoint 4.
8. Orientações para organizar as gravações da simulação e apresentar as funcionalidades do dispositivo.
9. Revisão final do repositório, incluindo documentação e organização dos arquivos.

## 2. Como a IA contribuiu para o desenvolvimento

A ferramenta foi utilizada principalmente como suporte técnico durante a implementação e os testes.

Entre as contribuições, destacam-se:

- Auxílio na compreensão e organização da arquitetura do firmware, utilizando como referência a estrutura trabalhada em aula.
- Orientações para implementação e integração dos sensores, display, LEDs, comporta e comunicação MQTT.
- Apoio na análise de erros de compilação e configuração do ambiente PlatformIO.
- Investigação de falhas de comunicação entre o ESP32 e o ThingSpeak.
- Sugestões de ajustes nos parâmetros de conexão MQTT para melhorar a estabilidade durante as simulações.
- Revisão da implementação dos requisitos de checkpoint, incluindo watchdog, interrupções, debounce, structs e enums.
- Apoio na identificação de funcionalidades que precisavam aparecer na demonstração do projeto.
- Revisão da documentação e organização final do repositório.

As sugestões apresentadas pela IA foram analisadas e aplicadas durante o desenvolvimento, conforme a necessidade identificada nos testes.

## 3. Minha participação no desenvolvimento e validação

A utilização da IA não substituiu a configuração, execução e validação prática do projeto. Durante o desenvolvimento, fiquei responsável por configurar o ambiente, realizar as simulações, testar os comportamentos do dispositivo e verificar os resultados obtidos.

As principais atividades realizadas foram:

- Configuração do canal e do dispositivo MQTT no ThingSpeak.
- Configuração das credenciais de acesso no arquivo `secrets.h`, mantido fora do repositório.
- Compilação do firmware no computador e execução da simulação pelo Wokwi, utilizando o VS Code.
- Testes dos estados de funcionamento do dispositivo, variando a distância medida pelo sensor HC-SR04:
  - NORMAL;
  - ALERTA, com distância de 140 cm;
  - INUNDAÇÃO, com distância de 84 cm.
- Verificação do acionamento automático da comporta conforme o nível identificado.
- Conferência do recebimento das informações publicadas no ThingSpeak.
- Testes dos comandos remotos pelo field 6, verificando as opções de fechamento, abertura e funcionamento automático.
- Validação da trava de segurança, verificando que o comando de abertura não era executado quando o dispositivo identificava uma situação de inundação.
- Testes de interrupção e recuperação da conexão Wi-Fi e MQTT, verificando a continuidade do funcionamento dos LEDs e da comporta durante a indisponibilidade da rede.
- Verificação da estabilidade da comunicação após os ajustes realizados.

Durante os testes, também identifiquei comportamentos inesperados, como reconexões frequentes do MQTT e recebimento duplicado de comandos. Esses problemas foram investigados com o auxílio da IA, e as alterações sugeridas foram posteriormente testadas na simulação.

Após os ajustes, realizei um novo teste de aproximadamente dois minutos, verificando a manutenção da conexão e a continuidade das publicações.

## 4. Considerações finais

O Claude foi utilizado como uma ferramenta de apoio ao longo do desenvolvimento, principalmente para auxiliar na compreensão de conceitos, investigação de problemas técnicos e revisão das implementações.

O processo envolveu a análise das sugestões fornecidas, sua aplicação no projeto e a validação dos resultados por meio da compilação e das simulações realizadas no Wokwi, além da conferência das informações recebidas pelo ThingSpeak.

A utilização da ferramenta contribuiu para agilizar a resolução de problemas e aprofundar a compreensão de aspectos relacionados à programação embarcada, comunicação MQTT e integração entre hardware e software.
