 _______  ______    ___   __    _  _______  _______  ______    _______  _______  _______
|  _    ||    _ |  |   | |  |  | ||       ||       ||    _ |  |   _   ||       ||       |
| |_|   ||   | ||  |   | |   |_| ||    ___||       ||   | ||  |  |_|  ||    ___||_     _|
|       ||   |_||_ |   | |       ||   |___ |       ||   |_||_ |       ||   |___   |   |
|  _   | |    __  ||   | |  _    ||    ___||      _||    __  ||       ||    ___|  |   |
| |_|   ||   |  | ||   | | | |   ||   |___ |     |_ |   |  | ||   _   ||   |      |   |
|_______||___|  |_||___| |_|  |__||_______||_______||___|  |_||__| |__||___|      |___|

================================================================================
Known Issues:
================================================================================

Physics are working almost perfectly. Every once and a while you can get an odd corner
case or move fast enough to get the game to lock inside of a while loop. This is
because I only had time to implement preventative physics, with little to no
corrective. 

If you put a block down inside yourself, the game will soft lock for the same
reasons as above (no corrective physics, only preventative).

================================================================================
How to Use:
================================================================================

Keyboard Controls:

Esc - Quit the Program
W - Fly Forward
A - Fly Left
S - Fly Backwards
D - Fly Right
Z - Fly Down
0-9 - Select a block to use
Spacebar - Fly Upward
Shift - Fly Faster
F5 - Change camera mode
P - Change physics mode

Debug:
A - Hold down to view current raytrace
L - Draw a debug point at current camera location
O - Clear all debug points