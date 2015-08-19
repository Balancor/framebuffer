SRC_FILES := logs.c logs.h shape.c
shape: $(SRC_FILES)
	gcc -c $(SRC_FILES)

clean:
	rm -rf *.o
