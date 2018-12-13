
function ripScript() {
  ./max_scriptrip_raw max.nes metal_max_table.tbl "$2" "$3" > "scripts_region$1.txt"
}

ripScript 14 0xBE80 34

# tilemaps
ripScript 09 0x10000 23
# tilemaps
ripScript 02 0x10119 171
# tilemaps
ripScript 03 0x10da3 25
# tilemaps
ripScript 04 0x112e2 43
# tilemaps
ripScript 12 0x1156C 26
ripScript 13 0x11923 41
ripScript 00 0x11a16 222

ripScript 0C 0x12000 35
# tilemaps
ripScript 0D 0x120a6 102
ripScript 06 0x124de 161
ripScript 10 0x13387 118

ripScript 05 0x14000 215

ripScript 07 0x16000 128
ripScript 0A 0x16bcd 200
ripScript 0F 0x174f3 62

ripScript 0E 0x18000 186

ripScript 11 0x1f98a 226

ripScript 01 0x21ae6 131
ripScript 08 0x21ebd 39

ripScript 0B 0x36000 197

ripScript 15 0x384a5 21

