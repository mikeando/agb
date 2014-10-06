all:
	mkdir -p build
	( cd build && cmake .. && make ; )


clean:
	mkdir -p build
	( cd build && cmake .. && make clean ; )
