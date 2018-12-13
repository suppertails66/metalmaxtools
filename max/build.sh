
echo "*******************************************************************************"
echo "Setting up environment..."
echo "*******************************************************************************"

set -o errexit

BASE_PWD=$PWD
PATH=".:./asm/bin/:$PATH"
INROM="max.nes"
OUTROM="max_en.nes"
WLADX="./wla-dx/binaries/wla-6502"
WLALINK="./wla-dx/binaries/wlalink"

DO_BUILDSCRIPT=1
DO_INSERTSCRIPT=1

cp "$INROM" "$OUTROM"

mkdir -p out

echo "*******************************************************************************"
echo "Building tools..."
echo "*******************************************************************************"

make blackt
make libnes
make

if [ ! -f $WLADX ]; then
  
  echo "********************************************************************************"
  echo "Building WLA-DX..."
  echo "********************************************************************************"
  
  cd wla-dx
    cmake -G "Unix Makefiles" .
    make
  cd $BASE_PWD
  
fi

echo "*******************************************************************************"
echo "Doing initial ROM prep..."
echo "*******************************************************************************"

mkdir -p out
romprep "$OUTROM" "$OUTROM" "out/max_chr.bin"

echo "*******************************************************************************"
echo "Patching graphics..."
echo "*******************************************************************************"

mkdir -p out/grp
nes_tileundmp grp/trans/font1.png 0xA0 out/grp/font1.bin
nes_tileundmp grp/trans/font2.png 0x20 out/grp/font2.bin
nes_tileundmp grp/trans/font3.png 0x20 out/grp/font3.bin
nes_tileundmp grp/trans/kanji.png 0xC0 out/grp/kanji.bin
nes_tileundmp grp/trans/menu1.png 0x40 out/grp/menu1.bin
nes_tileundmp grp/trans/menu2.png 0x40 out/grp/menu2.bin
nes_tileundmp grp/trans/menu3.png 0x40 out/grp/menu3.bin
nes_tileundmp grp/trans/race.png 0xC0 out/grp/race.bin
nes_tileundmp grp/trans/status.png 0x40 out/grp/status.bin

filepatch out/max_chr.bin 0x2000 out/grp/font1.bin out/max_chr.bin
filepatch out/max_chr.bin 0x2A00 out/grp/menu1.bin out/max_chr.bin
filepatch out/max_chr.bin 0x4800 out/grp/font2.bin out/max_chr.bin
filepatch out/max_chr.bin 0x4A00 out/grp/menu2.bin out/max_chr.bin
filepatch out/max_chr.bin 0x9C00 out/grp/status.bin out/max_chr.bin
filepatch out/max_chr.bin 0xD000 out/grp/menu3.bin out/max_chr.bin
filepatch out/max_chr.bin 0xD800 out/grp/font3.bin out/max_chr.bin
filepatch out/max_chr.bin 0x24000 out/grp/kanji.bin out/max_chr.bin
filepatch out/max_chr.bin 0x32000 out/grp/race.bin out/max_chr.bin
filepatch out/max_chr.bin 0x39800 grp/trans/frograce_bg_tilemap.bin out/max_chr.bin

# echo "*******************************************************************************"
# echo "Building tilemaps..."
# echo "*******************************************************************************"
# 
# #mkdir -p out/maps_raw
# #tilemapper_nes tilemappers/title.txt
# mkdir -p out/maps
# tilemapper_nes tilemappers/title.txt
# 
# #mkdir -p out/maps_conv
# #mapconv out/maps_conv/
# 
# filepatch out/max_chr.bin 0x1D010 out/grp/title_grp.bin out/max_chr.bin
# filepatch "$OUTROM" 0x15292 out/maps/title.bin "$OUTROM"
# filepatch "$OUTROM" 0x154D5 rsrc_raw/title_attrmap.bin "$OUTROM"

# echo "*******************************************************************************"
# echo "Patching other graphics..."
# echo "*******************************************************************************"
# 
# #rawgrpconv rsrc/misc/shiro.png rsrc/misc/shiro.txt out/sanma_chr.bin out/sanma_chr.bin
# #rawgrpconv rsrc/misc/kyojin.png rsrc/misc/kyojin.txt out/sanma_chr.bin out/sanma_chr.bin
# 
# for file in rsrc/misc/*.txt; do
#   bname=$(basename $file .txt)
#   rawgrpconv rsrc/misc/$bname.png rsrc/misc/$bname.txt out/max_chr.bin out/max_chr.bin
# done

echo "*******************************************************************************"
echo "Building script..."
echo "*******************************************************************************"

# mkdir -p out/script
# mkdir -p out/script/maps
# scriptconv script/ table/max_en.tbl out/script/
# 
# filepatch "out/max_chr.bin" 0x3BDC0 out/script/battle_inventory.bin "out/max_chr.bin"

#max_scriptbuild "$OUTROM" "$OUTROM" buildscript.txt > out/scriptbuild_log.txt 2> out/scriptbuild_error.txt

mkdir -p out/script

if [ ! $DO_BUILDSCRIPT == 0 ]
then
  scriptconv "script/" "table/metal_max_table.tbl" "out/script/"
  if [ ! $DO_INSERTSCRIPT == 0 ]
  then
    max_scriptbuild "buildscript.txt"
  fi
fi

rawscriptconv "script/" "table/metal_max_table_en.tbl" "out/script/"


echo "********************************************************************************"
echo "Applying ASM patches..."
echo "********************************************************************************"

mkdir -p "out/asm"
cp "$OUTROM" "asm/max.nes"

cd asm
  # apply hacks
  ../$WLADX -I ".." -o "boot.o" "boot.s"
  ../$WLALINK -v linkfile max_build.nes
cd $BASE_PWD

mv -f "asm/max_build.nes" "$OUTROM"
rm "asm/max.nes"
rm asm/*.o

echo "*******************************************************************************"
echo "Finalizing ROM..."
echo "*******************************************************************************"

romfinalize "$OUTROM" "out/max_chr.bin" "$OUTROM"

echo "*******************************************************************************"
echo "Success!"
echo "Output file:" $OUTROM
echo "*******************************************************************************"
