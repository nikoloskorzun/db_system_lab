FROM ubuntu:20.04

RUN mkdir /Project
RUN mkdir /Project/source/
COPY source /Project/source/
RUN apt-get update
RUN apt-get install -y gcc
RUN apt-get install -y gdb
