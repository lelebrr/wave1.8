# Exemplos Arduino – ESP32-S3 Touch AMOLED 1.8"

Esta pasta contém **16 exemplos Arduino (.ino)** organizados para demonstrar os periféricos da placa **ESP32-S3 Touch AMOLED 1.8&quot;**:

- Display QSPI 368×448 (GFX e LVGL)  
- Touch I2C  
- RTC PCF85063  
- IMU QMI8658  
- PMIC AXP2101 (ADC de energia)  
- SD Card (SDMMC)  
- Codec de áudio ES8311 (I2S)

Para compilar estes exemplos, certifique-se de que as bibliotecas em `../libraries/` foram instaladas no seu ambiente Arduino/VSCode (veja `../README.md`).

---

## Pré-requisitos comuns

- **Placa:** ESP32-S3 (idealmente a placa ESP32-S3 Touch AMOLED 1.8&quot; ou equivalente, com mesmo pinout).  
- **Core Arduino ESP32:** instalado via Boards Manager.  
- **Bibliotecas necessárias** (mínimo):
  - `GFX_Library_for_Arduino` (Arduino_GFX)  
  - `lvgl`  
  - `SensorLib`  
  - `ESP32_IO_Expander` (para alguns casos)  
  - `Mylibrary` (contém `pin_config.h` com o pinout da placa)  
  - `ui_a`, `ui_b`, `ui_c` (para exemplos LVGL que usam UIs geradas)

---

## Lista de exemplos

### 01 – 07: Exemplos GFX (Arduino_GFX)

#### `01_HelloWorld/01_HelloWorld.ino`

- **Objetivo:**  
  Verificar rapidamente se o display e o pinout estão corretos (texto “Hello World” / desenho básico).
- **Periféricos:**  
  - Display QSPI (Arduino_GFX)  
- **Use quando:**  
  - Acabou de montar a placa / cabos e quer testar o display.

---

#### `02_Drawing_board/02_Drawing_board.ino`

- **Objetivo:**  
  Criar um “quadro de desenho” na tela, desenhando com o dedo/caneta.
- **Periféricos:**  
  - Display QSPI (Arduino_GFX)  
  - Touch I2C (controlador compatível FT/CST, via SensorLib)  
- **Use quando:**  
  - Quer validar touch + display ao mesmo tempo.  

---

#### `03_GFX_AsciiTable/03_GFX_AsciiTable.ino`

- **Objetivo:**  
  Mostrar uma tabela de caracteres ASCII na tela, para testar fontes, alinhamento e renderização de texto.
- **Periféricos:**  
  - Display QSPI (Arduino_GFX)  
- **Use quando:**  
  - Quer ver todas as fontes/caracteres e testar performance de texto.

---

#### `04_GFX_FT3168_Image/04_GFX_FT3168_Image.ino`

- **Arquivos:**  
  - `04_GFX_FT3168_Image.ino`  
  - `16Bit_368x448px.h` (imagem em 16 bits, 368×448)
- **Objetivo:**  
  Exibir uma imagem em tela cheia e, possivelmente, testar o touch FT3168 (ou similar).
- **Periféricos:**  
  - Display QSPI (imagem 16 bits)  
  - Touch I2C (FT3168 ou equivalente)  
- **Use quando:**  
  - Quer testar carregamento de imagens estáticas.

---

#### `05_GFX_PCF85063_simpleTime/05_GFX_PCF85063_simpleTime.ino`

- **Objetivo:**  
  Ler o horário do **RTC PCF85063** e exibir de forma simples na tela usando GFX.
- **Periféricos:**  
  - Display QSPI (GFX)  
  - RTC PCF85063 (I2C, via SensorLib)  
- **Use quando:**  
  - Quer validar o RTC de tempo real com uma interface simples.

---

#### `06_GFX_ESPWiFiAnalyzer/06_GFX_ESPWiFiAnalyzer.ino`

- **Objetivo:**  
  Criar um analisador de redes WiFi, mostrando SSID, RSSI, canal etc. em uma interface gráfica.
- **Periféricos:**  
  - WiFi do ESP32-S3  
  - Display QSPI (GFX)  
- **Use quando:**  
  - Quer testar WiFi + visualização gráfica.

---

#### `07_GFX_Clock/07_GFX_Clock.ino`

- **Objetivo:**  
  Exibir um relógio gráfico (analógico/digital, dependendo da implementação) usando GFX.
- **Periféricos típicos:**  
  - Display QSPI (GFX)  
  - Fonte de tempo: pode usar RTC ou tempo configurado no código (veja o `.ino` para detalhes).  
- **Use quando:**  
  - Quer um relógio visual rápido, sem necessariamente usar LVGL.

---

### 08 – 16: Exemplos LVGL

#### `08_LVGL_Animation/ui/ui.ino`

- **Objetivo:**  
  Demonstração de **animações LVGL** (telas, widgets se movendo, transições).
- **Periféricos:**  
  - Display QSPI  
  - LVGL (renderização)  
- **Estrutura:**  
  - O `.ino` fica dentro de `ui/`, pois foi gerado/organizado por ferramentas de UI (como SquareLine).  
