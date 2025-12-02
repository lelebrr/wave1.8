# Arduino-v3.1.0 – Bibliotecas e Exemplos para ESP32-S3 Touch AMOLED 1.8"

Esta pasta contém tudo o que você precisa para trabalhar com a placa **ESP32-S3 Touch AMOLED 1.8&quot;** usando **Arduino** (IDE clássico, Arduino IDE 2.x ou VSCode com extensão Arduino / PlatformIO):

- **Bibliotecas** específicas para display, sensores, IO expander, LVGL, etc. (`libraries/`)  
- **Exemplos Arduino (.ino)** cobrindo display (GFX e LVGL), RTC, IMU, SD Card e áudio (`examples/`)

Toda a documentação aqui está em **Português BR**.

---

## Estrutura da pasta

- `examples/` – exemplos Arduino prontos:
  - `01_HelloWorld` – teste básico do display.  
  - `02_Drawing_board` – quadro de desenho com touch.  
  - `03_GFX_AsciiTable` – tabela ASCII usando GFX.  
  - `04_GFX_FT3168_Image` – imagem + touch FT3168.  
  - `05_GFX_PCF85063_simpleTime` – hora simples com RTC PCF85063.  
  - `06_GFX_ESPWiFiAnalyzer` – analisador de redes WiFi.  
  - `07_GFX_Clock` – relógio gráfico com GFX.  
  - `08_LVGL_Animation` – animações LVGL.  
  - `09_LVGL_change_background` – troca de background com LVGL.  
  - `10_LVGL_PCF85063_simpleTime` – hora em LVGL + RTC PCF85063.  
  - `11_LVGL_QMI8658_ui` – UI LVGL usando IMU QMI8658.  
  - `12_LVGL_AXP2101_ADC_Data` – dados de ADC do PMIC AXP2101 em LVGL.  
  - `13_LVGL_Widgets` – demonstração de widgets LVGL.  
  - `14_LVGL_SD_Test` – teste de SD card + LVGL.  
  - `15_ES8311` – exemplo de áudio com codec ES8311.  
  - `16_LVGL_Sqprj` – UI gerada por SquareLine (LVGL).

  Veja `examples/README.md` para descrições mais detalhadas de cada exemplo.

- `libraries/` – bibliotecas usadas pelos exemplos:
  - `ESP32_IO_Expander/` – driver Arduino para expansores de IO (esp_io_expander, TCA95xx, HT8574 etc.).  
  - `GFX_Library_for_Arduino/` – **Arduino_GFX**, uma biblioteca gráfica de alto desempenho.  
  - `SensorLib/` – drivers para RTC, IMU, touch, GPIO expander, sensores de luz, etc.  
  - `lvgl/` – biblioteca gráfica LVGL (com `README_pt_BR.md`).  
  - `Mylibrary/` – configurações específicas da placa (especialmente `pin_config.h`).  
  - `ui_a/`, `ui_b/`, `ui_c/` – UIs LVGL geradas por ferramentas como SquareLine Studio.  
  - `lv_conf.h` – arquivo de configuração do LVGL para este projeto.

  Veja `libraries/README.md` para um resumo de cada biblioteca.

---

## Requisitos de software (Arduino)

- **Core ESP32 / ESP32-S3 para Arduino** (Espressif):  
  - Instalar via Boards Manager.  
  - Versão recomendada: compatível com **Arduino-GFX**, **LVGL** e exemplos mais recentes (2.x).

- **VSCode + Arduino / PlatformIO (opcional, mas recomendado)**:
  - VSCode com extensão **Arduino** ou **PlatformIO IDE**.  
  - Facilita o trabalho com múltiplos projetos e bibliotecas.

---

## Instalação das bibliotecas

Os exemplos esperam encontrar as bibliotecas equivalentes às pastas de `libraries/`.  
Você tem duas opções principais:

### Opção 1 – Copiar para a pasta padrão do Arduino

1. Localize sua pasta de bibliotecas do Arduino  
   - Exemplo comum no Windows: `Documentos/Arduino/libraries`  
2. Copie as pastas a seguir para lá:
   - `Arduino-v3.1.0/libraries/ESP32_IO_Expander`  
   - `Arduino-v3.1.0/libraries/GFX_Library_for_Arduino`  
   - `Arduino-v3.1.0/libraries/SensorLib`  
   - `Arduino-v3.1.0/libraries/lvgl`  
   - `Arduino-v3.1.0/libraries/ui_a`  
   - `Arduino-v3.1.0/libraries/ui_b`  
   - `Arduino-v3.1.0/libraries/ui_c`  
   - `Arduino-v3.1.0/libraries/Mylibrary`  

3. Reinicie o Arduino IDE / VSCode se necessário.

### Opção 2 – Usar PlatformIO com este repositório como base

Se você prefere **PlatformIO**:

1. Crie um novo projeto PlatformIO para **ESP32-S3** (por exemplo, `esp32-s3-devkitc-1`, `esp32-s3` etc.) com `framework = arduino`.  
2. No `platformio.ini`, adicione este diretório de `libraries/` à opção `lib_extra_dirs` ou copie as bibliotecas para a pasta `lib/` do seu projeto.  
3. Copie o exemplo desejado de `examples/NN_Nome` para `src/` ou configure o `src_dir` para apontar para a pasta do exemplo.

---

## Como abrir e compilar um exemplo no VSCode (extensão Arduino)

1. **Abrir o repositório**  
   - No VSCode: `File → Open Folder…` e selecione a raiz do repositório.

