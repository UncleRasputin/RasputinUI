# RasputinUI
A basic UI implementation for olcPixelGameEngine

*** UNDER CONSTRUCTION ***

-- UI MANAGER PART GOES HERE --


Each Control has a ControlTheme.
Each ControlTheme contains 4 ControlStyle-s: Default, Disabled, Hover, Active.

Style Properties
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
