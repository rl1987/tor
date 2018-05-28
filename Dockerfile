FROM debian:jessie-slim

RUN apt-get update && apt-get install -y git automake autoconf make gcc libssl-dev openssl zlib1g-dev libevent-dev

COPY . /src/tor
WORKDIR /src/tor
RUN ./autogen.sh && ./configure --disable-asciidoc && make

EXPOSE 9050/tcp
EXPOSE 443/tcp
EXPOSE 9051/tcp

CMD /src/tor/src/or/tor
