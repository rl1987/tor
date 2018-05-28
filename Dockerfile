FROM debian:jessie-slim

RUN apt-get update && apt-get install -y git automake autoconf make gcc libssl-dev openssl zlib1g-dev libevent-dev

COPY . /src/tor
WORKDIR /src/tor
RUN ./autogen.sh && ./configure --disable-asciidoc && make && make install

EXPOSE 9050/tcp
EXPOSE 443/tcp
EXPOSE 9051/tcp

RUN groupadd -g 111 tor && useradd -r -u 111 -g tor tor
RUN mkdir /home/tor && cd /home/tor
RUN chown tor:tor /home/tor
USER tor

ENV PATH="/usr/local/bin:${PATH}"

CMD [ "tor" ]
