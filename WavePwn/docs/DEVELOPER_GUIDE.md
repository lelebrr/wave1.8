# WavePwn v2 — DEVELOPER GUIDE

Este guia é destinado a quem deseja **estender, portar ou auditar**
o firmware do WavePwn. Ele assume familiaridade com C++ moderno,
PlatformIO e desenvolvimento para ESP32.

---

## 1. Arquitetura geral

### 1.1 Visão em camadas

- **Camada de hardware / drivers**
  - ESP32‑S3, Wi‑Fi, BLE, PSRAM
  - Display (futuro driver LovyanGFX + LVGL)
  - Sensores (IMU, power management, RTC)

- **Camada de serviços**
  - Captura de pacotes (`src/capture.cpp`)
  - Sniffer Wi‑Fi (`wifi_sniffer.cpp` / ESP32Marauder)
  - Armazenamento em microSD (`storage.cpp`, `reports/`)

- **Camada de IA / análise**
  - NEURA9 (`src/neura9/`)
  - BLE PwnGrid (`ble_grid/`)

- **Camada de integração**
  - Webserver + WebSocket (`src/webserver.cpp`)
  - Home Assistant / Google Home (`src/home_assistant.cpp`)
  - Alexa (`src/assistants/alexa.cpp`)
  - Assistente dinâmico (`src/assistants/assistant_manager.cpp`)

- **Camada de UI**
  - LVGL + animações (`src/ui.cpp`)
  - Easter egg Konami / Modo Mestre (`easter_egg/`)

- **Camada de aplicação**
  - `Pwnagotchi` (`pwnagotchi.{h,cpp}`)
  - `main.cpp` (antigo `WavePwn.ino`)

---

## 2. Entrypoint e ciclo de vida

### 2.1 main.cpp

Arquivo: `src/main.cpp`

```cpp
#include <Arduino.h>
#include "pwnagotchi.h"

Pwnagotchi pwn;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=== WAVE PWN v2.0 - O RENASCIMENTO ===");
  pwn.begin();
}

void loop() {
  pwn.update();
}
```

### 2.2 Pwnagotchi::begin()

Arquivo: `pwnagotchi.cpp`

Principais etapas:

1. `anti_tamper_check()` — verificação de secure boot.
2. `initDisplay()` + `showBootAnimation()` — stub de display (LGFX).
3. `initSD()` — estrutura de pastas em `/sd`.
4. `capture_init()` — motor de captura de handshakes/PMKID.
5. `initSensors()` — sensores básicos (movimento / wake).
6. `initWiFiMonitor()` — modo promíscuo + AP.
7. Callback `esp_wifi_set_promiscuous_rx_cb` → `capture_packet_handler`.
8. `assistantManager.begin()` — lê `device_config.json` e configura assistentes.
9. `lv_init()` + `ui_init()` — inicializa LVGL e UI.
10. Tema inicial e idioma (`switch_theme(true)`, `load_language("pt-BR")`).
11. `show_premium_boot()` — animação de boot.
12. `neura9.begin()` — IA defensiva local.
13. `pwnGrid.begin()` — BLE PwnGrid cooperativo.
14. `webserver_start()` — dashboard web + OTA.
15. `ha_init()` — integração Home Assistant opcional.

### 2.3 Pwnagotchi::update()

- Atualiza `uptime`.
- Chama `ha_loop()` (NTP, timers do HA).
- Normaliza `battery_percent`.
- Entra em **Modo ZUMBI** se bateria ≤ 1%.
- Atualiza HUD (`ui_update_stats()`).
- A cada ~800ms:
  - `neura9.predict()` → `threat_level` + `threat_confidence`
  - Incrementa `threat_count` quando `cls > 0`
  - `pwnGrid.share_threat_level(cls)`
  - `ha_send_threat(NEURA9_THREAT_LABELS[cls])`
- Alterna tema dark/light a cada 10 min.
- Envia stats para o dashboard (`webserver_send_stats()`).
- A cada 60s: `assistantManager.send_status()`.
- Roda LVGL (`lv_timer_handler()`).

---

## 3. Estrutura do diretório src/

```text
src/
├── main.cpp
├── ui.cpp / ui.h
├── neura9/
│   ├── inference.cpp / inference.h
│   ├── features.cpp
│   ├── model.tflite
│   └── model.h
├── assistants/
│   ├── alexa.cpp / alexa.h
│   ├── google_home.cpp / google_home.h
│   └── assistant_manager.cpp / assistant_manager.h
├── config/
│   └── device_config.json
└── utils/
    ├── pdf_report.cpp / pdf_report.h
    └── ota_secure.cpp / ota_secure.h
```

