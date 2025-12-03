# AVISO LEGAL, ÉTICO E TÉCNICO — MÓDULO DE SIMULAÇÃO ACADÊMICA

> **LEIA ESTE DOCUMENTO COMPLETO ANTES DE COMPILAR QUALQUER COISA NESTA PASTA**

Este diretório (`src/lab_simulations/`) contém **APENAS SIMULAÇÕES ACADÊMICAS**,
**NÃO** ataques reais.  
Nenhuma função aqui deve ser usada em produção, em equipamentos de terceiros
ou em qualquer cenário fora de um **laboratório controlado, isolado e autorizado**.

---

## 1. NATUREZA DO CÓDIGO NESTE DIRETÓRIO

1. Todo o código aqui é projetado para:

   - Gerar **logs didáticos** em `/sd/lab_logs/`.
   - Acionar **indicadores visuais** na UI com a palavra **“SIMULAÇÃO”**.
   - Servir como **material acadêmico** para estudo de defesa, resposta a incidentes
     e visualização de ameaças.

2. Nenhuma função deste módulo deve:

   - Enviar frames 802.11 de desautenticação, beacon spoof, probe spoof, etc.
   - Criar Access Points maliciosos reais (evil twin / rogue AP).
   - Gerar tráfego Bluetooth/NFC malicioso.
   - Interferir, degradar ou interromper redes de terceiros.

3. Se em algum momento este código se desviar desta filosofia, ele deverá ser
   **revisado, auditado e possivelmente removido**.

---

## 2. HABILITAÇÃO DO “MODO LABORATÓRIO”

Por padrão, **tudo está desativado**.

O arquivo de controle é:

```text
/sd/.enable_lab_attacks
```

- Se **não** existir → todas as funções de `SimulationManager` apenas:
  - Registram que o modo lab está **desativado**.
  - Mostram um aviso na tela (“LAB MODE OFF”).
  - Saem imediatamente sem simular nada.

- Se existir → o firmware assume que:

  1. Você leu e entendeu **este documento**.
  2. Você está em um **ambiente de laboratório controlado**.
  3. Você possui **autorização formal, escrita e assinada** para executar
     qualquer tipo de experimento aqui descrito.

> Dica: armazene junto ao cartão microSD um PDF/termo de responsabilidade
> assinado com o escopo do laboratório e das simulações autorizadas.

---

## 3. LIMITES LEGAIS (EXEMPLO BRASIL + CONTEXTO GLOBAL)

### 3.1 Brasil — Lei 12.737/2012 e correlatas

No Brasil, o uso malicioso deste tipo de ferramenta pode se enquadrar em:

- **Invasão de dispositivo informático**.
- **Interferência em comunicações**.
- **Danos a sistemas de informação**.

Mesmo que não haja **dano aparente**, o simples ato de interferir em uma rede
ou dispositivo de terceiros **sem autorização** pode configurar crime.

### 3.2 Outros países

Legislação equivalente (mas não limitada a):

- Computer Fraud and Abuse Act (CFAA) — EUA  
- Computer Misuse Act — Reino Unido  
- Diretivas da União Europeia sobre ataques contra sistemas de informação  
- Leis nacionais de telecomunicações e radiocomunicação

Em qualquer jurisdição, o princípio é semelhante:

> **Não interfira, capture, modifique ou interrompa comunicações de terceiros
> sem consentimento explícito e autorização formal.**

---

## 4. FILOSOFIA DO WAVE PWN v2 / NEURA9

O projeto WavePwn v2 (e principalmente a NEURA9) foi desenhado com foco em:

- **Defesa**, não ataque.
- **Monitoramento e alerta**, não exploração.
- **Privacidade e ética**, não voyeurismo digital.

Este módulo de simulação existe para:

- **Ensinar o que é um ataque** sem realmente causar o ataque.
- Permitir que alunos e pesquisadores:
  - Visualizem na UI o “clima” de uma rede sob ataque.
  - Treinem processos de resposta a incidentes.
  - Validem dashboards, relatórios e alarmes.

NEURA9 **não deve aprender** a partir destas simulações para reproduzir ataques.
O uso recomendado é tratar estes eventos simulados como **cenários sintéticos**
para **refinar a detecção**, não para **alimentar modelos ofensivos**.

---

## 5. ESTRUTURA DO MÓDULO DE SIMULAÇÃO

Diretório:

```text
WavePwn/src/lab_simulations/
├── ATTACK_SIMULATION.md        ← este arquivo (LEITURA OBRIGATÓRIA)
├── simulation_manager.cpp / .h ← orquestração e checagem de /sd/.enable_lab_attacks
├── deauth_sim.cpp / .h         ← simulação acadêmica de “deauth burst”
├── evil_twin_sim.cpp / .h      ← simulação acadêmica de “evil twin”
├── beacon_spam_sim.cpp / .h    ← simulação acadêmica de “beacon spam”
├── rogue_ap_sim.cpp / .h       ← simulação acadêmica de “rogue AP”
├── nfc_replay_sim.cpp          ← simulação acadêmica de cenários NFC
└── bluetooth_spam_sim.cpp      ← simulação acadêmica de cenários BLE
```

Pontos importantes:

1. Toda lógica de **gating seguro** (checagem do arquivo `.enable_lab_attacks`)
   está centralizada em `SimulationManager`.

2. As funções de simulação são explicitamente marcadas como:

   - **SIMULAÇÃO** nos logs (`[LAB][SIM]`).
   - **SIMULAÇÃO** na UI (banner vermelho).
   - **SIMULAÇÃO** nos arquivos de log em `/sd/lab_logs/`.

3. Em nenhuma hipótese estes módulos devem ser conectados diretamente a
   bibliotecas de ataque ativo ou rotinas de jamming.

---

