; ============================================================
;  prog1.asm  -  Programa exemplo para o montador Neander
; ------------------------------------------------------------
;  Calcula:  MEM[82] = MEM[80] + MEM[81]   e depois para.
;  Lembrete: os enderecos estao em HEXADECIMAL.
;  Os valores de 80h e 81h sao digitados no Neander antes de rodar.
; ============================================================

LDA 80      ; AC <- MEM[80h]        -> gera 20 00 80 00
ADD 81      ; AC <- AC + MEM[81h]   -> gera 30 00 81 00
STA 82      ; MEM[82h] <- AC        -> gera 10 00 82 00
HLT         ; para a execucao       -> gera F0 00
