# Running on your machine.

If you download the application you will need to build the front end and put the build folder in the the same folder as the executable
Front End Code: https://github.com/tbpatj/raspi-led-display-site - build by running `npm run build` after you have installed node.js
You will also need a folder `./resources/animations` within the same directory as the executable this will hold all the animation images

# Libraries used

JSON Library
https://github.com/nlohmann/json
installed using the single_include `./resources/json.hpp` file

HTTP Server Library
https://github.com/yhirose/cpp-httplib
installed using the single_include file `./resources/httplib.h`

RPI_WS281x
https://github.com/jgarff/rpi_ws281x
Follow the instructions for the building with cMake

WIP
WS connection
https://github.com/zaphoyd/websocketpp.git
Installed an included in the compile instructions

## open cv installation

OpenCV Library
https://github.com/opencv/opencv/tree/4.x

## PiGPIO installation

instructions can be found at this website
https://abyz.me.uk/rpi/pigpio/download.html

### mac

install using brew
`brew install opencv`

`brew install pkg-config`

compiling will need \`pkg-config --cflags --libs opencv4\`

## Compiling

<!-- compile script
`g++ main.cpp -o app -std=c++11`

`g++ main.cpp -o app \`pkg-config --cflags --libs opencv4\`` -->

compile script
g++ main.cpp -o app -I/usr/local/include/opencv4 -std=c++14 \`pkg-config --libs opencv4\`

compiling with led device on raspi
g++ main.cpp -o app -std=c++14 \`pkg-config --cflags --libs opencv4\` -I/usr/local/include/ws2811 -lws2811 -lstdc++fs

## Branches

main - main branch where most updates are created but does not have raspberry pi output functionality

functioning-leds - the branch where the leds are actually implemented to work with raspberry pi

when updating the main branch make sure to rebase the functioning leds to main when you want to actually utilize the new features on a raspberry pi
use:

```
git checkout functioning-leds
git rebase -i main
-resolve conflicts, that don't involve the experimental::filesystem and that don't involve the ws2811 stuff
```
