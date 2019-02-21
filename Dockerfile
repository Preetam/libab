FROM golang:1.11.5 

ENV LIBAB_VERSION 0.6.5

RUN set -eux; \
	wget -O libab.a "https://github.com/Preetam/libab/releases/download/v$LIBAB_VERSION/libab-linux-x64.a" && \
	wget -O libab.so "https://github.com/Preetam/libab/releases/download/v$LIBAB_VERSION/libab-linux-x64.so" && \
	wget -O libuv.a "https://github.com/Preetam/libab/releases/download/v$LIBAB_VERSION/libuv_a-linux-x64.a" && \
	wget -O libuv.so "https://github.com/Preetam/libab/releases/download/v$LIBAB_VERSION/libuv-linux-x64.so" && \
	wget -O ab.h "https://github.com/Preetam/libab/releases/download/v$LIBAB_VERSION/ab.h" && \
	mv *.a /usr/local/lib && \
	mv *.so /usr/local/lib && \
	mv ab.h /usr/local/include
