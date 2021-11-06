#This is a sample Image 
FROM ubuntu 
MAINTAINER tpalit@cs.stonybrook.edu

RUN apt-get update 
RUN apt-get install -y libcap2-bin
RUN apt-get install -y sudo
RUN apt-get install -y kmod
RUN ln -sf /bin/bash /bin/sh
CMD ls

