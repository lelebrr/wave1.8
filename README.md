# ESP32-S3 Touch AMOLED 1.8 – Pacote de Exemplos (Arduino e ESP-IDF)

Este repositório reúne **exemplos completos, drivers e bibliotecas** para a placa com display **ESP32-S3 Touch AMOLED 1.8&quot;** (como a da Waveshare), cobrindo:

- **Arduino (v3.1.0)** – exemplos `.ino` prontos, usando GFX, LVGL, SensorLib etc.  
- **ESP-IDF (v5.3.2)** – projetos de referência para PMIC, RTC, IMU, SD/MMC, display QSPI com LVGL e codec de áudio.  
- **Firmware de fábrica** – binário oficial de fábrica para restaurar a placa.

Toda a documentação adicional criada aqui está em **Português BR**, pensada para uso com **VSCode** (extensão Arduino e/ou PlatformIO).

---

## Estrutura geral do repositório

- `Arduino-v3.1.0/`  
  Conjunto de **bibliotecas** e **exemplos Arduino** para a placa:
  - `examples/` – 16 exemplos organizados por tema (GFX, LVGL, RTC, IMU, SD, áudio etc.).  
  - `libraries/` – bibliotecas necessárias (GFX, LVGL, SensorLib, IO expander etc.) e arquivos de configuração de pinos.

- `ESP-IDF-v5.3.2/`  
  Projetos de exemplo para **ESP-IDF 5.3.2**, um por pasta:
  - `01_AXP2101/` – exemplo do PMIC AXP2101 (XPowersLib).  
  - `02_PCF85063/` – exemplo de RTC PCF85063.  
  - `03_QMI8658/` – exemplo do IMU QMI8658 (SensorLib).  
  - `04_SD_MMC/` – uso de SD card com SDMMC (exemplo oficial do ESP-IDF).  
  - `05_LVGL_WITH_RAM/` – display QSPI (SPD1020 / GC9B71 / SH8601) + LVGL, com RAM interna no display.  
  - `06_I2SCodec/` – exemplo de áudio com codec ES8311 via I2S.

- `Firmware/`  
  - `ESP32-S3-Touch-AMOLED-1-8-FactoryXiaozhi_250805.bin` – firmware de fábrica da placa.  
  - `README.txt` – aviso original (em chinês/inglês) informando que o **código-fonte do firmware de fábrica não é público**.  
  - `README_pt-BR.md` – explicação em português (criada neste repositório) sobre o binário e como regravar.

Para detalhes mais profundos de cada parte, veja:

- `Arduino-v3.1.0/README.md` – visão geral das bibliotecas e exemplos Arduino.  
- `Arduino-v3.1.0/examples/README.md` – lista e descrição de todos os exemplos Arduino.  
- `Arduino-v3.1.0/libraries/README.md` – resumo das bibliotecas incluídas.  
- `ESP-IDF-v5.3.2/README.md` – visão geral dos projetos ESP-IDF.  
- `Firmware/README_pt-BR.md` – informações sobre o firmware de fábrica.

---

## Hardware alvo

Os exemplos deste repositório foram organizados para um kit com as seguintes características (típicas da placa ESP32-S3 Touch AMOLED 1.8&quot;):

- **MCU:** ESP32-S3.  
- **Display:** AMOLED/QSPI 368×448, controlado via barramento SDIO (D0–D3 + CLK + CS).  
- **Touch:** I2C (pinos SDA/SCL, INT), com controlador compatível com a família FT/CST (via SensorLib).  
- **PMIC:** AXP2101 (gerenciamento de energia e leitura de ADC).  
- **RTC:** PCF85063 (relógio de tempo real).  
- **IMU:** QMI8658 (acelerômetro/giroscópio).  
- **Áudio:** Codec ES8311 via I2S (MCLK, BCLK, WS, DO/DI).  
- **SD card:** interface SDMMC (CLK, CMD, D0…).

Os pinos específicos usados pelos exemplos Arduino estão centralizados em:

