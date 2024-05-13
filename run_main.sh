make
clang -Wall -pedantic -std=gnu99 -gdwarf-4 prgsem.c computation.o event_queue.o gui.o main.o messages.o prg_io_nonblock.o utils.o xwin_sdl.o -o main -lSDL2 -lm
IS_OK=`echo $?`

if [[ "$IS_OK" -eq 0 ]]
then
    #valgrind --track-origins=yes --leak-check=full ./main
    ./main
fi