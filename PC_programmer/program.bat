SET "output=%1"
SETLOCAL EnableDelayedExpansion
SET output=!output:hex=bin!
ENDLOCAL & SET "output=%output%"

hex2bin -l 10000 %1
8052term.exe %output% -ser %2