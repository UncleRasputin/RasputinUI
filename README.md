# RasputinUI
A basic UI implementation for olcPixelGameEngine.

Requires olcPixelGameEngine.h  (https://github.com/OneLoneCoder/olcPixelGameEngine)

*** UNDER CONSTRUCTION ***

You only need RasputinUI.h.

Example.cpp and the button*.png files are an example using the UI system.

-----------------------------------------------

RasputinUI::UIManager

Create a UI Manager in OnUserCreate of your engine to initialize the UI.

You can call either CreateControl to create and add a new control to the UI, or you can call AddControl to add an already created control to the UI.

Create your controls in OnUserCreate (they can be created and modified at any time).

In OnUserUpdate, make sure to call UpdateUI: uiManager->UpdateUI(fElapsedTime);

-----------------------------------------------

RasputinUI::ControlTheme and RasputinUI::ControlStyle

Each Control has a ControlTheme.

Each ControlTheme contains 4 ControlStyles: Default, Disabled, Hover, Active.

Each Style has the following properties:
 olc::Pixel	ForegroundColor;
 olc::Pixel 	BackgroundColor;
 FullAlignment 	TextAlign;
 olc::vf2d 	TextScale;
 Spacing 	Padding;
 olc::Decal *	BackgroundDecal;
 FullAlignment 	BackgroundDecalAlign;
 olc::vf2d 	BackgroundDecalScale;

The control will theme based on its state at render time each frame.
If you want to use default for any state other than default, you can:
 1. use Default.Dupe() to make a full copy
 2. set .empty of the style to true, which will make it fall back to default
 3. define an entirely new style for this state
