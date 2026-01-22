FROM ubuntu:22.04
FROM gcc:latest
# FROM kitware/cmake:build-linux-x86_64-deps-2026-01-13

 RUN apt-get update && apt-get install -y cmake

 COPY . usr/src/cpp
 WORKDIR usr/src/cpp/build
 RUN rm -rf **
 RUN cmake ..
 RUN cmake --build .
 CMD [ "./FAT12" ]