SRC_FILES := logs.c logs.h shape.c
shape: $(SRC_FILES)
	gcc -o shape $(SRC_FILES)

clean:
	rm -rf *.o
	rm -rf *.gch
	rm -f shape