- `Arduino-v3.1.0/libraries/Mylibrary/pin_config.h`

Esse arquivo define, por exemplo:

- Pinos do **LCD** (LCD_SDIO0…3, LCD_SCLK, LCD_CS).  
- Pinos de **touch** (IIC_SDA, IIC_SCL, TP_INT).  
- Pinos de **áudio** (ES8311 / I2S).  
- Pinos de **SDMMC** (SDMMC_CLK, SDMMC_CMD, SDMMC_DATA).

---

## Mapa de exemplos por periférico

Aqui está um “mapa mental” para você saber rapidamente **qual exemplo testar para cada componente**.

### Display + GFX / LVGL

**Arduino:**

- `Arduino-v3.1.0/examples/01_HelloWorld` – teste básico de texto na tela.  
- `Arduino-v3.1.0/examples/02_Drawing_board` – quadro de desenho (touch + display).  
- `Arduino-v3.1.0/examples/03_GFX_AsciiTable` – tabela ASCII usando GFX.  
- `Arduino-v3.1.0/examples/04_GFX_FT3168_Image` – exibição de imagem + touch FT3168.  
- `Arduino-v3.1.0/examples/07_GFX_Clock` – relógio gráfico usando GFX.  
- `Arduino-v3.1.0/examples/08_LVGL_Animation` – animações com LVGL.  
- `Arduino-v3.1.0/examples/09_LVGL_change_background` – troca de background com LVGL.  
- `Arduino-v3.1.0/examples/13_LVGL_Widgets` – widgets LVGL (botões, sliders etc.).  
- `Arduino-v3.1.0/examples/16_LVGL_Sqprj` – UI gerada por SquareLine (LVGL).

**ESP-IDF:**

- `ESP-IDF-v5.3.2/05_LVGL_WITH_RAM` – exemplo completo de LCD QSPI + LVGL, usando drivers do `esp_lcd`.

---

### RTC (Relógio em tempo real – PCF85063)

- Arduino (GFX):  
  - `Arduino-v3.1.0/examples/05_GFX_PCF85063_simpleTime`
- Arduino (LVGL):  
  - `Arduino-v3.1.0/examples/10_LVGL_PCF85063_simpleTime`
- ESP-IDF:  
  - `ESP-IDF-v5.3.2/02_PCF85063`

---

### IMU (QMI8658 – acelerômetro/giroscópio)

- Arduino (UI em LVGL):  
  - `Arduino-v3.1.0/examples/11_LVGL_QMI8658_ui`
- ESP-IDF:  
  - `ESP-IDF-v5.3.2/03_QMI8658`

---

### PMIC / ADC de energia (AXP2101)

- Arduino (LVGL + dados de ADC):  
  - `Arduino-v3.1.0/examples/12_LVGL_AXP2101_ADC_Data`
- ESP-IDF (exemplo XPowersLib):  
  - `ESP-IDF-v5.3.2/01_AXP2101`

---

### SD Card (SDMMC)

- Arduino (teste de SD com LVGL):  
  - `Arduino-v3.1.0/examples/14_LVGL_SD_Test`
- ESP-IDF (exemplo oficial ESP-IDF):  
  - `ESP-IDF-v5.3.2/04_SD_MMC`

---

### Áudio – Codec ES8311 (I2S)

- Arduino:  
  - `Arduino-v3.1.0/examples/15_ES8311` – inclui `canon.h` e drivers ES8311.
- ESP-IDF:  
  - `ESP-IDF-v5.3.2/06_I2SCodec` – exemplo “I2S ES8311 Example” (modo música/echo).

---

## Ambientes de desenvolvimento

### 1. Arduino (VSCode – extensão Arduino ou PlatformIO)

Recomendado se você quer:

- Trabalhar com **.ino** de forma rápida.  
- Focar em exemplos de **interface gráfica (GFX/LVGL)** e integração com sensores.

Passos sugeridos:

1. **Instalar suporte ao ESP32/ESP32-S3 no Arduino**  
   - Via Board Manager: pacote “esp32” (Espressif Systems).  
2. **Instalar bibliotecas** (se ainda não estiverem no seu ambiente):  
   - Copie o conteúdo de `Arduino-v3.1.0/libraries/` para a pasta de bibliotecas do Arduino  
     (por exemplo, `Documentos/Arduino/libraries`), ou aponte o seu projeto/PlatformIO para essa pasta.  
3. **Abrir um exemplo**:  
   - No VSCode, abra este repositório.  
   - Navegue até `Arduino-v3.1.0/examples/NN_NomeDoExemplo/`.  
   - Abra o arquivo `.ino` correspondente (por exemplo, `01_HelloWorld.ino`).  
4. **Configurar placa e porta** na extensão Arduino / PlatformIO:
   - Selecione uma placa compatível com **ESP32-S3** (por ex. `ESP32S3 Dev Module` ou o perfil específico da sua placa).  
5. **Compilar e fazer upload**.

Mais detalhes específicos sobre cada exemplo estão em:

- `Arduino-v3.1.0/README.md`  
- `Arduino-v3.1.0/examples/README.md`

---

### 2. ESP-IDF (v5.3.2)

Recomendado se você quer:

- Exemplo mais “baixo nível” e otimizado.  
- Usar componentes oficiais do ESP-IDF (esp_lcd, sdmmc, i2s, etc.).  

Fluxo típico:

```bash
cd ESP-IDF-v5.3.2/05_LVGL_WITH_RAM   # escolha o projeto
idf.py set-target esp32s3            # definir o alvo, se ainda não estiver
idf.py menuconfig                    # (opcional) ajustar pinos e opções
idf.py -p PORT flash monitor         # gravar e monitorar
```

A maioria dos projetos já vem com `sdkconfig.defaults` e/ou `sdkconfig.ci` para facilitar.

Mais detalhes em:

- `ESP-IDF-v5.3.2/README.md`

---

## Projetos externos de referência (VolosR)

Este repositório foi inspirado/relacionado a alguns projetos públicos do **VolosR** (YouTube/ GitHub), muito úteis como **referência de UI e lógica de aplicação** para este tipo de placa:

- [`stopwatchAmoled`](https://github.com/VolosR/stopwatchAmoled)  
  Exemplo de **cronômetro** para displays AMOLED com ESP32.

- [`pocketClock`](https://github.com/VolosR/pocketClock)  
  Projeto de **relógio de bolso** (clock portátil) usando ESP32 e display pequeno, com interface gráfica em C/LVGL.

- [`stepCounter`](https://github.com/VolosR/stepCounter)  
  Projeto de **contador de passos (pedometer)** para a placa ESP32-S3 Touch AMOLED 1.8&quot;, integrando IMU, UI em LVGL e firmware dedicado.

Você pode usar esses repositórios como **inspiração** para:

- Reaproveitar layouts e ideias de interface (por exemplo, combinando com os UIs de `ui_a`, `ui_b`, `ui_c` e `16_LVGL_Sqprj`).  
- Ver como estruturar projetos completos (clock, step counter, stopwatch) por cima dos drivers deste pacote.

---

## Onde continuar

Se você acabou de clonar o repositório, uma boa sequência é:

1. Ler `Arduino-v3.1.0/README.md` para entender as bibliotecas e exemplos Arduino.  
2. Abrir `Arduino-v3.1.0/examples/01_HelloWorld` no VSCode e testar o display.  
3. Testar um exemplo LVGL (`08_LVGL_Animation` ou `13_LVGL_Widgets`).  
4. Testar um periférico específico (RTC, IMU, SD, áudio) usando os exemplos listados no **Mapa de exemplos**.  
5. Se quiser ir mais fundo, explorar os projetos em `ESP-IDF-v5.3.2/`.

Qualquer dúvida ou se quiser que eu crie mais documentação específica (por exemplo, um passo-a-passo de PlatformIO para um exemplo específico), é só pedir.