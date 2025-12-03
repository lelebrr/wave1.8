# CHANGELOG — WavePwn v2 / NEURA9

Este arquivo registra mudanças relevantes no projeto WavePwn v2,
com foco na IA NEURA9, integrações e arquitetura geral.

---

## [2.0.0] — ETAPA 7 DEFINITIVA

### Destaques

- Reorganização final do projeto WavePwn v2.
- Integração dinâmica com **Alexa** e **Google Home**.
- Nome do dispositivo e parâmetros da NEURA9 totalmente configuráveis.
- Documentação expandida (USER_MANUAL, DEVELOPER_GUIDE, SECURITY_POLICY, NEURA9_TRAINING_TUTORIAL).
- Estrutura de repositório e microSD mais limpa e consistente.

### Repositório e estrutura

- Removidas dependências/SDKs locais redundantes (Arduino/ESP-IDF/Firmware)
  da árvore principal do projeto (mantidos apenas quando estritamente necessários
  fora do firmware).
- Padronizada a estrutura interna do projeto `WavePwn/`:
  - `src/` centraliza o código do firmware:
    - `main.cpp` substitui `WavePwn.ino` como ponto de entrada.
    - `ui.cpp` e `ui.h` movidos para `src/`.
    - Diretório `src/neura9/` consolidando inferência da NEURA9.
    - Diretório `src/assistants/` para Alexa/Google e gerenciador de assistentes.
    - Diretório `src/utils/` para OTA seguro e geração de relatórios PDF.
  - `data/web/` passa a abrigar os assets do dashboard:
    - `index.html`, `style.css`, `chart.min.js`, `favicon.ico`.
  - `docs/` contém toda a documentação principal.
  - `ai_training/` preparado para notebooks e scripts de treinamento.

### Boot / entrypoint

- Introduzido `src/main.cpp` com:

  ```cpp
  Pwnagotchi pwn;
  void setup() { pwn.begin(); }
  void loop()  { pwn.update(); }
  ```

- Removido `WavePwn.ino` para evitar dupla definição de `setup()`/`loop()`.

### NEURA9

- Pastas reorganizadas:

  - `src/neura9/inference.{h,cpp}`
  - `src/neura9/features.cpp`
  - `src/neura9/model.{h,tflite}`
  - Assets de treino mantidos em `ai/`.

- Adicionado wrapper `src/neura9/model.h` para externs do modelo TFLite:

  ```cpp
  extern const unsigned char neura9_defense_model_tflite[];
  extern const unsigned int  neura9_defense_model_tflite_len;
  ```

- Mantido `ai/neura9_defense_model_data.{h,cpp}` como fonte real de dados,
  facilitando a regeneração via scripts de treino.
- Atualizado `src/neura9/inference.cpp` para usar o novo header `neura9/model.h`.

### Assistentes de voz

- Novo módulo `src/assistants/assistant_manager.{h,cpp}`:

  - Lê `/config/device_config.json` (copiado a partir de `src/config/device_config.json`).
  - Campos:
    - `device_name`
    - `assistant` (`"alexa"`, `"google"`, `"both"`, `"none"`)
    - `language`
    - `theme`
    - `owner`
    - `enable_voice_alerts`
    - `neura9_sensitivity`
  - Expõe:
    - `assistantManager.begin()`
    - `assistantManager.speak(const char*)`
    - `assistantManager.send_status()`
    - getters para nome, tipo de assistente e sensibilidade.

- Integração no ciclo de vida:

  - `Pwnagotchi::begin()` chama `assistantManager.begin()` após `initWiFiMonitor()`.

- Alexa:

  - `src/assistants/alexa.{h,cpp}` usando **fauxmoESP**.
  - Criação de dispositivos virtuais:
    - `[device_name]`
    - `[device_name] relatório`
  - Callback de estado gera relatórios simplificados em `/sd/reports/`
    via `generate_report()` quando o dispositivo de relatório é ativado.

- Google Home:

  - `src/assistants/google_home.{h,cpp}` integra com Home Assistant:
    - `google_send_status()` publica estado via `ha_send_threat()`.
  - Design pensado para uso com automações HA + Google Home.

### UI e nome do dispositivo

- `ui.cpp` agora inclui `assistants/assistant_manager.h`.
- `create_hud()` passa a buscar o nome do dispositivo em
  `assistantManager.getDeviceName()`; fallback para `PET_NAME` ou
  `"WAVE PWN"` quando o nome dinâmico não estiver disponível.

### OTA seguro

- Introduzido `src/utils/ota_secure.{h,cpp}`:

  - Funções:
    - `ota_begin_secure(size_t size)`
    - `ota_write_chunk(const uint8_t *data, size_t len)`
    - `ota_finalize(bool reboot)`
  - Encapsula chamadas à API `Update` do ESP32.
  - Facilita a futura inclusão de:
    - Verificação de hash do binário.
    - Assinatura digital.

- `src/webserver.cpp` atualizado para usar essas funções no fluxo OTA.

### Geração de relatórios

- Novo wrapper `src/utils/pdf_report.{h,cpp}`:

  - `generate_pdf_report()` chama `generate_report()` de `reports/tiny_pdf.h`.
  - Usado, por exemplo, no fluxo Alexa.

- `tiny_pdf.h` atualizado para incluir mais informações no relatório
  (bateria, contagem de ameaças).

### Dashboard web

- Assets HTML/CSS/JS movidos para `data/web/`.
- `platformio.ini` atualizado:

  ```ini
  board_build.embed_files =
      data/web/index.html,
      data/web/style.css,
      data/web/chart.min.js,
      data/web/favicon.ico,
      ota/update.html
  ```

- `src/webserver.cpp` atualizado para usar os novos símbolos embutidos
  (`_binary_data_web_*`).

### Dependências

- `platformio.ini`:

  - Adicionadas:
    - `FauxmoESP`
    - `AsyncTCP`
  - Mantidas todas as demais libs existentes.

### Documentação

- `docs/USER_MANUAL.md`
  - Guia completo para usuário final.
  - Explica:
    - Mudança de nome do dispositivo.
    - Seleção de assistentes de voz.
    - Dashboard web e OTA.
    - Easter egg / Modo Mestre.

- `docs/DEVELOPER_GUIDE.md`
  - Arquitetura completa do firmware.
  - Entrypoint, fluxo de inicialização, módulos e extensões.

- `docs/NEURA9_TRAINING_TUTORIAL.md`
  - Passo a passo de treinamento:
    - Coleta de dados.
    - Montagem de dataset.
    - Uso de `neura9_trainer.py`.
    - Uso de `ai_training/neura9_full_training.ipynb`.
    - Exportação para TFLite e C.

- `docs/SECURITY_POLICY.md`
  - Declaração de princípios éticos.
  - Medidas de segurança implementadas.
  - Recomendações para extensões.

---

## [1.x.x] — Histórico anterior (resumo)

> Nota: versões anteriores da linha 1.x.x focaram em:
>
> - Implementação inicial do Pwnagotchi v2.
> - UI LVGL com rostos, partículas e Easter Egg Konami.
> - Integração básica com ESP32Marauder para captura.
> - Primeiro esqueleto da NEURA9.
> - Primeiras versões do dashboard web em `/data/`.

Detalhamento fino das releases 1.x.x pode ser reconstruído a partir
do histórico de commits do repositório original.

---

Este changelog continuará a ser atualizado conforme o projeto evoluir
para novas versões (3.x, módulos adicionais de IA, drivers avançados
de display, etc.).