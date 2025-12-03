# SECURITY POLICY — WavePwn v2 / NEURA9

Este documento define os princípios de segurança, privacidade e ética
que orientam o desenvolvimento e o uso do WavePwn v2.

---

## 1. Objetivo do projeto

O WavePwn é um dispositivo de **defesa Wi‑Fi portátil**, projetado para:

- Monitorar o ambiente de rádio local.
- Detectar ataques e más configurações (deauth, APs abertos, etc.).
- Alertar o usuário de forma clara e responsável.
- Preservar a privacidade e a dignidade das pessoas ao redor.

Ele **não** é um brinquedo de ataque, nem uma ferramenta de “hacking
recreativo” fora de ambientes controlados.

---

## 2. Princípios éticos

1. **Consentimento e legitimidade**  
   Use o dispositivo apenas em redes:
   - Que você administra, ou
   - Para as quais você possui autorização explícita para monitoramento.

2. **Privacidade por design**  
   - O firmware não envia dados para a nuvem.
   - Nenhum payload de pacote é armazenado ou inspecionado para conteúdo.
   - Apenas metadados técnicos (endereços MAC, contadores, RSSI, etc.)
     são usados para análise.

3. **Minimização de dados**  
   - Capture apenas o que é necessário para deteção de risco.
   - Evite logs excessivos em longo prazo; estabeleça políticas de retenção.

4. **Transparência**  
   - Mensagens claras para o usuário sobre o que está sendo feito.
   - Facilitar auditoria do código (open source, documentação detalhada).

5. **Não causar dano intencional**  
   - Qualquer funcionalidade de ataque ativo (ex.: deauth) deve ser:
     - Desabilitada por padrão.
     - Limitada a ambientes de laboratório.
     - Claramente rotulada como perigosa/experimental.

---

## 3. Modelo de ameaça

### 3.1 Atacantes considerados

- Atacante passivo: observa o tráfego Wi‑Fi e tenta inferir informações.
- Atacante ativo: gera deauth, APs falsos (evil twin), etc.
- Atacante físico: tenta adulterar o dispositivo.

### 3.2 Ativos protegidos

- Integridade do firmware do WavePwn.
- Integridade dos relatórios.
- Confidencialidade da configuração do usuário (arquivos em `/config`).
- Reputação do projeto (evitar uso malicioso).

---

## 4. Medidas técnicas implementadas

### 4.1 Anti‑tamper / Secure Boot

- `anti_tamper/secure_boot.h` chama `esp_secure_boot_verify_signature()`
  quando compilado com `CONFIG_SECURE_BOOT` ativo.
- Em caso de falha:
  - O dispositivo entra em loop infinito e não continua a execução normal.
- Em builds de desenvolvimento (sem secure boot):
  - Apenas mensagens de log são exibidas para facilitar debugging.

### 4.2 Atualização OTA

- Implementada via `src/webserver.cpp` + `src/utils/ota_secure.cpp`.
- A lógica atual:
  - Usa a API padrão `Update` do ESP32.
  - Foi encapsulada em funções `ota_begin_secure()`, `ota_write_chunk()`,
    `ota_finalize()` para facilitar futura validação de imagem.
- Requisitos de uso seguro:
  - Proteger a página OTA com usuário/senha (`admin` / `wavepwn` por padrão).
  - Recomenda-se alterar credenciais em builds reais.
  - Considerar uso de rede isolada para atualização.

### 4.3 Armazenamento no microSD

- Estrutura em `/sd/wavepwn/` para capturas técnicas (handshakes, PMKID, logs).
- Diretório `/sd/reports/` para relatórios simplificados.
- Diretório `/config/` para arquivo de configuração (`device_config.json`).

Boas práticas:

- Se possível, cifrar o cartão microSD em sistemas externos.
- Não deixar o cartão exposto quando não estiver em uso.
- Evitar guardar dados por tempo maior que o necessário.

### 4.4 NEURA9 (IA defensiva)

