# FinalYearProject #
Prescribed Virtual Reality: An examination of the potential of VR applications in healthcare.
Game Title: Off The Shelf: On The Move 

## 36. Audio Assets ##

	- Add Audio files to project.

## 35. Sound Cue Implementation ##

	- Add two delegates to Game mode class.
	- Add Sound Cue and Audio Component to BasePickupClass.
	- Add Sound Cue to ShopFloor_P level blueprint.

## 34. Import Assets ##

	- Import Assets. Create corresponding blueprints.
	- Work on Level Design.

## 33. Stress Test ##

	- Stress game with high volume of assets.

## 32. Menu Fixes ##

	- Menus now disable when not used.
	- Clean up commented out code.
	
## 31. Pause Menu ##

	- Create WBP_PauseMenu.
	- Create BP_PauseMenuWidget.
	- Implement Pause menu. (Resume, return to mode select).

## 30. Objective Item List ##

	- Add item list generation implementation to VRGameMode class.
	
## 29. Teleport Cursor Spline Arc Implementation ##

	- Add USplineComponent implementation to VRController class.
	- Add M_SplineArcMat asset from VR Template.

## 28. Teleport Cursor visuals improvement ##

	- Add BeamMesh asset from VR Template.
	- Rename BeamMesh to SM_BeamMesh.
	- Add M_ArcEndpoint asset from VR Template.
	- Add M_TeleportPreviews asset from VR Template.
	- Add MI_TeleportCylinderPreview asset from VR Template.
	- Add MF_OccludedPixels asset from VR Template.
	- Add TeleportMCP asset from VR Template.
	- Apply MI_TeleportCylinderPreview to VRTeleportCursor static mesh.
	- Destroy Left controller's spawned teleport cursor.
	- Set TeleportCursor visibility to false on spawn.
	- Fix teleport feature being used when movement should be disabled.

## 27. Overlapping fixes ##

	- Remove BP_VRShoppingBasket from level.
	- Add Box Component to VRCharacter.
	- Setup Box Component to overlap with BasePickup objects and MotionControllers.
	- BasePickup overlap implementation fixes.

## 26. BasePickup Reset Implementation ##

	- Set up delegate in VRGameMode to broadcast when a game reset occurs.
	- Add implementation to BasePickup class to reset it to its original state when a game reset event is fired.

## 25. Shop list widget event binding ##

	- Further testing event binding for updated item list delegate.
	- Fix sizing of list on BP_Display.
	- Resize Static mesh for shopping basket blueprint.

## 24. Shop list widget event binding ##

	- Testing event binding for updated item list delegate.

## 24. Shopping Item Interactions ##

	- Overlap events implemented when pick up objects enter box collision of shopping basket.
	- Create Particle System (PS_ShoppingItem).
	- Add UParticleSystem to Base Pickup class.
	- Create delegate in VRGameMode class to broadcast updated shopping item list.

## 23. Shopping Basket Creation ##

	- Create Static mesh shopping basket from BSP geometry.
	- Create Collision for shopping basket.
	- Create VRShoppingBasket class.
	- Add Static mesh component and box component (trigger volume).
	- Create blueprint child of VRShoppingBasket, BP_VRShoppingBasket.

## 22. FYP Title Change ##

	- Update Project name.
	- Add game title.

## 21. Mode Select Interactions and Implementation ##

	- Update Menu Select Menu Widget.
	- Add UWidgetInteractionComponent to VRCharacter.
	- Click events implemented using widgetinteractioncomponent and mode select widget.
	- Menu select -> Timed Mode -> Mode Reset -> Menu Select implemented.
	- Fix formatting of displayed time remaining.

## 20. UI Adjustments ##

	- Create duplicate instances of BP_Display.
	- Place BP_Displays across ShopFloor_P.

## 19. Finish Camera Fade Implementation on End of Round Timer ##

	- Introduce fade delay timer.
	- Finish camera fade implementation for round finished.
	- Add socket, "menu_pos", to Skel_Hand_Skeleton.

## 18. Implementing game modes ##

	- Introduce ModeReset to EGameMode.
	- Implement timer to switch from a timed mode to mode reset in VRGameMode.
	- Implement temporary camera fade for resetting player position. 
	- Implement disabling and enabling player movement from game mode.

## 17. Timer Display ##

	- Create blueprint, BP_Display; for displaying text-based information in game, e.g. remaining time in round.
	- Update textfield string with current remaining time for round.
	- Implement blueprint callable function in VRGameMode to access remaining time from an active timer.

## 16. Menu System ##

	- Create widget blueprint, WBP_ModeSelectMenu; Contains menu select UI.
	- Create Blueprint, BP_ModeSelectWidget; Actor storing widget component for menu select.
	- Add enum, EGameMode, to VRGameMode.h for mode selection.
	- Implement accessor and mutator functions in VRGameMode for EGameMode variable.
	- Add implementation to VRCharacter to spawn Menu select widget on MenuSelect mode.