## 6. RESPONSABILIDADE DO OPERADOR

Ao criar o arquivo `/sd/.enable_lab_attacks`, você declara que:

1. **Lê e compreende** este documento na íntegra.
2. É **profissional ou pesquisador** de segurança de informação, ou está sob
   supervisão direta de um.
3. Está em um **laboratório claramente delimitado**, com:
   - Redes próprias isoladas, ou
   - Ambientes de teste autorizados (ex.: empresa, universidade, laboratório).
4. Possui **autorização por escrito**, contendo:
   - Quem é o responsável pelo ambiente.
   - Quais cenários de simulação são permitidos.
   - Qual a duração/cronograma dos testes.
   - Como os dados coletados serão armazenados e descartados.

Você também aceita que:

- O autor do projeto e colaboradores **não se responsabilizam** por qualquer uso
  indevido, ilegal ou antiético deste firmware.
- Qualquer violação de lei ou política interna é de responsabilidade **exclusiva**
  do operador.

---

## 7. BOAS PRÁTICAS PARA LABORATÓRIOS

Para manter o uso deste módulo dentro da ética e da legalidade, recomenda-se:

1. **Rede isolada de laboratório**
   - Use roteadores, APs e clientes dedicados exclusivamente ao teste.
   - Isolar fisicamente (blindagem) ou logicamente (potência mínima, sala
     controlada).

2. **Documentação clara**
   - Ter um documento de “Termo de Responsabilidade” assinado.
   - Incluir escopo, data, equipamentos e contatos.

3. **Registro de simulações**
   - Manter os logs em `/sd/lab_logs/` como evidência de:
     - O que foi simulado.
     - Quando foi simulado.
     - Quem estava responsável.

4. **Auditoria e revisão**
   - Revisar periodicamente o código de simulação.
   - Garantir que nenhuma rotina tenha sido alterada para virar ataque real.
   - Fazer code review por pelo menos 2 pessoas distintas.

5. **Desabilitar após o uso**
   - Remover o arquivo `/sd/.enable_lab_attacks` após o laboratório.
   - Guardar o cartão microSD em local seguro ou lacrado.

---

## 8. COMO O CÓDIGO SE COMPORTA (ALTO NÍVEL)

Todas as funções de simulação seguem este padrão conceitual:

1. **Checagem de modo lab**

   ```text
   se /sd/.enable_lab_attacks NÃO existir:
       - Registrar aviso no Serial
       - Exibir “LAB MODE OFF” na tela
       - Encerrar a função
   ```

2. **Em modo lab habilitado:**

   - Registrar no Serial algo como:

     ```text
     [LAB][SIM] Deauth burst (simulado) no canal X, pacotes=Y
     ```

   - Exibir na UI um banner vermelho com “SIMULAÇÃO” + descrição.
   - Gravar um log em `/sd/lab_logs/` com timestamp e parâmetros.
   - **Não** enviar nenhum pacote malicioso real.

3. **Integração com NEURA9**

   - NEURA9 pode ser configurada para **enxergar** estes eventos simulados
     como “cenários sintéticos” em testes.
   - Em usos normais, recomenda-se que:
     - NEURA9 não utilize estes eventos para treinar comportamento ofensivo.
     - Os datasets de treino continuem focados em **detecção e defesa**.

---

## 9. CENÁRIOS DE USO PERMITIDOS

Exemplos de usos compatíveis com este módulo:

- Aulas universitárias de:
  - Segurança de redes sem fio.
  - Resposta a incidentes.
  - Forense de redes.
- Workshops em conferências de segurança, com laboratório isolado.
- Testes internos em empresas para:
  - Treinar equipes de Blue Team.
  - Validar alarmes e dashboards.
  - Demonstrar o impacto de ataques **sem realmente atacar**.

---

## 10. CENÁRIOS DE USO PROIBIDOS

Este módulo **não** deve ser usado para:

- Interromper Wi‑Fi de vizinhos, escolas, empresas ou qualquer terceiro.
- “Testar” redes públicas (cafés, aeroportos, shoppings) sem autorização formal.
- Qualquer tipo de **trote, brincadeira, trollagem ou vingança pessoal**.
- Projetos de demonstração em eventos **sem laboratório controlado**.

Se você está em dúvida se um cenário é permitido, assuma que **não é**,
até obter:

1. Parecer jurídico.
2. Autorização por escrito.
3. Aprovação de um responsável formal pela segurança do ambiente.

---

## 11. COMO REVERTER PARA UM USO 100% DEFENSIVO

Se em algum momento você quiser garantir que o firmware está em modo
estritamente defensivo:

1. Delete o arquivo `/sd/.enable_lab_attacks`.
2. Opcional: apague também a pasta `/sd/lab_logs/`.
3. Recompile o firmware ignorando completamente `src/lab_simulations/`
   (ex.: via flags de build específicas, se configuradas futuramente).
4. Verifique no Serial e na UI que nenhuma função de simulação é chamada.

---

## 12. RESUMO FINAL (LEIA EM VOZ ALTA SE PRECISAR)

- Este módulo é de **SIMULAÇÃO**, não de ataque real.
- Tudo está **DESATIVADO por padrão**.
- Só “liga” se você criar manualmente `/sd/.enable_lab_attacks`.
- Uso sem autorização formal pode ser **crime** em vários países.
- O autor do projeto não se responsabiliza por uso indevido.
- Você, operador, é o único responsável pelas ações realizadas.

Se você concorda com tudo isso **e** está em um ambiente de laboratório
controlado, com autorização formal:

> Então, e somente então, crie o arquivo  
> `/sd/.enable_lab_attacks`  
> e use as simulações de forma ética, responsável e legal.

Caso contrário:

> **NÃO HABILITE O MODO LABORATÓRIO.**