- Executada localmente, em TFLite Micro, sem dependência de servidores externos.
- Modelo e dados são auditáveis (scripts em `ai/` e `ai_training/`).
- Sensibilidade ajustável via `"neura9_sensitivity"` em `device_config.json`.

---

## 5. Áreas de atenção e riscos conhecidos

1. **Conflito de portas / serviços de rede**  
   - O WavePwn expõe um AP Wi‑Fi, HTTP server, WebSocket e integração Alexa.
   - Ajustes incorretos em portas podem criar superfícies de ataque
     (ex.: endpoints expostos sem autenticação).
   - A página OTA é protegida com HTTP Basic Auth, mas isso não substitui
     TLS em redes potencialmente hostis.

2. **Exposição de logs**  
   - O dashboard exibe logs em tempo real e também pode registrá-los
     em `/sd/wavepwn/logs/`.
   - Logs podem conter informações sensíveis sobre o ambiente:
     - SSIDs, MACs, horários de atividade.
   - Policie o acesso ao dashboard e ao microSD.

3. **Integrações com serviços de terceiros**  
   - Home Assistant e Google Home, quando configurados, recebem estado
     do WavePwn (nível de ameaça, bateria, etc.).
   - Configurações fracas no HA podem resultar em exposição do estado
     da rede ou do dispositivo para a Internet.

4. **Atualizações de firmware de fontes não confiáveis**  
   - Nunca aplique firmwares de origem desconhecida via OTA.
   - Em builds de produção, recomenda-se:
     - Verificação de assinatura do binário.
     - Distribuição de firmware apenas por canais autenticados.

---

## 6. Recomendações para quem estende o projeto

### 6.1 Ao adicionar novas funcionalidades

- Evite qualquer ação ativa em redes de terceiros:
  - Deauth, jamming, spoofing, etc.
- Caso funcionalidades ofensivas sejam estritamente necessárias
  em laboratório:
  - Mantenha-as atrás de flags de compilação explícitas (`LAB_MODE`).
  - Documente claramente os riscos.
  - Nunca habilite por padrão em builds públicos.

### 6.2 Ao trabalhar com IA

- Não treine modelos com dados pessoais sensíveis (payloads, nomes, etc.).
- Priorize features agregadas:
  - Contagens, médias, variância, taxas.
- Documente os datasets usados sempre que possível.

### 6.3 Ao trabalhar com integrações na nuvem

- Minimizar dados enviados.
- Evitar enviar identificadores únicos (MACs, BSSIDs) para serviços externos.
- Caso seja necessário, anonimizar/Hasher valores antes.

---

## 7. Bug bounty / divulgação responsável

Se você encontrar uma vulnerabilidade ou comportamento que possa:

- Comprometer a segurança do usuário, ou
- Ser explorado para fins ofensivos,

recomenda-se:

1. Documentar claramente o cenário.
2. Notificar os mantenedores do projeto (repositório oficial).
3. Aguardar um prazo razoável para correção / mitigação antes de divulgação pública.

---

## 8. Licenciamento e uso

Consulte o arquivo de licença do projeto para detalhes sobre:

- Direitos de modificação e redistribuição.
- Restrições de uso em contextos proibidos por lei.
- Avisos de isenção de responsabilidade (“as is”).

---

## 9. Resumo

- O WavePwn existe para **defender**, não para atacar.
- A arquitetura foi pensada para minimizar riscos de vazamento de dados.
- O usuário final é responsável por usar o dispositivo de forma ética
  e conforme a legislação local.
- Contribuições são bem‑vindas, desde que respeitem estes princípios.

Se você for usar o WavePwn em pesquisas acadêmicas, ambientes corporativos
ou projetos maiores, considere:

- Revisar o código com especialistas em segurança.
- Integrar mecanismos adicionais de criptografia e autenticação.
- Documentar claramente o escopo de cada estudo/teste.

Manter o foco em **segurança defensiva, transparência e respeito à
privacidade** é o que torna o WavePwn uma ferramenta legítima para
proteger ambientes Wi‑Fi.