make
clang -Wall -pedantic -gdwarf-4 module.c computation.o computation_module.o event_queue.o keyboard_module.o main_module.o messages.o prg_io_nonblock.o utils.o -o module
IS_OK=`echo $?`

if [[ "$IS_OK" -eq 0 ]]
then
    #valgrind --track-origins=yes --leak-check=full ./module
    ./module
fi