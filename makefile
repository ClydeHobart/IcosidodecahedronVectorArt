L = Logging
F = Phi
P = Poly
V = Vector
FV = $F$V
FV3 = $(FV)3
PH = $Phedron
FPH = $F$(PH)
E = Extrema
V2 = $V2
V3 = $V3
PG = $Pgon
C = Color
S = Svg
M = main

GPP = g++
CFLAGS = -Wall -pedantic -ggdb -std=c++20

.PHONY: clean

$M: $M.o $L.o $(FV).o $(FV3).o $(FPH).o $(V3).o $E.o $(V2).o $C.o $S.o $(PG).o $(PH).o
	$(GPP) $(CFLAGS) $^ -o $@

$M.o: $M.cpp $(FPH).h $(FV).h $(FV3).h $(PH).h
	$(GPP) $(CFLAGS) -c $<

$L.o: $L.cpp $L.h
	$(GPP) $(CFLAGS) -c $<

$(FV).o: $(FV).cpp $(FV).h
	$(GPP) $(CFLAGS) -c $<

$(FV3).o: $(FV3).cpp $(FV3).h $(FV).h
	$(GPP) $(CFLAGS) -c $<

$(FPH).o: $(FPH).cpp $(FPH).h $(FV).h $(FV3).h
	$(GPP) $(CFLAGS) -c $<

$(V3).o: $(V3).cpp $(V3).h $(FV3).h
	$(GPP) $(CFLAGS) -c $<

$E.o: $E.cpp $E.h $(V2).h
	$(GPP) $(CFLAGS) -c $<

$(V2).o: $(V2).cpp $(V2).h $(E).h $(V3).h
	$(GPP) $(CFLAGS) -c $<

$C.o: $C.cpp $C.h
	$(GPP) $(CFLAGS) -c $<

$S.o: $S.cpp $S.h $(V2).h
	$(GPP) $(CFLAGS) -c $<

$(PG).o: $(PG).cpp $(PG).h $(V2).h $S.h $L.h
	$(GPP) $(CFLAGS) -c $<

$(PH).o: $(PH).cpp $(PH).h $(FPH).h $(FV3).h $(V3).h $(PG).h
	$(GPP) $(CFLAGS) -c $<

clean:
	rm -f *.o *~ *.dSYM $M