Outros diretórios relevantes na raiz de `WavePwn/`:

- `ble_grid/` — BLE PwnGrid.
- `reports/` — `tiny_pdf.h`.
- `anti_tamper/` — `secure_boot.h`.
- `ai/` — assets de treino (TFLite e array C).
- `data/` — dashboard web (`data/web/`).
- `ota/` — página HTML de atualização.

---

## 4. Assistentes de voz

### 4.1 AssistantManager

Arquivo: `src/assistants/assistant_manager.{h,cpp}`

Responsável por:

- Abrir `/config/device_config.json` no microSD.
- Carregar:

  ```json
  {
    "device_name": "CyberGuard Pro",
    "assistant": "alexa",
    "language": "pt-BR",
    "theme": "dark",
    "owner": "Seu Nome",
    "enable_voice_alerts": true,
    "neura9_sensitivity": 0.78
  }
  ```

- Inicializar:

  - `alexa_init(device_name)` quando `assistant` contém `"alexa"`.
  - `google_init(device_name)` quando `assistant` contém `"google"`.

- Unificar chamadas de voz:

  ```cpp
  assistantManager.speak("ameaça detectada");
  assistantManager.send_status();
  ```

### 4.2 Alexa (fauxmoESP)

Arquivo: `src/assistants/alexa.cpp`

- Usa `fauxmoESP` para criar dispositivos virtuais Alexa.
- Cria:

  - Um dispositivo com o nome do WavePwn (ex.: `"CyberGuard Pro"`).
  - Um dispositivo adicional com `"relatório"` no nome.

- Callback principal:

  ```cpp
  fauxmo.onSetState([](unsigned char id,
                       const char *device_name,
                       bool state,
                       unsigned char value) {
      if (strstr(device_name, "relatório")) {
          if (state) generate_report();
      }
  });
  ```

- `alexa_speak()` e `alexa_send_status()` são wrappers para logs e para
  uso em automações Alexa.

> Observação: integração fina com diferentes gerações de dispositivos Alexa
> pode exigir ajustes de porta e/ou servidor HTTP externo (ver exemplos
> `fauxmoESP_External_Server` na documentação da biblioteca).

### 4.3 Google Home

Arquivo: `src/assistants/google_home.cpp`

- Inicializa com um nome amigável.
- Usa Home Assistant como ponte:
  - `google_send_status()` chama `ha_send_threat()` com o rótulo atual.
- Focado em logs e automações no HA/Google Home.

---

## 5. NEURA9

### 5.1 Inferência

Arquivos:

- `src/neura9/inference.h`
- `src/neura9/inference.cpp`
- `src/neura9/features.cpp`
- `src/neura9/model.h`
- `ai/neura9_defense_model_data.{h,cpp}`

Pontos importantes:

- Usa TensorFlow Lite Micro (`TensorFlowLite_ESP32`).
- Arena de 160 KB em PSRAM:

  ```cpp
  uint8_t tensor_arena[160 * 1024];
  ```

- Carrega o modelo via:

  ```cpp
  const tflite::Model* model = tflite::GetModel(neura9_defense_model_tflite);
  ```

- `extract_features()` popula um vetor `float features[72]` a partir de
  `Pwnagotchi` (APS, handshakes, deauths, bateria, etc.).
- `predict()`:
  - Copia as features para o tensor de entrada.
  - Chama `interpreter->Invoke()`.
  - Escolhe a classe de maior confiança.
  - Ajusta o humor do WavePwn via `ui_set_mood()`.

### 5.2 Treino

Ver:

- `ai/neura9_trainer.py`
- `docs/NEURA9_TRAINING_TUTORIAL.md`
- `ai_training/` (notebook + scripts de exportação).

---

## 6. Webserver e dashboard

Arquivo: `src/webserver.cpp`

### 6.1 Embedding de assets

Em `platformio.ini`:

```ini
board_build.embed_files =
    data/web/index.html,
    data/web/style.css,
    data/web/chart.min.js,
    data/web/favicon.ico,
    ota/update.html
```

No código:

```cpp
extern const uint8_t data_web_index_html_start[] asm("_binary_data_web_index_html_start");
extern const uint8_t data_web_index_html_end[]   asm("_binary_data_web_index_html_end");
// idem para style.css, chart.min.js, favicon.ico
```

`serve_embedded()` escreve diretamente o conteúdo da flash para o cliente HTTP.

### 6.2 WebSocket

