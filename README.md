# irc-bot
Simple IRC bot written in C for Windows

This was originally coded for UNIX and was converted to use Windows libraries.

## To compile this on windows, you must enter the VS Tools command prompt. 

To do so, navigate to C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools and type "vsdevcmd".

Now navigate to the folder where the project was cloned and compile with the command "cl irc.c win.c main.c"

Run with the command "irc.exe" for normal execution, or "irc.exe -v" for verbose execution. 
