# FinalYearProject
Standards In Player Movement In Virtual Reality Games

## 01. Create Project ##

## 02. Set up Project with Template and Prototypes ##

	- Add FPS Template for boilerplate code
	- Implement first pass teleport locomotion
	- Implement first pass Gesture locomotion
	- Implement first pass Gamepad locomotion

## 03. Comment Code and Minor Code Refactors ##

	- Comment code for better code interpretation and improved readability
	- Refactored variable names for improved readability
	- Removed unused "boiler-plate" code used for enabling touchscreen input
	- Reformatted README file layout

## 04. Add Implementation of gesture based movement ##

	- Add implementation for capturing motion controller movement and direction.
	- Using this information to calculate player movement.
	- Added Comments.

## 05. Further work on teleport locomotion & VR Controller class ##

	- Add Class to incorporate motion controller. (.cpp currently empty)
	- Updated teleportation with visual cue and 2-step (aim, cancel or verify) implementation
	- Created new Directory for Meshes.
	- Created new level for testing (TestingGrounds_P) - ERROR: level file was never saved.
	- Added Comments.

## 06. Further work on Teleport locomotion including fade screen ##

	- Create level for testing locomotion mechanics (TestingGround_P)
	- Populated TestingGround_P with various geometry volumes of different sizes and colours.
	- Create BP_TeleportCursor
	- Update VRTeleportCursor.cpp so the cursor mesh no longer generates a shadow.
	- Implemented Teleport with fade out/in, in VRCharacter.cpp

## 07. Incorporate Motion Controllers and HMD ##

	- Fixed bug where calling AVRCharacter::StopTeleport while an invalid teleport location is currently selected didn't cancel the teleport and return the teleport state back to 'Wait'.
	- Motion Controllers now appear in vr preview and transforms are tracked accurately.
	- 'Arm Swinger' movement implemented using vive grip buttons as flag activators.
	- implemented traditional gamepad style movement with motion controller. Mapped directional movement to the corresponding face buttons on the left hand motion controller.
	- Set up teleport implementation using motion controller. If hmd is enabled, linetrace to determine teleport location will be calculated using the right motion controller instead of the player's camera.
	- Stop HUD class from drawing crosshair.

## 08. ShopFloor level blocked out ##

	- Initial block out of ShopFloor_P level.

## 09. Prototype Object interaction implemented ##

	- BasePickup class created.
	- BP class derived from base pickup
	- Grabbing/dropping pickups with motion controllers implemented (via trigger inputs)

## 10. Import VR Hand Assets and Begin work on animation ##

	- Import assets from 'Taiku Prototype VR Hands'
	- Create Animation blueprint to handle VR hand animations.
	- Implement state machine in animation blueprint, "Hand State". Containing states; Open (Idle), Point (for teleporting) and Fist (for grabbing objects).
	- Add boolean flags to VRController class for adjusting animation states. (NOTE: refactor code so teleport flag for each controller can be updated independent of each other)
