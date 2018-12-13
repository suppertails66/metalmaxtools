
;.include "sys/pce_arch.s"
;.include "base/macros.s"

.memorymap
   defaultslot     1
   
   ;===============================================
   ; RAM area
   ;===============================================
   
   slotsize        $800
   slot            0       $0000
   slotsize        $2000
   slot            1       $6000
   
   ;===============================================
   ; ROM area
   ;===============================================
   
   slotsize        $2000
   slot            2       $8000
   slot            3       $A000
   slotsize        $2000
   slot            4       $C000
   slot            5       $E000
.endme

.rombankmap
  bankstotal 64
  banksize $2000
  banks 64
.endro

.emptyfill $FF

.background "max.nes"

; unbackground expanded ROM space, excluding area reserved for scripts
;.unbackground $78000 $7BFFF

; unbackground expanded ROM space
.unbackground $40000 $7BFFF

; unbackground free space at end of banks
.unbackground $21FAB $21FFF
.unbackground $29FBF $29FFF
.unbackground $2FFF4 $2FFFF
.unbackground $33FBB $33FFF
.unbackground $7FF11 $7FF3A

; unused?
; sample data immediately follows, but this doesn't seem to be read...
;.unbackground $7CF40 $7CFBF
;.bank $3E slot 4
;.org $0F40
;.section "test" overwrite
;  .repeat $80 index count
;    .db count
;  .endr
;.ends

.define PPUCTRL $2000
.define PPUMASK $2001
.define PPUSTATUS $2002
.define OAMADDR $2003
.define OAMDATA $2004
.define PPUSCROLL $2005
.define PPUADDR $2006
.define PPUDATA $2007
.define OAMDMA $4014

;===============================================
; Constants
;===============================================

.define activeRomLow  $0020
.define activeRomHigh $0021

.define scriptIdNum $00CC
.define scriptRegionNum $00CD

.define lineNum $00DD

.define activeSaveData $6400

.define textRightOffset $05A0
.define spkrId $05A8

.define scratchLo $0509



.define twoLineItemRegion $19
.define pluralEnemyNameRegion $1A

;===============================================
; Existing routines
;===============================================

.define switchRomLow  $D3F9
.define switchRomHigh $D414
.define mult $D614

.define setMsgPos $EAE8

.define nmiWait $F7F4

.define keyWait $F8C5

.define clearRemainingBattleText $F970

.define clearPackagedWram $FA63
.define pendingWramToPpu $EA40

.define fetchScriptByte $FAAD

.define clearBoxAndRunPendingScript $F5EB
.define runPendingScript $F5F7

; BANK $19
.define doLinebreak $BD94

;===============================================
; Additional memory
;===============================================

.define expMem $6790

; block at $6790-$67FF is not used
.enum expMem
  expMemStart .db
  
  ; this flag is checked after KEY commands, etc. if set, the speaker's
  ; name is printed, then the flag is cleared.
  spkrPrintNeededFlag db
  
  ; after a HALFBR op, this is set to FF.
  ; when the next HALFBR is encountered, the linenum is incremented and
  ; this flag cleared.
  halfBrOn db
  
  ; address of "longjmp" routine, loaded into memory during game boot.
  ; must be last!! wla-dx cannot understand the size of the routine well
  ; enough to use it in an enum, so we'd better hope we don't need to load
  ; anything else
  longjmpWramLoadAddr .db
  longjmp .db
  expMemEnd .db
.ende

.macro doLongjmp
  jsr longjmp
  .db :\1
  .dw \1
.endm

.macro endNoRetLongjmp
  ; replace longjmp retaddr with our own target
  
  ; pull local retaddr
  pla
;        tax
  pla
;        tay
  
  ; pull banknum
  pla
  tay
    
      ; pull longjmp retaddr
      pla
      pla
      
      ; push new target (make up work)
      lda #>(\1-1)
      pha
      lda #<(\1-1)
      pha
    
    ; restore local retaddr
;        tya
;        pha
;        txa
;        pha
  
  ; push banknum
  tya
  pha
  
  jmp longjmpWramLoadAddr+(longjmpEndCall-longjmpWramStart)
.endm
  
;===============================================
; Disable diacritics
;===============================================

; handakuten handler
.unbackground $33BDC $33BEA
; dakuten handler
.unbackground $33C1A $33C28
; shared logic for writing diacritics
.unbackground $33C29 $33C3A

.bank $19 slot 3
.org $1BCC
.section "no diacritics" overwrite
  ; branch if control code (E2-FE, plus FF = space)
;  cmp #$E2
  cmp #lowestOpcode
  bcs controlCodeHandler
  ; branch if regular character (00-A5)
;  cmp #$A6
;  bcc literalHandler
  bcc literalHandler
.ends

;.bank $19 slot 3
;.org $1C06
;.section "control code handler" overwrite
;  controlCodeHandler:
;    ; branch to literal print if FF (space)
;    cmp #$FF
;    ; ...
;.ends

.bank $19 slot 3
.org $1BFB
.section "literal handler" overwrite
  literalHandler:
    ; write character to output
    sta ($00B6),Y
    ; ...
.ends

;===============================================
; New script
;===============================================

;.slot 2
;.section "new script 00" superfree
;  regionRegion00:
;    .incbin "out/script/script_region00.bin"
;.ends

.include "out/script/script_include.inc"

; uncomment to use the auto-generated superfree sections for each region
;.include "out/script/script_include_sections.inc"

; however, due to the bankswitching NMI wait penalty, we can achieve
; some speedup by manually arranging the regions so that scripts
; in different regions that are frequently accessed together go in the
; same bank.
;
; per the original game, the groupings are as follows:
; * regions 00, 02, 03, 04, 09, 12, 13 -- bank 0x8
.slot 2
.section "script regions 1a" superfree
  script_region00:
    .incbin "out/script/script_region00.bin"
  script_region03:
    .incbin "out/script/script_region03.bin"
  script_region04:
    .incbin "out/script/script_region04.bin"
  script_region09:
    .incbin "out/script/script_region09.bin"
  script_region12:
    .incbin "out/script/script_region12.bin"
  script_region13:
    .incbin "out/script/script_region13.bin"
.ends
.slot 2
.section "script regions 1b" superfree
  script_region02:
    .incbin "out/script/script_region02.bin"
.ends

; * regions 01, 08 -- bank 0x10
.slot 2
.section "script regions 2" superfree
  script_region01:
    .incbin "out/script/script_region01.bin"
  script_region08:
    .incbin "out/script/script_region08.bin"
  script_region0A:
    .incbin "out/script/script_region0A.bin"
.ends

; * regions 06, 0C, 0D, 10 -- bank 0x9
.slot 2
.section "script regions 3a" superfree
  script_region06:
    .incbin "out/script/script_region06.bin"
  script_region0C:
    .incbin "out/script/script_region0C.bin"
.ends
.slot 2
.section "script regions 3b" superfree
  script_region0D:
    .incbin "out/script/script_region0D.bin"
.ends
.slot 2
.section "script regions 3c" superfree
  script_region10:
    .incbin "out/script/script_region10.bin"
.ends

; * regions 07, 0A, 0F -- bank 0xB
.slot 2
.section "script regions 4a" superfree
  script_region07:
    .incbin "out/script/script_region07.bin"
.ends
.slot 2
.section "script regions 4b" superfree
  script_region0F:
    .incbin "out/script/script_region0F.bin"
.ends

; all other regions (05, 0B, 0E, 11, 14, 15) have unique banks
.slot 2
.section "script regions 5" superfree
  script_region05:
    .incbin "out/script/script_region05.bin"
.ends
.slot 2
.section "script regions 6" superfree
  script_region0B:
    .incbin "out/script/script_region0B.bin"
.ends
.slot 2
.section "script regions 7" superfree
  script_region0E:
    .incbin "out/script/script_region0E.bin"
.ends
.slot 2
.section "script regions 8" superfree
  script_region11:
    .incbin "out/script/script_region11.bin"
.ends
.slot 2
.section "script regions 9" superfree
  script_region14:
    .incbin "out/script/script_region14.bin"
.ends
.slot 2
.section "script regions 10" superfree
  script_region15:
    .incbin "out/script/script_region15.bin"
.ends
.slot 2
.section "script regions 11" superfree
  script_region16:
    .incbin "out/script/script_region16.bin"
.ends
.slot 2
.section "script regions 12" superfree
  script_region17:
    .incbin "out/script/script_region17.bin"
.ends
.slot 2
.section "script regions 13" superfree
  script_region18:
    .incbin "out/script/script_region18.bin"
.ends
.slot 2
.section "script regions 14" superfree
  script_region19:
    .incbin "out/script/script_region19.bin"
.ends
.slot 2
.section "script regions 15" superfree
  script_region1A:
    .incbin "out/script/script_region1A.bin"
.ends

;===============================================
; Use new script region tables
;===============================================

  ; old region tables
  .unbackground $33AAA $33AEB

  ; split points for regions that have been split due to size
  .define region05_part2_start $6B
  .define region05_part2_regionnum $16
  .define region0B_part2_start $62
  .define region0B_part2_regionnum $17
  .define region0E_part2_start $5C
  .define region0E_part2_regionnum $18

  ;=====
  ; region splitting logic
  ;=====
  
  ; regions 05, 0B, and 0E are too large to contain the translated text
  ; even after compression, so they're split into two parts and redirected
  ; in code

  .bank $3F slot 5
  .org $158D
  .section "region splitting 1" overwrite
    jsr triggerDoRegionSplit
  .ends

  .bank $19 slot 3
  .section "region splitting 2" free
    triggerDoRegionSplit:
      doLongjmp doRegionSplit

      ; make up work
      jmp $BAEC
  .ends

  .slot 3
  .section "region splitting 3" superfree
    doRegionSplit:
      lda scriptRegionNum
      
      cmp #$0E
      beq @region0E
      cmp #$0B
      beq @region0B
      cmp #$05
      beq @region05
      
      @done:
      rts
      
      @region05:
        lda scriptIdNum
;        cmp region05_part2_start
        ; subtract split value from script ID
        sec
        sbc #region05_part2_start
        ; branch if went past 0
        bcc @done
        
        ; correct script identifier
        sta scriptIdNum
        lda #region05_part2_regionnum
        sta scriptRegionNum
        
        rts
      @region0B:
        lda scriptIdNum
        ; subtract split value from script ID
        sec
        sbc #region0B_part2_start
        ; branch if went past 0
        bcc @done
        
        ; correct script identifier
        sta scriptIdNum
        lda #region0B_part2_regionnum
        sta scriptRegionNum
        
        rts
      @region0E:
        lda scriptIdNum
        ; subtract split value from script ID
        sec
        sbc #region0E_part2_start
        ; branch if went past 0
        bcc @done
        
        ; correct script identifier
        sta scriptIdNum
        lda #region0E_part2_regionnum
        sta scriptRegionNum
        
        rts
      
  .ends

; after testing: indexing scripts is no faster than just
; seeking through them as the game already does. wonderful.
.define USE_DIRECT_SCRIPT_LOOKUP 0

.if USE_DIRECT_SCRIPT_LOOKUP != 0

  ;===============================================
  ; Use index tables instead of script skipping
  ;===============================================

  .include "out/script/script_index_include.inc"

  .bank $19 slot 3
  .org $1AEC
  .section "use script index tables" overwrite
    ; $00CC = target script index
    ; $00CD = target region
    ;
    ; goal: switch to correct bank for region and return script pointer
    ; in $00B8
    
    ; get bank
    ldx $00CD
    lda scriptRegionToIndexRoutineBank.w,X
    ; save to longjmp
    sta longjmpWramLoadAddr+(longjmpWramStart@callBankOp-longjmpWramStart+1).w
    
    ; get pointer
    txa
    asl
    tax
    ; save to longjmp
    lda scriptRegionToIndexRoutinePointer.w,X
    sta longjmpWramLoadAddr+(longjmpWramStart@callAddrOp-longjmpWramStart+1).w
    lda scriptRegionToIndexRoutinePointer+1.w,X
    sta longjmpWramLoadAddr+(longjmpWramStart@callAddrOp-longjmpWramStart+2).w
    
    ; call manual longjmp
    jmp longjmpWramLoadAddr+(longjmpWramStart@longjmpManualCall-longjmpWramStart)
    
  ;  rts
    
  .ends

.else

  ;=====
  ; new region lookup tables
  ;
  ; (now generated by the script builder and included instead of being
  ; handled manually here)
  ;=====
  
/*  ; if dictionary is disabled, wla-dx will forcibly and completely unavoidably
  ; throw away the empty sections for regions 11, 13, and/or 14.
  ; this breaks the build since the appropriate labels are no longer defined.
  ; we cannot even check for their presence with IFDEF to insert filler
  ; values.
  ; nor can we append a filler byte at this point, as the sections are thrown
  ; away as soon as the include file is done processing.
  ; we have no choice but to do it in the include file generator, which is
  ; more trouble than i'm willing to go to.
  ; i can only hope the empty-section-discard bug-feature will be removed
  ; in the future.

  ; region->bank table
  .bank $19 slot 3
  ;.org $1AD6
  .section "script region to bank table" free
    scriptRegionBankTable:
      .db :script_region00
      .db :script_region01
      .db :script_region02
      .db :script_region03
      .db :script_region04
      .db :script_region05
      .db :script_region06
      .db :script_region07
      .db :script_region08
      .db :script_region09
      .db :script_region0A
      .db :script_region0B
      .db :script_region0C
      .db :script_region0D
      .db :script_region0E
      .db :script_region0F
      .db :script_region10
      .db :script_region11
      .db :script_region12
      .db :script_region13
      .db :script_region14
      .db :script_region15
      .db :script_region16
      .db :script_region17
      .db :script_region18
  .ends

  ; region pointer table
  ; NOTE: pointers must be 1 less than actual value due to preincrement!
  .bank $19 slot 3
  ;.org $1AAA
  .section "script region to decremented pointer table" free
    scriptRegionBasePtrTable:
      .dw script_region00-1
      .dw script_region01-1
      .dw script_region02-1
      .dw script_region03-1
      .dw script_region04-1
      .dw script_region05-1
      .dw script_region06-1
      .dw script_region07-1
      .dw script_region08-1
      .dw script_region09-1
      .dw script_region0A-1
      .dw script_region0B-1
      .dw script_region0C-1
      .dw script_region0D-1
      .dw script_region0E-1
      .dw script_region0F-1
      .dw script_region10-1
      .dw script_region11-1
      .dw script_region12-1
      .dw script_region13-1
      .dw script_region14-1
      .dw script_region15-1
      .dw script_region16-1
      .dw script_region17-1
      .dw script_region18-1
  .ends */

  ;=====
  ; use new region lookup tables
  ;=====

  .bank $19 slot 3
  .org $1AEE
  .section "use new script tables 1" overwrite
    lda scriptRegionBankTable.w,X
  .ends

  .bank $19 slot 3
  .org $1B00
  .section "use new script tables 2" overwrite
    lda (scriptRegionBasePtrTable+0).w,X
    sta $00B8
    lda (scriptRegionBasePtrTable+1).w,X
    sta $00B9
  .ends
  
