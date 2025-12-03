# WAVE PWN v2 — MANUAL DO USUÁRIO

Bem-vindo ao **WavePwn v2 — CyberGuard Pro**, um dispositivo de defesa Wi‑Fi
portátil, 100% ético, projetado para monitorar seu ambiente e alertar você
sobre riscos em redes sem fio usando a IA **NEURA9**.

---

## 1. Visão geral

- Hardware-alvo: ESP32‑S3 com display touch AMOLED 1.8"
- Foco: defesa, monitoramento, detecção de ataques e más configurações
- Processamento 100% local, sem envio de dados para a nuvem
- Integração opcional com:
  - Alexa (via fauxmoESP)
  - Google Home (via Home Assistant)
  - Dashboard web em tempo real
- IA defensiva **NEURA9** com 10 classes de risco diferentes

---

## 2. Estrutura de arquivos principal

No diretório `WavePwn/` você encontrará:

- `src/` — código-fonte do firmware
- `src/config/device_config.json` — configuração do dispositivo
- `src/assistants/` — integração com Alexa / Google Home
- `src/neura9/` — inferência da IA NEURA9
- `data/web/` — dashboard web (HTML/CSS/JS) embutido no firmware
- `docs/` — documentação (este manual, guia do desenvolvedor, etc.)
- `ai_training/` — scripts e notebooks para treino da IA

---

## 3. Primeiro uso

### 3.1 Gravação do firmware

