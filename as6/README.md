# AS5: Audio GUI

Authored by Marlo Ongkingco for CS381 (Game Engine Architecture)

## Instructions

In order to run this assignment:

1. In a terminal, move into a directory that you want to copy this assignment in.
2. Run the following command to clone this repository:

   > git clone https://github.com/Marlo-Ong/cs381-game-architecture.git
   >
3. Move into the as5 and fetch the raylib submodule:

   > cd cs381-game-architecture/as5

   > git submodule update --init --recursive

4. Create a build folder, move into it, and compile:

   > mkdir build

   > cd build

   > cmake ..

   > make

5. If it compiled successfully (this may take a bit), an executable should be created, which you can run with the following command:

   > ./as5

6. Once the executable is run, a window should open with an audio controls GUI. Use your keyboard to control the audio according to the following:

## Controls

Mouse:
- Click and hold on any slider, and:

- Drag your mouse to the right to increase the volume.

- Drag your mouse to the left to decrease the volume.


Keyboard:

- Press TAB to cycle through which slider to select.

- Press W/up arrow to increase the selected slider's volume.

- Press S/down arrow to decrease the selected slider's volume.

- Press apostrophe (') to toggle between light and dark modes.