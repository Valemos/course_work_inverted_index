Project implements index for quick search in directories with text files

CMake is required for build

to create index for specific directory and save it to file, run cmake built executable "create_index".
It prompts directory path to index and path for index to be saved.

to query index and read files run "server". 
while running it can handle requests from running "client" using server ip address.
