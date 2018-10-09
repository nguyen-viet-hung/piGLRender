#!/bin/sh

g++ -Wall  -I/opt/vc/include -L/opt/vc/lib -o glrender main.cpp -lavformat -lavcodec -lavutil -lswresample -ldl -lz -lpthread -lssl -lcrypto -lm -lmmal_core -lmmal_util -lmmal_vc_client -lbcm_host
