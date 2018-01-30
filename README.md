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
