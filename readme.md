# Libraries used

JSON Library
https://github.com/nlohmann/json
installed using the single_include `./resources/json.hpp` file

HTTP Server Library
https://github.com/yhirose/cpp-httplib
installed using the single_include file `./resources/httplib.h`

## open cv installation

OpenCV Library
https://github.com/opencv/opencv/tree/4.x

### mac

install using brew
`brew install opencv`

compiling will need \`pkg-config --cflags --libs opencv4\`

## Compiling

compile script
`g++ main.cpp -o app -std=c++11`

`g++ main.cpp -o app \`pkg-config --cflags --libs opencv4\``

g++ main.cpp -o app `pkg-config --cflags --libs opencv4` -std=c++11
