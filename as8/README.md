# AS8: Data Oriented Entity Component Ships and Planes

Authored by Marlo Ongkingco for CS381 (Game Engine Architecture)

## Instructions

In order to run this assignment:

1. In a terminal, move into a directory that you want to copy this assignment in.
2. Run the following command to clone this repository:

   > git clone https://github.com/Marlo-Ong/cs381-game-architecture.git
   >
3. Move into the as8 and fetch the raylib submodule:

   > cd cs381-game-architecture/as8

   > git submodule update --init --recursive

4. Create a build folder, move into it, and compile:

   > mkdir build

   > cd build

   > cmake ..

   > make

5. If it compiled successfully (this may take a bit), an executable should be created, which you can run with the following command:

   > ./as8

6. Once the executable is run, a window should open, and there should be 5 ships and 5 planes atop an ocean and a sky. Use your keyboard to control the plane according to the following:

## Controls

- (WASD or arrow key format)

- Press TAB to cycle through selectable vehicles (cycles through boats, then planes in order).

- Press W/up arrow to increase the selected vehicle's velocity.

- Press S/down arrow to decrease the selected vehicle's velocity.

- Press A/left arrow to increase the selected vehicle's heading.

- Press D/right arrow to decrease the selected vehicle's heading.

- If a plane is selected, press Q to move it upward.

- If a plane is selected, press E to move it downward.

- Press SPACE to smoothly stop the selected vehicle.