.endif

;===============================================
; new save file block.
; we need extra space in the save file for
; extended character (tank?) names.
; fortuitously, there are #$22 unused bytes in
; the save file, so we add some extra code to
; save and load them.
;===============================================

.define newSaveBlockDst $676E
.define newSaveBlockSize $22

; blocks 2 and 3 are copied to contiguous memory locations.
; free up space by reducing this to a single combined copy
.bank $14 slot 3
.org $1D79
.section "load new save file blocks 1" overwrite
  lda #$40      ; copy size (old = #$20)
.ends

; load our new "block 6" in place of block 3
.bank $14 slot 3
.org $1D84
.section "load new save file blocks 2" overwrite
  ; dst
  lda #<newSaveBlockDst
  sta $000A
  lda #>newSaveBlockDst
  sta $000B
  ; size
  lda #<newSaveBlockSize
  sta $000C
  lda #>newSaveBlockSize
  sta $000D
.ends

.bank $14 slot 3
.org $1DEF
.section "save new save file blocks 1" overwrite
  lda #$40      ; copy size (old = #$20)
.ends

; save our new "block 6" in place of block 3
.bank $14 slot 3
.org $1DFA
.section "save new save file blocks 2" overwrite
  ; src
  lda #<newSaveBlockDst
  sta $0008
  lda #>newSaveBlockDst
  sta $0009
  ; size
  lda #<newSaveBlockSize
  sta $000C
  lda #>newSaveBlockSize
  sta $000D
.ends

;===============================================
; expanded names
;
; The original game allocates #$47 bytes in the
; save file for character/tank names. We add
; #$22 non-contiguous bytes to this for a total
; of #$69 bytes.
;===============================================

  ;===============================================
  ; basic definitions
  ;===============================================

  .define charNameFullLen $9
  .define tankNameFullLen $8
  .define charNameCharCount charNameFullLen-1
  .define tankNameCharCount tankNameFullLen-1
  .define saveBlock0Offset $0
  .define saveBlock6Offset $36E
  .define saveBlock0Addr activeSaveData+saveBlock0Offset
  .define saveBlock6Addr activeSaveData+saveBlock6Offset
  .define saveBlock0NameChunkSize $47
  .define saveBlock6NameChunkSize $22

  ;.define char1NameOffset activeSaveData+(charNameFullLen*0)
  ;.define char2NameOffset activeSaveData+(charNameFullLen*1)
  ;.define char3NameOffset activeSaveData+(charNameFullLen*2)
  ;.define tank1NameOffset activeSaveData+(charNameFullLen*3)
  ;.define tank2NameOffset activeSaveData+tank1NameOffset+(tankNameFullLen*1)
  ;.define tank3NameOffset activeSaveData+tank1NameOffset+(tankNameFullLen*2)

  ; Block 0 names
  .enum activeSaveData+saveBlock0Offset
    newBlock0NameChunkStart .db
    
    ; 3 characters = #$1B bytes
    char1Name ds charNameFullLen
    char2Name ds charNameFullLen
    char3Name ds charNameFullLen
    ; 5 tanks = #$28 bytes
    tank1Name ds tankNameFullLen
    tank2Name ds tankNameFullLen
    tank3Name ds tankNameFullLen
    tank4Name ds tankNameFullLen
    tank5Name ds tankNameFullLen
    
    newBlock0NameChunkEnd .db
  .ende

  ; Block 6 names
  .enum activeSaveData+saveBlock6Offset
    newBlock6NameChunkStart .db
    
    ; 3 tanks = #$18 bytes
    tank6Name ds tankNameFullLen
    tank7Name ds tankNameFullLen
    tank8Name ds tankNameFullLen
    
    newBlock6NameChunkEnd .db
  .ende
  
  ; rental taaaaAAAAAAAAANNNNNNNNNKKKSSSSSS
  .define rentalTankNameBase $6447
  .define rentalTank1Name rentalTankNameBase+(0*7)
  .define rentalTank2Name rentalTankNameBase+(1*7)
  .define rentalTank3Name rentalTankNameBase+(2*7)

  .define newBlock0NameChunkSize newBlock0NameChunkEnd-newBlock0NameChunkStart
  .define newBlock6NameChunkSize newBlock6NameChunkEnd-newBlock6NameChunkStart

  .if newBlock0NameChunkSize > saveBlock0NameChunkSize
    .printt "Error: New block 0 name chunk too big"
    .fail
  .endif

  .if newBlock6NameChunkSize > saveBlock6NameChunkSize
    .printt "Error: New block 6 name chunk too big"
    .fail
  .endif
  
  ;===============================================
  ; copy to new name offsets when finishing up
  ; on naming screen
  ;===============================================
  
  .bank $10 slot 3
  .org $16C6
  .section "update name pointers 1" overwrite
    .dw char1Name
    
    .dw tank1Name
    .dw tank2Name
    .dw tank3Name
    .dw tank4Name
    .dw tank5Name
    .dw tank6Name
    .dw tank7Name
    .dw tank8Name
  .ends
  
  ;===============================================
  ; use new name offsets in variable prints
  ;===============================================
  
  .bank $19 slot 3
  .org $1CAA
  .section "update name pointers 2" overwrite
    .dw char1Name
    .dw char2Name
    .dw char3Name
  .ends
  
  ;===============================================
  ; use new name offsets everywhere else(?)
  ;===============================================
  
  .bank $3F slot 5
  .section "new tank name table" free
    newTankNameTable:
      .dw tank1Name
      .dw tank2Name
      .dw tank3Name
      .dw tank4Name
      .dw tank5Name
      .dw tank6Name
      .dw tank7Name
      .dw tank8Name
      
      .dw rentalTank1Name
      .dw rentalTank2Name
      .dw rentalTank3Name
  .ends
  
  .bank $3F slot 5
  .org $138C
  .section "use new tank name table 1" SIZE $D overwrite
    ; 13 available bytes
    ; get tank index
;    lda $00D0
    ; look up address
    txa
    asl
    tax
    lda (newTankNameTable+0).w,X
    sta $000C
    ; do stuff that won't fit
    jsr finishNewTankNameLookup
    bpl tankOrCharNameBufferCopyLoop
    
  .ends
  
  ; branch target
  .bank $3F slot 5
  .org $13A6
  .section "use new tank name table 2" overwrite
    tankOrCharNameBufferCopyLoop:
      ldx #$00
  .ends
  
  .bank $3F slot 5
  .section "use new tank name table 3" free
    finishNewTankNameLookup:
      lda (newTankNameTable+1).w,X
      sta $000D
      ; Y = 0 for copy loop
      ldy #$00
      rts
  .ends
  
  .bank $3F slot 5
  .org $06FE
  .section "use long char names 1" overwrite
    ; table of offsets in save file at which character names are stored
    .db charNameFullLen*0
    .db charNameFullLen*1
    .db charNameFullLen*2
  .ends
  
  ;===============================================
  ; use new name offsets on tank equipment
  ; transfer screen
  ;===============================================
  
  .bank $19 slot 3
  .org $1173
  .section "use new tank name table item give 1" overwrite
    doLongjmp useNewTankTableItemGive
    jmp $B17E
  .ends
  
  .slot 3
  .section "use new tank name table item give 2" superfree
    useNewTankTableItemGive:
      
;      lda $00D7 ; receipient tank index
      lda $0005 ; receipient tank index
      asl
      tax
      lda (newTankNameTable+0).w,X
      sta $000C
      lda (newTankNameTable+1).w,X
      sta $000D
      
      ; copy loop
      ldy #$00
      ldx #$00
      -:
        lda ($000C),Y
        sta $0524,X
        inx
        iny
        cmp #$9F
        bne -
      
      rts
    
    useNewHeroTableItemGive:
      
      ldx $0005 ; receipient hero index
      lda $E6FE,X
      sta $00D7
      
      ; copy giver name to $0500
      jsr $F37F
      
;      lda #$00
;      sta $000C
;      lda #$64
;      sta $000D
      
      ; copy loop
      ldx #$00
      ldy $00D7
      -:
        lda ($000C),Y
        sta $0524,X
        inx
        iny
        cmp #$9F
        bne -
      
      rts
  .ends
  
  .bank $19 slot 3
  .org $114E
  .section "use new hero name table item give 1" overwrite
    doLongjmp useNewHeroTableItemGive
    lda #$17
    jmp $B17F
  .ends
  
  ;===============================================
  ; initialize new block 6 to #$9F in new game
  ; save file (otherwise, game will zero-init
  ; it and crash trying to show the non-existent
  ; tank names)
  ;===============================================
  
;  .define longjmpWramLoadAddr $6790
;  .define longjmp longjmpWramLoadAddr
  
  .bank $14 slot 3
  .org $003F
  .section "block 6 init 1" overwrite
    jsr newBlock6Init
  .ends
  
  .bank $14 slot 3
  .section "block 6 init 2" free
    newBlock6Init:
      ; make up work
      jsr $A083
      
      ; memset
      ; dst
      lda #<saveBlock6Addr
      sta $000A
      lda #>saveBlock6Addr
      sta $000B
      ; size
      lda #<saveBlock6NameChunkSize
      sta $000E
      lda #>saveBlock6NameChunkSize
      sta $000F
      ; value
      lda #$9F
      sta $0000
      
      ; fill
      jsr $A083
      
      ; copy longjmp routine to WRAM
      ; address to ret into
      lda #>(longjmpWramCopy-1)
      pha
      lda #<(longjmpWramCopy-1)
      pha
      ldx #:longjmpWramCopy
      jmp switchRomHigh
      
/*      pha
      txa
      pha
      
          ldx #(longjmpWramEnd-longjmpWramStart-1)
        -:
          lda longjmpWramStart.w,X
          sta longjmpWramLoadAddr.w,X
          
          dex
          bpl -
      
      pla
      tax
      pla */
      
;      rts
  .ends
  
  ;========
  ; copy longjmp routine to WRAM
  ;========
  
  .slot 3
  .section "longjmp WRAM routine copy" superfree
;    test:
;      nop
;      nop
;      nop
;      rts
  
    longjmpWramCopy:
      
    
;      pha
;      txa
;      pha
      
          ldx #(longjmpWramEnd-longjmpWramStart-1)
        -:
          lda longjmpWramStart.w,X
          sta longjmpWramLoadAddr.w,X
          
          dex
          bpl -
      
;      pla
;      tax
;      pla

;      jsr longjmp
;      .db :test
;      .dw test
      
      ; return to original location
      ldx #:newBlock6Init
      jmp switchRomHigh
      
;      rts
  
    longjmpWramStart:
      ; return address (+1) points to:
      ; * target bank (slot 3)
      ; * target address (LE)
      
      ; pull low byte of ret addr (adding 1 to get param pointer)
      pla
      clc
      adc #$01
      sta longjmpWramLoadAddr+(@srcDataAddrOp-longjmpWramStart+1).w
      
      ; pull high byte of ret addr
      pla
      adc #$00
      sta longjmpWramLoadAddr+(@srcDataAddrOp-longjmpWramStart+2).w
      
      ; increment to get param pointer
;      inc longjmpWramLoadAddr+(@srcDataAddrOp-longjmpWramStart+1).w
;      bne +
;        inc longjmpWramLoadAddr+(@srcDataAddrOp-longjmpWramStart+2).w
;      +:

;      txa
;      pha
      
;        ; set up return jump
;        lda longjmpWramLoadAddr+(@srcDataAddrOp-longjmpWramStart+1).w
;        clc
;        adc #$03
;        sta longjmpWramLoadAddr+(@retAddrOp-longjmpWramStart+1).w
;        lda longjmpWramLoadAddr+(@srcDataAddrOp-longjmpWramStart+2).w
;        adc #$00
;        sta longjmpWramLoadAddr+(@retAddrOp-longjmpWramStart+2).w
        ; save return address
        lda longjmpWramLoadAddr+(@srcDataAddrOp-longjmpWramStart+1).w
        clc
        ; add 2 to get (retaddr - 1)
        adc #$02
        ; save low byte to X
        tax
        lda longjmpWramLoadAddr+(@srcDataAddrOp-longjmpWramStart+2).w
        adc #$00
        ; push high byte
        pha
        txa
        ; push low byte
        pha
        
        ldx #$00
        @fetchParams:
          @srcDataAddrOp:
          lda $FFFF.w,X
          
          cpx #$00
          beq @setCallBank
          @setCallAddr:
            sta longjmpWramLoadAddr+(@callAddrOp-longjmpWramStart-0).w,X
            bne @fetchParamLoopEnd
          @setCallBank:
            sta longjmpWramLoadAddr+(@callBankOp-longjmpWramStart+1).w
          
          @fetchParamLoopEnd:
          inx
          cpx #$03
          bne @fetchParams
        
        @longjmpManualCall:
        lda activeRomHigh
        pha
          
          ; so, think about it: we have to disable IRQs to safely write the
          ; MMC3 banking register. no IRQs means no mid-frame CHR swaps, which
          ; means if we don't wait for NMI before doing the bankswitch, we can
          ; and will occasionally miss an IRQ, resulting in a glitched frame.
          ;
          ; on the other hand, waiting for vblank every time we call this
          ; routine causes an absurd amount of lag (over 90 frames to open
          ; the main menu vs. 30 in the original game, and 45 in the translation
          ; without manual section arrangement). which is worse: a rare major
          ; aesthetic issue or incessant minor gameplay annoyances?
          ; for now, I'm taking the former over the latter.
;          jsr nmiWait
        
            @callBankOp:
            ; modified to target bank num
            ldx #$00
            jsr switchRomHigh
          
            @callAddrOp:
            ; modified to target routine address
            jsr $FFFF
          
;          jsr nmiWait
          
          ; params
;          txa
;          sta longjmpWramLoadAddr+(@retX-longjmpWramStart).w
;          tya
;          sta longjmpWramLoadAddr+(@retY-longjmpWramStart).w
          
        ; restore old bank, ret into original location (+3)
        longjmpEndCall:
        pla
        tax
        jmp switchRomHigh
        
        ; restore ret params
;        lda longjmpWramLoadAddr+(@retX-longjmpWramStart).w
;        tax
;        lda longjmpWramLoadAddr+(@retY-longjmpWramStart).w
;        tay
    
;      @retAddrOp:
;      ; modified to call return address
;      jmp $FFFF
      
;      @retAddr:
;      .dw $FFFF
      
