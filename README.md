# Montador Neander

Programa em C que traduz um programa em Assembly do Neander (`prog1.asm`)
para código de máquina (`prog1.mem`), que abre direto no simulador Neander.

## Compilar

```sh
gcc -std=c11 -o montador.out montador.c
```

## Usar

```sh
./montador.out prog1.asm
```

Isso gera o `prog1.mem`. (No Windows, use `montador.exe` e `.\montador.exe prog1.asm`.)

## Instruções

| Assembly  | Opcode | Tem endereço? |
|-----------|:------:|:-------------:|
| `NOP`     | `00`   | não           |
| `STA end` | `10`   | sim           |
| `LDA end` | `20`   | sim           |
| `ADD end` | `30`   | sim           |
| `OR  end` | `40`   | sim           |
| `AND end` | `50`   | sim           |
| `NOT`     | `60`   | não           |
| `JMP end` | `80`   | sim           |
| `JN  end` | `90`   | sim           |
| `JZ  end` | `A0`   | sim           |
| `HLT`     | `F0`   | não           |

- Uma instrução por linha. Endereços em **hexadecimal**.
- Texto após `;` é comentário.

## Exemplo (`prog1.asm`)

```asm
LDA 80   ; AC <- MEM[80h]
ADD 81   ; AC <- AC + MEM[81h]
STA 82   ; MEM[82h] <- AC
HLT      ; para
```

## Abrir no Neander

1. Abra o simulador Neander e carregue o `prog1.mem`.
2. Coloque os valores nas posições usadas (ex.: `80h` e `81h`).
3. Execute e veja o resultado na posição de destino (ex.: `82h`).
