FROM consol/ubuntu-icewm-vnc

USER 0

RUN apt update && apt install -y \
  wget \
  cmake \
  mpich \
  gdb \
  valgrind \
  libsdl2-dev \
  libsdl2-image-dev \
  libsdl2-ttf-dev

USER 1000

WORKDIR /app