;      ; space to store returned X/Y
;      @retX:
;      .db $FF
;      @retY:
;      .db $FF
      
    longjmpWramEnd:
  .ends
  .define longjmpWramSize longjmpWramEnd-longjmpWramStart
  
  .define longjmpDone longjmpWramLoadAddr+(longjmpEndCall-longjmpWramStart)
  
  ;========
  ; this routine is copied to WRAM on startup
  ;========
  
;  .bank $14 slot 3
;  .bank $20 slot 3
;  .section "longjmp WRAM routine" free
;  .ends
  
;  .define longjmpWramSize longjmpWramEnd-longjmpWramStart
;  .if longjmpWramSize > #$60
;    .printt "Error: longjmpWramSize too large"
;    .fail
;  .endif
  
  ;===============================================
  ; the game relies on both character and tank
  ; names being copied to a buffer at 051D.
  ; this buffer is exactly 7 bytes in size.
  ; we need 9 now.
  ; so, we have to move this buffer and update
  ; all references to it.
  ;===============================================
  
  .define newNameBuffer $0500
  
  ; script variable table
  .bank $19 slot 3
  .org $1CB8
  .section "move name buffer 1" overwrite
    .dw newNameBuffer
  .ends
  
  ; !!! naming screen !!!
  .bank $10 slot 3
  .org $1555
  .section "move name buffer 2" overwrite
    sta newNameBuffer+1.w,X
  .ends
  
  ; !!! naming screen !!!
  .bank $10 slot 3
  .org $155D
  .section "move name buffer 3" overwrite
    sta newNameBuffer+0.w,X
  .ends
  
  ; !!! naming screen !!!
  .bank $10 slot 3
  .org $165F
  .section "move name buffer 4" overwrite
    lda newNameBuffer
  .ends
  
  ; !!! naming screen !!!
  .bank $10 slot 3
  .org $1680
  .section "move name buffer 5" overwrite
    lda newNameBuffer.w,X
  .ends
  
  ; !!! naming screen !!!
  .bank $10 slot 3
  .org $1688
  .section "move name buffer 6" overwrite
    lda newNameBuffer.w,X
  .ends
  
  ; !!! naming screen !!!
  .bank $10 slot 3
  .org $16AB
  .section "move name buffer 7" overwrite
    lda newNameBuffer,Y
  .ends
  
  ; !!! naming screen !!!
  .bank $10 slot 3
  .org $16B4
  .section "move name buffer 8" overwrite
    sta newNameBuffer,Y
  .ends
  
  ; !!! naming screen !!!
  .bank $10 slot 3
  .org $16BD
  .section "move name buffer 9" overwrite
    lda newNameBuffer,Y
  .ends
  
  ; !!! naming screen !!!
  .bank $14 slot 3
  .org $1B71
  .section "move name buffer 10" overwrite
    lda newNameBuffer+0
    clc 
    adc newNameBuffer+1
    clc 
    adc newNameBuffer+2
    clc 
    adc newNameBuffer+3
  .ends
  
  ; !!! naming screen !!!
  .bank $14 slot 3
  .org $1B9E
  .section "move name buffer 11" overwrite
    lda newNameBuffer+0
    sec
    sbc newNameBuffer+1
    sec
    sbc newNameBuffer+2
    sec
    sbc newNameBuffer+3
  .ends
  
  ; !!! naming screen !!!
  .bank $14 slot 3
  .org $1BCE
  .section "move name buffer 12" overwrite
    lda newNameBuffer,Y
  .ends
  
  .bank $18 slot 3
  .org $021B
  .section "move name buffer 13" overwrite
    sta newNameBuffer+0
  .ends
  
  ; MOVED
/*  .bank $18 slot 3
  .org $0220
  .section "move name buffer 14" overwrite
    sta newNameBuffer+1
  .ends */
  
  .bank $18 slot 3
  .org $1DAE
  .section "move name buffer 15" overwrite
    lda newNameBuffer,X
  .ends
  
  .bank $3F slot 5
  .org $13AA
  .section "move name buffer 16" overwrite
    sta newNameBuffer,X
  .ends
  
  .bank $3F slot 5
  .org $1757
  .section "move name buffer 17" overwrite
    sta newNameBuffer,X
  .ends
  
  ; HERO
  .bank $19 slot 3
  .org $1E68
  .section "move name buffer 18" overwrite
    lda #<newNameBuffer
    sta $000C
    lda #>newNameBuffer
    sta $000D
  .ends
  
  ;=====
  ; number entry on noah code
  ;=====
  
  .bank $10 slot 3
  .org $028A
  .section "move name buffer 19" overwrite
    ; initialization pos
    sta newNameBuffer-1.w,X
  .ends
  
  .bank $10 slot 3
  .org $0294
  .section "move name buffer 20" overwrite
    ; position of numstring terminator
    sta newNameBuffer+6
  .ends
  
  .bank $18 slot 3
  .org $1D4F
  .section "move name buffer 21" overwrite
    ; write position for updates
    sta newNameBuffer,Y
  .ends

;===============================================
; naming screen
;===============================================

; WRAM buffer for displayed name during hero name entry
.define heroWramTarget1 $608C     ; orig: 608E
.define heroWramTarget2 $60AC     ; orig: 60AE
.define tankWramTarget1 $608C     ; orig: 608D
.define tankWramTarget2 $60AC     ; orig: 60AD

;.bank $10 slot 3
;.org $153D
;.section "hero name length" overwrite
;  ldx #(charNameFullLen-2)
;.ends

.bank $3F slot 5
.org $16A9
.section "hero name PPU WRAM src" overwrite
  .dw heroWramTarget1
.ends

.bank $3F slot 5
.org $16E9
.section "hero name PPU dimensions" overwrite
  .db $0C,$04 ; x/y (orig: 0D, 04)
  .db $08,$02 ; w/h (orig: 06, 02)
.ends

; when clearing content
.bank $3F slot 5
.org $19DE
.section "hero name WRAM target" overwrite
  .dw heroWramTarget1
.ends

.bank $3F slot 5
.org $1543
.section "hero name WRAM target 2" overwrite
  lda #<heroWramTarget2
.ends




.bank $10 slot 3
.org $1539
.section "hero/tank name length 1" overwrite
  jsr getHeroOrTankNameLen
  nop
  nop
  nop
.ends

.bank $10 slot 3
.section "hero/tank name length 2" free
  getHeroOrTankNameLen:
    lda $00D5
    bne @namingTank
    
    @namingHero:
      ldx #(charNameFullLen-2)
      rts
    
    @namingTank:
      ldx #(tankNameFullLen-2)
      rts
.ends

.bank $3F slot 5
.org $1549
.section "tank name WRAM target 2" overwrite
  lda #<tankWramTarget2
.ends

  ;=======
  ; new name position indicator x-position table
  ;=======
  
  .bank $10 slot 3
  .section "new name position indicator x-position table" free
    newNamePosXTable:
      .db $60,$68,$70,$78,$80,$88,$90,$98
  .ends
  
  .bank $10 slot 3
  .org $158A
  .section "use new name position indicator x-position table 1" overwrite
    lda #<newNamePosXTable
    sta $00CA
    lda #>newNamePosXTable
    sta $00CB
        
    lda #$4C
    ldx $00D5
    beq +
      ; if naming tank
      lda #<newNamePosXTable
      sta $00CA
      lda #>newNamePosXTable
      sta $00CB
      lda #$4E
    +:

  .ends

  ;===============================================
  ; new character map
  ;===============================================
  
  .define namingOptionsBaseY $48
  .define namingDoneIndex $0F
  .define namingNextIndex $23
  .define namingPrevIndex $37
  
  .define maxNamingCursorY $98
  
  .bank $10 slot 3
  .org $146C
  .section "use new name entry table layout 1" overwrite
    .incbin "out/script/name_entry.bin"
  .ends
  
  ; remove hardcoded diacritic in もどる
  .bank $10 slot 3
  .org $15E8
  .section "use new name entry table layout 2" overwrite
    jmp $B5ED
  .ends
  
  ; disable diacritics in name
  .bank $10 slot 3
  .org $1613
  .section "use new name entry table layout 3" overwrite
    jmp $B61B
  .ends
  
  ; move options to top
  .bank $10 slot 3
  .org $178A
  .section "use new name entry table layout 4" overwrite
    ; array of indices which redirect to an option
    .db namingPrevIndex+1,namingPrevIndex+2,namingPrevIndex+3,namingPrevIndex+4
    .db namingNextIndex+1
    .db namingDoneIndex+1
  .ends
  
  .bank $10 slot 3
  .org $1790
  .section "use new name entry table layout 5" overwrite
    ; array of redirects for options
    .db namingDoneIndex,namingNextIndex,namingPrevIndex
  .ends
  
  .bank $10 slot 3
  .org $17A8
  .section "use new name entry table layout 6" overwrite
    sbc #namingOptionsBaseY
  .ends
  
  .bank $10 slot 3
  .org $161B
  .section "use new name entry table layout 7" overwrite
    ; check new option indices
    cpy #namingPrevIndex
    beq namingPrev
    cpy #namingNextIndex
    beq namingNext
    cpy #namingDoneIndex
    beq namingDone
  .ends
  
  .bank $10 slot 3
  .org $1646
  .section "naming prev" overwrite
    namingPrev:
      ldx $00D0
  .ends
  
  .bank $10 slot 3
  .org $162D
  .section "naming next" overwrite
    namingNext:
      lda $00D0
  .ends
  
  .bank $10 slot 3
  .org $1658
  .section "naming done" overwrite
    namingDone:
      jsr $B69B
  .ends
  
  ;=======
  ; disable access to now-unused bottom rows
  ;=======
  
  .bank $10 slot 3
  .org $1732
  .section "naming bottom row 1" overwrite
    cmp #maxNamingCursorY+1
    bcc namingBottomRowSkip
      lda #maxNamingCursorY
  .ends
  
  .bank $10 slot 3
  .org $1743
  .section "naming bottom row 2" overwrite
    namingBottomRowSkip:
      lda $00D1
  .ends
  
  ;=======
  ; cursor sensitivity
  ;=======

  ; the original game's naming screen cursor is very sensitive and makes
  ; fine adjustments difficult, so i've toned down the movement rate a bit
  .bank $10 slot 3
  .org $1534
  .section "naming screen cursor sensitivity" overwrite
    ldx #$05+2  ; higher = slower (frames between movement)
  .ends

;===============================================
; new party naming logic
;===============================================


.bank $14 slot 3
.org $1B4F
.section "use new party naming logic" overwrite
  doLongjmp newPartyNamingLogic
  rts
.ends

.slot 3
.section "new party naming logic" superfree
  specialParties:
    .incbin "out/script/names_specialparties.bin" FSIZE specialPartiesSize
  randNamesChar2:
    .incbin "out/script/names_random_char2.bin" FSIZE randNamesChar2Size
  randNamesChar3:
    .incbin "out/script/names_random_char3.bin" FSIZE randNamesChar3Size
  
  .define specialPartySize (charNameCharCount * 4)
  .define numSpecialParties (specialPartiesSize / specialPartySize)
  .define numChar2Names (randNamesChar2Size / charNameCharCount)
  .define numChar3Names (randNamesChar3Size / charNameCharCount)
  
  newPartyNamingLogic:
    ldx #$00
    @specialPartyCheckLoop:
      ; derive pointer to default party data in 000C
      stx $0000
      
      ; multiply index by specialPartySize
      lda #specialPartySize
      sta $0001
      jsr mult
      
      ; add result to base party array position
      lda $000E
      clc 
      adc #<specialParties
      sta $000C
      lda $000F
      adc #>specialParties
      sta $000D
      
      ; check if current special party matches
      jsr checkSpecialParty
      
      ; done if result of call is zero (party matched and was used)
      bne @noPartyMatch
      jmp @done
        @noPartyMatch:
        ; check all special parties
        inx 
        cpx #numSpecialParties
        bne @specialPartyCheckLoop
      
    @noSpecialParty:
    
    ;=====
    ; choose a random name for character 2
    ;=====
    
    ; sum the bytes in the hero name buffer to get a "random" number
    lda newNameBuffer
    .repeat charNameCharCount-1 INDEX count
      clc
      adc newNameBuffer+count+1
    .endr
    ; result to 0000
    sta $0000
    
    ; multiply by number of random names
    lda #numChar2Names
    sta $0001
    jsr mult
    
    ; high byte of result *= name length
    lda $000F
;    asl 
;    asl 
    sta $0000
    lda #charNameCharCount
    sta $0001
    jsr mult
    lda $000E
    
    ; result to 0000
    sta $0000
    ; base pointer
    lda #<randNamesChar2
    sta $000C
    lda #>randNamesChar2
    sta $000D
    ldy $0000
    ; dstindex
    ldx #(charNameFullLen*1)
    jsr copySpecialCharName
    
    ;=====
    ; choose a random name for character 3
    ;=====
    
    ; subtract-sum the bytes in the hero name buffer to get a "random" number
    lda newNameBuffer
    .repeat charNameCharCount-1 INDEX count
      sec
      sbc newNameBuffer+count+1
    .endr
    ; result to 0000
    sta $0000
    
    ; multiply by number of random names
    lda #numChar3Names
    sta $0001
    jsr mult
    
    ; high byte of result *= name length
    lda $000F
;    asl 
;    asl 
    sta $0000
    lda #charNameCharCount
    sta $0001
    jsr mult
    lda $000E
    
    ; result to 0000
    sta $0000
    ; base pointer
    lda #<randNamesChar3
    sta $000C
    lda #>randNamesChar3
    sta $000D
    ldy $0000
    ; dstindex
    ldx #(charNameFullLen*2)
    jsr copySpecialCharName
      
    @done:
    rts

  checkSpecialParty:
    ; check for a special party
    ; $000C = special party data pointer

    ; if player name does not match check name, reject party
    ldy #$00
    -:
      lda newNameBuffer,Y
      cmp ($000C),Y
      bne @done
      iny 
      cpy #charNameCharCount
      bne -
    ; write player's name (name 1)
    ldx #(charNameFullLen*0)      ; dstoffset (in 6400)
    ldy #(charNameCharCount*1)    ; srcoffset
    jsr copySpecialCharName
    ; write char 2 name
    ldx #(charNameFullLen*1)
    ldy #(charNameCharCount*2)
    jsr copySpecialCharName
    ; write char 3 name
    ldx #(charNameFullLen*2)
    ldy #(charNameCharCount*3)
    jsr copySpecialCharName

    ; flag special party as having been used so caller knows
    lda #$00

    @done:
    rts 

  copySpecialCharName:
    ; copy special character name
    ; $000C = src base address
    ; X = dst offset
    ; Y = src offset
    
    ; copy length
    lda #charNameCharCount
    sta $0089
    -:
      lda ($000C),Y
      sta $6400,X
      iny 
      inx 
      dec $0089
      bne -
      
      ; write 9F terminator after name
      lda #$9F
      sta $6400,X
      rts 
