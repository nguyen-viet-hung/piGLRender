#!/bin/sh

g++ -Wall -DUSING_OPENGL -L/opt/vc/lib -o glrender main.cpp -lGL -lGLU -lglut -lavformat -lavcodec -lavutil -lswresample -ldl -lz -lpthread -lssl -lcrypto -lm -lmmal_core -lmmal_util -lmmal_vc_client -lbcm_host
