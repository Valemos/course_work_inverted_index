Project implements index for quick search in directories with text files

CMake is required for build

to build all executables for this project in cmd in the same folder as CMakeLists.txt should run these commands

```
cmake -B "build"
cmake --build "build" --target {target name} --config Release
```

target name can be one of

  * ALL_BUILD
Builds everything at once

  * create_index
creates index for specific directory and save it to file.
It prompts directory path to index and save path for it.

  * server
to queries created index and reads files.
must be used only after index was created 
while running it can handle requests from running "client" using server ip address.

  * client
sends requests to server and receives search results

  * client_server
build both client and server targets at once

  * tests
index tests using Google Test framework

  * performance
time performance measurements using different number of threads to build index