1. Instale o [PlatformIO](https://platformio.org/) (CLI ou extensão VS Code).
2. Abra a pasta `WavePwn/` como projeto PlatformIO.
3. Conecte o ESP32‑S3 via USB.
4. No terminal do PlatformIO, execute:

   ```bash
   pio run -t upload
   ```

5. Aguarde o flash terminar; o dispositivo irá reiniciar automaticamente.

### 3.2 Preparando o microSD

1. Formate um cartão microSD com **FAT32**.
2. Crie as seguintes pastas na raiz do cartão (se ainda não existirem):

   - `/sd`
   - `/sd/wavepwn`
   - `/sd/wavepwn/handshakes`
   - `/sd/wavepwn/pmkid`
   - `/sd/wavepwn/sae`
   - `/sd/wavepwn/logs`
   - `/sd/wavepwn/session`
   - `/sd/lang`
   - `/sd/reports`
   - `/config`

3. Copie o arquivo de configuração de exemplo:

   - `WavePwn/src/config/device_config.json` → `/config/device_config.json`

4. Insira o cartão microSD no dispositivo e ligue-o.

---

## 4. Configuração do dispositivo

Todas as personalizações principais são feitas em:

```text
/config/device_config.json
```

Exemplo de conteúdo:

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

### 4.1 Como mudar o nome do dispositivo

1. Monte o microSD no computador.
2. Edite `/config/device_config.json` e altere o campo:

   ```json
   "device_name": "Seu Nome Aqui"
   ```

3. Salve o arquivo e ejete o cartão com segurança.
4. Reinsira o microSD no WavePwn e reinicie o dispositivo.

O novo nome aparecerá:

- No rosto do WavePwn (HUD principal).
- Nas integrações com Alexa / Google Home (como nome do dispositivo virtual).

### 4.2 Como escolher Alexa ou Google Home

No mesmo arquivo `device_config.json`, defina:

```json
"assistant": "alexa"
```

Valores possíveis:

- `"alexa"` — integra apenas com Alexa
- `"google"` — integra apenas com Google Home (via Home Assistant)
- `"both"` — habilita os dois
- `"none"` — desativa integrações de voz

Após alterar, reinicie o dispositivo.

### 4.3 Idioma e tema

- `"language"`: código de idioma, por exemplo:
  - `"pt-BR"`
  - `"en-US"`
- `"theme"`: modo de exibição, atualmente:
  - `"dark"` (padrão)
  - `"light"` (comportamento futuro, via `switch_theme`)

---

## 5. Dashboard Web

O WavePwn expõe um dashboard web em tempo real via Wi‑Fi.

### 5.1 Acessando o dashboard

1. Após ligar, o dispositivo cria um Access Point:

   - SSID: `WavePwn` (ou o nome definido em `PET_NAME` via `config.h`)
   - Senha: `wavepwn`

2. Conecte seu computador ou smartphone a essa rede.
3. No navegador, acesse:

   ```text
   http://192.168.4.1/
   ```

   (ou o IP exibido no log serial).

### 5.2 O que o dashboard mostra

- **Uptime** — tempo desde o boot
- **Bateria** — percentual estimado e barra animada
- **Redes Vistas (APS)** — quantidade de APs únicos
- **Handshakes** — WPA2 4-way capturados
- **PMKID** — capturas no formato Hashcat 16800
- **NEURA9** — classe atual de ameaça (SAFE, HIGH_RISK, etc.)
- **Threat Level Timeline** — gráfico em tempo real usando Chart.js minimalista
- **Event log** — stream de logs do WavePwn em tempo real

### 5.3 Atualização OTA segura

No dashboard:

1. Clique em **OTA UPDATE**.
2. Autentique com:
   - Usuário: `admin`
   - Senha: `wavepwn`
3. Selecione o arquivo firmware `.bin` gerado pelo PlatformIO.
4. Aguarde a mensagem de sucesso; o dispositivo será reiniciado.

A camada `utils/ota_secure.cpp` foi preparada para endurecimento futuro
(ver `docs/SECURITY_POLICY.md`).

---

## 6. Integração com Alexa

> Requer que o ESP32‑S3 esteja na mesma rede que o dispositivo Alexa.

### 6.1 Pré-requisitos

- Rede Wi‑Fi com acesso local entre o ESP32‑S3 e o Echo / Alexa.
- Biblioteca `FauxmoESP` já incluída no projeto (via `platformio.ini`).

### 6.2 Como funciona

- O módulo `src/assistants/alexa.cpp` usa a biblioteca **fauxmoESP**.
- Um dispositivo virtual com o nome definido em `device_config.json` é criado.
- Um segundo dispositivo virtual é criado para acionar a geração de relatórios.

### 6.3 Comandos de voz (Alexa)

Depois de descobrir o dispositivo na Alexa, você pode usar:

- `Alexa, ligar o [nome do dispositivo]`
- `Alexa, desligar o [nome do dispositivo]`
- `Alexa, ligar o [nome do dispositivo] relatório`  
  → gera um relatório de segurança em PDF simplificado em `/sd/reports/`.

Sugestões:

- `Alexa, ligar o CyberGuard Pro`
- `Alexa, pedir relatório do CyberGuard Pro`

### 6.4 Status pelo assistente

Periodicamente o firmware chama `assistantManager.send_status()`, que gera
uma frase texto do tipo:

> Status de CyberGuard Pro: ambiente SAFE, confiança 0.97, bateria 84%.

Essa frase pode ser usada em automações/rotinas no app da Alexa.

---

## 7. Integração com Google Home

A integração com o Google Home é feita de forma leve, usando o Home Assistant
como ponte opcional.

### 7.1 Pré-requisitos

- Instância do Home Assistant acessível na rede.
- Configuração da integração do Home Assistant com o Google Home.

### 7.2 Como funciona

- O módulo `src/home_assistant.cpp` publica sensores no Home Assistant:
  - `sensor.wavepwn_threat_level`
  - `sensor.wavepwn_battery`
  - `binary_sensor.wavepwn_deauth_attack`
- O módulo `src/assistants/google_home.cpp`:
  - Inicializa o nome amigável do dispositivo.
  - Envia mensagens de status via log (podem ser associadas a automações).
  - Chama `ha_send_threat()` com o rótulo da classe NEURA9 vigente.

### 7.3 Configuração básica

1. Habilite `HA_ENABLED` e defina `HA_BASE_URL` e `HA_TOKEN` em `platformio.ini`
   ou `config.h`. Exemplo:

   ```ini
   build_flags =
       -DHA_ENABLED=1
       -DHA_BASE_URL=\"http://homeassistant.local:8123\"
       -DHA_TOKEN=\"SEU_LONG_LIVED_ACCESS_TOKEN\"
   ```

2. No Home Assistant, crie entidades/automations que exponham esses sensores
   para o Google Home.

### 7.4 Exemplos de uso

- Criar uma automação que anuncia no Google Home:

  - Quando `sensor.wavepwn_threat_level` mudar para `DEAUTH_DETECTED`
  - E `binary_sensor.wavepwn_deauth_attack` estiver em `ON`

---

## 8. NEURA9 – IA defensiva local

A **NEURA9** é uma rede neural leve, executada 100% localmente via
TensorFlow Lite Micro. Ela classifica o ambiente em 10 classes:

0. SAFE  
1. CROWDED  
2. OPEN_NETWORK  
3. EVIL_TWIN_RISK  
4. DEAUTH_DETECTED  
5. ROGUE_AP  
6. HIGH_RISK  
7. BATTERY_CRITICAL  
8. GESTURE_COMMAND  
9. LEARNING_MODE  

### 8.1 O que ela considera

As features atuais incluem:

- Número de APs vistos
- Handshakes capturados
- PMKID detectados
- Contagem de deauths
- Percentual de bateria
- Estado de carga (carregando ou não)
- Detecção de movimento básico
- Uptime aproximado
- Canal Wi‑Fi atual

### 8.2 Reação visual

A NEURA9 influencia o humor do WavePwn:

- SAFE → rosto feliz
- CROWDED → rosto “cool”
- EVIL_TWIN_RISK → rosto chocado
- DEAUTH_DETECTED → rosto irritado
- HIGH_RISK → rosto triste
- BATTERY_CRITICAL → avisos de bateria
- GESTURE_COMMAND / LEARNING_MODE → expressões especiais

### 8.3 Treinamento

O treinamento completo é descrito em:

- `docs/NEURA9_TRAINING_TUTORIAL.md`
- `ai_training/neura9_full_training.ipynb`

---

## 9. Relatórios em PDF

O WavePwn gera relatórios simplificados em `/sd/reports/` usando:

- `reports/tiny_pdf.h`
- `src/utils/pdf_report.cpp`

### 9.1 Como gerar manualmente

- Via Alexa: comando de voz associado ao dispositivo “relatório”.
- Via código: chamar `generate_pdf_report();`

O arquivo conterá:

- Contagem de handshakes
- Número de ameaças detectadas
- Percentual de bateria no momento

---

## 10. Easter Egg — MODO MESTRE

Existe um Easter Egg avançado, inspirado no “código Konami”:

1. No teclado virtual (ou via input remoto), insira a sequência:
   - Cima, Cima, Baixo, Baixo, Esquerda, Direita, Esquerda, Direita, B, A
2. Toque e segure por **5 segundos** o olho esquerdo do WavePwn.
3. Crie um arquivo vazio em `/sd/.master`.

Quando todas as condições forem satisfeitas, o MODO MESTRE é desbloqueado
(`ui_show_secret_menu()` exibe um painel especial). Este modo destina-se
a ajustes avançados e experimentos futuros.

---

## 11. Boas práticas de uso

- Use o WavePwn apenas em redes **que você administra** ou onde você tem
  autorização explícita para monitorar.
- Nunca utilize os recursos de captura para fins ofensivos.
- Mantenha o firmware e o cartão microSD cifrados e sob seu controle.
- Leia atentamente `docs/SECURITY_POLICY.md` para entender o posicionamento
  ético do projeto.

---

## 12. Solução de problemas (FAQ)

**O dashboard não abre.**

- Verifique se você está conectado ao AP do WavePwn.
- Confira o IP no log serial.
- Certifique-se de que nenhuma VPN/firewall local esteja bloqueando o acesso.

**A NEURA9 sempre mostra SAFE.**

- Em ambientes muito “limpos” é normal.
- Gere dados mais variados (lugares diferentes, horários diferentes).
- Veja o tutorial de treinamento para ajustar o modelo.

**Alexa não descobre o dispositivo.**

- Verifique se:
  - ESP32‑S3 e Echo estão na mesma rede.
  - A porta 80 está liberada na sua rede local.
- Repare que integrações de voz podem variar conforme a geração do dispositivo
  Alexa e as características da rede.

---

## 13. Próximos passos

- Personalize o `device_name` e o `owner`.
- Configure a integração com Home Assistant / Google Home.
- Colete logs reais e treine a NEURA9 com seus próprios dados.
- Compartilhe melhorias, dashboards, skins de UI e modelos de IA defensiva.

O WavePwn foi pensado para ser uma plataforma de defesa aberta,
auditável e extensível. Aproveite, experimente, e mantenha sempre o foco
na **segurança ética**.