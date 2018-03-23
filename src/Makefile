

CFLAGS=-g
LDFLAGS=-llz4 -g

OBJ_D	= jsonlz4_json.o
DECOMP	= jsonlz4_json
OBJ_C	= json_jsonlz4.o
COMP	= json_jsonlz4

ALL:	$(DECOMP) $(COMP)

$(DECOMP):	$(OBJ_D)
$(COMP):	$(OBJ_C)

clean:
	rm *.o $(DECOMP) $(COMP)
