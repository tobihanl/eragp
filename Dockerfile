FROM consol/ubuntu-icewm-vnc

USER 0

RUN apt update && apt install -y \
  wget \
  mpich \
  libsdl2-dev \
  libsdl2-image-dev \
  libsdl2-ttf-dev

COPY ./install_cmake.sh /

RUN /install_cmake.sh

USER 1000

WORKDIR /app


