# Bibliotecas Arduino – ESP32-S3 Touch AMOLED 1.8"

Esta pasta reúne as **bibliotecas usadas pelos exemplos** em `Arduino-v3.1.0/examples/`.  
Elas cobrem:

- Display (GFX / LVGL)  
- Sensores (RTC, IMU, touch, luz, IO expander, etc.)  
- Configuração de pinos específicos da placa  
- UIs LVGL geradas por ferramentas externas (SquareLine, etc.)

Todas as bibliotecas aqui são pensadas para uso com **ESP32/ESP32-S3** e **framework Arduino**.

---

## Visão geral das subpastas

- `ESP32_IO_Expander/` – Biblioteca para expansores de IO.  
- `GFX_Library_for_Arduino/` – Biblioteca gráfica Arduino_GFX.  
- `SensorLib/` – Drivers multi-plataforma para sensores I2C/SPI.  
- `lvgl/` – Biblioteca gráfica LVGL (com README em PT-BR).  
- `Mylibrary/` – Pinout e definições específicas da placa (arquivo `pin_config.h`).  
- `ui_a/`, `ui_b/`, `ui_c/` – UIs LVGL prontas, exportadas de projetos de UI.  
- `lv_conf.h` – Arquivo de configuração do LVGL utilizado pelos exemplos.

Abaixo, um resumo do propósito de cada uma.

---

## ESP32_IO_Expander/

- **Nome oficial:** ESP32_IO_Expander  
- **Função:**  
  Biblioteca Arduino para dirigir **chips expansores de IO** com ESP32.
- **Principais recursos:**
  - Controlar pinos individuais (`pinMode`, `digitalRead`, `digitalWrite`).  
  - Controlar múltiplos pinos simultaneamente.  
  - Encapsula drivers do **Espressif Components Registry** (esp_io_expander, TCA95xx, HT8574, etc.).
- **Uso típico neste repositório:**  
  - Para projetos que precisem expandir o número de GPIOs disponíveis além do ESP32-S3.  
- **Documentação de referência:**  
  - `ESP32_IO_Expander/README.md` (em inglês).

---

## GFX_Library_for_Arduino/ (Arduino_GFX)

- **Nome oficial:** Arduino_GFX  
- **Função:**  
  Biblioteca gráfica para diversos displays, com várias interfaces (SPI, paralelo, RGB, etc.).
- **Principais recursos:**
  - API inspirada em Adafruit_GFX, LovyanGFX, TFT_eSPI etc.  
  - Suporte a várias placas ESP32 / ESP32-S3.  
  - Desenho de textos, linhas, retângulos, triângulos, círculos, imagens, canvas, etc.  
  - Suporte a fontes U8g2 (incluindo fontes com caracteres Unicode).
- **Uso neste repositório:**
  - Base de todos os exemplos **GFX** em `examples/01` a `examples/07`.
- **Documentação de referência:**  
  - `GFX_Library_for_Arduino/README.md` (em inglês) contém detalhes completos sobre displays suportados e exemplos adicionais.

---

## SensorLib/

- **Nome oficial:** SensorLib (por lewisxhe)  
- **Função:**  
  Coleção de drivers para **sensores comumente usados** via I2C/SPI:
  - RTC (PCF8563, PCF85063, HYM8563)  
  - IMU (QMI8658, BHI260AP, BMA423)  
  - Sensores magnéticos (QMC6310, BMM150)  
  - GPIO expander (XL9555, XL9535)  
  - Haptic (DRV2605)  
  - Sensores de luz (CM32181, LTR553)  
  - Múltiplos controladores de touch (FT5xxx, CST8xx/CST9xxx, GT911, etc.)  
  - Driver de LED (AW9364)
- **Uso neste repositório:**
  - RTC PCF85063 → exemplos `05_GFX_PCF85063_simpleTime` e `10_LVGL_PCF85063_simpleTime`.  
  - IMU QMI8658 → exemplo `11_LVGL_QMI8658_ui`.  
  - Touch capacitivo → exemplos de LVGL e GFX com toque.  
- **Documentação de referência:**
  - `SensorLib/README.md` (em inglês), com lista completa de sensores suportados.  
  - `SensorLib/examples/` contém exemplos adicionais (Arduino/ESP-IDF).

---

## lvgl/

- **Nome:** LVGL – Light and Versatile Graphics Library  
- **Função:**  
  Biblioteca de GUI avançada para microcontroladores, com widgets, layout tipo CSS (Flex/Grid), temas etc.
