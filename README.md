# RasputinUI
A basic UI implementation for olcPixelGameEngine.

Requires olcPixelGameEngine.h  (https://github.com/OneLoneCoder/olcPixelGameEngine)

--------------------------------------------------

Totally free UI from Rasputin (aka UncleTime)

License Type: Relief

Please take this code and do not return it, it's site offends me and torments my soul.
You may do whatever you want with it, including remove this comment if you feel the need to.
If you make changes or derive from it to add functionality, consider sharing it as well.

--------------------------------------------------

*** UNDER CONSTRUCTION ***

You only need RasputinUI.h.

Example.cpp contains an example using the UI system.

-----------------------------------------------

RasputinUI::UIManager

Create a UIManager PGEX in OnUserCreate of your engine to initialize the UI.

You can call either CreateControl to create and add a new control to the UI, or you can call AddControl to add an already created control to the UI.

Create your controls in OnUserCreate (they can be created and modified at any time).

-----------------------------------------------

RasputinUI::ControlTheme and RasputinUI::ControlStyle

Each Control has a ControlTheme.

Each ControlTheme contains 4 ControlStyles: Default, Disabled, Hover, Active.

Each Style has the following properties:

 olc::Pixel	ForegroundColor;
 
 FullAlignment 	TextAlign;
 
 olc::vf2d 	TextScale;
 
 Spacing 	Padding;
 
 UIBackground *Background; 
 
 Enums::BorderType BorderType;

The control will theme based on its state at render time each frame.
If you want to use default for any state other than default, you can:
 1. use Default.DeepCopy() to make a full copy
 2. set .empty of the style to true, which will make it fall back to default
 3. define an entirely new style for this state
