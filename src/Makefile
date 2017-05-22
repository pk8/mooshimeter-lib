CC=g++
CXXFLAGS+=-std=c++1y
LDLIBS+=-lbluetooth -lz -lpthread

.PHONY: all
all: app1 app2 app3

app1: app1.o ble.o config_tree.o measurement.o mooshimeter.o reader.o util.o writer.o

app2: app2.o ble.o config_tree.o measurement.o mooshimeter.o reader.o util.o writer.o

app3: app3.o ble.o config_tree.o measurement.o mooshimeter.o reader.o util.o writer.o

.PHONY: clean
clean:
	rm -f *.o app1 app2 app3
#
