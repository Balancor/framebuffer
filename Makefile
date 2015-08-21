COMMON_FILES := logs.c logs.h
SHAPE_SRC_FILES := shape.c
DUMP_SRC_FILES := dump_framebuffer.c
shape: $(SRC_FILES)
	gcc -g -o shape $(COMMON_FILES) $(SHAPE_SRC_FILES)

dump: dump_framebuffer.c
	gcc -g -o dump $(DUMP_SRC_FILES) 

clean:
	rm -rf *.o
	rm -rf *.gch
	rm -f shape
	rm -f logs/running.log
	rm -f core
	rm -f dump 
