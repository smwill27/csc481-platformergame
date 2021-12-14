Location: Solution is CSC481HW4, projects are CSC481HW4Server and CSC481HW4Client

Steps for Compilation and Execution:
1. Load CSC481HW4.sln in Visual Studio 2019.
2. In the toolbar at the top, set the two dropdowns to the left of the green arrow (Solution Configurations
   and Solution Platforms) to the values Release and Win32.
3. Go into both project's properties and make sure that the configuration options are set to match those in the
   toolbar (Release and Win32).
4. In both project's properties, go to the General tab under the C/C++ tab and in the Additional Include Directories
   field add the path to your <libzmq-v141-4_3_2> directory and the path <sfml-install-path>\include.
5. In both project's properties, go to the General tab under the Linker tab and in the Additional Library
   Directories field add the path to your <libzmq-v141-4_3_2> directory and the path <sfml-install-path>\lib.
6. In both project's properties, go to the Input tab under the Linker tab and in the Additional Dependencies field
   add the dependencies libzmq-v141-mt-4_3_2.lib, sfml-graphics.lib, sfml-window.lib, and sfml-system.lib.
7. In CSC481HW4Client's properties, go to the General tab under the C/C++ tab and in the Additional Include
   Directories add the path to the CSC481HW4Server folder.
8. Right-click on the solution CSC481HW4 in in the solution explorer and select Build Solution or Rebuild Solution.
9. After the projects have finished building, right-click on the project CSC481HW4Server in the solution explorer
and select Debug -> Start New Instance to start the server.
10. To run clients, right-click on the project CSC481HW4Client in the solution explorer
    and select Debug -> Start New Instance for each client. Each client will open an 800x600 window with a
    blue background, a set of still and moving platforms, and a character. Each client will see their own
    character colored magenta and the characters of all other clients colored red. A client's character may
    not be visible if the characters of other clients share its position.
11. Closing a window will disconnect a client.

Handling Character Input:
-To move the character to the left, press or hold down the left arrow key.
-To move the character to the right, press or hold down the right arrow key.
-To make the character jump, press the up arrow key. The left and right arrow keys can be used to make the
 character move to the left and right while they are jumping up into the air or falling down. The character
 cannot jump while currently jumping up into the air or falling down.

Collision:
-The character should collide with both the static and moving platforms.
-The character should be moved by the moving platforms, either being carried with it if standing on top of it
or being pushed out of the way if not standing on top of it.
-When a moving platform pushes a character, there is a chance that the character may become temporarily stuck
in the moving platform. This is because the moving platform cannot push the character any more without
causing them to become stuck in another object such as a static platform. I improved the layout of the game
somewhat to minimize this occurrence, but it can still happen in a few places. If it does, the character
should eventually be able to move again once the moving platform has moved enough to allow the character
to stop colliding with it.
-If the character falls past the bottom edge of the screen, they will collide with a death zone and be
respawned at their starting position.

Replays:
-To start recording a replay, press the R key. Only one replay can be recorded at a time across all clients.
-While a replay is being recorded, press either the 1, 2, or 3 key to stop the recording. The replay will
immediately start to play after the recording ends. The initial speed of the replay will correspond to
the button used to stop the recording: 1 = half speed, 2 = normal speed, 3 = double speed.
-The replay will be played on all clients. No clients will be able to move while the replay is playing.
-While a replay is playing, press the 1 key to switch to half speed, the 2 key to switch to normal speed,
and the 3 key to switch to double speed. There may be a slight pause in the replay when the speed is switched.
-Once the replay completes, the state of all clients should match that before the replay was played and all
clients will be able to move again.

Quitting the Program:
-To disconnect a client, close the window by hitting the red x in the upper right corner.
-To quit all execution, click the stop button in the debug menu.

Code Sources Consulted:
-SFML documentation
-ZeroMQ guide/documentation and examples
-C++ standard library documentation