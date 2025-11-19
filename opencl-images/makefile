all: bin/fake_tracer \
	 bin/neuro

bin/fake_tracer: src/img.cpp src/kernel.cpp
	$(CXX) $(CXXFLAGS) src/img.cpp src/kernel.cpp -o bin/fake_tracer -std=c++17 -pthread -O -Wno-comment -Iinclude -Llib -lOpenCL

bin/neuro: src/neuro.cpp src/kernel.cpp
	$(CXX) $(CXXFLAGS) src/neuro.cpp src/kernel.cpp -o bin/neuro -std=c++17 -pthread -O -Wno-comment -Iinclude -Llib -lOpenCL

clean:
	rm -f bin/*
