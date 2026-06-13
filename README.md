# Montador (Assembler) para o processador NEANDER

Programa em **C** que traduz um programa em **Assembly** do Neander
(`prog1.asm`) para **linguagem de máquina** num arquivo binário
(`prog1.mem`), que é aberto diretamente no simulador Neander.

```
prog1.asm  ──►  [ montador em C ]  ──►  prog1.mem  ──►  Simulador Neander
 (texto)                                 (binário)
```

## Arquivos

| Arquivo       | Descrição                                          |
|---------------|----------------------------------------------------|
| `montador.c`  | Código-fonte do montador                           |
| `prog1.asm`   | Programa de exemplo (`MEM[82] = MEM[80] + MEM[81]`) |
| `README.md`   | Este arquivo                                        |

---

## 1. Compilando o montador

É necessário o padrão C99/C11.

**Linux / Mac:**
```sh
gcc -std=c11 -Wall -Wextra -o montador.out montador.c
```

**Windows (gcc do Dev-C++):**
```powershell
& "C:\Program Files (x86)\Dev-Cpp\MinGW64\bin\gcc.exe" -std=c11 -Wall -Wextra -o montador.exe montador.c
```

---

## 2. Como escrever o código em Assembly (`prog1.asm`)

O `prog1.asm` é um **arquivo de texto** comum (crie/edite no Bloco de Notas,
VS Code, etc.). As regras são simples:

### 2.1. Uma instrução por linha

Cada linha contém **um mnemônico** e, quando a instrução exige, **um endereço**:

```asm
LDA 80
ADD 81
STA 82
HLT
```

### 2.2. Tabela de instruções

| Assembly  | Opcode | Tem endereço? | Significado                         |
|-----------|:------:|:-------------:|-------------------------------------|
| `NOP`     | `00`   | não           | nenhuma operação                    |
| `STA end` | `10`   | sim           | `MEM[end] ← AC`                     |
| `LDA end` | `20`   | sim           | `AC ← MEM[end]`                     |
| `ADD end` | `30`   | sim           | `AC ← AC + MEM[end]`                |
| `OR  end` | `40`   | sim           | `AC ← AC OR MEM[end]`               |
| `AND end` | `50`   | sim           | `AC ← AC AND MEM[end]`              |
| `NOT`     | `60`   | não           | `AC ← NOT AC`                       |
| `JMP end` | `80`   | sim           | desvia: `PC ← end`                  |
| `JN  end` | `90`   | sim           | desvia se N=1 (resultado negativo)  |
| `JZ  end` | `A0`   | sim           | desvia se Z=1 (resultado zero)      |
| `HLT`     | `F0`   | não           | encerra a execução                  |

> `NOP`, `NOT` e `HLT` **não** levam endereço. Todas as outras **exigem** um endereço.

### 2.3. Endereços em HEXADECIMAL

Os endereços (operandos) são escritos em **hexadecimal**, de `0` a `FF`
(0 a 255). Por exemplo, `LDA 80` lê a posição de memória `80h` (= 128 decimal).

### 2.4. Comentários

Tudo após um ponto-e-vírgula `;` é ignorado pelo montador — use para anotações:

```asm
LDA 80      ; carrega o primeiro valor no acumulador
```

### 2.5. Maiúsculas/minúsculas

Os mnemônicos podem ser escritos em maiúsculas ou minúsculas (`LDA`, `lda`, `Lda`).

### 2.6. Exemplo completo (o `prog1.asm` deste projeto)

```asm
; Calcula:  MEM[82] = MEM[80] + MEM[81]   e depois para.
LDA 80      ; AC <- MEM[80h]
ADD 81      ; AC <- AC + MEM[81h]
STA 82      ; MEM[82h] <- AC
HLT         ; para a execucao
```

---

## 3. Como introduzir o arquivo `.asm` no montador

O arquivo `.asm` é **fornecido ao montador como argumento na linha de comando**.
O montador **não** abre janelas nem pede o arquivo interativamente: você digita o
nome do `.asm` logo após o nome do programa, no terminal.

Formato geral do comando:

```
montador   <arquivo_de_entrada.asm>   [arquivo_de_saida.mem]
   ▲                 ▲                          ▲
programa      arquivo Assembly a traduzir   (opcional) nome do .mem;
                  (OBRIGATÓRIO)             se omitido, usa o mesmo
                                            nome do .asm com extensão .mem
```

### Passo a passo

**1) Coloque o `prog1.asm` na mesma pasta do montador** (`montador.exe` /
`montador.out`). É o caminho mais simples, pois aí basta usar o nome do arquivo.

