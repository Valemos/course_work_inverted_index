Project implements index for quick search in directories with text files

CMake is required for build

to create index for specific directory and save it to file, run cmake built executable "create_index".
It prompts directory path to index and save path for it.

to query index and read files run "server". 
while running it can handle requests from running "client" using server ip address.

tests can be run using executable "tests"

time performance measurements using different number of threads to build index
can be run using "performance" executable