- Porta 80 para HTTP (`WebServer`).
- Porta 81 para WebSocket (`WebSocketsServer`).
- `webserver_send_stats()`:
  - Chama `neura9.predict()` se houver clientes conectados.
  - Envia JSON com uptime, bateria, APS, handshakes, PMKID, AI e log.

---

## 7. OTA seguro

Arquivos:

- `src/utils/ota_secure.{h,cpp}`
- `src/webserver.cpp` (handlers HTTP para OTA)

Funções expostas:

```cpp
bool ota_begin_secure(size_t size);
bool ota_write_chunk(const uint8_t *data, size_t len);
bool ota_finalize(bool reboot);
```

No handler de upload:

```cpp
if (upload.status == UPLOAD_FILE_START) {
    ota_begin_secure(UPDATE_SIZE_UNKNOWN);
} else if (upload.status == UPLOAD_FILE_WRITE) {
    ota_write_chunk(upload.buf, upload.currentSize);
} else if (upload.status == UPLOAD_FILE_END) {
    ota_finalize(true);
}
```

Este design prepara o código para integrar:

- Verificação de hash/assinatura do firmware.
- Limites de tamanho.
- Políticas mais rígidas (ver `docs/SECURITY_POLICY.md`).

---

## 8. Estrutura do microSD

Criada em `Pwnagotchi::initSD()`:

```text
/sd/
/sd/wavepwn/
/sd/wavepwn/handshakes/
/sd/wavepwn/pmkid/
/sd/wavepwn/sae/
/sd/wavepwn/logs/
/sd/wavepwn/session/
/sd/lang/
/sd/reports/
/config/
```

Arquivos típicos:

- `/config/device_config.json`
- `/sd/reports/relatorio_*.pdf` (texto com extensão `.pdf`)
- `/sd/lang/pt-BR.json`, `/sd/lang/en-US.json` etc.

---

## 9. Padrões de código

- C++17 (na prática, subset suportado pelo toolchain do ESP32).
- Evitar `new`/`delete` quando possível; preferir buffers estáticos.
- Evitar exceções (`-fno-exceptions` é comum em toolchains embedded).
- Uso de `String` é aceitável em pontos de alto nível, com cuidado.

Convenções:

- Namespaces não utilizados; código organizado por módulos.
- Headers com `#pragma once`.
- Comentários breves e focados em contexto, não em óbvio.

---

## 10. Como adicionar novas features

### 10.1 Novo sensor

1. Crie um módulo em `sensors.cpp` ou um arquivo dedicado.
2. Inicialize em `Pwnagotchi::initSensors()`.
3. Exponha os valores em `Pwnagotchi` (ex.: `float ambient_noise_db;`).
4. Integre na NEURA9:
   - Adicione a feature em `Neura9::extract_features()`.
   - Atualize o dataset e o modelo de treino.

### 10.2 Novo assistente de voz

1. Crie `src/assistants/meu_assistente.{h,cpp}`.
2. Exponha funções:

   ```cpp
   void my_assistant_init(const String &deviceName);
   void my_assistant_speak(const char *text);
   void my_assistant_send_status(const char *status);
   ```

3. Atualize `AssistantManager` para entender o novo valor em `assistant`.
4. Ajuste `device_config.json` conforme necessário.

### 10.3 Nova ação de relatório

- Use `generate_report()` ou `generate_pdf_report()` como base.
- Crie funções adicionais em `pdf_report.cpp` se precisar de formatos extras
  (ex.: JSON, CSV extensos, etc.).

---

## 11. Segurança e ética

- Leia `docs/SECURITY_POLICY.md` antes de adicionar qualquer funcionalidade
  relacionada a ataques, injeções ou fuzzing.
- A filosofia do WavePwn é **defensiva**:
  - Monitorar, alertar, registrar.
  - Nunca realizar ataques ativos em redes de terceiros.

---

## 12. Fluxo de desenvolvimento recomendado

1. Faça alterações em módulos isolados.
2. Compile com PlatformIO:

   ```bash
   cd WavePwn
   pio run
   ```

3. Use o monitor serial para validar:

   ```bash
   pio device monitor
   ```

4. Adicione logs claros com prefixos (`[NEURA9]`, `[WEB]`, `[HA]`, `[Alexa]`, etc.).
5. Ao mexer em NEURA9:
   - Ajuste o dataset.
   - Re-treine.
   - Regenere `neura9_defense_model_data.cpp`.

---

Este guia deve servir como mapa para navegar e evoluir o código do WavePwn
com segurança, clareza e respeito ao foco do projeto: **defesa ética**.