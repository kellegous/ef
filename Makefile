CFLAGS := -std=c++17 -Wall -I. \
	$(shell PKG_CONFIG_PATH=/usr/local/opt/openssl/lib/pkgconfig pkg-config cairomm-1.16 grpc protobuf --cflags)
LDFLAGS := -Wall -lgrpc++_reflection -lpthread \
	$(shell PKG_CONFIG_PATH=/usr/local/opt/openssl/lib/pkgconfig pkg-config cairomm-1.16 grpc++ protobuf --libs)

ALL: efa

%.o: %.cc %.h
	g++ $(CFLAGS) -c -o $@ $<

efa: point.o vec2.o charge.o theme.pb.o rpc.pb.o rpc.grpc.pb.o efa.o
	g++ $(LDFLAGS) -o $@ $^

color/pkg/rpc/rpc.proto:
	git clone git@github.com:kellegous/color.git

color/pkg/theme.proto:
	git clone git@github.com:kellegous/color.git

pkg/rpc/rpc.pb.cc: color/pkg/rpc/rpc.proto
	protoc -Icolor --cpp_out=. $<

pkg/rpc/rpc.grpc.pb.cc: color/pkg/rpc/rpc.proto
	protoc -Icolor --grpc_out=. --plugin=protoc-gen-grpc=$(shell which grpc_cpp_plugin) $<

rpc.grpc.pb.o: pkg/rpc/rpc.grpc.pb.cc
	g++ -c $(CFLAGS) -o $@ $<

pkg/theme.pb.cc: color/pkg/theme.proto
	protoc -Icolor --cpp_out=. $<

rpc.pb.o: pkg/rpc/rpc.pb.cc pkg/theme.pb.cc
	g++ -c $(CFLAGS) -o $@ $<

theme.pb.o: pkg/theme.pb.cc
	g++ -c $(CFLAGS) -o $@ $<

clean:
	rm -f efa *.o