# Dataset NEURA9

Coloque aqui os dados rotulados usados para treinar a NEURA9.

Formato sugerido (CSV):

- Arquivo: `neura9_dataset.csv`
- Colunas: `f0,f1,...,f71,label`
- `label` é um inteiro de 0 a 9, mapeado para:

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

Gere esse CSV a partir dos logs do WavePwn ou de capturas feitas com outras
ferramentas de monitoramento **defensivo**. Em seguida execute:

```bash
python3 ai/neura9_trainer.py --dataset ai/dataset/neura9_dataset.csv
```

para obter um novo `neura9_defense_model.tflite` e o array C correspondente.