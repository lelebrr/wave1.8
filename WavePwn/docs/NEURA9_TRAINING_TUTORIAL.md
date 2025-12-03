# COMO TREINAR A NEURA9 DO ZERO (PASSO A PASSO)

Este documento descreve, de ponta a ponta, como coletar dados reais,
treinar a IA defensiva **NEURA9**, converter o modelo para TensorFlow Lite
e embuti-lo no firmware do WavePwn.

---

## 1. Conceito

A NEURA9 é uma rede neural feed‑forward leve:

- Entrada: **72 features** numéricas normalizadas
- Arquitetura típica: `72 → 128 → 64 → 32 → 10`
- Saída: 10 classes de risco/estado:

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

O objetivo é que, a partir de métricas do ambiente (número de APs, volume
de deauth, nível de bateria, etc.), o modelo classifique o cenário atual.

---

## 2. Coleta de dados reais

### 2.1 Configurando o WavePwn para logging

A versão atual do firmware já gera diversos contadores em tempo real:

- `pwn.aps_seen`
- `pwn.handshakes`
- `pwn.pmkids`
- `pwn.deauths`
- `pwn.battery_percent`
- `pwn.is_charging`
- `pwn.is_moving`
- `pwn.uptime`
- `pwn.current_channel`

Uma estratégia recomendada é registrar periodicamente esses valores em CSV
no microSD, juntamente com um rótulo (label) indicando a classe real
observada (0–9). Esse rótulo pode ser definido manualmente ou via scripts
de anotação.

Sugestão de esquema de arquivo:

```text
/sd/logs/features_YYYYMMDD_HHMM.csv
```

Cada linha:

```text
f0,f1,...,f71,label
```

Onde:

- `f0..f71` — 72 features (as primeiras preenchidas com métricas reais,
  as restantes podem ser zero até que novas fontes sejam integradas)
- `label` — inteiro de 0 a 9 (classe NEURA9)

### 2.2 Estratégia de coleta

Para obter um modelo robusto, deixe o WavePwn rodando por vários dias em
ambientes distintos:

- Casa (rede doméstica normal)
- Escritório / coworking (rede mais densa)
- Cafés / espaços públicos (muitos APs abertos)
- Cenários controlados:
  - Simulação de ataques de deauth em laboratório
  - APs “evil twin” em ambiente isolado
  - Bateria em diferentes níveis (incluindo quase vazia)

Busque coletar pelo menos **50.000 amostras** rotuladas, distribuídas
entre as 10 classes (o ideal é balancear o número de exemplos por classe).

---

## 3. Preparando o dataset

### 3.1 Estrutura de diretórios

No repositório:

- `WavePwn/ai/dataset/` — local sugerido para o CSV principal
- `WavePwn/ai_training/dataset/` — diretório alternativo para datasets maiores

Crie (ou una) todos os CSVs coletados em um único arquivo:

```text
WavePwn/ai/dataset/neura9_dataset.csv
```

Formato (com cabeçalho):

```text
f0,f1,...,f71,label
0.10,0.00,...,0.0,0
0.35,0.02,...,0.0,6
...
```

### 3.2 Normalização

O script de treino (ver abaixo) pode incluir normalização simples, mas é
recomendável:

- Escalar contadores brutos (APS, handshakes, etc.) para faixas “humanas”
  (por exemplo, log10 ou min‑max).
- Garantir que `label` seja sempre inteiro entre 0 e 9.

---

## 4. Executando o treinamento com Python

O repositório inclui um treinador de referência:

- `WavePwn/ai/neura9_trainer.py`

### 4.1 Dependências

Crie um ambiente Python 3.x com:

```bash
python3 -m venv .venv
source .venv/bin/activate  # ou .venv\Scripts\activate no Windows
pip install numpy tensorflow keras
```

### 4.2 Treinando

Na raiz do repositório:

```bash
python3 ai/neura9_trainer.py --dataset ai/dataset/neura9_dataset.csv \
                             --epochs 40 \
                             --batch-size 256
```

O script:

1. Carrega o CSV.
2. Constrói o modelo `neura9_defense`.
3. Treina com validação (20%).
4. Salva:

   - `ai/neura9_defense_model.tflite`
   - `ai/neura9_defense_model_data.cpp` (array C com os bytes do modelo)

Acurácia esperada (exemplo realista): **≈98.7% em 50.000 amostras**
bem rotuladas e balanceadas.

---

## 5. Notebook Jupyter: neura9_full_training.ipynb

Para uma experiência mais visual e interativa, utilize o notebook:

- `WavePwn/ai_training/neura9_full_training.ipynb`

Fluxo típico do notebook:

1. Carrega `ai/dataset/neura9_dataset.csv`.
2. Explora estatísticas básicas:
   - Distribuição de classes
   - Histogramas das features principais
3. Divide em treino/validação/teste.
4. Constrói o modelo Keras (72 → 128 → 64 → 32 → 10).
5. Treina com callbacks:
   - EarlyStopping
   - ModelCheckpoint (melhor peso em `best_model.h5`)