**2) Abra um terminal nessa pasta:**

- **Windows:** abra a pasta no Explorador de Arquivos, clique na barra de
  endereço, digite `powershell` e tecle **Enter** (abre o PowerShell já na pasta).
  Ou abra o PowerShell e navegue com:
  ```powershell
  cd "C:\Users\danig\OneDrive\Desktop\CompiladorNeander"
  ```
- **Linux / Mac:** abra o terminal e use `cd` até a pasta do projeto.

**3) Execute o montador passando o `.asm`:**

**Windows:**
```powershell
.\montador.exe prog1.asm
```

**Linux / Mac:**
```sh
./montador.out prog1.asm
```

**4) Pronto:** será criado o `prog1.mem` na mesma pasta. Saída esperada:
```
Montagem concluida com sucesso.
  Entrada : prog1.asm
  Saida   : prog1.mem  (abra no simulador Neander)
```

### Variações úteis

- **Escolher o nome do arquivo de saída** (segundo argumento, opcional):
  ```powershell
  .\montador.exe prog1.asm meu_programa.mem
  ```
- **Usar um `.asm` que está em outra pasta** — informe o caminho completo:
  ```powershell
  .\montador.exe "C:\Users\danig\Desktop\testes\soma.asm"
  ```
- **Renomeie seu próprio programa:** o arquivo de entrada não precisa se chamar
  `prog1.asm`; pode ser qualquer nome com extensão `.asm`
  (ex.: `.\montador.exe fatorial.asm`).

> ⚠️ **Erros comuns ao introduzir o arquivo**
> - Esquecer o nome do `.asm` (rodar só `.\montador.exe`) → o programa mostra o
>   modo de uso. **Sempre informe o arquivo de entrada.**
> - O `.asm` não está na pasta atual → erro *"nao foi possivel abrir"*. Coloque o
>   arquivo na pasta ou passe o caminho completo.
> - No Windows, lembre do prefixo `.\` antes de `montador.exe`.

### Como o montador traduz

Cada instrução vira o seu **opcode**; quando há endereço, ele é gravado em
seguida. **Para cada byte gerado (opcode ou endereço) é acrescentado um byte
`00`**, e o arquivo começa com o cabeçalho `03 4E 44 52`. Assim o `prog1.asm`
acima produz:

```
03 4E 44 52   ← cabeçalho (sempre)
20 00         ← LDA   (opcode 20)
80 00         ← endereço 80h
30 00         ← ADD   (opcode 30)
81 00         ← endereço 81h
10 00         ← STA   (opcode 10)
82 00         ← endereço 82h
F0 00         ← HLT   (opcode F0)
00 00 ...     ← restante da memória (até 256 posições)
```

> 💡 Você pode conferir esses bytes abrindo o `prog1.mem` em um **editor
> hexadecimal** (ex.: HxD).

---

## 4. Como abrir o `prog1.mem` no Neander

1. **Abra o simulador Neander** (o executável do simulador usado na disciplina).
2. No menu, escolha **Arquivo → Abrir** (ou *Carregar memória*) e selecione o
   arquivo **`prog1.mem`**. O código aparecerá na memória a partir do
   endereço `0`:
   - `0: LDA 80`
   - `2: ADD 81`
   - `4: STA 82`
   - `6: HLT`
3. **Digite os dados de entrada na memória.** Como este programa soma os valores
   das posições `80h` e `81h`, antes de executar coloque os números nessas
   posições (clique na célula de memória e edite). Exemplo:
   - posição `80h` = `05`
   - posição `81h` = `07`
4. **Execute** o programa (botão *Rodar*/*Run* ou passo-a-passo com *Step*).
5. **Veja o resultado** na posição `82h`: deverá conter `0C` (hexadecimal),
   que é `12` em decimal (5 + 7). O acumulador (AC) também mostrará `0C`.

> Se preferir alterar o programa, edite o `prog1.asm`, rode o montador de novo
> para regerar o `prog1.mem` e recarregue no Neander.

---

## 5. Mensagens de erro

O montador para e informa o **número da linha** em casos como:
instrução desconhecida, operando ausente em instrução que exige endereço,
operando hexadecimal inválido, endereço fora da faixa `00..FF`, instrução que
não aceita operando recebendo um, e programa maior que 256 bytes.

---

## 6. Formato do arquivo `.mem` (resumo)

`03 4E 44 52` + 256 posições de memória, cada uma gravada como `valor 00`
(16 bits, byte alto sempre 0). Tamanho total: `4 + 256 × 2 = 516 bytes`.