- **Características principais:**
  - Widgets: botões, listas, gráficos, sliders, imagens, etc.  
  - Suporte a múltiplos dispositivos de entrada (touch, mouse, teclado, encoder).  
  - Baixo consumo de memória, alto nível de personalização.  
  - Usado amplamente em ESP32, STM32, Linux framebuffer etc.
- **Uso neste repositório:**
  - Base para todos os exemplos LVGL em `examples/08` a `examples/16`.  
  - Integração com o display QSPI e touch da placa.
- **Documentação em Português BR:**
  - `lvgl/README_pt_BR.md` – excelente ponto de partida para entender LVGL.  
- **Arquivos importantes:**
  - `lvgl.h` – cabeçalho principal.  
  - `lv_conf_template.h` – template de configuração.  
  - `lv_conf.h` (na raiz de `libraries/`) – configuração concreta usada pelos exemplos deste repositório.

---

## Mylibrary/

- **Função:**  
  Guardar **configurações específicas da placa**, principalmente o **pinout**.
- **Arquivo principal:**
  - `Mylibrary/pin_config.h`
- **O que há em `pin_config.h`:**
  - Definição do chip de energia: `XPOWERS_CHIP_AXP2101`.  
  - Pinos do **display** (LCD_SDIO0…3, LCD_SCLK, LCD_CS) e dimensões (`LCD_WIDTH`, `LCD_HEIGHT`).  
  - Pinos de **touch** (`IIC_SDA`, `IIC_SCL`, `TP_INT`).  
  - Pinos de **áudio / I2S / ES8311** (`I2S_MCK_IO`, `I2S_BCK_IO`, `I2S_DI_IO`, `I2S_WS_IO`, `I2S_DO_IO`, `PA`).  
  - Pinos de **SDMMC** (`SDMMC_CLK`, `SDMMC_CMD`, `SDMMC_DATA`).
- **Importância:**
  - Se você mudar de placa ou quiser adaptar para outro hardware, esse é o primeiro arquivo a ser ajustado.

---

## ui_a/, ui_b/, ui_c/

- **Função:**  
  Guardar **interfaces prontas em LVGL**, normalmente exportadas de ferramentas como **SquareLine Studio**.
- **Tipicamente contêm:**
  - Arquivos `.h` e `.c/.cpp` gerados automaticamente.  
  - Estruturas de widgets, telas, estilos, callbacks básicos.
- **Uso:**  
  - São consumidas pelos exemplos LVGL e por projetos mais complexos (como os do VolosR – `stepCounter`, `pocketClock`, etc.).
- **Como usar em novos projetos:**
  - Inclua o cabeçalho correto (por exemplo, `#include &lt;ui_a.h&gt;`).  
  - Chame a função de inicialização da UI (`ui_init()` ou equivalente) após inicializar o LVGL e o display.

---

## lv_conf.h

- **Função:**  
  Arquivo de **configuração do LVGL** utilizado pelos exemplos.
- **Pontos típicos de configuração:**
  - Tamanho do buffer de desenho (draw buffer).  
  - Resolução horizontal/vertical padrão.  
  - Ativação de recursos (widgets, layouts, temas).  
  - Nível de log e assert.
- **Quando mexer:**
  - Apenas se você **souber exatamente** o que está fazendo (por exemplo, ajustar para outra resolução de tela, reduzir memória, habilitar recursos adicionais do LVGL).

---

## Como integrar estas bibliotecas em um novo projeto

### Arduino IDE / VSCode (extensão Arduino)

1. Copie as pastas desejadas de `Arduino-v3.1.0/libraries/` para a pasta de bibliotecas do Arduino (`…/Arduino/libraries`).  
2. No seu sketch `.ino`, inclua o que precisar, por exemplo:

```cpp
#include &lt;Arduino_GFX_Library.h&gt;
#include &lt;lvgl.h&gt;
#include &lt;SensorLib.h&gt;
#include &lt;pin_config.h&gt;   // de Mylibrary
```

3. Configure seu display/touch/RTC/IMU conforme os exemplos em `../examples`.

### PlatformIO (framework = arduino)

1. Crie um novo projeto para **ESP32-S3**.  
2. No `platformio.ini`, você pode apontar `lib_extra_dirs` para esta pasta `libraries/`, por exemplo:

```ini
[env:esp32s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino

lib_extra_dirs = 
    ${PROJECT_DIR}/../Arduino-v3.1.0/libraries
```

3. Use os includes da mesma forma que no Arduino IDE.

---

Se você quiser, posso preparar **um exemplo de projeto completo** (Arduino ou PlatformIO) usando uma combinação específica destas bibliotecas (por exemplo, `LVGL + SensorLib + ES8311` com base em algum UI do VolosR).