6. Avalia:
   - Acurácia top‑1
   - Matriz de confusão
   - Precision/Recall por classe
7. Exporta os arquivos finais:

   - `best_model.h5`
   - `neura9_defense_model.tflite`

---

## 6. Exportando para TensorFlow Lite (.tflite)

Caso utilize um modelo salvo em `best_model.h5`, o script recomendado é:

- `WavePwn/ai_training/export_to_tflite.py`

Uso:

```bash
cd WavePwn
python3 ai_training/export_to_tflite.py \
    --model ai_training/best_model.h5 \
    --output ai/neura9_defense_model.tflite
```

Esse script:

1. Carrega `best_model.h5`.
2. Converte para TensorFlow Lite (float32).
3. Salva o `.tflite` no caminho desejado.

---

## 7. Convertendo .tflite para C (modelo embutido)

Para embutir o modelo no firmware, você tem duas opções:

### 7.1 Usar o script neura9_trainer.py

O próprio `neura9_trainer.py` já possui uma função `export_c_array()` que:

- Lê `ai/neura9_defense_model.tflite`
- Gera `ai/neura9_defense_model_data.cpp` com:

  ```cpp
  const unsigned char neura9_defense_model_tflite[] = { ... };
  const unsigned int neura9_defense_model_tflite_len = sizeof(neura9_defense_model_tflite);
  ```

### 7.2 Usar xxd (alternativa manual)

Em sistemas Unix-like:

```bash
xxd -i ai/neura9_defense_model.tflite > ai/neura9_defense_model_data.cpp
```

E depois ajuste o arquivo para declarar os símbolos com o nome esperado
(`neura9_defense_model_tflite` e `neura9_defense_model_tflite_len`).

---

## 8. Conectando o modelo ao firmware

No firmware:

- `src/neura9/model.h` declara:

  ```cpp
  extern const unsigned char neura9_defense_model_tflite[];
  extern const unsigned int  neura9_defense_model_tflite_len;
  ```

- `src/neura9/inference.cpp` usa esses símbolos para criar o
  `tflite::MicroInterpreter`.

Certifique-se de que:

- `ai/neura9_defense_model_data.cpp` está sendo compilado.
- O arquivo inclui a declaração correta (seja via `neura9_defense_model_data.h`
  ou `neura9/model.h`).

---

## 9. Atualizando o dispositivo com o novo modelo

Depois de gerar `neura9_defense_model_data.cpp`:

1. Recompile o firmware:

   ```bash
   cd WavePwn
   pio run
   ```

2. Atualize o dispositivo:
   - Via USB (`pio run -t upload`), ou
   - Via OTA (carregando o `.bin` pelo dashboard web).

3. Monitore o log serial:

   ```text
   [NEURA9] IA defensiva NEURA9 carregada — modelo TFLite em RAM — 100% offline
   ```

4. Observe a mudança de comportamento da NEURA9 nas situações de teste
   (mais sensível, menos sensível, etc.).

---

## 10. Afinando a sensibilidade

O campo `"neura9_sensitivity"` em `device_config.json` permite ajustar
dinamicamente a sensação de “paranoia” da IA:

- Valores próximos de `0.5` → comportamento equilibrado.
- Valores maiores (ex.: `0.8`) → mais propenso a sinalizar riscos.
- Valores menores (ex.: `0.3`) → mais conservador (menos alertas).

Esse valor pode ser usado para:

- Ajustar thresholds para certas classes (ex.: só disparar alertas
  audíveis quando a confiança for maior que `neura9_sensitivity`).
- Controlar quantas vezes o WavePwn avisa sobre o mesmo tipo de ameaça.

A lógica de uso fica em `src/neura9/inference.cpp` e/ou em camadas
superiores (ex.: integração com assistentes de voz).

---

## 11. Boas práticas de treinamento

- **Balanceamento**: evite datasets onde uma classe (ex.: SAFE) domina.
- **Qualidade de labels**: um modelo é tão bom quanto a qualidade da
  rotulagem. Dedique tempo a rotular corretamente.
- **Generalização**: colete dados em ambientes e horários distintos.
- **Validação cruzada**: não use o mesmo período de logs para treino e teste.

---

## 12. Checklist rápido

1. Coletar logs reais em `/sd/logs/`.
2. Unificar em `ai/dataset/neura9_dataset.csv` (72 features + label).
3. Rodar `ai/neura9_trainer.py` ou o notebook
   `ai_training/neura9_full_training.ipynb`.
4. Gerar `neura9_defense_model.tflite` + `neura9_defense_model_data.cpp`.
5. Recompilar o firmware e fazer upload/OTA.
6. Validar o comportamento da NEURA9 no campo.
7. Ajustar `"neura9_sensitivity"` em `device_config.json` conforme necessário.

Seguindo esses passos, você terá uma NEURA9 **realmente sua**: treinada
com os padrões de tráfego, riscos e ambientes que importam para você.