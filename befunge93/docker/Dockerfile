FROM ubuntu:20.04

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y xinetd 

RUN useradd -m ctf
RUN chown -R root:root /home/ctf
RUN chmod -R 755 /home/ctf

WORKDIR /

COPY readflag /
COPY flag /

RUN chown 0:0 readflag flag
RUN chmod 400 flag && chmod 755 readflag
RUN chmod +x readflag
RUN chmod +s readflag

CMD ["/usr/sbin/xinetd","-dontfork"]
