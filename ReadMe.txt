 _______  ___      _______  __   __  ______   __   __  _______  ______    _______  _______  _______
|       ||   |    |       ||  | |  ||      | |  | |  ||       ||    _ |  |   _   ||       ||       |
|       ||   |    |   _   ||  | |  ||  _    ||  |_|  ||       ||   | ||  |  |_|  ||    ___||_     _|
|       ||   |    |  | |  ||  |_|  || | |   ||       ||       ||   |_||_ |       ||   |___   |   |
|      _||   |___ |  |_|  ||       || |_|   ||_     _||      _||    __  ||       ||    ___|  |   |
|     |_ |       ||       ||       ||       |  |   |  |     |_ |   |  | ||   _   ||   |      |   |
|_______||_______||_______||_______||______|   |___|  |_______||___|  |_||__| |__||___|      |___|

================================================================================
Introduction:
================================================================================

Thank you very much for trying out my demo of CloudyCraft! This is my personal
recreation of the core functionality in Minecraft, with a few extras not found
in the game (such as a real-time alternate dimension portals, colored lighting, and
functional stained glass). The game was built from the ground up using my own 
homemade engine, written in C++ and using OpenGL for rendering.

If you're here to try out the real-time portals, here's a quick primer on how to 
get started. While holding the middle mouse button, right click on a surface to create
a portal. Since this has just been for debugging and has no gameplay meaning yet, it's 
a bit unweildy, but it works fine. There are a few physics bugs associated with them,
especially on the edges, but they're a lot of fun already! You can technically close the portal 
in the same way as opening one, but you have to select the same block face, 
meaning you need to build up to cover up the portal in the other dimension and then right click.

Please note that I've been taking more classes while working on this demo, so
the code is a little messy (with some in-class demos added in the renderer that I
haven't had time yet to clean up, etc.). Also, as this is a work in progress,
there are a few bugs and leaks that I haven't been able to take care of yet. 
If you find anything, I'd love to hear about it so that I can fix it for the 
next release.

If you have any questions about the game, any trouble running it, or find any
bugs, please feel free to shoot me an email at acloudy@smu.edu

Thank you very much for taking the time to play CloudyCraft! I hope you enjoy
playing it as much as I enjoyed creating it!

================================================================================
Installation / Running the game:
================================================================================
If you have Visual Studio 2015 installed, go ahead and skip to step 2.

1. Install the Visual Studio 2015 redistributable if you don't already have it.
It's located under CloudyCraft\Visual Studio 2015 Runtime\vc_redist.x86.exe

2. Navigate to CloudyCraft\Game Code\SimpleMiner\Run_Win32\

3. Read the "How to Use" section below for the controls.

4. Run "SimpleMiner.exe"

5. Enjoy!

================================================================================
How to Use:
================================================================================

Keyboard Controls:
W - Forward
A - Left
S - Backwards
D - Right
Z - Fly Downward
Spacebar - Jump / Fly Upward
Left Click - Dig Block
Right Click - Place Block
Middle Click + Right Click - Place portal
Scroll Wheel - Move through block selection list.
0-9 - Select a block to use
U - Swap Dimensions (Check out the sky dimension and the earth dimension!)
Shift - Fly Faster
F5 - Change camera mode
  ~ First-Person: Default camera mode. Shows the world through the player's eyes.
  ~ Third-Person: Move the camera while keeping it centered on the player.
      Opaque textures may not render correctly due to the offset camera.
  ~ Fixed-Angle Tracking: Camera is set in a fixed position. Turning on debug
      rendering will show the line of sight of the player. In this mode, you
      can observe view-frustum culling as well as the opacity issues that
      are solved for the first person perspective.
V - Change physics mode
  ~ NoClip: Default physics mode. Pass through all collision and fly freely
  ~ Flying: Collisions are enabled, but gravity is not. Can fly freely in open air.
  ~ Normal: All physics enabled. Walking, with running and jumping included.
Esc - Quit the Program

Debug Commands:
B - Toggle all debug rendering
` - Opens the console. Currently broken in favor of work in another class.

================================================================================
Known Issues:
================================================================================

There may be some edge cases I haven't accounted for, specifically at the top
and bottom of the world. You might be able to crash the game or get unexpected results
whenever you manipulate blocks in that area or fly too high up or down. This is just
something I haven't had the time to address, but I know what I need to do!

The game's decisions on which chunks to render works well at higher framerates,
but might not pick the correct ones when in debug or when the frame drops. As a
result, you may end up with a "missing" chunk, or fragmented chunk loading when
moving fast. The data for these chunks have been loaded into memory, so you're
able to interact with the blocks and force them to build their vertex arrays.

You can put blocks down inside yourself. Fortunately, you can dig yourself back
out. If you get stuck doing anything in the game, just use the V key to change 
into noclip mode and free yourself. :)

Depending on the framerate, you can have various issues crop up, such as
physics errors (getting stuck inside blocks).

There are also a few lingering Physics bugs, especially if the framerate gets
below 60 fps.

================================================================================
Special Thanks:
================================================================================

Squirrel Eiserloh - For putting up with me in class and being an amazing professor.
I could not have made it this far without your mentorship and guidance.

Chrisopher Forseth - For letting me attend your directed focus study, even though
I wasn't even enrolled in it. Thank you for putting up with all of my dank memes in
class, and for taking time out of your weekends to meet with me while I was off in 
Seattle.

Ken Harward - For letting me take a directed focus study with you earlier than
intended, and putting up with me for about an hour at the start of each day
in class.

Patrick Owens - For helping me design a new "universal light" block and for your
mentorship.

Ryan See - For always helping me out whenever I need audio, and for the world swapping SFX

Hayden Huffines - For staying after class with me for 5 hours and helping me
find and fix that one corner case in the physics.

You - For playing this game. Thank you for your time!
