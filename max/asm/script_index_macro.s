
    
  .macro generateLookupRegionScriptRoutine
    lda $0001
    pha
    lda $0002
    pha
    lda $0003
    pha
      
      ; get index pointer
      ; 0000 = base index pointer
      lda #<\2
      sta $0000
      lda #>\2
      sta $0001
    
      ; 0002 = scriptindex * 2
      lda $00CC   ; target script index
      sta $0002
      lda #$00
      sta $0003
      asl $0002
      rol $0003
      
      ; 0000 = pointer to script pointer
      lda $0000
      clc
      adc $0002
      sta $0000
      lda #$00
      adc $0001
      sta $0001
      
      ; $00B8 = target script pointer (from index)
      ldy #$00
      lda ($0000),Y
      sta $00B8
      iny
      lda ($0000),Y
      sta $00B9
      
      ;==============================================
      ; switch to bank of actual script
      ;==============================================
    
      ; get bank for this region
      lda #\1
      
      ; switch bank
      cmp activeRomLow
      beq +
        tax
        jsr nmiWait
        jsr switchRomLow
      +:
    
    pla
    sta $0003
    pla
    sta $0002
    pla
    sta $0001
    
    rts
  .endm
  