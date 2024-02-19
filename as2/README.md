# AS2: Plane Takeoff!

Authored by Marlo Ongkingco for CS381 (Game Engine Architecture)

## Instructions

In order to run this assignment:

1. In a terminal, move into a directory that you want to copy this assignment in.
2. Run the following command to clone this repository:

   > git clone https://github.com/Marlo-Ong/cs381-game-architecture.git
   >
3. Move into the as2 and fetch the raylib submodule:

   > cd cs381-game-architecture/as2

   > git submodule update --init --recursive

4. Create a build folder, move into it, and compile:

   > mkdir build

   > cd build

   > cmake ..

   > make

5. If it compiled successfully (this may take a bit), an executable should be created, which you can run with the following command:

   > ./as2

6. Once the executable is run, a window should open, and there should be a plane object atop a grassy ground and a sky. Use your keyboard to control the plane according to the following:

## Controls

- (WASD or arrow key format)

- Press W/up arrow to move the plane away from the camera.

- Press A/left arrow to move the plane to the left.

- Press S/down arrow to move the plane toward the camera.

- Press D/right arrow to move to plane to the right.

- Press Q to move the plane upward.

- Press E to move the plane downward.

- Press SPACE to stop the plane's velocity.

## Assignment Requirement

> 5. Can we simplify the problem by remapping camera directions to specific world axes (e.g. W moves along say -Z)? Why or why not can we use this simplification in this assignment?

This simplification can be made in this assignment; right now, the W key is already moving the plane along the +Z axis, since that is the axis defined by raylib's Vector3::Forward() function. However, that Vector3 can easily be substituted with Back() to move it along the -Z axis, or Left(), or Up(), or even our own custom axis by constructing a Vector3 such as {-1, 1, 2), as long as we increment the plane's velocity by this direction.