- **Use quando:**  
  - Quer ver o comportamento de animações LVGL no display real.

---

#### `09_LVGL_change_background/ui/ui.ino`

- **Objetivo:**  
  Demonstrar troca dinâmica de **background / tema** em LVGL (por exemplo, através de touch ou eventos).
- **Periféricos:**  
  - Display QSPI  
  - LVGL  
  - Touch (provavelmente)  
- **Use quando:**  
  - Quer aprender como trocar temas, imagens de fundo ou paletas em tempo de execução.

---

#### `10_LVGL_PCF85063_simpleTime/10_LVGL_PCF85063_simpleTime.ino`

- **Objetivo:**  
  Mostrar horas/minutos/segundos em uma **interface LVGL** usando RTC **PCF85063**.
- **Periféricos:**  
  - LVGL + Display QSPI  
  - RTC PCF85063 (I2C, via SensorLib)  
- **Use quando:**  
  - Quer um exemplo de relógio **LVGL + RTC** (ótimo ponto de partida para projetos tipo “pocket clock”).

---

#### `11_LVGL_QMI8658_ui/11_LVGL_QMI8658_ui.ino`

- **Objetivo:**  
  Exibir dados do **IMU QMI8658** (aceleração, giroscópio etc.) em uma UI LVGL, com gauges, gráficos ou campos de texto.
- **Periféricos:**  
  - LVGL + Display QSPI  
  - IMU QMI8658 (I2C/SPI, via SensorLib)  
- **Use quando:**  
  - Quer base para projetos como **contador de passos, tilt indicators, jogos com movimento**, etc.

---

#### `12_LVGL_AXP2101_ADC_Data/12_LVGL_AXP2101_ADC_Data.ino`

- **Objetivo:**  
  Ler dados de **ADC do PMIC AXP2101** (tensões, correntes, estado de bateria) e exibir em LVGL.
- **Periféricos:**  
  - LVGL + Display QSPI  
  - PMIC AXP2101 (via XPowersLib / SensorLib)  
- **Use quando:**  
  - Quer monitorar energia da placa (tensão de bateria, 3V3, etc.) com interface gráfica.

---

#### `13_LVGL_Widgets/13_LVGL_Widgets.ino`

- **Objetivo:**  
  Demonstrar **vários widgets LVGL** em uma tela: botões, sliders, listas, gráficos, etc.
- **Periféricos:**  
  - LVGL + Display QSPI  
  - Touch (para interação)  
- **Use quando:**  
  - Quer aprender a usar diferentes widgets LVGL na prática.

---

#### `14_LVGL_SD_Test/14_LVGL_SD_Test.ino`

- **Objetivo:**  
  Testar acesso ao **SD card** e possivelmente exibir informações/arquivos usando LVGL.
- **Periféricos:**  
  - SD card (SDMMC, pinos em `Mylibrary/pin_config.h`)  
  - LVGL + Display QSPI  
- **Use quando:**  
  - Quer validar a interface de **cartão SD** da placa.

---

#### `15_ES8311/15_ES8311.ino`

- **Arquivos adicionais:**  
  - `canon.h` – dados de áudio em memória.  
  - `es8311.c`, `es8311.h`, `es8311_reg.h` – driver do codec ES8311.
- **Objetivo:**  
  Demonstrar reprodução de áudio usando o **codec ES8311** via I2S (por exemplo, tocando a música Canon).
- **Periféricos:**  
  - Codec ES8311 (I2S)  
  - Pinos I2S definidos em `Mylibrary/pin_config.h`  
- **Use quando:**  
  - Quer testar **áudio** (saída para fones/alto-falantes) na placa.

---

#### `16_LVGL_Sqprj/16_LVGL_Sqprj.ino`

- **Objetivo:**  
  Rodar uma UI completa gerada em **SquareLine Studio** (ou ferramenta similar) integrada com LVGL.
- **Periféricos:**  
  - LVGL + Display QSPI  
  - Possivelmente touch (dependendo do layout do projeto).  
- **Use quando:**  
  - Quer um exemplo de projeto “real” de UI LVGL desenhado em ferramenta gráfica, similar ao que é usado em projetos como o **stepCounter** do VolosR.

---

## Dicas de uso com VSCode (PIO/Arduino)

- Se estiver usando **extensão Arduino do VSCode**:
  - Abra o `.ino` desejado diretamente.  
  - Certifique-se de que as bibliotecas de `../libraries` foram instaladas no sistema.  
  - Se houver erros de include, verifique o *board* selecionado e se os paths de biblioteca estão corretos.

- Se estiver usando **PlatformIO (framework = arduino)**:
  - Crie um projeto, copie o conteúdo da pasta do exemplo para `src/`,  
  - Adicione as bibliotecas (GFX, LVGL, SensorLib, etc.) via:
    - `lib_extra_dirs` apontando para `Arduino-v3.1.0/libraries`, ou  
    - copiando cada biblioteca para `lib/` do seu projeto.

---

Se precisar, posso ajudar a transformar qualquer exemplo específico desta pasta em um **projeto PlatformIO completo** (com `platformio.ini` pronto e ajustes necessários).