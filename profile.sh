cmake -DCMAKE_CXX_FLAGS=-pg -DCMAKE_EXE_LINKER_FLAGS=-pg -DCMAKE_SHARED_LINKER_FLAGS=-pg -DDO_PROFILE=1 ../
make -j8 && $1
gprof $1 gmon.out > analysis.txt
gprof2dot analysis.txt | dot -Tpng -o output.png
gprof2dot analysis.txt | dot -Tsvg -o output.svg
