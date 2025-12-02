# ESP-IDF-v5.3.2 – Exemplos para ESP32-S3 Touch AMOLED 1.8"

Esta pasta reúne **projetos de exemplo para ESP-IDF 5.3.2**, focados na mesma família de hardware usada pelos exemplos Arduino:

- PMIC AXP2101 (gerenciamento de energia)  
- RTC PCF85063  
- IMU QMI8658  
- SD/MMC (cartão SD)  
- Display QSPI com RAM interna + LVGL  
- Codec de áudio ES8311 via I2S  

Os projetos seguem o padrão do ESP-IDF: cada pasta é um projeto independente, com seus próprios `CMakeLists.txt`, `main/`, `sdkconfig.defaults` etc.

---

## Estrutura dos projetos

- `01_AXP2101/` – Exemplo do PMIC AXP2101 usando a biblioteca XPowersLib.  
- `02_PCF85063/` – Exemplo de RTC PCF85063.  
- `03_QMI8658/` – Exemplo do sensor de movimento QMI8658.  
- `04_SD_MMC/` – Exemplo oficial de SD card usando SDMMC (com README detalhado).  
- `05_LVGL_WITH_RAM/` – Exemplo de display QSPI (SPD1020 / GC9B71 / SH8601) com LVGL.  
- `06_I2SCodec/` – Exemplo de áudio com codec ES8311 via I2S.

Cada projeto pode ser compilado e gravado de forma independente com `idf.py`.

---

## Pré-requisitos (ESP-IDF)

- **ESP-IDF 5.3.x** instalado e configurado (`install.sh` / `export.sh`).  
- Placa alvo compatível (ESP32, ESP32-S2/S3, etc.). Muitos exemplos são pensados para **ESP32-S3**.

Exemplo de preparação (caso ainda não tenha o ESP-IDF):

```bash
mkdir -p ~/esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
. ./export.sh   # ou "source export.sh"
```

Depois disso, você pode voltar a esta pasta (`ESP-IDF-v5.3.2`) e trabalhar normalmente com `idf.py`.

---

## Uso geral dos exemplos

Fluxo típico para qualquer projeto dentro desta pasta:

```bash
cd ESP-IDF-v5.3.2/NN_Projeto     # por exemplo, 05_LVGL_WITH_RAM
idf.py set-target esp32s3        # ou outro alvo, se aplicável
idf.py menuconfig                # (opcional) ajustar pinos e opções
idf.py -p PORT flash monitor     # compila, grava e abre o monitor serial
```

- Substitua `PORT` pela porta serial da sua placa (por ex. `/dev/ttyUSB0`, `COM5` etc.).  
- Use `Ctrl+]` para sair do monitor.  

Alguns projetos já têm `sdkconfig.defaults` e `sdkconfig.ci` para facilitar a configuração.

---

## 01_AXP2101 – PMIC AXP2101

- **Pasta:** `ESP-IDF-v5.3.2/01_AXP2101/`  
- **Descrição:**  
  Exemplo para o **AXP2101 Power Management IC**, usando a biblioteca **XPowersLib**.

- **Principais pontos (segundo `README.md`):**
  - Baseado em exemplos do repositório `XPowersLib` para ESP-IDF.  
  - Demonstra inicialização do barramento I2C, detecção e configuração do PMIC.  
  - Mostra o status de cada DCDC/ALDO/BLDO (enable/disable, tensão configurada).  
  - Log típico:

    ```text
    I (345) mian: I2C initialized successfully
    I (355) AXP2101: Init PMU SUCCESS!
    I (385) AXP2101: DCDC=======================================================================
    I (385) AXP2101: DC1  :ENABLE    Voltage:3300 mV
    ...
    ```

- **Como usar:**

```bash
cd ESP-IDF-v5.3.2/01_AXP2101
idf.py set-target esp32s3
idf.py -p PORT flash monitor
```

- **Relacionamento com os exemplos Arduino:**
  - Em Arduino, o PMIC AXP2101 é utilizado em  
    `Arduino-v3.1.0/examples/12_LVGL_AXP2101_ADC_Data`.

---

## 02_PCF85063 – RTC PCF85063

- **Pasta:** `ESP-IDF-v5.3.2/02_PCF85063/`  
- **Descrição:**  
  Exemplo para o **relógio de tempo real PCF85063** usando ESP-IDF.

- **Arquivos principais:**
  - `main/pcf85063.c` – implementação do exemplo.  
  - `sdkconfig` / `sdkconfig.ci` – configurações para compilar e rodar.

- **Uso básico:**

```bash
cd ESP-IDF-v5.3.2/02_PCF85063
idf.py set-target esp32s3
idf.py -p PORT flash monitor
```

- **Relacionamento com os exemplos Arduino:**
  - Em Arduino, o mesmo RTC é demonstrado em:
    - `05_GFX_PCF85063_simpleTime` (GFX)  
    - `10_LVGL_PCF85063_simpleTime` (LVGL)

---

## 03_QMI8658 – IMU QMI8658

- **Pasta:** `ESP-IDF-v5.3.2/03_QMI8658/`  
- **Descrição:**  
  Exemplo para o **IMU QMI8658** (acelerômetro + giroscópio).

- **Arquivos principais:**
  - `components/SensorLib/` – driver SensorLib para o QMI8658.  
  - `main/qmi8658c.cpp` – código de exemplo.  
  - `sdkconfig` / `sdkconfig.ci`.

- **Uso básico:**

```bash
cd ESP-IDF-v5.3.2/03_QMI8658
idf.py set-target esp32s3
idf.py -p PORT flash monitor
```

- **Relacionamento com os exemplos Arduino:**
  - Em Arduino, o IMU QMI8658 é usado em  
    `11_LVGL_QMI8658_ui`.

