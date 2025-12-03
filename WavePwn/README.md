# WAVE PWN v2.0 — O Pwnagotchi que Enterra o Raspberry Pi

O primeiro e único Pwnagotchi 100% nativo para ESP32-S3 Touch AMOLED 1.8"

### Recursos já funcionando ou em desenvolvimento:
- Interface LVGL com 20+ expressões animadas
- Captura passiva + ativa de WPA/WPA2 handshakes e PMKID
- Deauth inteligente ativado por movimento (IMU)
- microSD 128 GB com PCAPs hashcat-ready
- Display AMOLED 368x448 com touch
- Bateria monitorada + modo sleep profundo
- Sons fofos com ES8311
- Futuro: BLE Pwngrid, web server OTA, IA local

## ETAPA 2 DO RENASCIMENTO – CONCLUÍDA
- 22 expressões animadas (melhor que qualquer Tamagotchi)
- Partículas 3D com física realista
- Olhos que seguem o toque
- Braços com animações independentes
- Código Konami → GOD MODE
- Wake por toque ou movimento
- 60 FPS no AMOLED
- Menu secreto completo
- Esta interface já é uma obra de arte

## ETAPA 3 DO RENASCIMENTO – CONCLUÍDA
- Parser 100% funcional de WPA2 4-way handshake
- Captura de PMKID automática (hashcat -m 16800)
- PCAPs com rotação automática (150 MB cada)
- Deduplicação em RAM + persistente
- Arquivos 100% compatíveis com hashcat/aircrack
- Captura WPA3 SAE (em desenvolvimento)
- Mais de 200 pacotes/segundo processados
- Seu cartão de 128 GB agora é um arsenal nuclear de handshakes

## ETAPA 4 REFORMULADA – NEURA9 DEFENSIVA
- NEURA9: primeira IA local defensiva embarcada em ESP32-S3
- Modelo leve em TensorFlow Lite Micro rodando 100% offline
- Extrai dezenas de features do ambiente (redes, bateria, movimento)
- Classifica o nível de risco Wi‑Fi em 10 categorias
- Detecta cenário suspeito (deauth em massa, redes abertas de risco, etc.)
- Adapta o humor do WavePwn conforme o “clima de segurança” ao redor
- Pipeline completo para treinar/atualizar o modelo no PC (Python + TFLite)
- Estrutura pronta para integrar IMU, RTC e métricas avançadas

## MÓDULO DE SIMULAÇÃO ACADÊMICA (Etapa 8)
Contém simulações didáticas de:
- Deauthentication attacks (simulados)
- Evil Twin / Rogue AP (simulados)
- Beacon spam (simulado)
- NFC replay (simulado)
- Bluetooth spam (simulado)
- MouseJack simulation (conceitual)
- e mais cenários extensíveis

STATUS: 100% DESATIVADO por padrão  
Ativação: criar manualmente o arquivo `/sd/.enable_lab_attacks`  
Uso: apenas em laboratório isolado com autorização escrita. Nenhuma rotina
de simulação envia tráfego malicioso real; tudo é registrado em `/sd/lab_logs/`
e sinalizado na UI como **SIMULAÇÃO**.

### Compilação:
- Use PlatformIO (já configurado)
- Board: ESP32S3 Dev Module
- PSRAM: Enabled
- Flash: 16MB
- Partition: Huge App

Este projeto vai fazer história.