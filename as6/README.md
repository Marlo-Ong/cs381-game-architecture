# AS6: Audio GUI

Authored by Marlo Ongkingco for CS381 (Game Engine Architecture)

## Instructions

In order to run this assignment:

1. In a terminal, move into a directory that you want to copy this assignment in.
2. Run the following command to clone this repository:

   > git clone https://github.com/Marlo-Ong/cs381-game-architecture.git
   >
3. Move into the as6 and fetch the raylib submodule:

   > cd cs381-game-architecture/as6

   > git submodule update --init --recursive

4. Create a build folder, move into it, and compile:

   > mkdir build

   > cd build

   > cmake ..

   > make

5. If it compiled successfully (this may take a bit), an executable should be created, which you can run with the following command:

   > ./as6

6. Once the executable is run, a window should open, and there should be a plane object atop a grassy ground and a sky. Use your keyboard to control the plane according to the following:

## Controls

- (WASD or arrow key format)

- Press TAB to cycle through selectable planes.

- Press W/up arrow to increase the selected plane's velocity.

- Press S/down arrow to decrease the selected plane's velocity.

- Press A/left arrow to turn the plane counter-clockwise.

- Press D/right arrow to turn the plane clockwise.

- Press Q to move the selected plane upward.

- Press E to move the selected plane downward.

- Press SPACE to stop the selected plane.