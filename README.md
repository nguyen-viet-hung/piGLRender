# piGLRender
This demonstrates how to use MMAL H.264 decoder on pi and draw video frame using GLSL.
Tested with FFMPEG 4.0 with --enable-mmal --enable-omx --enable-omx-rpi.
The command line for build source code (ffmpeg was built as static):

```shell
g++ -Wall  -L/opt/vc/lib -o glrender main.cpp -lGL -lGLU -lglut -lavformat -lavcodec -lavutil -lswresample -ldl -lz -lpthread -lssl -lcrypto -lm -lmmal_core -lmmal_util -lmmal_vc_client -lbcm_host
```