---

## 04_SD_MMC – SD Card (SDMMC)

- **Pasta:** `ESP-IDF-v5.3.2/04_SD_MMC/`  
- **Descrição:**  
  Exemplo oficial do ESP-IDF para uso de **SD card** via **SDMMC**.

- **Arquivo importante:**  
  - `README.md` – muito completo, cobrindo:
    - Pinout padrão para ESP32, ESP32-S3, ESP32-P4.  
    - Diferença entre modo 1-line e 4-line.  
    - Considerações sobre GPIO2 / GPIO12 na família ESP32.  
    - Como formatar e montar o cartão (FAT/FAT32).  
    - Mensagens de log típicas e troubleshooting.

- **Fluxo do exemplo:**
  1. Inicializa SDMMC e monta o sistema de arquivos FAT.  
  2. Mostra informações do cartão (nome, tipo, capacidade, frequência).  
  3. Cria um arquivo, escreve e lê dados.  
  4. (Opcional) formata o cartão se a montagem falhar.

- **Uso básico:**

```bash
cd ESP-IDF-v5.3.2/04_SD_MMC
idf.py set-target esp32s3
idf.py -p PORT flash monitor
```

- **Relacionamento com os exemplos Arduino:**
  - Em Arduino, o SD card é testado em  
    `14_LVGL_SD_Test`.

---

## 05_LVGL_WITH_RAM – Display QSPI + LVGL

- **Pasta:** `ESP-IDF-v5.3.2/05_LVGL_WITH_RAM/`  
- **Descrição:**  
  Exemplo de **display QSPI com RAM interna** (controladores suportados: SPD2010, GC9B71, SH8601) integrado ao **LVGL** via `esp_lcd`.

- **Arquivo importante:**  
  - `README.md` – explica:
    - Displays suportados (SPD2010, GC9B71, SH8601).  
    - Como o `esp_lcd` permite drivers de painel fora do IDF.  
    - Uso de `esp_timer` para gerar ticks do LVGL.  
    - Uso de mutex para garantir acesso thread-safe ao LVGL.  
    - Exemplo de conexão entre a placa ESP e o display QSPI (CS, CLK, IO0–IO3, RST, I2C para touch).

- **Fluxo do exemplo:**
  - Inicializa barramento QSPI e driver do display.  
  - Inicializa LVGL e registra o driver de display.  
  - Inicia uma task dedicada para `lv_timer_handler()`.  
  - Apresenta uma **animação/GUI** no display.

- **Uso básico:**

```bash
cd ESP-IDF-v5.3.2/05_LVGL_WITH_RAM
idf.py set-target esp32s3
idf.py -p PORT flash monitor
```

- **Relacionamento com os exemplos Arduino:**
  - Correspondente “baixo nível” dos exemplos LVGL em  
    `Arduino-v3.1.0/examples/08` a `examples/16`, que fazem LVGL usando o core Arduino em vez de ESP-IDF puro.

---

## 06_I2SCodec – Codec ES8311 (I2S)

- **Pasta:** `ESP-IDF-v5.3.2/06_I2SCodec/`  
- **Descrição:**  
  Exemplo de áudio com o **codec ES8311**, usando a interface **I2S**.

- **Arquivo importante:**  
  - `README.md` – descreve:
    - Funcionalidades do ES8311 (ADC/DAC, SNR, THD+N etc.).  
    - Modo **music**: toca o arquivo PCM embutido (`canon.pcm`).  
    - Modo **echo**: o áudio capturado pelo microfone é reproduzido em tempo real no fone.  
    - Conexões de hardware detalhadas (pinos de I2S, I2C, alimentação).  
    - Como personalizar sua própria música (via `ffmpeg` convertendo para `.pcm`).

- **Arquivos principais:**
  - `main/i2s_es8311_example.c` – código do exemplo.  
  - `main/canon.pcm` – áudio exemplo.  
  - `main/example_config.h`, `main/idf_component.yml` – configuração de pinos e dependências.

- **Uso básico:**

```bash
cd ESP-IDF-v5.3.2/06_I2SCodec
idf.py set-target esp32s3
idf.py menuconfig        # escolher modo music/echo, ajustar pinos se necessário
idf.py -p PORT flash monitor
```

- **Relacionamento com os exemplos Arduino:**
  - Em Arduino, o codec ES8311 é demonstrado em  
    `Arduino-v3.1.0/examples/15_ES8311`.

---

## Dica: ligando Arduino e ESP-IDF mentalmente

Uma maneira prática de organizar seu estudo:

1. Escolha um periférico (por exemplo, **RTC**).  
2. Rode primeiro um **exemplo Arduino** (mais simples, `.ino`).  
3. Depois, rode o **exemplo ESP-IDF** correspondente para ver uma implementação mais detalhada.

Por exemplo:

- **RTC PCF85063**
  - Arduino: `05_GFX_PCF85063_simpleTime` / `10_LVGL_PCF85063_simpleTime`  
  - ESP-IDF: `02_PCF85063`

- **IMU QMI8658**
  - Arduino: `11_LVGL_QMI8658_ui`  
  - ESP-IDF: `03_QMI8658`

- **SD Card**
  - Arduino: `14_LVGL_SD_Test`  
  - ESP-IDF: `04_SD_MMC`

- **Áudio ES8311**
  - Arduino: `15_ES8311`  
  - ESP-IDF: `06_I2SCodec`

---

Se você quiser, posso ajudar a:

- Ajustar qualquer projeto destes para o **seu pinout específico**.  
- Criar um **novo projeto ESP-IDF** combinando vários periféricos (por exemplo, `LVGL + IMU + RTC + AXP2101`, no estilo dos projetos do VolosR).