.ends

;===============================================
; modify behavior of character attribution text
;
; in the original game, every time a KEY op
; ends and the game starts to print new text,
; it prints the name of the current speaker
; (determined by $05A8).
; since the English script frequently needs to
; split sentences across multiple boxes, this
; looks awkward, and we instead print the name
; only at the start of the box.
;===============================================

; number of characters by which to offset printing x-position after
; a linebreak to get the x-position at which to print the speaker name.
; this is 2 in the original game.
.define speakerNameOffset $02
;.define speakerNameOffset $01

/*.bank $19 slot 3
.org $1D7D
.section "test attribution line offset" overwrite
  sbc #$01
.ends*/

;.bank $19 slot 3
;.org $1D7A
;.section "test attribution line offset" overwrite
;  ; check if speaker name print needed
;  lda spkrPrintNeededFlag
;.ends


  ;=====
  ; on SPKR, set speaker print flag
  ;=====

  .bank $19 slot 3
  .org $1D6E
  .section "new SPKR 1" overwrite
    jsr triggerNewSpkrLogic
    jmp newBoxStartLogic
  .ends

/*  .slot 3
  .section "new SPKR 2" superfree
    newSpkrLogic:
      ; make up work
      jsr fetchScriptByte
      sta spkrId
    
      ; set speaker print flag
      dec spkrPrintNeededFlag
      
      rts
  .ends */

  .bank $19 slot 3
  .section "new SPKR 3" free
    triggerNewSpkrLogic:
;      jsr longjmp
;      .db :newSpkrLogic
;      .dw newSpkrLogic

      ; make up work
      jsr fetchScriptByte
      sta spkrId
    
      ; set speaker print flag
      dec spkrPrintNeededFlag
    
      ; do normal box break work
      jsr keyWait
      jsr doLinebreak
      
      ; offset WRAM putaddr in anticipation of speaker print
      sec 
      lda $00B6
      sbc #speakerNameOffset
      sta $00B6
      lda $00B7
      sbc #$00
      sta $00B7
      
      rts
  .ends

  ;=====
  ; check print flag before printing speaker name
  ;=====

  .bank $19 slot 3
  .org $1D7A
  .section "new box start logic 1" SIZE $17 overwrite
    newBoxStartLogic:
      lda spkrPrintNeededFlag
      beq +
        ; clear flag
        inc spkrPrintNeededFlag
        ; print speaker
        printSpeaker:
          lda #$0C
          sta $00CD
          lda $05A8
          jmp $BE2D
      +:
  ;    jmp $BD87
  ;    jmp $BC7D
      ; print character?
  ;    jmp $BBFB
      rts
  .ends

  ; update call for initial attribution when first box started
  .bank $3F slot 5
  .org $159D
  .section "new box start logic 2" overwrite
    jsr printSpeaker
  .ends

;  .bank $3F slot 5
;  .org $12C2
;  .section "x" overwrite
;    lda #$4C
;  .ends

/*  ;=====
  ; alter standard messages to yield a margin of 1 tile on the left instead
  ; of 2 tiles
  ;=====

  ; printing offset for standard dialogue box (non-speaker lines)
  .bank $3F slot 5
  .org $0B68
  .section "standard dialogue box base printing offset" overwrite
    lda #$0E-1    ; orig #$0E
  .ends

  ; fix linebreak (shifting the offset will also shift the area copied by a
  ; linebreak, resulting in the left column of the box getting copied upward)
  .bank $19 slot 3
  .org $1DCD
  .section "linebreak standard box offset correction 1" overwrite
    jsr checkForRightBoxOffset
  .ends

  .bank $19 slot 3
  .section "linebreak standard box offset correction 2" free
    checkForRightBoxOffset:
      lda $05A7 ; get message box type
      cmp #$00  ; branch if not 00 (right dialogue box)
      bne +
        ; use address 627F instead of 627E as base
        lda #$7F
        bne @done
      +:
      ; default
      lda #$7E
      
      @done:
      ; make up work
      clc
      rts
  .ends*/

  ;=====
  ; extend linebreak to cover previously empty right margin of box, so we
  ; can fit text there if we have to (we need this so we can have 16-char
  ; item names in conjunction with punctuation)
  ;=====
  
  ; for WRAM transfer
  .bank $19 slot 3
  .org $1DDB
  .section "widen linebreak copy area 1" overwrite
    lda #$12+1  ; orig #$12
  .ends
  
  ; for PPU transfer
  .bank $14 slot 3
  .org $179B
  .section "widen linebreak copy area 2" overwrite
    .db $12+1  ; orig #$12
  .ends
  
  ; for WRAM box clear
  .bank $3F slot 5
  .org $19E8
  .section "widen linebreak copy area 3" overwrite
    .db $12+1  ; orig #$12
  .ends

;===============================================
; correctly initialize right offset when
; printing "enemy drew near" text so linebreaks
; will function correctly
;===============================================

;.define enemyNameRightOffset $0E
.define enemyNameRightOffset $0D

/*.bank $17 slot 3
.org $1D7D
.section "enemy approach right offset 1" overwrite
  jsr longjmp
  .db :initTextRightOffsetEnemyName
  .dw initTextRightOffsetEnemyName
.ends*/

.bank $17 slot 3
.org $1D89
.section "enemy approach right offset 1" overwrite
  doLongjmp initTextRightOffsetEnemyName
.ends

