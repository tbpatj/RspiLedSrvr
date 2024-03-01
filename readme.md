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

`brew install pkg-config`

compiling will need \`pkg-config --cflags --libs opencv4\`

## Compiling

<!-- compile script
`g++ main.cpp -o app -std=c++11`

`g++ main.cpp -o app \`pkg-config --cflags --libs opencv4\`` -->

compile script
g++ main.cpp -o app -I/usr/local/include/opencv4 -std=c++14 \`pkg-config --libs opencv4\`