2. **Abrir um exemplo**  
   - Vá até `Arduino-v3.1.0/examples/NN_AlgumExemplo/`.  
   - Abra o arquivo `.ino` correspondente:
     - `01_HelloWorld/01_HelloWorld.ino`  
     - `08_LVGL_Animation/ui/ui.ino`  
     - etc.

3. **Selecionar a placa** (Command Palette → “Arduino: Select Board”)  
   - Escolha uma placa compatível com **ESP32-S3**:
     - `ESP32S3 Dev Module`, ou  
     - Perfil específico da sua placa, se existir (por exemplo, algo como `ESP32-S3-Touch-AMOLED` caso você crie um board definition próprio).

4. **Selecionar a porta** (Command Palette → “Arduino: Select Serial Port”).  

5. **Compilar e fazer upload**  
   - Use o botão de upload da extensão Arduino ou a opção `Arduino: Upload`.

Se aparecerem erros de biblioteca não encontrada, verifique se a pasta `libraries` foi instalada corretamente (conforme seção anterior).

---

## Pinout e arquivo de configuração da placa (Mylibrary)

Os exemplos Arduino usam uma biblioteca local chamada **Mylibrary**, que centraliza o mapeamento de pinos:

- Arquivo principal: `libraries/Mylibrary/pin_config.h`

Neste arquivo você encontrará:

- Pinos do **display QSPI** (`LCD_SDIO0`–`LCD_SDIO3`, `LCD_SCLK`, `LCD_CS`)  
- Dimensões do display (`LCD_WIDTH`, `LCD_HEIGHT`) – 368×448  
- Pinos de **touch** (`IIC_SDA`, `IIC_SCL`, `TP_INT`)  
- Pinos de **áudio / I2S / ES8311** (`I2S_MCK_IO`, `I2S_BCK_IO`, `I2S_WS_IO`, `I2S_DO_IO`, `I2S_DI_IO`, `PA`)  
- Pinos de **SDMMC** (`SDMMC_CLK`, `SDMMC_CMD`, `SDMMC_DATA`)

Se você estiver usando **outra placa** com o mesmo conjunto de exemplos, pode precisar ajustar esse arquivo para refletir o seu pinout.

---

## Visão geral das bibliotecas principais

Resumo rápido (detalhado em `libraries/README.md`):

- **GFX_Library_for_Arduino (Arduino_GFX)**  
  - Biblioteca gráfica de alto desempenho, com suporte a diversos displays, incluindo ESP32-S3 e placas com displays RGB/QSPI.  
  - Usada principalmente nos exemplos `01` a `07` (parte GFX).

- **LVGL** (`libraries/lvgl/`)  
  - Biblioteca gráfica avançada para GUIs complexas.  
  - A documentação em português está em `libraries/lvgl/README_pt_BR.md`.  
  - Usada em todos os exemplos `08` em diante (LVGL_xxx, widgets, animações, etc.).

- **SensorLib**  
  - Conjunto de drivers para sensores I2C/SPI (RTC PCF85063, IMU QMI8658, touch, haptic, etc.).  
  - Base dos exemplos que usam RTC, IMU, touch.

- **ESP32_IO_Expander**  
  - Drivers para expansores de IO (TCA95xx, HT8574, etc.), caso sua aplicação precise de mais GPIOs.

- **UI_a / UI_b / UI_c**  
  - Bibliotecas de interface geradas (por exemplo, SquareLine Studio) contendo telas LVGL prontas para uso.

---

## Lista de exemplos (resumo)

A lista completa com descrições mais detalhadas está em `examples/README.md`.  
Resumo rápido:

1. **01_HelloWorld** – texto simples no display, ideal para validar cabos e pinout.  
2. **02_Drawing_board** – desenhar na tela via touch.  
3. **03_GFX_AsciiTable** – demonstração de fontes / caracteres.  
4. **04_GFX_FT3168_Image** – carregamento de imagem + touch FT3168.  
5. **05_GFX_PCF85063_simpleTime** – leitura de hora do RTC PCF85063 com GFX.  
6. **06_GFX_ESPWiFiAnalyzer** – scan de redes WiFi com visualização gráfica.  
7. **07_GFX_Clock** – relógio com interface gráfica em GFX.  
8. **08_LVGL_Animation** – animações LVGL (UI em `ui/`).  
9. **09_LVGL_change_background** – trocar tema/fundo em LVGL.  
10. **10_LVGL_PCF85063_simpleTime** – hora + LVGL + RTC PCF85063.  
11. **11_LVGL_QMI8658_ui** – dados do IMU com UI LVGL.  
12. **12_LVGL_AXP2101_ADC_Data** – monitor de tensões/correntes via AXP2101.  
13. **13_LVGL_Widgets** – showcase de controles LVGL.  
14. **14_LVGL_SD_Test** – acesso ao SD card com LVGL.  
15. **15_ES8311** – exemplo de áudio, tocando `canon.h` via ES8311.  
16. **16_LVGL_Sqprj** – exemplo LVGL baseado em projeto SquareLine.

---

## Próximos passos sugeridos

1. Instalar as bibliotecas (seção **Instalação das bibliotecas**).  
2. Abrir e rodar `examples/01_HelloWorld` para garantir que o display está OK.  
3. Rodar um exemplo LVGL, como `08_LVGL_Animation` ou `13_LVGL_Widgets`.  
4. Depois, escolher um periférico específico (RTC, IMU, SD, áudio) e testar o exemplo correspondente.  

Se quiser, posso gerar também um **passo-a-passo específico para PlatformIO** para qualquer exemplo desta pasta – basta dizer qual você quer usar como base.