.slot 3
.section "enemy approach right offset 2" superfree
  initTextRightOffsetEnemyName:
    ; make up work
    jsr $EB01
  
    lda #enemyNameRightOffset
    sta textRightOffset

    ; clear text box (otherwise previous enemy's name will remain in box)
;    ldx $05A7   ; get box type
;    ldy $F61D,X ; get corresponding WRAM package
    ldy #$02    ; 05A7 is apparently not initialized here (as with
                ; textRightOffset), so we just use the value we know to
                ; be correct (WRAM package #$02)
    jsr clearPackagedWram       ; clear WRAM
    jsr $EA40   ; send to PPU
    
    ; save regionnum of enemy name
    lda $00E3
    pha
      
      ; if multiple enemies of type exist, use plural enemy name
;      lda $7064 ; enemy count (saved during init) (init not yet done)
      ldx $7062         ; enemy slot num
      lda $7006.w,X     ; enemy count array
      cmp #$02
      bcc +
        lda #pluralEnemyNameRegion
        sta $00E3
      +:
    
      ; make up work: run script
      jsr $F5FC
    
    ; restore regionnum of enemy name
    pla
    sta $00E3
    
    rts
  
;    ; make up work
;    lda #$0D
;    jsr $EB31
;  
;    lda #enemyNameRightOffset
;    sta textRightOffset
;    
;    ; make up work
;    ldx $7061
;    
;    rts 
.ends

.bank $3F slot 5
.org $1C91
.section "red wolf right offset 1" overwrite
  doLongjmp doRedWolfRightOffsetReset
.ends

; WE NEED THESE TWO BYTES (REALLY)
.unbackground $7FC97 $7FC98

.slot 3
.section "red wolf right offset 2" superfree
  doRedWolfRightOffsetReset:
    ; set right offset for left dialogue box
    lda #$02
    sta textRightOffset
    
    ; reset msgpos
    lda #$42
    jsr setMsgPos
    
    ; queue script a-55
    lda #$55
    jsr $EB31
    
    ; run script
    jsr longjmp
    .db $17
    .dw $A734
    
    ; make up work
    endNoRetLongjmp $FAFC
.ends

;===============================================
; avoid displaying multiple messages in one
; "box" of battle text
;===============================================

  ;=====
  ; most stuff
  ;=====

  .bank $17 slot 3
  .org $0734
  .section "battle box reset 1" overwrite
    doLongjmp battleBoxReset1
  .ends

  .bank $17 slot 3
  .org $0609
  .section "battle box reset 3" overwrite
    doLongjmp battleBoxReset2
  .ends

  ;=====
  ; battle status report
  ;=====

  .bank $17 slot 3
  .org $010D
  .section "battle box reset 4" overwrite
    doLongjmp battleBoxReset3
;    jmp $A116
    nop
    nop
    nop
  .ends

  ;=====
  ; hacks
  ;=====

  .slot 3
  .section "battle box reset 2" superfree
    battleBoxResetWithLineReset:
      ; target linenum = 0
      lda #$00
      sta lineNum
    battleBoxReset:
      ; base msgpos = #$42
      lda #$42
      ; write to RAM as 16-bit
      jsr setMsgPos
      ; clear box
      jmp clearRemainingBattleText

    battleBoxReset1:
      jsr battleBoxReset
      
      ; make up work
      jsr $D01D
      jmp $F5FC

    battleBoxReset2:
      jsr battleBoxReset
      
      ; replace longjmp retaddr with our own target
      endNoRetLongjmp $EA54
      
      ; pull local retaddr
/*      pla
;        tax
      pla
;        tay
      
      ; pull banknum
      pla
      tay
        
          ; pull longjmp retaddr
          pla
          pla
          
          ; push new target (make up work)
          lda #>($EA54-1)
          pha
          lda #<($EA54-1)
          pha
        
        ; restore local retaddr
;        tya
;        pha
;        txa
;        pha
      
      ; push banknum
      tya
      pha
      
      jmp longjmpWramLoadAddr+(longjmpEndCall-longjmpWramStart)
      
;      rts */

    battleBoxReset3:
;      jsr battleBoxResetWithLineReset
      ; clear WRAM
      ldy #$02
      jsr clearPackagedWram
      jsr pendingWramToPpu
      
      ; target linenum = 0
      lda #$00
      sta lineNum
      ; base msgpos = #$42
;      lda #$42
;      jsr setMsgPos
      
      ; make up work
      lda #$64
      jsr $EB31
      jsr longjmp
      .db $17
      .dw $A12D
      lda #$07
      sta $00D3
     
      rts

;      ; make up work
;      -:
;        lsr $00D5
;        bcc +
;          ; "[name] is..."
;          lda $00DE
;;          jsr $A12A
;          jsr longjmp
;          .db $17
;          .dw $A12A
;        +:
;        
;        inc $00DE
;        dec $00D3
;        bpl -
;      
;      rts

    postBattleBoxReset1:
;      jsr battleBoxResetWithLineReset
      
      ; prep for EXP message print
      ; target linenum = 1
;      lda #$01
;      sta lineNum

;      lda #$72
;      jsr $D350

      ; target linenum = 0
      lda #$00
      sta lineNum

      ; clear box
      ldx $05A7
      ldy $F61D,X
      jsr clearPackagedWram
      ; send to PPU
      jsr $EA40
      ; ?
;      lda #$01
;      sta $059F
      
      ; make up work
      ; queue region 2 script (EXP gained)
      lda #$61
      jmp $EB0F
      
    setLineMsgPosLeftBox:
      ldx lineNum
      lda leftBoxLineNumToMsgPos.w,X
      jmp setMsgPos
      
    leftBoxLineNumToMsgPos:
      ; original
      .db $0D-$0B,$4D-$0B,$8D-$0B,$CD-$0B,$CD-$0B,$CD-$0B,$CD-$0B,$CD-$0B
      ; extended
      .db $CD-$0B,$CD-$0B,$CD-$0B,$CD-$0B,$CD-$0B,$CD-$0B,$CD-$0B,$CD-$0B
      .db $CD-$0B,$CD-$0B,$CD-$0B,$CD-$0B,$CD-$0B,$CD-$0B,$CD-$0B,$CD-$0B
      
    setLineMsgPosLeftBox_statgain:
      jsr setLineMsgPosLeftBox
      
      jsr $EB01
      stx $059D
      inc lineNum
      
      rts
    
    resetMsgPos_new:
      lda #$42
      jmp setMsgPos
    
;    postLevelUpMsg:
;      ; make up work
;      jsr resetMsgPos_new
;      jsr $F5EB
;      
;      inc lineNum
;      rts
    
;    postLevelUpMsg:
;      lda #$01
;      sta lineNum
;      lda #$00
;      sta $00DE
;      rts
      
  .ends

  ;=====
  ; "enemy drew near" messages
  ;=====

  .bank $17 slot 3
  .org $1D80
  .section "enemy approach box reset 1" SIZE 6 overwrite
    ; #D == msgpos corresponding to first line of right-side message box
    lda #$0D
    jmp $BD86
  .ends

  ;=====
  ; post-battle EXP/G
  ;=====

  .bank $17 slot 3
  .org $01D6
  .section "post-battle message box reset 1" SIZE 9 overwrite
  
  
    ; use this if EXP message is shortened to 1 line
    
    ; message linenum
    lda #$01
    sta lineNum
    ; "got X EXP"
    lda #$61
    jsr $A0EF   ; printStatMessage

    ; otherwise, this version clears the box
    ; (and assumes the linebreak at the start of the original EXP message
    ; has been removed)

;    doLongjmp postBattleBoxReset1
;    ; print stat message (skipping the scriptnum queue call, which has been
;    ; folded into the hack routine)
;    jsr $A0F2
  .ends

  ;=====
  ; level up
  ;=====

  .bank $17 slot 3
  .org $00F2
  .section "level up message box reset 1" SIZE 11 overwrite
    doLongjmp setLineMsgPosLeftBox_statgain
;    jmp $A0FD
    jmp $F5F7
  .ends

  .bank $17 slot 3
  .org $00BC
  .section "level up message box reset 2" SIZE 6 overwrite
;    doLongjmp postLevelUpMsg
    jsr postLevelUpMsg
    nop
    nop
    nop
  .ends
    
  .section "level up message box reset 3" free
    postLevelUpMsg:
      lda #$01
      sta lineNum
      lda #$00
      sta $00DE
      rts
  .ends

;  .bank $17 slot 3
;  .org $0100
;  .section "level up message box reset 2" overwrite
;    nop
;    nop
;    nop
;  .ends

;  ; set up proper line num after "X leveled up" message, which is now
;  ; 2 lines
;  .bank $17 slot 3
;  .org $0221
;  .section "level up message box reset 2" overwrite
;;    doLongjmp postLevelUpMsg
;    jsr postLevelUpMsg
;  .ends
;  
;  .bank $3F slot 5
;  .section "level up message box reset 3" free
;    postLevelUpMsg:
;      ; set proper initial linenum for stat gains
;      inc lineNum
;      ; print message
;      jmp $A0B7
;  .ends

;===============================================
; use new rental tank names
;===============================================
  
  .define rentalTankModelByteOffset 5

  ; initial names in new save file
  .bank $14 slot 2
  .org $00A7
  .section "rental tank names 1" overwrite
    .incbin "out/script/rental_tanks.bin"
  .ends

  ; digit of name to update when tank rented
  .bank $18 slot 3
  .org $0372
  .section "rental tank names 2" overwrite
    sta $640F+rentalTankModelByteOffset.w,X
  .ends

;===============================================
; move tank info screen descriptions up a half-
; line
;===============================================

.define tankInfoScreenBasePutpos $6264  ; orig $6284

.bank $3F slot 5
.org $1766
.section "tank info screen descriptions 1" overwrite
  lda #<tankInfoScreenBasePutpos
  sta $00B6
  lda #>tankInfoScreenBasePutpos
.ends

.bank $3F slot 5
.org $16BD
.section "tank info screen descriptions 2" overwrite
  ; PPU dst package for diacritical line of tank info menu descriptions.
  ; original game sets width to 0x10 bytes even though descriptions may be
  ; up to 0x17 bytes long because this covers the worst case of actual
  ; diacritical usage in the original text.
  .db $04,$13 ; x/y (orig: 04, 13)
  .db $17,$01 ; w/h (orig: 10, 01)
.ends

.bank $10 slot 3
.org $065A
.section "tank info screen descriptions 3" overwrite
  ; initial msgpos used for first line printed when tank info menu is opened
  lda #$64
.ends

;===============================================
; on tank overview screen, original game
; redraws only part condition/weight after
; closing the weapon subwindow. the rearranged
; layout in the translation requires that we
; also redraw the part type/name list.
;===============================================

.bank $19 slot 3
.org $1478
.section "tank status screen subwindow redraw 1" overwrite
  doLongjmp fullWeaponSubwindowCloseRedraw
.ends

.slot 3
.section "script access trampoline" superfree
  ; this call changes the bank
  trampolineFC56:
    jmp $FC56
.ends

.slot 3
.section "tank status screen subwindow redraw 2" superfree
  fullWeaponSubwindowCloseRedraw:
    ; make up work
    jsr $FA61
;    jsr $FC56
    doLongjmp trampolineFC56
    
    
/*    ; set msgpos
    lda #$60
    jsr setMsgPos
    ; ?
    jsr longjmp
    .db $19
    .dw $AD90 */
    ; ?
    jsr longjmp
    .db $19
    .dw $B600
    ; ?
    lda $00DE
    sta $00D4
    
    ; set up part type/name array for script access
    jsr longjmp
    .db $19
    .dw $B643
    
    ;  run script 12-11 (part types/names)
    lda #$11
    ; queue region 12
    jsr $EB38
    ; run script
    jmp $F53A
.ends

/*.bank $3F slot 5
.org $1A0D
.section "tank status screen subwindow initial draw 1" overwrite
  .db $10+1     ; height of WRAM clear
.ends

.bank $14 slot 3
.org $17B0+4
.section "tank status screen subwindow initial draw 2" overwrite
  .db $03+1     ; outer PPU transfer loop count
.ends

.bank $14 slot 3
.org $17A9+4
.section "tank status screen subwindow initial draw 3" overwrite
  .db $05-1     ; inner PPU transfer loop count
.ends */

;===============================================
; extend width of switchable right column on
; the various tank part summary screens
;===============================================

.bank $10 slot 3
.org $0DA5
.section "adjust msgpos of part type header" overwrite
  lda #$6D      ; orig #$6E
.ends

.bank $10 slot 3
.org $0DAA
.section "adjust msgpos of part name" overwrite
  lda #$AD      ; orig #$AC
.ends

.bank $3F slot 5
.org $1A16
.section "wram clear pack width #30 / tank status right col" overwrite
  .db $07       ; w (orig: $06)
.ends

.bank $3F slot 5
.org $16C9
.section "ppu transfer pack #0E / tank status right col" overwrite
  ; pack #0E (col header)
  .db $17,$02   ; x/y (orig $17/$02)
  .db $07,$02   ; w/h (orig $06/$02)
  
  ; pack #10 (first 4 parts in list)
  .db $17,$05   ; x/y (orig $17/$05)
  .db $07,$01   ; w/h (orig $05/$02)
  
  ; pack #12 (last 4 parts in list)
  .db $17,$0D   ; x/y (orig $17/$0D)
  .db $07,$01   ; w/h (orig $05/$02)
.ends

;.bank $3F slot 5
;.org $16D1
;.section "x" overwrite
;  .db $07       ; w (orig: $06)
;.ends

;.bank $3F slot 5
;.org $16CD
;.section "ppu transfer pack #10 / tank status right col" overwrite
;  .db $17,$05   ; x/y (orig $17/$05)
;  .db $06,$01   ; w/h (orig $05/$01)
;.ends

;===============================================
; clear message box and reset position when
; displaying "hero got item" message (original
; is hardcoded to display on fourth line of
; box, since the preceding content is
; expected to be only three lines)
;===============================================

; set linenum to 00
.bank $3F slot 5
.org $10AD
.section "hero got item fix 1" overwrite
  lda #$00      ; orig #$02
.ends

; set msgpos to target first line
.bank $3F slot 5
.org $10B3
.section "hero got item fix 2" overwrite
  lda #$40      ; orig #$C0
.ends

; clear box before printing
.bank $3F slot 5
.org $10BB
.section "hero got item fix 3" overwrite
  jsr clearBoxAndRunPendingScript       ; orig runPendingScript
.ends

;===============================================
; clear message box and reset position for
; frog race messages
;===============================================

/*; routine to clear text box (copied from original routine, which
; requires a script to execute afterwards)
;.bank $3F slot 5
;.section "clear current text box" free
.slot 3
.section "new text box clears" superfree
  clearCurrentTextBox:
    ; clear box WRAM
    ldx $05A7
    ldy $F61D,X
    jsr clearPackagedWram
    ; result to PPU
    jmp pendingWramToPpu
  
  clearCurrentTextBoxNoPpu:
    ; clear box WRAM
    ldx $05A7
    ldy $F61D,X
    jmp clearPackagedWram
  
  clearFrogBetBox:
    ; make up work
    lda #$8E
    jsr setMsgPos
    sta $00DE
    
    jmp clearCurrentTextBoxNoPpu
    
    
.ends

.bank $18 slot 3
.org $048E
.section "clear frog box 1" overwrite
  doLongjmp clearFrogBetBox
  nop
.ends */

.bank $18 slot 3
.org $048E
.section "prep frog box 1" overwrite
  lda #$8E+$40  ; initial msgpos
.ends

.bank $18 slot 3
.org $04A0
.section "prep frog box 2" overwrite
  doLongjmp prepFrogBetBox
  nop
.ends

.slot 3
.section "prep frog box 3" superfree
  prepFrogBetBox:
    ; make up work
    inc $00DE
    
    ; queue script 2-8D
    lda #$8D
    jsr $EB0F
    ; run script
    jsr $F5FC
    
    ; increase msgpos
    clc
    lda #$20
    adc $00C4
    sta $00C4
    lda #$00
    adc $00C5
    sta $00C5
    rts
    
.ends

;===============================================
; adjust position of names in small character
; selection box (for e.g. selecting character
; or tank to equip a purchased item)
;===============================================

.define smallCharselWinColNewOffset (-2)

;.bank $19 slot 3
;.org $0304
;.section "x" overwrite
;  lda #$4C
;.ends

.bank $3F slot 5
.org $0D70
.section "small charsel window: adjust base msgpos for names" overwrite
  lda #$65+smallCharselWinColNewOffset      ; orig #$65
.ends

.bank $10 slot 5
.org $0249
.section "small charsel window: adjust base ypos for E" overwrite
  ; y-position of "E" indicating equipment equipability
  lda #$1F+(smallCharselWinColNewOffset*8)  ; orig #$1F
.ends

;.bank $3F slot 5
;.org $07F3
;.section "small charsel window: adjust base xpos for hand cursor 1" overwrite
;.ends

;.bank $3F slot 5
;.org $07F2
;.section "small charsel window: adjust base xpos for hand cursor 2" overwrite
;  ; most other selection menus (NOTE: shared with some topwin positions!)
;  .db $02      ; orig #$0F
;  ; tank equipment purchase
;  .db $02      ; orig #$0F; minimum without losing sprites off left edge
;               ; of screen is #$01
;.ends

; x-table
.bank $3F slot 5
.section "small charsel win cursor x 2" free
  smallCharselWinXPosTable:
    ; cursor X must be 1 or greater (original value is $0F)
    .if smallCharselWinColNewOffset <= -2
      ; NOTE: shared with some topwin positions!
      .db $02     ; "column 1" x-pos?? affects e.g. tank rental topwin (orig $0F)
                  ; ...but also selection cursor for human-selection menus
      .db $02     ; "column 2" x-pos?? affects small charsel window (orig $0F)
    .else
      .db $0F     ; column 1 x-pos (orig $0F)
      .db $0F+(heroTankLeftColNewOffset*8)     ; column 1 x-pos (orig $0F)
    .endif
.ends

  ;====
  ; -- MOVED, SEE REFERENCES TO smallCharselWinXPosTable BELOW --
  ;====

/*; x-table pointer
.bank $3F slot 5
.org $0896
.section "small charsel win cursor x 1" overwrite
  .dw smallCharselWinXPosTable   ; orig E7F2
.ends */

;===============================================
; adjust hardcoded HP/SP label positions during
; battle to match updated scripts (see scripts
; 2-1 through 2-3)
;===============================================

.define newCharHpSpOffset 1

.bank $3F slot 5
.org $1924
.section "new hp/sp label pos 1" overwrite
  ; WRAM positions of character HP/SP labels
  .dw $6298+newCharHpSpOffset
  .dw $62F8+newCharHpSpOffset
  .dw $6358+newCharHpSpOffset
  .dw $62B8+newCharHpSpOffset
  .dw $6318+newCharHpSpOffset
  .dw $6378+newCharHpSpOffset
.ends

;===============================================
; fix a bug in the original game where pressing
; A on the Trunk Room summary screen causes
; it to switch to the damage summary "display 
; mode", corrupting the list if it's scrolled
;===============================================

/*.bank $10 slot 3
.org $0850
.section "fix trunk room A button bug 1" overwrite
  lda #$A0      ; orig #$9E
.ends */

.bank $10 slot 3
.org $085A
.section "fix trunk room A button bug 1" overwrite
  ; what happens is that 00CF gets set to #9E when the trunk room menu
  ; is opened (20850).
  ;
  ; this seems to be the "submenu type" or something like that
  ; -- it determines what value 00CE gets when A or B is pressed.
  ; while a value of #9E is needed to initialize the cursor correctly,
  ; #A0 seems to be the actual intended value.
;  doLongjmp trunkRoomInitFix
  jsr trunkRoomInitFix
  
.ends

.bank $10 slot 3
.section "fix trunk room A button bug 2" free
  trunkRoomInitFix:
    ; make up work
;    jsr longjmp
;    .db $10
;    .dw $AA8D
;    jsr longjmp
;    .db $10
;    .dw $A63B
    jsr $A63B
    
    ; update menu type
    lda #$A0
    sta $00CF
    
    ; use new y-pos table
;    lda #<newTrunkRoomCursorYTable
;    sta $00BE
;    lda #>newTrunkRoomCursorYTable
;    sta $00BF
    
    ; make up work
;    lda #$1E
;    sta $00CE
    
;    endNoRetLongjmp $A7EA
    rts
    
.ends

;===============================================
; adjust hero.....tank listings with expanded
; names
;===============================================

; offsets, in tiles, of new column x-positions from original ones
.define heroTankLeftColNewOffset 0
.define heroTankRightColNewOffset 1

.bank $19 slot 5
.org $0304
.section "hero...tank msgpos 1" overwrite
  ; hero list msgpos
  lda #$4E+heroTankLeftColNewOffset      ; orig #$4E
.ends

.bank $19 slot 5
.org $0329
.section "hero...tank msgpos 2" overwrite
  ; tank list msgpos
  lda #$56+heroTankRightColNewOffset      ; orig #$56
.ends

.bank $19 slot 5
.org $0355
.section "hero...tank msgpos 3" overwrite
  ; tank list (left col) msgpos
;  lda #$4E+heroTankLeftColNewOffset      ; orig #$4E
  
  ; only used for one-col menus(?) and has separate cursor table, so
  ; just use original value
  lda #$4E
.ends

;.bank $3F slot 5
;.org $0812
;.section "hero...tank cursor xpos 2" overwrite
;  ; hero column cursor xpos
;  .db $58      ; orig #$60
;  ; tank column cursor xpos
;  .db $A8      ; orig #$A0
;.ends

; due to some zealous optimization in the menu cursor position tables
; (some x-positions double as y-positions!), we have to create new entries
; for rearranged menus

; y-table pointer
;.bank $3F slot 5
;.org $086A
;.section "hero...tank msgpos 3" overwrite
;  
;.ends

; x-table pointer
;.bank $3F slot 5
;.org $088C
;.section "hero...tank menus cursor x 1" overwrite
;;  .dw heroToTankMenuXPosTable   ; orig E812
;.ends

; x-table
.bank $3F slot 5
.section "hero...tank menus cursor x 2" free
  heroToTankMenuXPosTable:
    .db $60+(heroTankLeftColNewOffset*8)     ; column 1 x-pos (orig $60)
    .db $A0+(heroTankRightColNewOffset*8)    ; column 2 x-pos (orig $A0)
.ends

;===============================================
; adjust tank damage summary/repair screen
;===============================================

.bank $14 slot 3
.org $17A1
.section "width of damage screen list PPU transfer" overwrite
  .db $13+$7    ; orig $13
.ends

;===============================================
; move tanks on summary screen
;===============================================

.bank $10 slot 3
.org $0676
.section "msgpos of tanks on summary screen" overwrite
  lda #$63-$00
.ends

;===============================================
; move cursor table lookup out to a new routine
; so we can add additional tables as needed
;===============================================

.bank $3F slot 5
.org $09A6
.section "use new cursor lists 1" overwrite
  doLongjmp useNewCursorLists
  rts
;  jmp $E9E1
.ends

; unbackground space freed
.unbackground $7E9AD $7E9E0

/*.bank $3F slot 5
.org $09AA
.section "use new cursor lists 1" overwrite
  lda $E8AA,Y
;  txa
  pha
    doLongjmp useNewCursorLists
  pla
  tax
  jmp $E9CB
.ends */

;.bank $3F slot 5
;.org $09C1
;.section "use new cursor lists 2" overwrite
;  txa
;  pha
;    doLongjmp useNewCursorXLists
;  pla
;  tax
;.ends

.slot 3
.section "use new cursor lists 3" superfree
  ; table of pointers to Y-position tables used for menu cursors
  ; original table: 3E868
  newCursorYList:
    .dw $E814
    .dw $E828
    .dw $E824
    .dw $E81C
    .dw $E817
    .dw $E813
    .dw $E82C
    .dw $E818
    .dw $E820
    .dw $E825
    .dw $E82E
    .dw $E831
;    .dw $E834
    .dw computerScreenCursorYTable
    .dw $E836
    .dw $E838
    ; index 0F
  
  ; orig: 3E886
  newCursorXList:
    .dw $E80D
    .dw $E7E8
    .dw $E811
;    .dw $E812
    .dw heroToTankMenuXPosTable   ; orig E812
    .dw $E7EA
    .dw $E7EC
    .dw $E7EE
    .dw $E805
    .dw $E7F2
;    .dw smallCharselWinXPosTable   ; orig E7F2
    .dw $E7E6
    .dw $E7E3
    .dw $E80A
    .dw $E7F4
    .dw $E7F8
    .dw $E806
    .dw $E7FA
    .dw $E7FF
;    .dw computerScreenCursorXTable
    .dw $E801
  
  newCursorXYList:
  ; FUCKIING
    ; y-index $21, x-index $12
    .dw newTrunkRoomCursorYTable
    ; y-index $22, x-index $13
    .dw oneColumnConversionYTable9E
  
;  menuTypeToSubtypeTable:

;  specialMenuYTypes:
;    .db $00
;  specialMenuXTypes:
;    .db $00
  
  newSpecialCursorYList:
    .dw oneColumnConversionYTable9E     ; 80 == item/equipment menus
    .dw newTrunkRoomCursorYTable+5      ; 81 == tank summary screen
    .dw oneColumnConversionYTable9E+4   ; 82 == field item menus
    .dw oneColumnConversionYTable4E     ; 83 == field equip menus
    .dw oneColumnConversionYTable15     ; 84 == teleport menus
    .dw oneColumnConversionYTable15     ; 85 == shop sell menus
    .dw oneColumnConversionYTable4E+1   ; 86 == special shells (status/sell)
    .dw oneColumnConversionYTable9E     ; 87 == special shells (battle)
    .dw $E820                           ; 88 == charsel boxes (human/tank)
    .dw $E820                           ; 89 == charsel boxes (rental tank)
    .dw $E82E                           ; 8A == vending machines
    .dw $E828                           ; 8B == "other" menu in battle
  newSpecialCursorXList:
    .dw $E7E8
    .dw $E7EC
    .dw $E811
    .dw $E7EA
    .dw teleportMenuXTable
    .dw $E7E8
    .dw $E7FA   ; $20
    .dw $E7E8
    .dw smallCharselWinXPosTable
    .dw smallCharselWinXPosTable
;    .dw $E7E3
    .dw vendingMachineXTable
    .dw battleOtherMenuXTable
  ; this is an array with one entry per item indicating which directions
  ; the cursor can be scrolled on that item in the list (same bitfield
  ; format as controller input at 0018/0019)
  newSpecialCursorLayoutList:
;    .dw $E751
    .dw $E76A
    .dw $E76A
    .dw $E761
;    .dw $E751
    .dw $E76A
    .dw teleportMenuLayoutTable
    .dw $E76A
    .dw $E76A
    .dw $E76A
    .dw $E749
    .dw $E761
    .dw $E79E
    .dw $E751
  specialMenuNumCols:
    .db $01
    .db $01
    .db $01
    .db $01
    .db $02
    .db $01
    .db $01
    .db $01
    .db $02     ; ??? why do human/tank charsels windows have two columns???
    .db $01     ; ??? yet rental charsel windows have one??
    .db $03
    .db $02

  checkSpecialMenus:
    ; Y = menu type
    ; X = menu subtype
    
    txa
    and #$7F
    tax
    
    ;=====
    ; read from Y-pointer table
    ;=====
    
    ; get Y-table type
;    lda specialMenuYTypes.w,X
;    txa
    
    ; get offset into pointer table
    asl
    tay
    
    ; copy to $00BE
    lda newSpecialCursorYList+0,Y
    sta $00BE
    lda newSpecialCursorYList+1,Y
    sta $00BF
    
    ;=====
    ; read from X-pointer table
    ;=====
    
    ; get X-table type
;;    lda specialMenuXTypes.w,X
;    txa
    
    ; get offset into pointer table
;    asl
;    tay
    
    ; copy to $00BC
    lda newSpecialCursorXList+0,Y
    sta $00BC
    lda newSpecialCursorXList+1,Y
    sta $00BD
    
    ;=====
    ; make up work
    ;=====
    
    ; get number of columns in menu
    lda specialMenuNumCols.w,X
    sta $05A2
    
    ; get ? table type?
;    lda $E8FE.w,X
;    tax
;    asl
;    tay
    ; ?
    lda newSpecialCursorLayoutList+0,Y
    sta $00C0
    lda newSpecialCursorLayoutList+1,Y
    sta $00C1
    
    rts
  
  useNewCursorLists:
    ;=====
    ; regs have been trashed, so make up work required to do lookup
    ;=====
    
    ; get menu type??
    lda $00CF
    lsr
    tay
    
    ;=====
    ; 
    ;=====
    
    ; get ?? menu subtype??? used to derive further information
    ; damage = 4F -> 1A
    ; trunk room = 50 -> 1C
    ; field tool usage = 07 -> 13
    ldx $E8AA,Y
    
    ;=====
    ; check for (new) hardcoded exceptions
    ;=====
    
    bpl +
      jmp checkSpecialMenus
    +:
    
    @notSpecial:
    
    ;=====
    ; read from Y-pointer table
    ;=====
    
    ; get Y-table type
    lda $E935.w,X
    
    ; get offset into pointer table
    asl
    tay
    
    ; copy to $00BE
    lda newCursorYList+0,Y
    sta $00BE
    lda newCursorYList+1,Y
    sta $00BF
    
    ;=====
    ; read from X-pointer table
    ;=====
    
    ; get X-table type
    lda $E96C.w,X
    
    ; get offset into pointer table
    asl
    tay
    
    ; copy to $00BC
    lda newCursorXList+0,Y
    sta $00BC
    lda newCursorXList+1,Y
    sta $00BD
    
    ;=====
    ; make up work
    ;=====
    
    ; get ? table type?
    lda $E8FE.w,X
    tax
    asl
    tay
    ; ?
    lda $E840,Y
    sta $00C0
    lda $E841,Y
    sta $00C1
    
    ; get number of columns in menu
    lda $E7CF.w,X
    sta $05A2
    
    rts
  
;  smallCharselWinXPosTable:
;    ; NOTE: shared with some topwin positions!
;    .db $02     ; "column 1" x-pos?? affects e.g. tank rental topwin (orig $0F)
;                ; ...but also selection cursor for human-selection menus
;    .db $02     ; "column 2" x-pos?? affects small charsel window (orig $0F)
.ends

/*.bank $10 slot 3
.org $0A90
.section "msgpos of trunk room list" overwrite
  lda #$64-$20  ; orig #$64
.ends

.define trunkRoomCursorYOffset (-8)

.bank $3F slot 5
.section "cursor y-positions of trunk room list 1" free
  newTrunkRoomCursorYTable:
    .rept 8 index count
      .db (count*$10)+$15+trunkRoomCursorYOffset
    .endr
.ends */



;.bank $10 slot 3
;.org $0870
;.section "cursor y-positions of trunk room list 2" overwrite
;  .dw newTrunkRoomCursorYTable
;.ends

/*.bank $10 slot 3
.org $0B70
.section "msgpos of damage list" overwrite
  lda #$62-$20  ; orig #$62
.ends

.bank $10 slot 3
.org $0A90
.section "msgpos of trunk room list" overwrite
  lda #$64-$20  ; orig #$64
.ends

.define trunkRoomCursorYOffset (-8)

;.bank $3F slot 3
;.org $0817
;.section "cursor y-positions of trunk room list" overwrite
;  .rept 
;.ends
.bank $3F slot 5
.section "cursor y-positions of trunk room list 1" free
  newTrunkRoomCursorYTable:
    .rept 8 index count
      .db (count*$10)+$15+trunkRoomCursorYOffset
    .endr
.ends

.bank $3F slot 5
.org $0870
.section "cursor y-positions of trunk room list 2" overwrite
  .dw newTrunkRoomCursorYTable
.ends

.bank $10 slot 3
.org $081D
.section "cursor y-positions of trunk room list 3" overwrite
  lda #<newTrunkRoomCursorYTable
  sta $00BE
  lda #>newTrunkRoomCursorYTable
  sta $00BF
.ends */

;===============================================
; one-column converions
;===============================================

.bank $3F slot 5
.section "one-column conversion tables 1" free
;  computerScreenCursorXTable:
;    .db $18,$80

  oneColumnConversionYTable15:
    .rept 8 index count
      .db (count*$8)+$15
    .endr
    
  oneColumnConversionYTable4E:
    .rept 8 index count
      .db (count*$8)+$4E
    .endr
    
  oneColumnConversionYTable9E:
    .rept 8 index count
      .db (count*$8)+$9E
    .endr
    
  vendingMachineXTable:
    .db $03,$53,$A3
.ends

.bank $3F slot 5
.section "one-column conversion tables 2" free
  teleportMenuXTable:
    .db $09,$69
.ends

.bank $3F slot 5
.section "one-column conversion tables 3" free
  teleportMenuLayoutTable:
    .db $05,$06,$0D,$0E,$0D,$0E,$0D,$0E,$0D,$0E,$09,$0A
.ends

.bank $3F slot 5
.section "one-column conversion tables 4" free
  battleOtherMenuXTable:
    .db $60-8,$98
.ends

  ;=====
  ; item and equipment menus
  ;=====

  ; battle tools
  .bank $3F slot 5
  .org $08AA+$04
  .section "new special table types 1" overwrite
    .db $80     ; orig $05
  .ends

  ; battle equipment
  .bank $3F slot 5
  .org $08AA+$05
  .section "new special table types 2" overwrite
    .db $80     ; orig $05
  .ends

  ; field tools
  .bank $3F slot 5
  .org $08AA+$0A
  .section "new special table types 3" overwrite
    .db $80     ; orig $05
  .ends

  ;=====
  ; field equipment menu WRAM clear
  ;=====

  ; height of clear
  .bank $3F slot 5
  .org $19F7
  .section "field equip menu wram clear 1" overwrite
    .db $09     ; orig $08
  .ends

  ;=====
  ; teleport menu
  ;=====

  ; dog system
  .bank $3F slot 5
  .org $08AA+($54/2)
  .section "teleport menu subtype 1" overwrite
    .db $84     ; orig ?
  .ends

  ; town teleporters
  .bank $3F slot 5
  .org $08AA+($70/2)
  .section "teleport menu subtype 2" overwrite
    .db $84     ; orig ?
  .ends

  ;=====
  ; sell menus
  ;=====

  .bank $3F slot 5
  .org $08AA+($7E/2)
  .section "sell menu subtype 1" overwrite
    .db $85     ; orig ?
  .ends

  ;=====
  ; special shell menus
  ;=====

  ; sell
  .bank $3F slot 5
  .org $08AA+($6C/2)
  .section "shells menu subtype 1" overwrite
    .db $86     ; orig ?
  .ends

  ; battle
  .bank $3F slot 5
  .org $08AA+($04/2)
  .section "shells menu subtype 2" overwrite
    .db $87     ; orig ?
  .ends

  ;=====
  ; small charsel boxes
  ;=====

  ; human/tank select
  .bank $3F slot 5
  .org $08AA+($88/2)
  .section "charsel menu subtype 1" overwrite
    .db $88     ; orig ?
  .ends

  ; rental tank select
  .bank $3F slot 5
  .org $08AA+($86/2)
  .section "charsel menu subtype 2" overwrite
    .db $89     ; orig ?
  .ends

  ;=====
  ; vending machines
  ;=====
  
  .bank $3F slot 5
  .org $08AA+($8E/2)
  .section "vending machine menu subtype 1" overwrite
    .db $8A     ; orig ?
  .ends

  ;=====
  ; battle "other" menu
  ;=====
  
  .bank $3F slot 5
  .org $08AA+($06/2)
  .section "battle other menu subtype 1" overwrite
    .db $8B     ; orig ?
  .ends

;===============================================
; fix teleport location menu
;===============================================

; array of msgpos changes after each name is drawn
.bank $19 slot 5
.org $06CD
.section "teleport menu msgpos increment array" overwrite
;  .db $0A,$0A,$2C,$0A,$0A,$2C,$0A,$0A,$2C,$0A,$0A      ; orig
  .db $0C,$14,$0C,$14,$0C,$14,$0C,$14,$0C,$14,$0C
.ends

;===============================================
; fix field equipment menus
;===============================================

.define equipmentIconBaseY $50
.define equipmentIconX $26

.bank $19 slot 3
.org $0C98
.section "equipment icon init" SIZE $19 overwrite
  ; set icon y-positions
  ldx #$07
  lda #equipmentIconBaseY+(7*8)
  -:
    sta $0346.w,X
    sec
    sbc #8
    dex
    bpl -
  
  ; set icon x-positions
  ldx #$07
  lda #equipmentIconX
  -:
    sta $0336.w,X
    dex
    bpl -
  
  rts
.ends

; equip (human)
.bank $3F slot 5
.org $08AA+($34/2)
.section "field equip menu subtypes 1" overwrite
  .db $83     ; orig ?
.ends

; give/drop (human)
.bank $3F slot 5
.org $08AA+($28/2)
.section "field equip menu subtypes 2" overwrite
  .db $83     ; orig ?
.ends

; equip (tank)
.bank $3F slot 5
.org $08AA+($40/2)
.section "field equip menu subtypes 3" overwrite
  .db $83     ; orig ?
.ends

; give (tank)
.bank $3F slot 5
.org $08AA+($50/2)
.section "field equip menu subtypes 4" overwrite
  .db $83     ; orig ?
.ends

; drop (tank)
.bank $3F slot 5
.org $08AA+($42/2)
.section "field equip menu subtypes 5" overwrite
  .db $83     ; orig ?
.ends

;===============================================
; fix item use menu
;===============================================

.bank $3F slot 5
.org $08AA+($16/2)
.section "field item use subtype" overwrite
  .db $82     ; orig ?
.ends

;===============================================
; fix tank overview screen
;===============================================

; move mid-frame CHR switch up a few scanlines so we can shift the menu
; itself up a tile without glitching the cursor
.bank $3E slot 4
.org $15A5
.section "tank status screen irq 1" overwrite
  lda #$34-6      ; irq latch value
.ends

; subtype of tank overview from start menu
.bank $3F slot 5
.org $08AA+($94/2)
.section "tank overview screen subtype 1" overwrite
  .db $81     ; orig ?
.ends

; subtype of tank overview from status menu
.bank $3F slot 5
.org $08AA+($3C/2)
.section "tank overview screen subtype 2" overwrite
  .db $81     ; orig ?
.ends

;===============================================
; fix "computer" screens (e.g. load menu) so
; cursor points to top line of buttons
;===============================================

.bank $3F slot 5
.section "computer menu cursor pos 1" free
;  computerScreenCursorXTable:
;    .db $18,$80
    
  computerScreenCursorYTable:
    .db $A7-$08,$C7-$08
.ends

;===============================================
; modify trunk room so we can use a two-line
; display
;===============================================

.define trunkRoomCursorYOffset (-8)

.bank $3F slot 5
.section "cursor y-positions of trunk room list 1" free
  newTrunkRoomCursorYTable:
    .rept 16 index count
      .db (count*$10)+$15+trunkRoomCursorYOffset
    .endr
.ends

.bank $3F slot 5
.org $0951
.section "cursor y-positions of trunk room list 2" overwrite
  ; Y-table index of menu subtype 1C (used by e.g. A0 (supposed-to-be
  ; trunk room))
;  .db $0F ; orig #$05
  .db $21 ; orig #$05
.ends

.bank $3F slot 5
.org $094F
.section "cursor y-positions of trunk room list 3" overwrite
  ; Y-table index of menu subtype 1A (used by e.g. BE (damage summary))
  .db $21 ; orig #$04
.ends

.bank $10 slot 3
.org $0B70
.section "msgpos of damage list" overwrite
  lda #$62-$20  ; orig #$62
.ends

.bank $10 slot 3
.org $0A90
.section "msgpos of trunk room list" overwrite
  lda #$64-$20  ; orig #$64
.ends

;===============================================
; fix clearing of tank weight data on equipment
; screen
;===============================================

.bank $3F slot 5
.org $1A00
.section "width of equipment screen tank weight WRAM clear" overwrite
  .db $0E+$03  ; orig $0E
.ends

.bank $3F slot 5
.org $19B6
.section "dstaddr of equipment screen tank weight WRAM clear" overwrite
  .dw $604F-$2  ; orig $604F
.ends

;===============================================
; adjust shop name position (original starts
; 2 tiles right of left side of box)
;===============================================

.bank $18 slot 3
.org $02BF
.section "msgpos of shop name script" overwrite
  lda #$83-2    ; orig #$83
.ends

;===============================================
; adjust shop item layout
;===============================================


  ;=====
  ; adjust PPU transfer region of shop list when
  ; scrolled to match new layout (item name/price
  ; on alternating rows)
  ;=====
  
  .bank $14 slot 3
  .org $17A5
  .section "y-pos of shop scroll content PPU transfer" overwrite
    .db $02+1     ; orig #$02
  .ends

  ;=====
  ; x-pos of up/down scroll indicators
  ;=====
  
  .bank $18 slot 3
  .org $07E5
  .section "shop scroll arrows x-pos" overwrite
;    lda #$AC-$50 ; orig #$AC
;    lda #$AC+(9*8)-3      ; orig #$AC
    lda #$AC+(8*8)      ; orig #$AC
  .ends

;===============================================
; adjust position of character status summary
; messages on main status screen
;===============================================

.bank $19 slot 3
.org $1763
.section "msgpos of status screen char summaries" overwrite
  lda #$64-1    ; orig #$64
.ends

.bank $19 slot 3
.org $177F
.section "msgpos of status screen tank summaries" overwrite
  lda #$64-1    ; orig #$64
.ends

;===============================================
; status effect labels
;===============================================

.bank $3F slot 5
.org $1135
.section "status effect labels human" overwrite
  .db $70,$71   ; ice
  .db $72,$73   ; fire
  .db $74,$75   ; acid
  .db $76,$77   ; conf
  .db $78,$79   ; cement
  .db $7A,$7B   ; sleep
  .db $7C,$7D   ; paralyzed
  .db $7E,$7F   ; dead
.ends

.bank $3F slot 5
.org $1147
.section "status effect labels tank" overwrite
  .db $70,$71   ; ice
  .db $72,$73   ; fire
  .db $74,$75   ; acid
  .db $76,$77   ; conf
  .db $78,$79   ; cement
  .db $6E,$6F   ; gas
  .db $6E,$6F   ; gas
  .db $6E,$6F   ; gas
.ends

;===============================================
; move battle "label" box messages up a line
;===============================================

;.bank $3F slot 5
;.org $1569
;.section "battle label box pos" overwrite
;  lda #$82-$20  ; low byte of WRAM putpos (orig #$6282)
;.ends

;.bank $3F slot 5
;.section "battle label box pos 2-line 1" free
;  drawTwoLineBattleLabel:
;    ldy #$04
;    jsr clearPackagedWram
;    lda #$62
;    jmp $F56B
;.ends

  ;=====
  ; human weapon
  ;=====

  .bank $17 slot 3
  .org $0311
  .section "battle label box pos 2-line human" overwrite
    doLongjmp battleBoxWeaponPrep
    
  .ends

  ;.bank $17 slot 3
  ;.section "battle label box pos 2-line 3" free
  ;  doLongjmp battleBoxWeaponPrint
  ;.ends

  ;=====
  ; tank weapon
  ;=====

  .bank $17 slot 3
  .org $03DC
  .section "battle label box pos 2-line tank" overwrite
  ;  bcs @doesntHaveAmmo
  ;    doLongjmp battleBoxTankWeaponPrepWithAmmo
  ;  @doesntHaveAmmo:
  ;    doLongjmp battleBoxTankWeaponPrepNoAmmo
    
    doLongjmp battleBoxTankWeaponPrepPre
    jmp $A401
    
  .ends

  ;=====
  ; tool
  ;=====

  .bank $17 slot 3
  .org $04A5
  .section "battle label box pos 2-line tool" overwrite
    doLongjmp battleBoxToolPrep
    nop
  .ends

  ;=====
  ; handlers
  ;=====

  .slot 3
  .section "battle label box pos 2-line logic" superfree
    
    drawTwoLineBattleLabel:
      ldy #$04
      jsr clearPackagedWram
      lda #$62
      jmp $F56B
    
    shellsBattleTwoLine:
      ; get shell ID
      lda $00D6
      ; add offset to two-line version
      clc
      adc #$66
      sta $00CC
      
      ; set region
      lda #$0D
      sta $00CD
      
      ; make up work
      lda $00D6
      ldx $00D1
      sta $704F.w,X
      
      jmp drawTwoLineBattleLabel
    
    battleBoxWeaponPrep:
      ldy $00D1
      lda $64C6,Y
      sta $0000
      ldx $E71F,Y
      
      ; two-line item region
      lda #twoLineItemRegion
      sta $00CD
      
      ldy #$00
      
      ; look for equipped weapon
      @checkLoop:
        asl $0000
        bcc +
          ; ID >= #$23: weapon
          lda $6496.w,X
          sta $00D6
          cmp #$23
          bcs @done
        +:
        inx 
        iny 
        cpy #$08
        bne @checkLoop
      
      ; no weapon equipped: use "barehanded" string (8-22)
      sty $00CD
      lda #$22
      sta $00D6
      
      @done:
      sta $00CC
      
      ; draw two-line
      jsr drawTwoLineBattleLabel
      
      endNoRetLongjmp $A401
      
    battleBoxTankWeaponPrepPre:
      ldx $00D1
      lda $7052.w,X
      tax
      
      ; get weapon id
      ldy $6627.w,X
      sty $00D6
      
      ; set script num
      sty $00CC
      lda #$00
      sta $00CD
      
      ; get ammo count
      lda $6535,X
      bmi @isBroken     ; branch if component broken (top bit set)
        cpy #$65          ; branch if over 100 ammo (includes infinite ammo)
        bcs @infiniteAmmo
          and #$3F
          tay
          beq @outOfAmmo
      
      @finiteAmmo:
      sta $0524
      ; queue script 2-7 ("ammo: XX")
      lda #$07
      jsr $EB0F
      jmp $F564   ; one-line WRAM transfer
  ;    rts
  ;    endNoRetLongjmp $A3FE
      
      @infiniteAmmo:
      ; target two-line item names
      lda #twoLineItemRegion
      sta $00CD
      jmp drawTwoLineBattleLabel
      
      @isBroken:
      endNoRetLongjmp $A377
      
      @outOfAmmo:
      iny
      tya
      ; queue script A-1 ("no ammo")
      jsr $EB31
      endNoRetLongjmp $A37E
      
    battleBoxToolPrep:
      lda #twoLineItemRegion
      sta $00CD
      jmp drawTwoLineBattleLabel
      
  .ends

;===============================================
; short item forms
;===============================================

  .slot 3
  .section "status screen short items 2" superfree
    prepShortItemList:
      jsr $F365   ; set up item list at 0524+
      
    prepShortItemList_correction:
      ; correct script regions to short item form
      ldx #$00
      -:
        ; if region is zero, change to two-line item region
        lda $0525.w,X
        bne +
          lda #twoLineItemRegion
          sta $0525.w,X
        +:
        inx
        inx
        cpx #$10
        bne -
      rts
      
    prepShortItemList_statusScreen:
      ; make up work
      lda $00D1
      sta $00D0
      
      jmp prepShortItemList
      
    ; wait why am i doing this??
;    prepShortItemList_shells:
;      lda #twoLineItemRegion
;      sta $00E3
;    
;      ; make up work
;      jsr $F558
;      jmp $EB01
    
      
;    prepShortItemList_tankStatus:
;      
;      jsr $EB38   ; queue region 12 script
;      jsr $F558   ; run script
      
  .ends

  ;=====
  ; status screen
  ;=====

  .bank $19 slot 3
  .org $1713
  .section "status screen short items 1" overwrite
    doLongjmp prepShortItemList_statusScreen
    nop
  .ends

  ;=====
  ; tank status screens
  ;=====

  .bank $10 slot 3
  .org $0D79
  .section "tank status screens short items 1" overwrite
    jsr prepShortItemList_tankStatus_pre
  .ends

  .bank $10 slot 3
  .section "tank status screens short items 2" free
    prepShortItemList_tankStatus_pre:
      ; queue region 12 script
      jsr $EB38
      
      ; apply corrections to item region nums
;      doLongjmp prepShortItemList_correction
      lda $00E3
      bne +
        lda #twoLineItemRegion
        sta $00E3
      +:
      
      ; run script
      jmp $F558
  .ends

  ;=====
  ; tank status screen part summary subwindow
  ;=====

  .bank $19 slot 3
  .org $1463
  .section "tank status screens part summary short items 1" overwrite
    doLongjmp tankSummarySubwindowShortItems
  .ends

  .slot 3
  .section "tank status screens part summary short items 2" superfree
    tankSummarySubwindowShortItems:
      ; adjust item region
      lda $00E3
      bne +
        lda #twoLineItemRegion
        sta $00E3
      +:
    
      ; run script
      jsr $F53A
      jmp $D01D
  .ends

  ;=====
  ; vending machines
  ;=====

  .bank $18 slot 3
  .org $0EE3
  .section "vending machine short items 1" overwrite
    doLongjmp vendingMachineShortItems
  .ends

  .slot 3
  .section "vending machine short items 2" superfree
    vendingMachineShortItems:
      ; adjust item region
      lda $00E3
      bne +
        lda #twoLineItemRegion
        sta $00E3
        bne @adjustmentDone
      +:
      ; if region is #$0D (shells, armor tiles), adjust script index
      cmp #$0D
      bne +
        lda $00E2
        clc
        adc #$66
        sta $00E2
      +:
      
      @adjustmentDone:
      
      ; make up work
      lda #$1B
      jsr $EB16
      
      dec $00DC
      bpl @loopNotDone
        endNoRetLongjmp $AEEC
      @loopNotDone:
      endNoRetLongjmp $AED8
  .ends
  
  ; sprite objects
  .bank $18 slot 3
  .org $0EFD
  .section "vending machine short items 3" overwrite
    ; y-positions
    lda #$18-2
    sta $0346.w,X
    lda #$40-2
    sta $0349.w,X
    nop
    nop
  .ends

;===============================================
; special shells stuff
;===============================================

.bank $3F slot 5
.org $19EE
.section "special shells weapon listing WRAM clear 1" overwrite
  .db $06+1     ; width of tank name WRAM clear
.ends

.bank $19 slot 3
.org $0A76
.section "special shells weapon listing msgpos" overwrite
  lda #$6C-$20
.ends

.bank $18 slot 3
.org $0549
.section "normal shells buy listing msgpos" overwrite
  lda #$64-$20
.ends

;.bank $19 slot 3
;.org $0AD1
;.section "special shells weapon listing short item names 1" overwrite
;  doLongjmp prepShortItemList_shells
;.ends

  ;=====
  ; use two-line forms for battle window
  ;=====

  .bank $17 slot 3
  .org $0466
  .section "shells battle two-line 1" overwrite
    doLongjmp shellsBattleTwoLine
    jmp $A474
  .ends

;===============================================
; use two-line names on wanted bounty menu
;===============================================

.define shortWantedOffset $82

.bank $19 slot 3
.org $1A7C
.section "short enemy names 1" overwrite
  doLongjmp prepShortEnemyNames_wantedList
  jmp $BA88
.ends

.slot 3
.section "short enemy names 2" superfree
  prepShortEnemyNames_wantedList:
    ; make up work
    clc
    lda $00DE
    adc #$5F
    jsr $FE9F
    beq +
      inc $00CC
    +:
    
    ; copy enemy ID to a new location for "no." field (original game just
    ; prints the raw ID used for the name field as a number)
    lda #$01    ; region num
    sta $0527
    ; get enemy ID
    lda $00E2
    ; copy
    sta $0526
    
    ; add offset to short version of enemy name
    clc
    adc #shortWantedOffset
    sta $00E2
    rts
    
.ends

.bank $19 slot 3
.org $1A5A
.section "short enemy names 3" overwrite
  lda #$63-1    ; initial msgpos
.ends

;===============================================
; use two-line names on rental tank chasses
;===============================================

.bank $18 slot 3
.org $021E
.section "short rental tank names 1" overwrite
  doLongjmp setUpRentalChassisName
  jmp $A228
.ends

.slot 3
.section "short rental tank names 2" superfree
  setUpRentalChassisName:
    ; make up work
    lda #$9F
;    sta $051E
    sta newNameBuffer+1
    
    ; use two-line region
    lda #twoLineItemRegion
    sta $00E3
    
    ; make up work
    lda #$4B
    jmp $EB16
.ends

.bank $18 slot 3
.org $0453
.section "msgpos of return tanks" overwrite
  lda #$65-2
.ends

/*.slot 3
.section "short rental tank names 2" superfree
  setUpRentalChassisName:
    ; make up work
    ldy $00DD
    lda ($00CA),Y
    tax
    jsr $EE35
    ldy $E701.w,X
    lda $8005.w,Y
    jsr $EACE
    
    lda #twoLineItemRegion
    sta $00E3
    
    ; make up work
    txa
    sec
    sbc #$07
    sta $051D
    lda #$9F
    sta $051E
    rts
.ends */

;===============================================
; use plural enemy names where appropriate
;===============================================

;.bank $18 slot 3
;.org $021E
;.section "plural enemy names 1" overwrite
;
;.ends

;.bank $17 slot 3
;.org $0B1E
;.section "plural enemy names explosion 1" overwrite
;  doLongjmp doPluralEnemyExplosion
;.ends

;.slot 3
;.section "plural enemy names explosion 2" superfree
;  doPluralEnemyExplosion:
;    ; make up work
;    ldy 
;    
;    
;    rts
;.ends

.bank $17 slot 3
.org $0424
.section "plural enemy names target menu 1" overwrite
  doLongjmp doPluralEnemyTargets
.ends

.slot 3
.section "plural enemy names target menu 2" superfree
  doPluralEnemyTargets:
    ; get enemy slotnum
    ldx $00DC
    ; get enemy typeid
    lda $7002,X
    ; queue script var 0A
    jsr $EAD5
    
    ; check if enemy count (previously saved to $00DE) is >= 2
    lda $00DE
    cmp #$02
    bcc +
      ; set plural name region
      lda #pluralEnemyNameRegion
      sta $00E3
    +:
    
    ; run script 2-B
;    lda #$0B
;    jmp $EAFB

    rts

.ends

;===============================================
; add space before enemy ID string
;===============================================

.bank $17 slot 3
.org $105D
.section "add space before enemy ID string 1" overwrite
  doLongjmp addEnemyIdSpace
  rts
.ends

.slot 3
.section "add space before enemy ID string 2" superfree
  addEnemyIdSpace:
    
    ; copy ID character to second pos
    lda $0534
    sta $0535
    ; first char = space
    lda #$FF
    sta $0534
    ; write terminator
    lda #$9F
    sta $0536
    
    ; if enemy quantity < 2, set first character to terminator
    ldy $70E8
    ldx $700A.w,Y
    cpx #$02
    bcs +
      sta $0534
    +:

    rts
  
  wanderingTilesFix:
  
    ; make up work
    ldx $00D4
    lda $724E.w,Y
    sta $00C0
    adc $724E.w,X
    bcc +
      lda #$FF
    +:
    sta $724E.w,X
    sty $00D4
    
    ; make up call to B06D
    lda $71E2.w,X
    sta $0534
    lda $71C7.w,X
    sta $7068
    jsr $EAD5
    
    ; copy ID character to second pos
    lda $0534
    sta $0535
    ; first char = space
    lda #$FF
    sta $0534
    ; write terminator
    lda #$9F
    sta $0536
    
    ; make up work
    dec $70D3
    lda #$4B
    sta $00DD
    
    rts

.ends

; fix wandering tiles attachment messages
.bank $17 slot 3
.org $136F
.section "add space before enemy ID string 3" overwrite
  doLongjmp wanderingTilesFix
  rts
.ends

;===============================================
; use translated name for Red Wolf
;
; for the two battles that Wolf appears in, the
; game temporarily copies over a player name
; slot with ウルフ. we need to update this for
; the translation
;===============================================

.bank $10 slot 3
.section "translate wolf battle name 1" free
  wolfName:
    .incbin "out/script/name_wolf.bin" FSIZE wolfNameSize
.ends

.bank $10 slot 3
.org $1A74
.section "translate wolf battle name 2" overwrite
  ; transfer size
  ldx #wolfNameSize-1
.ends

.bank $10 slot 3
.org $1A7C
.section "translate wolf battle name 3" overwrite
  ; name pointer
  lda wolfName.w,X
.ends

.bank $10 slot 3
.org $1A6E
.section "translate wolf battle name 4" overwrite
  ; second battle offset to char1 name (last byte + 1)
  ldy #(char1Name-newBlock0NameChunkStart)+wolfNameSize
  
  ; init char status flags(?) (needs to be #$04, we've moved this initialization
  ; elsewhere)
  nop
  nop
  nop
.ends

.bank $10 slot 3
.org $1A9B
.section "translate wolf battle name 5" overwrite
  ; first battle offset to char2 name (last byte)
  ldy #(char2Name-newBlock0NameChunkStart)+wolfNameSize-1
.ends

/*.bank $17 slot 3
.org $029C
.section "translate wolf battle name 6" overwrite
  ; char2 name restore ptr
  sta char2Name.w,X
.ends

.bank $17 slot 3
.org $0297
.section "translate wolf battle name 7" overwrite
  ; char2 name restore size
  ldx #wolfNameSize-1
.ends */

  ;=====
  ; add extra code to fix an oversight:
  ; it's possible to run from the bionic pooch battle where wolf first
  ; appears (which is probably a bug in itself but whatever).
  ; char2's name is not copied to the 0580 buffer until wolf's tank actually
  ; enters the battle. if the player successfully runs from the battle on the
  ; first turn, the empty buffer will still be copied back to char2's name,
  ; changing it to "0000".
  ;=====

  .bank $17 slot 3
  .org $0297
  .section "translate wolf battle name 6" overwrite
    doLongjmp restoreChar2Name
    jmp $A2A2
  .ends

  .slot 3
  .section "translate wolf battle name 7" superfree
    restoreChar2Name:
      ; if first character of char2's name is "0", assume the buffer is empty
      ; and do not copy the name back.
      ; (char2 cannot be given a custom name, so this is safe)
      lda $0580
      beq @done
        
        ; char2 name restore size
        ldx #wolfNameSize-1
        -:
          lda $0580.w,X; char2 name restore ptr
          sta char2Name.w,X
          dex
          bpl -
        
      @done:
      rts
  .ends

/*.bank $17 slot 3
.org $0FAB
.section "translate wolf battle name 8" overwrite
  ; char1 name restore pointer
  sta char1Name.w,X
.ends

.bank $17 slot 3
.org $0FAB
.section "translate wolf battle name 9" overwrite
  ; char1 name restore size
  sta char1Name.w,X
.ends */

.bank $17 slot 3
.org $0F9A
.section "translate wolf battle name 8" overwrite
  ; char 1 restore after battle 2 (name + stats)
  doLongjmp restoreChar1NameAndStats
  jmp $AFB1
.ends

.slot 3
.section "translate wolf battle name 9" superfree
  restoreChar1NameAndStats:
    ; name
    ldx #wolfNameSize-1
    -:
      lda $0580.w,X
      sta char1Name.w,X
      dex
      bpl -
    
    ; stats
    ldx #$03
    -:
      lda $0576.w,X
      sta $6764.w,X
      lda $057A.w,X
      sta $6478.w,X
      dex
      bpl -
    
    rts
.ends

; correctly init char status flags(?)
.bank $10 slot 3
.org $1A68
.section "translate wolf battle name 10" overwrite
  doLongjmp wolfNameBattle04Init
.ends

.slot 3
.section "translate wolf battle name 11" superfree
  wolfNameBattle04Init:
    ; make up work
    lda #$00
    sta $7073
    sta $7076
    
    ; init char status flags?
    ldy #$04
    sty $6478
    
    rts
.ends

;===============================================
; use short part type names where needed
;===============================================

.define shortPartFormOffset $27

  ;=====
  ; status screen part type headers
  ;=====

  .bank $10 slot 3
  .org $0D9E
  .section "use short part type names on status screens 1" overwrite
    doLongjmp useShortPartTypes_statusHeader
    nop
  .ends

  .slot 3
  .section "use short part type names on status screens 2" superfree
    useShortPartTypes_statusHeader:
      ; make up work
      jsr $EABA
      
      ; get part type
      lda $00DE
      ; add offset to short form
      clc
      adc #shortPartFormOffset
      ; save for script access
      sta $00E2
      
      rts
  .ends

  ;=====
  ; tank summary
  ;=====

  .bank $19 slot 3
  .org $1655
  .section "use short part type names on tank summary screens 1" overwrite
    doLongjmp useShortPartTypes_tankSummary
    rts
  .ends

  .slot 3
  .section "use short part type names on tank summary screens 2" superfree
    useShortPartTypes_tankSummary:
      ldy #$FE
      @checkPartsLoop:
        ldx $0000
        lda $6627.w,X
        beq +
          iny 
          iny 
        +:
        
        asl $0006
        bcc +
          ; get part type
          lda $0001
          
          ; add offset to short form
          clc
          adc #shortPartFormOffset
          
          ; save for script access
          sta $0544.w,Y
          
          lda #$08
          sta $0545.w,Y
        +:
        
        jsr $EB3F
        inc $0001
        lda $0001
        cmp #$08
        bne @checkPartsLoop
      rts 
      
  .ends

  ;=====
  ; shop previews
  ;=====

  .bank $19 slot 3
  .org $138F
  .section "use short part type names on shop previews 1" overwrite
    ; originally a subtraction by #$1C to get the part type
    clc
    adc #shortPartFormOffset-$1C
  .ends

;  .slot 3
;  .section "use short part type names on shop previews 2" superfree
;    useShortPartTypes_shopPreview:
;      ; add offset to short form
;      lda $052C
;      
;      rts
;  .ends

;===============================================
; offset "other" menu"
;===============================================

  .bank $17 slot 3
  .org $04EB
  .section "use offset other menu 1" overwrite
    doLongjmp useOffsetOtherMenu
  .ends

  .slot 3
  .section "use offset other menu 2" superfree
    useOffsetOtherMenu:
      ; make up work
      lda #$06
      tay
;      jmp $F32E
      sty $00CF
      jsr $EB0F
      jsr $FA55
      jsr $EB68
      
      ; set msgpos
;      dec $00C4
      lda #$4E-1
      jsr setMsgPos
      
      ; make up work
      jsr $F5FC
      jmp $EA40
  .ends

;===============================================
; width of WRAM clear for damage/trunk screens
; (if tank 4 or 8 has 7-character name,
; rightmost character won't be cleared)
;===============================================

.bank $3F slot 5
.org $1A2A
.section "damage/trunk wram clear" overwrite
  .db $1C+1
.ends

;===============================================
; use new S/H battle icons
;===============================================

.bank $3F slot 5
.org $127E
.section "new s/h icons 1" overwrite
  ldy #$5F      ; H
.ends

.bank $3F slot 5
.org $1282
.section "new s/h icons 2" overwrite
  ldy #$5E      ; S
.ends

;===============================================
; add credits page
;===============================================

.define oldCreditsEndId $0F
.define creditsExtensionStartId $15
.define newCreditsEndId $16

; n.b. executes out of slot 2, unlike most code
.bank $1C slot 2
.org $047A
.section "new credits 1" overwrite
  doLongjmp newCredits
.ends

.slot 3
.section "new credits 2" superfree
  newCredits:
    ldy $0052
    iny
    ; if ID is less than end of old credits, run as normal
    cpy #oldCreditsEndId
    bcc @doStandardPage
    
    @doExtendedPage:
      ; if ID is greater than end of old credits, but less than start of new
      ; credits, set ID to start of new credits
      cpy #creditsExtensionStartId
      bcc @startExtendedCredits
      
        ; if ID is greater than or equal to the end of the new credits, we're
        ; done; otherwise, run as normal
        cpy #newCreditsEndId
        bcc @doStandardPage
        
        @done:
          endNoRetLongjmp $8480
      
    @startExtendedCredits:
      ldy #creditsExtensionStartId
    
    @doStandardPage:
      sty $0052
      endNoRetLongjmp $845E
      
    
    rts
.ends

;===============================================
; clear moved "special shells" label in
; shells menu
;===============================================

.bank $3F slot 5
.org $19A8
.section "clear shells label 1" overwrite
  .dw $6142-$20 ; WRAM dstaddr
.ends

.bank $3F slot 5
.org $19F3
.section "clear shells label 2" overwrite
  .db $08+1     ; height
.ends

;===============================================
; additional opcodes
;
; NOTE: no new skip handlers implemented!
; new ops must be 1 byte, no terminators!
;===============================================

.define halfBrOpcode $E1
.define inPlaceScrE2Opcode $E0
.define keyNoBr $DF

.define lowestOpcode keyNoBr

;.bank $19 slot 3
;.org $1BCC
;.section "additional opcodes 1" overwrite
;  ; change lowest recognized opnum
;  cmp #halfBrOpcode
;.ends

.bank $19 slot 3
.org $1C06
.section "additional opcodes 2" overwrite
  ; this is used as a relative branch target elsewhere.
  ; WLA-DX does not allow it to be addressed directly; it MUST have a label
  controlCodeHandler:
  jmp checkAddOps
  nop
.ends

.bank $19 slot 3
.section "additional opcodes 3" free
  checkAddOps:
    ; make up work: check for space character
    cmp #$FF
    bne +
      jmp $BBFB
    +:
    
    ; jump to regular handlers if >= #$E2
    cmp #$E2
    bcc +
      jmp $BC0A
    +:
    
    tax
    lda scratchLo
    pha
      txa
      sta scratchLo 
      doLongjmp checkExtendedOpcodes
    pla
    sta scratchLo
    rts
    
.ends

.slot 3
.section "additional opcodes 4" superfree
  checkExtendedOpcodes:
    lda scratchLo
    
    cmp #halfBrOpcode
    bne +
;      doLongjmp handleHalfBr
      jsr handleHalfBr
      rts
    +:
    cmp #inPlaceScrE2Opcode
    bne +
      ; source pointer for script num/region == $00E2
      lda #$E2
      sta $000C
      lda #$00
      sta $000D
      
      ; save dstaddr
      lda $00B6
      pha
      lda $00B7
      pha
      
        ; run script
;        jsr $BD56
        jsr longjmp
        .db $19
        .dw $BD56
      
      ; restore dstaddr
      pla
      sta $00B7
      pla
      sta $00B6
      
;      endNoRetLongjmp 
      
      rts
    +:
    cmp #keyNoBr
    bne +
      jmp $F8C5
    +:
    
    ; should never happen
    rts
    
  handleHalfBr:
    ; don't even try to use this in scrolling dialogue -- the scrolling
    ; logic isn't implemented.
    ; this is specifically intended for menus
    
    ; AND WRAM putaddr with #$FFE0
    lda $00B6
    and #$E0
    sta $00B6
    ; get $05A0 + #$20 (1 line of tilemap)
    clc 
    lda $05A0
    adc #$20
    sta $0000
    ; add to putaddr
    clc 
    lda $0000
    adc $00B6
    sta $00B6
    lda #$00
    adc $00B7
    sta $00B7
    
    lda halfBrOn
    bne @endHalfBr
    @startHalfBr:
      dec halfBrOn
      rts
    @endHalfBr:
      inc halfBrOn
      ; increment line counter
      inc $059D
      rts
    
.ends

