# Firmware de fábrica – ESP32-S3 Touch AMOLED 1.8"

Esta pasta contém o **firmware original de fábrica** da placa, fornecido pelo fabricante.  
Ele é útil caso você queira **restaurar o estado de fábrica** do dispositivo.

---

## Arquivos nesta pasta

- `ESP32-S3-Touch-AMOLED-1-8-FactoryXiaozhi_250805.bin`  
  - Imagem de firmware **pré-compilada** (“factory firmware”).  
  - Inclui o software original que vem gravado na placa quando nova.

- `README.txt`  
  - Arquivo original em chinês/inglês informando que o **código-fonte do firmware de fábrica não é público**:  
    > “sorry, factory firmware source code companies are not open to the public”

- `README_pt-BR.md` (este arquivo)  
  - Explicação em português sobre o conteúdo da pasta e como utilizar o binário.

---

## Avisos importantes

- O **código-fonte** do firmware de fábrica **não está disponível**.  
  Você só possui o **binário compilado** (`.bin`), fornecido para regravação.

- Gravar esta imagem `.bin` pode:  
  - **Apagar** o firmware customizado que você estiver utilizando (Arduino, ESP-IDF, PlatformIO etc.).  
  - Sobrescrever configurações de boot/particionamento conforme definido na imagem.

- **Faça backup** de qualquer firmware ou dados importantes antes de regravar.

---

## Quando usar este firmware

Você pode querer restaurar o firmware de fábrica se:

- Quiser **voltar ao comportamento original** da placa para comparação.  
- Estiver com problemas após testar vários firmwares e quiser um “ponto de referência” funcional.  
- For vender/transferir a placa e quiser deixá-la como veio de fábrica.

Se você está desenvolvendo com **Arduino/PlatformIO** ou **ESP-IDF**, normalmente não é necessário regravar o firmware de fábrica, a menos que precise desse comportamento original por algum motivo específico.

---

## Como gravar o firmware de fábrica (`.bin`)

Existem várias maneiras de gravar o firmware em um ESP32-S3. Abaixo, duas opções comuns:

### 1. Usando `esptool.py` diretamente

1. Instale o `esptool.py` (se ainda não tiver):

```bash
pip install esptool
```

2. Conecte a placa e descubra a porta serial (por exemplo, `COM5` no Windows ou `/dev/ttyUSB0` no Linux).

3. Na raiz do repositório (ou dentro da pasta `Firmware/`), execute:

```bash
cd Firmware

# Exemplo de comando (ajuste a porta e o endereço conforme necessário):
esptool.py --chip esp32s3 --port COM5 --baud 460800 \
    write_flash 0x0000 ESP32-S3-Touch-AMOLED-1-8-FactoryXiaozhi_250805.bin
```

> Observações:
> - O endereço `0x0000` é um valor comum para imagens “all-in-one”,  
>   mas **confira a documentação da sua placa** para verificar se o firmware de fábrica foi gerado como imagem única ou se depende de um layout específico de partições.  
> - Se tiver dúvidas, é mais seguro usar o ambiente/documentação original fornecido pelo fabricante.

### 2. Usando ESP-IDF (`idf.py` + `esptool` interno)

Se você já tem o **ESP-IDF** configurado:

1. Ative o ambiente (`. ./export.sh`).  
2. Vá até a pasta `Firmware/`.  
3. Use o comando `esptool.py` incluído no ESP-IDF, por exemplo:

```bash
cd Firmware

$IDF_PATH/components/esptool_py/esptool/esptool.py \
    --chip esp32s3 --port COM5 --baud 460800 \
    write_flash 0x0000 ESP32-S3-Touch-AMOLED-1-8-FactoryXiaozhi_250805.bin
```

---

## Dica: organização entre firmware de fábrica e projetos próprios

Minha sugestão de fluxo de trabalho:

1. **Guardar uma cópia segura** deste `.bin` (por exemplo, em outro diretório ou backup).  
2. Usar o repositório normalmente com:
   - `Arduino-v3.1.0/` para projetos Arduino/PlatformIO.  
   - `ESP-IDF-v5.3.2/` para projetos nativos ESP-IDF.
3. Quando quiser **comparar** com o firmware de fábrica:
   - Grave o `.bin`, teste a funcionalidade.  
   - Depois, volte a gravar seu firmware customizado (Arduino/ESP-IDF).

Se você quiser, posso te ajudar a:
- Montar um script `.bat` (Windows) ou `.sh` (Linux) para gravar rapidamente o firmware de fábrica.  
- Criar um passo-a-passo para backup do firmware atual da placa antes de sobrescrever.