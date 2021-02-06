	/* Flasher : a custom control that just pulses based on the update clock.  It alpha blends the background channel over time with the clock.*/
	/* Mostly intended for composite and custom controls, for a cursor indicator for example*/
	class Flasher : public ControlBase
	{
	private:
		/* Alpha: Current alpha value for the flasher */
		int Alpha = 0;

		/* Factor: Current direction the alpha channel is moving */
		int Factor = 1;

	public:
		/* Constructor: normal ControlBase, location and parent */
		Flasher(Rect location, ControlBase* parent = NULL)
			: ControlBase(location, parent)
		{
			Theme.Default.ForegroundColor = olc::WHITE;
			Theme.Default.BackgroundColor = olc::GREY;
			Location = location;
			Parent = parent;
			if (Parent != NULL)
			{
				Parent->Controls.push_back(this);
			}
		}
		
		/* Pace: The speed at which the pulse occurs.  Pace * fElapsedTime is added or subtracted from the alpha of the background until it reaches the limit, then changes direction */
		int Pace = 300;

		/* DrawBackground: override the parent function so that we can make it pulse */
		void DrawBackground(olc::PixelGameEngine *engine, float fElapsedTime) override 
		{
			int amt = (int)(Pace * fElapsedTime);
			int a = Alpha + (amt * Factor);
			if (a > 255)
			{
				a = 255;
				Factor *= -1;
			}
			else if (a < 0)
			{
				a = 0;
				Factor *= -1;
			}
			Alpha = a;
			engine->FillRectDecal(ScreenPos(), Location.Size, olc::Pixel( Theme.Default.BackgroundColor.r, Theme.Default.BackgroundColor.g, Theme.Default.BackgroundColor.b, Alpha) );
		}
	};