## 15. Clean up Code ##

	- Remove unused, commented out code that has been refactored.

## 14. Update Teleportation Code ##

	- Fix issue where cancelling teleport didn't stop the cursor from updating and blocked any further teleport input.
	- Create Material, "M_Floor".
	- Create Physics Material, "PM_Floor".
	- Add Surface Type, "Floor", to project.
	- Apply surface type, "Floor" to PM_Floor.
	- Apply PM_Floor to M_Floor.
	- Apply M_Floor to Floor geometry in ShopFloor_P.
	- Update Line trace to check physical material's surface type on a successful hit.
	- Floor Geometry is now the only valid Teleport surface.
	- Remap Teleport input to right motion controller "face buttons".

## 13. Update Motion Controllers ##

	- Update Motion Controllers to use VR Hand skeletal mesh.
	- Update teleportCursor linetrace; Now StartPos location is gotten from mesh socket, "teleport_start_pos".
	- Fix Left Motion Controller Grab.
	- Set editor start up level to ShopFloor_P.
	- Fix socket location for grabbed objects.
	- Readjust Meshes relative location in BP_VRController.
	- Boolean flags in VRCharacter, storing whether controller grips are pressed, are renamed so names are more meaningful.
	- Fix initial "velocity jump" when motion controller grip is pressed.

## 12. Add VRGameMode class files and Timer for timed game mode ##
	
	- Create VRGameMode and blueprint child class BP_VRGameMode.
	- Begin implementation of Timer for timed game modes.
	- Replace BP_Blueprint in ShopFloor level with PlayerStart.
	- Set GameMode to BP_VRGameMode.

## 11. Code refactoring and README reordering ##

	- Reorder entries in README so newest entries appear at the top of the file.
	- Refactor TeleportCursor related code in VRCharacter and move it into VRController (Needs to be tested in VR)
	- Update current BP_BasePickup static mesh with simplified collision sphere

## 10. Import VR Hand Assets and Begin work on animation ##

	- Import assets from 'Taiku Prototype VR Hands'
	- Create Animation blueprint to handle VR hand animations.
	- Implement state machine in animation blueprint, "Hand State". Containing states; Open (Idle), Point (for teleporting) and Fist (for grabbing objects).
	- Add boolean flags to VRController class for adjusting animation states. (NOTE: refactor code so teleport flag for each controller can be updated independent of each other)

## 09. Prototype Object interaction implemented ##

	- BasePickup class created.
	- BP class derived from base pickup
	- Grabbing/dropping pickups with motion controllers implemented (via trigger inputs)

## 08. ShopFloor level blocked out ##

	- Initial block out of ShopFloor_P level.

## 07. Incorporate Motion Controllers and HMD ##

	- Fixed bug where calling AVRCharacter::StopTeleport while an invalid teleport location is currently selected didn't cancel the teleport and return the teleport state back to 'Wait'.
	- Motion Controllers now appear in vr preview and transforms are tracked accurately.
	- 'Arm Swinger' movement implemented using vive grip buttons as flag activators.
	- implemented traditional gamepad style movement with motion controller. Mapped directional movement to the corresponding face buttons on the left hand motion controller.
	- Set up teleport implementation using motion controller. If hmd is enabled, linetrace to determine teleport location will be calculated using the right motion controller instead of the player's camera.
	- Stop HUD class from drawing crosshair.

## 06. Further work on Teleport locomotion including fade screen ##

	- Create level for testing locomotion mechanics (TestingGround_P)
	- Populated TestingGround_P with various geometry volumes of different sizes and colours.
	- Create BP_TeleportCursor
	- Update VRTeleportCursor.cpp so the cursor mesh no longer generates a shadow.
	- Implemented Teleport with fade out/in, in VRCharacter.cpp

## 05. Further work on teleport locomotion & VR Controller class ##

	- Add Class to incorporate motion controller. (.cpp currently empty)
	- Updated teleportation with visual cue and 2-step (aim, cancel or verify) implementation
	- Created new Directory for Meshes.
	- Created new level for testing (TestingGrounds_P) - ERROR: level file was never saved.
	- Added Comments.

## 04. Add Implementation of gesture based movement ##

	- Add implementation for capturing motion controller movement and direction.
	- Using this information to calculate player movement.
	- Added Comments.

## 03. Comment Code and Minor Code Refactors ##

	- Comment code for better code interpretation and improved readability
	- Refactored variable names for improved readability
	- Removed unused "boiler-plate" code used for enabling touchscreen input
	- Reformatted README file layout

## 02. Set up Project with Template and Prototypes ##

	- Add FPS Template for boilerplate code
	- Implement first pass teleport locomotion
	- Implement first pass Gesture locomotion
	- Implement first pass Gamepad locomotion

## 01. Create Project ##