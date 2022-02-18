# build
gcc thumbnail.c main.c -o task `pkg-config --cflags --libs gstreamer-1.0 gstreamer-pbutils-1.0`

# run
## localplayer
e.g: ./task file:///home/zhangqianbo/Desktop/Gstreamer/out.mp4

## http/https
e.g: ./task https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm