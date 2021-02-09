/*
Totally free UI from Rasputin (aka UncleTime)

License Type: Relief

Please take this code and do not return it, it's site offends me and torments my soul.
You may do whatever you want with it, including remove this comment if you feel the need to.
If you make changes or derive from it to add functionality, consider sharing it as well.

Requires olcPixelGameEngine.h
*/

#ifndef RASPUTIN_UI_DEF
#define RASPUTIN_UI_DEF
#include "olcPixelGameEngine.h"

namespace RasputinUI
{
	// Forward declares to handle dependencies
	class ControlBase;

	/* Class: EventHandler   Handles subscription and dispatch of events that take only the initiating control as a parameter */
	class EventHandler
	{
	private:
		std::list<std::function<void(ControlBase*)>> subscribers;
	public:
		void Subscribe(std::function<void(ControlBase*)> callback)
		{
			subscribers.push_back(callback);
		}

		void Invoke(ControlBase* sender)
		{
			for (auto sub : subscribers)
			{
				try
				{
					sub(sender);
				}
				catch (...) {}  // this sucks, as it will hide that an event isnt working to the consumer, but it also keeps one event handler from breaking the world
			}
		}
	};

	/* Class: EventHandler   Handles subscription and dispatch of events that take the initiating control and one additional parameter.
	   defined by template class T */
	template <class T>
	class EventHandler1Param
	{
	private:
		std::list<std::function<void(ControlBase*, T)>> subscribers;
	public:
		void Subscribe(std::function<void(ControlBase*, T)> callback)
		{
			subscribers.push_back(callback);
		}
		void Invoke(ControlBase* sender, T val = NULL)
		{
			for (auto sub : subscribers)
			{
				try
				{
					sub(sender, val);
				}
				catch (...) {}
			}
		}
	};

	/* Rectangle class, not strictly needed, but makes some calculations more convenient*/
	struct Rect
	{
		olc::vi2d Position;
		olc::vi2d Size;

		bool Contains(olc::vi2d point)
		{
			return (point.x >= Position.x && point.x < (Position.x + Size.x) && point.y >= Position.y && point.y < (Position.y + Size.y));
		}

		bool ContainsRect(Rect otherRect)
		{
			return Contains(otherRect.Position) && Contains({ otherRect.right(),otherRect.Position.y }) && Contains({ otherRect.Position.x,otherRect.bottom() }) && Contains({ otherRect.right(),otherRect.bottom() });
		}

		int32_t right()
		{
			return (Position.x + Size.x);
		}

		int32_t bottom()
		{
			return (Position.y + Size.y);
		}

		Rect(olc::vi2d position, olc::vi2d size)
		{
			Position = position;
			Size = size;
		}
		Rect()
		{
			Position = { 0,0 };
			Size = { 0,0 };
		}
	};

	/* Spacing: a structure to define space around an object, like margins or padding.  4 values: top,right,bottom,left (starts at top and continues clockwise)*/
	struct Spacing
	{
		int top;
		int right;
		int bottom;
		int left;

		int vert() { return top + bottom; }
		int horiz() { return left + right; }
	};


	class Enums
	{
	public:
		/* BorderType: indicates what type of border to draw */
		enum BorderType { None, Raised, Sunken, Solid };

		/* Alignment: a basic Near/Center/Far alignment system */
		enum Alignment { Near, Far, Center };
	};

	/* FullAlignment: a structure containing both Horizontal and Vertical alignment */
	struct FullAlignment
	{
		Enums::Alignment Horizontal;
		Enums::Alignment Vertical;
	};

	/* UIBackground: an abstraction for the background of a control */
	class UIBackground
	{
	public:
		void virtual Render(olc::PixelGameEngine* pge, Rect area, float fElapsedTime) = 0;
	};

	/* ControlStyle: determines the appearance of a control */
	struct ControlStyle
	{
		/* empty: set this to true for the structure to be seen as NULL/Undefined */
		bool empty = false;
		/* ForegroundColor: the color of text on the control */
		olc::Pixel ForegroundColor = olc::BLANK;
		/* TextAlign: the horizontal and vertical alignment for text on the control */
		FullAlignment TextAlign = { Enums::Alignment::Center, Enums::Alignment::Center };

		/* TextScale: the scaling value for the text of the control */
		olc::vf2d TextScale = { 1,1 };

		/* Padding: the space the will be excluded from the client rectangle of the control, for alignment */
		Spacing Padding = { 0,0,0,0 };

		/* Background: the background for this control */
		UIBackground* Background = NULL;

		/* BorderType: The type of border on this control */
		Enums::BorderType BorderType= Enums::BorderType::None;

		/* Empty: a quick way to get an empty style */
		static ControlStyle Empty()
		{
			ControlStyle result;
			result.empty = true;
			return result;
		}

		/* DeepCopy: Copy the values for everything EXCEPT background, to allow backgrounds to be shared */
		ControlStyle DeepCopy()
		{
			ControlStyle result;
			result.empty = empty;
			result.ForegroundColor = ForegroundColor;
			result.TextAlign = TextAlign;
			result.TextScale = TextScale;
			result.Padding = Padding;
			result.Background = Background;
			result.BorderType = BorderType;
			return result;
		}
	};

	/* ControlTheme: allows a control to have varied ControlStyle based on it's state (Enabled, Default, Active, Hover) */
	struct ControlTheme
	{
		ControlStyle Disabled = ControlStyle::Empty();
		ControlStyle Default;
		ControlStyle Hover = ControlStyle::Empty();
		ControlStyle Active = ControlStyle::Empty();

		ControlStyle GetStyle(bool enabled, bool hover, bool active)
		{
			if (!enabled && !Disabled.empty)
				return Disabled;
			else if (active && !Active.empty)
				return Active;
			else if (hover && !Hover.empty)
				return Hover;

			return Default;
		}

		ControlTheme DeepCopy()
		{
			ControlTheme result;
			result.Disabled = Disabled.DeepCopy();
			result.Default = Default.DeepCopy();
			result.Hover = Hover.DeepCopy();
			result.Active = Active.DeepCopy();
			return result;
		}
	};

	/* Class: UI   Just a place for common static functions */
	class UI
	{
		public:
		/* OutputText: this handles text overrun of control boundries */
		static void OutputText(olc::PixelGameEngine* pge, olc::vi2d location, olc::vi2d size, std::string text, ControlStyle style)
		{
			olc::vi2d tSize = pge->GetTextSize(text);
			if (tSize.x <= size.x && tSize.y <= size.y)
			{
				pge->DrawStringDecal(location, text, style.ForegroundColor, style.TextScale);
				return;
			}
			olc::vi2d charSize = pge->GetTextSize("W"); // traditionally a wider character, just for a sample
			int chars_to_remove = ((tSize.x - size.x) / charSize.x) + 1;
			pge->DrawStringDecal(location, text.substr(0, text.length() - chars_to_remove), style.ForegroundColor, style.TextScale);
		}

		/* Draw Bevel: draw the specified bevel around the control */
		static void DrawBevel(olc::PixelGameEngine *pge, Enums::BorderType borderType, Rect area)
		{
			if (borderType == Enums::BorderType::None)
				return;
			olc::Pixel Highlight1 = olc::Pixel(255, 255, 255, 160);
			olc::Pixel Highlight2 = olc::Pixel(255, 255, 255, 80);
			olc::Pixel Shadow1 = olc::Pixel(0, 0, 0, 160);
			olc::Pixel Shadow2 = olc::Pixel(0, 0, 0, 80);

			olc::Pixel t1 = Highlight1;
			switch (borderType)
			{
			case Enums::BorderType::Sunken:
				// default, already set
				break;
			case Enums::BorderType::Raised:
				// swap the colors
				Highlight1 = Shadow1;
				Shadow1 = t1;
				t1 = Highlight2;
				Highlight2 = Shadow2;
				Shadow2 = t1;
				break;
			case Enums::BorderType::Solid:
				Highlight2 = Shadow2 = Shadow1;
				Highlight1 = Shadow1 = olc::BLANK; // so we only draw the single, outer border
				break;
			}
			olc::vf2d spos = area.Position;

			pge->FillRectDecal({ spos.x + 1,spos.y + 1 }, { (float)area.Size.x - 2,1 }, Shadow1);
			pge->FillRectDecal(spos, { (float)area.Size.x,1 }, Shadow2);

			pge->FillRectDecal({ spos.x + 1, (float)spos.y + (area.Size.y - 1) }, { (float)area.Size.x - 2,1 }, Highlight1);
			pge->FillRectDecal({ spos.x + 2, (float)spos.y + (area.Size.y - 2) }, { (float)area.Size.x - 4,1 }, Highlight2);

			pge->FillRectDecal({ (float)spos.x + (area.Size.x - 2), spos.y + 2 }, { 1,(float)area.Size.y - 3 }, Highlight2);
			pge->FillRectDecal({ (float)spos.x + (area.Size.x - 1), spos.y + 1 }, { 1,(float)area.Size.y - 1 }, Highlight1);

			pge->FillRectDecal({ spos.x, spos.y + 1 }, { 1,(float)area.Size.y - 1 }, Shadow2);
			pge->FillRectDecal({ spos.x + 1, spos.y + 2 }, { 1,(float)area.Size.y - 3 }, Shadow1);
		}

		/* AlignTextIn: A convenience function to calculate position within a rectangle given an alignment, size, and scale. */
		static olc::vf2d AlignTextIn(olc::PixelGameEngine* pge, std::string text, Rect destination, FullAlignment textAlign, olc::vf2d scale)
		{
			olc::vf2d spos = destination.Position;

			olc::vi2d tsize = pge->GetTextSize(text);
			tsize = { (int)(tsize.x * scale.x),(int)(tsize.y * scale.y) };

			if (textAlign.Horizontal == Enums::Alignment::Far)
				spos.x += destination.Size.x - tsize.x;
			else if (textAlign.Horizontal == Enums::Alignment::Center)
				spos.x += (destination.Size.x - tsize.x) / 2;

			if (textAlign.Vertical == Enums::Alignment::Far)
				spos.y = (float)(destination.bottom() - tsize.y);
			else if (textAlign.Vertical == Enums::Alignment::Center)
				spos.y += (destination.Size.y - tsize.y) / 2;

			return spos;
		}
	};
	   
	/* SolidBackground: creates a control background of a solid color */
	class SolidBackground : public UIBackground
	{
	private:
		SolidBackground() {}
	public:
		static SolidBackground* BLANK; // explicitly have a transparent one, as it is the default
		olc::Pixel Color = olc::BLANK;
		SolidBackground(olc::Pixel clr)
		{
			Color = clr;
		}
		void Render(olc::PixelGameEngine* pge, Rect area, float fElapsedTime) override
		{
			pge->FillRectDecal(area.Position, area.Size, Color);
		}
	};
	SolidBackground* SolidBackground::BLANK = new SolidBackground(olc::BLANK);

	/* DecalBackground: creates a control background with a single decal.  It can be scaled, tinted, and aligned */
	class DecalBackground : public UIBackground
	{
	private:
		DecalBackground();
	public:
		olc::Decal* oDecal;
		FullAlignment ImageAlignment;
		olc::vf2d ImageScale;
		olc::Pixel Tint = olc::WHITE; 

		DecalBackground(olc::Decal* decal, FullAlignment alignment = { Enums::Alignment::Center, Enums::Alignment::Center }, olc::vf2d scale = { 1,1 }, olc::Pixel tint = olc::WHITE)
		{
			oDecal = decal;
			ImageAlignment = alignment;
			ImageScale = scale;
			Tint = tint;
		}

		void Render(olc::PixelGameEngine* pge, Rect area, float fElapsedTime) override
		{
			olc::vf2d spos = area.Position;
			olc::vi2d dsize = { oDecal->sprite->width, oDecal->sprite->height };
			dsize *= ImageScale;
			if (ImageAlignment.Horizontal == Enums::Alignment::Far)
				spos.x = (float)(area.right() - dsize.x);
			else if (ImageAlignment.Horizontal == Enums::Alignment::Center)
				spos.x += (area.Size.x - dsize.x) / 2;

			if (ImageAlignment.Vertical == Enums::Alignment::Far)
				spos.y = (float)(area.bottom() - dsize.y);
			else if (ImageAlignment.Vertical == Enums::Alignment::Center)
				spos.y += (area.Size.y - dsize.y) / 2;

			pge->DrawDecal(spos, oDecal, ImageScale, Tint);
		}
	};

	/* TiledDecalBackground: creates a control background that tiles a decal on the X and Y axis.  It can be scaled and tinted */
	class TiledDecalBackground : public UIBackground
	{
	private:
		TiledDecalBackground();
	public:
		olc::Decal* oDecal;
		olc::vf2d ImageScale;
		olc::Pixel Tint = olc::WHITE; 

		TiledDecalBackground(olc::Decal* decal, olc::vf2d scale = { 1,1 }, olc::Pixel tint = olc::WHITE)
		{
			oDecal = decal;
			ImageScale = scale;
			Tint = tint;
		}

		void Render(olc::PixelGameEngine* pge, Rect area, float fElapsedTime) override
		{
			olc::vi2d dsize = { oDecal->sprite->width, oDecal->sprite->height };
			dsize *= ImageScale;

			int cols = area.Size.x / dsize.x;
			int rows = area.Size.y / dsize.y;

			for (int y = 0; y < area.Size.y; y += dsize.y)
			{
				for (int x = 0; x < area.Size.x; x += dsize.x)
				{
					int w = dsize.x;
					int h = dsize.y;

					if (x + dsize.x > area.Size.x)
						w = (area.Size.x - x);
					if (y + dsize.y > area.Size.y)
						h = (area.Size.y - y);

					olc::vi2d loc1 = { area.Position.x + x,area.Position.y + y };
					olc::vi2d siz1 = { w,h };
					pge->DrawPartialDecal(loc1, oDecal, { 0,0 }, siz1, ImageScale, Tint);

				}
			}
		}
	};

	/* ControlBase: the base control for the UI system. 
		This can be used as a label, a panel, a button, an image, just about anything.
		You can also derive from it and create addition custom controls, as well as build composite controls.
	*/
	class ControlBase
	{
	protected:
		/* Location: The controls location and size, relative to its parent */
		Rect Location;

	public:
		/* PopToTop: use this function to grab the top level parent of this control and pop him to the front of z order */
		virtual void PopToTop() 
		{
			if (Parent && Parent->Parent == NULL)  
			{
				Parent->Controls.remove(this);
				Parent->Controls.push_back(this);
			}
			else if (Parent)
			{
				Parent->PopToTop();
			}
		}

		/* SetPosition: set the position of the control, relative to its parent */
		void SetPosition(olc::vi2d loc)
		{
			Location.Position = loc;
			OnMove.Invoke(this);
		}

		/* Position: get the position of the control, relative to its parent */
		olc::vi2d Position()
		{
			return Location.Position;
		}

		/* SetSize: set the size of the control */
		void SetSize(int w, int h)
		{
			Location.Size = { w, h };
			OnResize.Invoke(this);
		}

		/* Size: get the size of the control */
		olc::vi2d Size()
		{
			return Location.Size;
		}

		/* OnClick: Event handler, invoked when any (left,right,middle) mouse buttons are pressed while over this control, AND it was enabled. */
		EventHandler1Param<int> OnClick;
		/* OnMouseDown: Event handler, invoked when any (left,right,middle) mouse buttons enters the pressed state while over this control, AND it was enabled. */
		EventHandler1Param<int> OnMouseDown;
		/* OnMouseDown: Event handler, invoked when any (left,right,middle) mouse buttons enters the released state while over this control OR the control was the last target of mouse down, AND it was enabled. */
		EventHandler1Param<int> OnMouseUp;
		/* OnMouseEnter: Event Handler, invoked when the mouse STARTS hovering over this control */
		EventHandler OnMouseEnter;
		/* OnMouseLeave: Event Handler, invoked when the mouse STOPS hovering over this control */
		EventHandler OnMouseLeave;
		/* OnMouseMove: Event Handler, invoked when the mouse moves while hovering over this control, or the control is the current mouse down target */
		EventHandler OnMouseMove;
		/* OnFocus: Event Handler, called when this control gains input focus */
		EventHandler OnFocus;
		/* OnFocus: Event Handler, called when this control loses input focus */
		EventHandler OnBlur;
		/* OnMove: Event Handler, called when this control's location changes */
		EventHandler OnMove;
		/* OnResize: Event Handler, called when this control's size changes */
		EventHandler OnResize;

		/* Parent: The control's parent if any */
		ControlBase *Parent = NULL;

		/* Theme: the ControlTheme for this control */
		ControlTheme Theme;

		/* Text: Any text to display on this control.  Text Scale and Text Align will be used to determine its size and location */
		std::string Text;

		/* Active: wether the control is currently "Active" is used for an additional state to be used differently as needed.  In list items, it can serve as the selected item color, for example. */
		bool Active = false;

		/* Active: wether the control is currently "Active" is used for an additional state to be used differently as needed.  In list items, it can serve as the selected item color, for example. */
		bool Visible = true;

		/* Enabled: wether the control uses its Disabled state (if not .empty), and wether it responds to hover and click state transitions */
		bool Enabled = true;

		/* CanFocus: Determines wether the control can take input focus.  If it cant, it can still remove the focus of the current control if selected. */
		bool CanFocus = false;

		/* Name: entirely an external ID to allow you to track where an event came from */
		std::string Name = "";

		/* Controls: all Controls ARE containers, so we need a list of controls, to render and manage user input */
		std::list<ControlBase*> Controls;

		/* Hovering: is the mouse currently hovering over this control... don't set it unless you are writing an input handler, but it can be read any time */
		bool Hovering = false;

		/* Constructor: Basic constructor for a control to create it within a parent */
		ControlBase(Rect location, ControlBase *parent = NULL)
		{
			Theme.Default.ForegroundColor = olc::WHITE;
			Location = location;
			Parent = parent;
			if (Parent != NULL)
			{
				Parent->Controls.push_back(this);
			}
		}

		/* Constructor: create a control within a parent and set its theme */
		ControlBase(Rect location, ControlTheme theme, ControlBase *parent = NULL)
		{
			Location = location;
			Parent = parent;
			ApplyTheme(theme);

			if (Parent != NULL)
			{
				Parent->Controls.push_back(this);
			}
		}

		/* Constructor: base simple constructor for future expansion and complex custom controls */
		ControlBase() { Theme.Default.ForegroundColor = olc::WHITE; } // just so by default objects show text

		/* MouseOver: meant to be handled in the OnUserUpdate call in olcPixelGameEngine, with the mouse coordinates, to handle mouse interaction */
		virtual ControlBase *MouseOver(olc::vi2d mpos)
		{
			if (Visible)
			{
				Rect sRect;
				sRect.Position = ScreenPos();
				sRect.Size = Location.Size;
				if (Enabled && sRect.Contains(mpos))
				{
					std::list<ControlBase *> tmpControls = Controls;
					tmpControls.reverse();
					for (auto control : tmpControls)
					{
						ControlBase* res = control->MouseOver(mpos);
						if (res != NULL)
						{
							Hovering = false;
							return res;
						}
					}
					Hovering = true;
					return this;
				}
			}
			Hovering = false;
			return NULL;
		}

		/* ApplyTheme: Change the theme of a control.  individual aspects of a theme can also be changed at any time. */
		void ApplyTheme(ControlTheme theme)
		{
			Theme = theme.DeepCopy();
		}

		/* Focus: Give this control input focus. */
		virtual void Focus() 
		{
			OnFocus.Invoke(this);
		};

		/* Blur: remove the current input focus. */
		virtual void Blur() 
		{
			OnBlur.Invoke(this);
		};

		/* MouseEnter: The mouse started hovering over your control. */
		virtual void MouseEnter() 
		{
			OnMouseEnter.Invoke(this);
		};

		/* MouseLeave: The mouse was hovering over your control, and has now left. */
		virtual void MouseLeave() 
		{
			OnMouseLeave.Invoke(this);
		};

		/* MouseDown: Event when a mouse is in the bPressed state on this control. */
		virtual void MouseDown(int mButton) 
		{
			OnMouseDown.Invoke(this, mButton);
		};

		/* MouseUp: Event when a mouse is in the bReleased state, to the control previously notified by mousedown. */
		virtual void MouseUp(int mButton) 
		{
			OnMouseUp.Invoke(this, mButton);
		};

		/* MouseMove: The mouse moved. */
		virtual void MouseMove() 
		{
			OnMouseMove.Invoke(this);
		};

		/* HandleFocusInput: Called during game update if you have input focus, so you can check for any keyboard atcivity */
		virtual void HandleFocusInput(olc::PixelGameEngine* pge, float fElapsedTime) { }

		/* Destructor: In case noone cleaned up the children before destroying the control */
		~ControlBase()
		{
			for (auto control : Controls)
			{
				try
				{
					delete control;
				}
				catch (...) {} // may not be our memory to clean, but dont want to leave it hanging if we are disappearing
			}
		}
	protected:
		/* ScreenPos: Our location in screen space */
		olc::vi2d ScreenPos()
		{
			int x = 0;
			int y = 0;
			ControlBase *cb = this;
			while (cb != NULL)
			{
				x += cb->Location.Position.x;
				y += cb->Location.Position.y;
				cb = cb->Parent;
			}
			return { x,y };
		}

		/* Render: How we draw!  This can be completely overridden, and each piece can be as well,
		   DrawBackground, DrawCustom, and DrawText are called, in that order, and are all virtual and can be overridden */
		virtual void Render(olc::PixelGameEngine* pge, float fElapsedTime)
		{
			if (Visible)
			{
				DrawBackground(pge, fElapsedTime);
				DrawCustom(pge, fElapsedTime);
				DrawText(pge, fElapsedTime);

				for (auto control : Controls)
				{
					control->Render(pge, fElapsedTime);
				}
			}
		}

		/* GetClientRect: get the rectangle defined by the control, minus its padding. */
		virtual Rect GetClientRect()
		{
			ControlStyle cs = Theme.GetStyle(Enabled, Hovering, Active);
			Rect result;
			result.Position = ScreenPos();
			result.Size = Location.Size;
			result.Position.x += cs.Padding.left;
			result.Position.y += cs.Padding.top;
			result.Size.x -= cs.Padding.horiz();
			result.Size.y -= cs.Padding.vert();
			return result;
		}

		/* DrawBackground: The basic draw background, which fills the control with BackgroundColor.
			It will also draw BackgroundDecal if one is available, using the current scaling and alignment settings	*/
		virtual void DrawBackground(olc::PixelGameEngine *pge, float fElapsedTime)
		{
			if (Visible)
			{
				ControlStyle cs = Theme.GetStyle(Enabled, Hovering, Active);

				if (cs.Background)
				{
					cs.Background->Render(pge, { ScreenPos(), Location.Size }, fElapsedTime);
				}
				if (cs.BorderType != Enums::BorderType::None)
					UI::DrawBevel(pge, cs.BorderType, GetClientRect());
			}
		}

		/* DrawCustom: A layer to draw anything needed for custom controls, sits between the foreground and background. */
		virtual void DrawCustom(olc::PixelGameEngine *pge, float fElapsedTime) { }

		/* DrawText: The text positioning and drawing layer, handles proper color, scaling, position. */
		virtual void DrawText(olc::PixelGameEngine *pge, float fElapsedTime)
		{
			if (Visible)
			{
				if (Text.length() > 0)
				{
					ControlStyle cs = Theme.GetStyle(Enabled, Hovering, Active);
					olc::vf2d spos = UI::AlignTextIn(pge, Text, GetClientRect(), cs.TextAlign, cs.TextScale);
					//pge->DrawStringDecal(spos, Text, cs.ForegroundColor, cs.TextScale);
					UI::OutputText(pge, spos, GetClientRect().Size, Text, cs);
				}
			}
		}
	};

	/* DragHandle: a control that is used for a drag handler.  this allows an object to be moved based on dragging the handle around */
	class DragHandle : public ControlBase
	{
	public:
		ControlBase* DragControl = NULL;
		olc::PixelGameEngine* pge;
		Rect BoundingRectangle;

		DragHandle(Rect location, ControlBase* parent, olc::PixelGameEngine* engine)
			: ControlBase(location, parent)
		{
			pge = engine;
		}
		
		EventHandler OnDrag;

		bool bDragging = false;
		olc::vi2d mLoc;

		void MouseDown(int mButton) override
		{
			bDragging = true;
			mLoc = pge->GetMousePos();
		}
		void MouseUp(int mButton) override
		{
			bDragging = false;
		}
		void MouseEnter() override
		{
		}
		void MouseLeave() override
		{
		}
		void MouseMove() override
		{
			if (bDragging)
			{
				if (DragControl != NULL)
				{
					
					olc::vi2d cloc = pge->GetMousePos();

					olc::vi2d pos = DragControl->Position();
					olc::vi2d yopos = pos;
					pos.x += (cloc.x - mLoc.x);
					olc::vi2d xopos = pos;
					pos.y += (cloc.y - mLoc.y);
					yopos.y = pos.y;

					if (BoundingRectangle.Size.x == 0 && BoundingRectangle.Size.y == 0) // if there is an empty rect
					{
						DragControl->SetPosition({ pos.x, pos.y });
						mLoc = cloc;
					}
					else if (BoundingRectangle.ContainsRect({ pos,DragControl->Size() })) // if we are in the rect
					{
						DragControl->SetPosition({pos.x, pos.y});
						mLoc = cloc;
					}
					else if (BoundingRectangle.ContainsRect({ xopos,DragControl->Size() })) // try only moving x
					{
						DragControl->SetPosition({ xopos.x, xopos.y });
						mLoc = cloc;
					}
					else if (BoundingRectangle.ContainsRect({ yopos,DragControl->Size() })) // try only moving y
					{
						DragControl->SetPosition({ yopos.x, yopos.y });
						mLoc = cloc;
					}
					if (mLoc == cloc)
						OnDrag.Invoke(this);
				}
			}
		}
	};

	/* ResizeHandle: a control that is used for a resize handler.  this allows an object to be resized based on dragging the handle around */
	class ResizeHandle : public ControlBase
	{
	public:
		ControlBase* ResizeControl = NULL;
		olc::PixelGameEngine* pge;

		ResizeHandle(Rect location, ControlBase* parent, olc::PixelGameEngine* engine)
			: ControlBase(location, parent)
		{
			pge = engine;
		}

		bool bSizing = false;
		olc::vi2d mLoc;
		olc::vi2d MinimumSize = { 30,30 };
		olc::vi2d MaximumSize = { 0,0 };

		void MouseDown(int mButton) override
		{
			bSizing = true;
			mLoc = pge->GetMousePos();
		}
		void MouseUp(int mButton) override
		{
			bSizing = false;
		}
		void MouseEnter() override
		{
		}
		void MouseLeave() override
		{
		}
		void MouseMove() override
		{
			if (bSizing)
			{
				if (ResizeControl != NULL)
				{
					olc::vi2d cloc = pge->GetMousePos();
					olc::vi2d sz = ResizeControl->Size();
					int xamt = sz.x+(cloc.x - mLoc.x);
					int yamt = sz.y+(cloc.y - mLoc.y);

					if (xamt >= MinimumSize.x && (xamt < MaximumSize.x || MaximumSize.x == 0))
						sz.x = xamt;
					if (yamt >= MinimumSize.y && (yamt < MaximumSize.y || MaximumSize.y == 0))
						sz.y = yamt;

					ResizeControl->SetSize(sz.x,sz.y);
					mLoc = cloc;
				}
			}
		}
	};

	/* ListControl: a derived and composite ControlBase that allows listbox like functionality. */
	class ListControl : public ControlBase
	{
	public:
		/* SelectionChanged: A callback function to indicate that the selection of the list has changed */
		std::function<void(ListControl*)> SelectionChanged;

		/* ItemTheme: The theme for the items in the list (independant of the theme of the list). */
		ControlTheme ItemTheme;

		/* EmptyTheme: EmptyTheme is a default theme that is used to draw any region of list that doesnt have an item available to draw */
		ControlTheme EmptyTheme;

		/* Constructor: Constructor to create and set ItemTheme */
		ListControl(Rect location, ControlBase* parent)
			: ControlBase(location, parent)
		{
		}

		/* Constructor: Constructor to create and set ItemTheme and configure a border. */
		ListControl(Rect location, ControlBase* parent, ControlTheme itemTheme)
			: ControlBase(location, itemTheme, parent)
		{
			ItemTheme = itemTheme;
		}

		/* ItemHeight: the height of each item in the list. */
		int ItemHeight = 16;

		/* SetItems: resets the items in the list.  It also resets any selection and scrolls to the top of the list. */
		void SetItems(std::vector<std::string> items)
		{
			for (auto control : Controls)
			{
				try { delete control; }
				catch (...) {} // not managed by ui manager!
			}
			Controls.clear();

			Items = items;
			TopIndex = 0; // reset position
			SelectedIndex = -1;
			if (SelectionChanged != NULL)
				SelectionChanged(this);
			createListItems();
		}

		/* GetSelection: returns the text of the selected item or an empty string if there is no selection. */
		std::string GetSelection()
		{
			if (SelectedIndex == -1)
				return "";
			else
				return Items.at(SelectedIndex);
		}

		/* SetSelection: attempts to set the selection of the list to the Item that matches the item paramater. */
		void SetSelection(std::string item)
		{
			SelectedIndex = -1;
			for (unsigned int i = 0; i < Items.size(); i++)
			{
				if (Items[i].compare(item) == 0)
				{
					SelectedIndex = i;
					break;
				}
			}
			setItemText();
			if (SelectionChanged != NULL)
				SelectionChanged(this);
		}

		/* Scroll: scroll the list, in a positive or negative amt.  it is clamped at the first and list items in the current list. */
		void Scroll(int amt)
		{
			TopIndex = std::min(std::max(0, TopIndex + amt), ((int)Items.size()) - 1);
			setItemText();
		}

		/* CanScroll: are there items in the specificed direction that i cannot see?  parameter is true if UP or false if DOWN */
		bool CanScroll(bool up)
		{
			if (up)
				return TopIndex >= 0;
			else
				return TopIndex < ((int)Items.size() - 1);
		}

	private:
		/* setItemText: this redraws the text of each control created to match the current position in the list, as well as show the current selection */
		void setItemText()
		{
			for (unsigned int i = 0; i < ListItems.size(); i++)
			{
				unsigned int ridx = i + TopIndex;
				if (ridx < Items.size())
				{
					ListItems.at(i)->Text = Items.at(ridx);
					ListItems.at(i)->Active = (ridx == SelectedIndex);
					ListItems.at(i)->ApplyTheme(ItemTheme);
				}
				else
				{
					ListItems.at(i)->ApplyTheme(EmptyTheme);
				}
			}
		}

		/* Items: the collection of strings that make up the list. */
		std::vector<std::string> Items;

		/* TopIndex: the index that is currently the top of the visible items in the list. */
		int TopIndex = 0; // our scroll position

		/* ListItems: the controls on the screen that display the current selection of Items */
		std::vector<ControlBase *> ListItems;

		/* createListItems: this recreates all of the controls that are used for the display after the list changes */
		void createListItems()
		{
			// clean up!
			ListItems.clear();
			Rect cr = GetClientRect();
			int toDraw = cr.Size.y / ItemHeight;
			for (int i = 0; i < toDraw; i++)
			{
				ControlStyle cs = Theme.GetStyle(Enabled, Hovering, (SelectedIndex == TopIndex + toDraw));
				olc::vi2d loc = { cs.Padding.left, cs.Padding.top + (i * ItemHeight) };
				olc::vi2d size = { cr.Size.x, ItemHeight };
				ControlBase* listItem = new ControlBase({ loc,size }, ItemTheme, this);
				ListItems.push_back(listItem);
				Controls.push_back(listItem);
				itemClickedBind = std::bind(&ListControl::ItemClicked, this, std::placeholders::_1, std::placeholders::_2);
				listItem->OnClick.Subscribe(itemClickedBind);
			}
			setItemText();
		}

		/* SelectedIndex: the current index of the selection, IN Items, or -1 if no selection. */
		int SelectedIndex = -1;

	protected:
		std::function<void(ControlBase*,int)> itemClickedBind;
		/* ItemClicked: Callback function to handle item selection */
		void ItemClicked(ControlBase* control, int mButton)
		{
			// find the index of the string
			std::string tofind = control->Text;
			for (unsigned int i = 0; i < Items.size(); i++)
			{
				if (Items.at(i).compare(tofind) == 0)
				{
					SelectedIndex = i;
					if (SelectionChanged != NULL)
						SelectionChanged(this);
				}
			}
			setItemText(); // handles selection display as well
		}

		/* ItemAt: converts screen coordinates to an index into Items */
		int ItemAt(olc::vi2d location)
		{
			Rect cr = GetClientRect();
			if (location.x < cr.Position.x || location.y < cr.Position.y || location.x > cr.right() || location.y > cr.bottom())
				return -1;
			int visidx = ((location.y - cr.Position.y) / ItemHeight) + TopIndex;
		}
	};

	/* UIManager: my implementation of a UI management system, feel free to modify or use your own. */
	class UIManager : public ControlBase
	{
	private:
		/* PGE: handle to the PixelGameEngine that this UI is running on. */
		olc::PixelGameEngine *PGE;

		/* curControl: the control, if any, the mouse is currently over */
		ControlBase* curControl = NULL;

		/* m_controls: a list of all of the controls that UIManager has been asked to manage, and clean up */
		std::vector<ControlBase *> m_controls;

		/* focusControl: The control that currently has input focus. */
		ControlBase* focusControl = NULL;
	public:
		/* FocusControl: The control that currently has input focus. */
		ControlBase *FocusControl() { return focusControl; }

		/* Constructor: it nees an engine! */
		UIManager(olc::PixelGameEngine *pge)
		{
			Location.Size = { pge->ScreenWidth(),pge->ScreenHeight() };
			PGE = pge;
		}

		/* Descructor: clean up (or try) all of the controls you were managing. */
		~UIManager()
		{
			for (auto control : Controls)
			{
				try
				{
					delete control;
				}
				catch (...) {}
			}
		}

		/* lastmouse: the position of the mouse on the last draw call, to let us know if it moved. */
		olc::vi2d lastmouse = { 0,0 };

		/* mDownControl: the last control to get an mDownEvent */
		ControlBase *mDownControl = NULL; 

		/* UpdateUI: the main update loop for the ui, to be called by OnUserUpdate in the game loop. */
		void UpdateUI(float fElapsedTime)
		{
			olc::vi2d mpos = PGE->GetMousePos();
			
			if (mpos != lastmouse)
			{
				lastmouse = mpos;
				if (mDownControl != NULL)
				{
					mDownControl->MouseMove();
				}
			}


			if (curControl && curControl->Hovering)
				curControl->Hovering = false; // we will set it in the next call anyway, just makes sure we dont get "stuck" hovers

			ControlBase *nControl = NULL;

			nControl = MouseOver(mpos);
			if (nControl != curControl)
			{
				if (curControl != NULL)
					curControl->MouseLeave();
				curControl = nControl;
				nControl->MouseEnter();
			}
			else
			{
				curControl = nControl;
			}

			if (focusControl != NULL)
			{
				focusControl->HandleFocusInput(PGE, fElapsedTime);
			}

			Render(PGE, fElapsedTime);

			for (int i = 0; i < 3; i++)
			{
				if (PGE->GetMouse(i).bPressed)
				{
					if (curControl != NULL)
					{
						// mDown event
						curControl->MouseDown(i);
						curControl->PopToTop();

						mDownControl = curControl;
					}
					if (focusControl != NULL)
					{
						focusControl->Blur();
					}
					focusControl = NULL;

					if (curControl != NULL)
					{
						if (curControl->CanFocus)
						{
							focusControl = curControl;
							focusControl->Focus();
						}

						curControl->OnClick.Invoke(curControl, i);
					}

				}
				else if (PGE->GetMouse(i).bReleased)
				{
					if (mDownControl != NULL)
						mDownControl->MouseUp(i);

					mDownControl = NULL;
				}
			}
		}

		/* CreateControl: use this to create a default control and add it to the parent at the specified location. */
		ControlBase *CreateControl(Rect location, ControlBase* parent = NULL)
		{
			ControlBase* result = new ControlBase(location, parent);
			m_controls.push_back(result);
			return result;
		}

		/* CreateControl: use this to create a themed control and add it to the parent at the specified location. */
		ControlBase *CreateControl(Rect location, ControlTheme theme, ControlBase* parent = NULL)
		{
			ControlBase* result = CreateControl(location, parent);
			result->ApplyTheme(theme);
			m_controls.push_back(result);
			return result;
		}

		/* AddControl: Add a custom control, or precreated control, that derives from ControlBase. */
		void AddControl(ControlBase* control)
		{
			m_controls.push_back(control);
		}

		static olc::Sprite* ScratchPad;
	};
	olc::Sprite* UIManager::ScratchPad = new olc::Sprite(512, 512);

	/* TextEdit : a simplified text entry field.  Check keyList for list of supported keys. */
	class TextEdit : public ControlBase
	{
	private:
		/* focused: is the control currently focused? */
		bool focused = false;

		/* keylist: the keys we monitor for input */
		olc::Key keyList[41] = {
									olc::Key::A,olc::Key::B,olc::Key::C,olc::Key::D,olc::Key::E,olc::Key::F,olc::Key::G,olc::Key::H,olc::Key::I,olc::Key::J,
									olc::Key::K,olc::Key::L,olc::Key::M,olc::Key::N,olc::Key::O,olc::Key::P,olc::Key::Q,olc::Key::R,olc::Key::S,olc::Key::T,
									olc::Key::U,olc::Key::V,olc::Key::W,olc::Key::X,olc::Key::Y,olc::Key::Z,
									olc::Key::K0,olc::Key::K1,olc::Key::K2,olc::Key::K3,olc::Key::K4,olc::Key::K5,olc::Key::K6,olc::Key::K7,olc::Key::K8,olc::Key::K9,
									olc::Key::SPACE, olc::Key::PERIOD, olc::Key::MINUS, olc::Key::COMMA, olc::Key::BACK
		};

		/* keyValues: the values of the keyList in characters, first set is shifted, second set is unshifted */
		char keyValues[82] = {
									'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
									')','!','@','#','$','%','^','&','*','(',
									' ','>','-', '<', 0x08,
									'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
									'0','1','2','3','4','5','6','7','8','9',
									' ','.','-', ',', 0x08
		};

		/* Alpha: Current alpha value for the flasher */
		int Alpha = 0;

		/* Factor: Current direction the alpha channel is moving */
		int Factor = 1;

		/* Pace: The speed at which the pulse occurs.  Pace * fElapsedTime is added or subtracted from the alpha of the background until it reaches the limit, then changes direction */
		int Pace = 500;

	public:
		std::function<void(ControlBase*, std::string)> OnTextChanged;

		/* CursorColor: the color of the text entry cursor */
		olc::Pixel CursorColor = olc::BLACK;

		/* Constructor: Basic default constructor */
		TextEdit(Rect location, ControlBase* parent) : ControlBase(location, parent)
		{
			CanFocus = true;
		}

	protected:
		/* DrawText: overridden to handle all text input and the editing of the string within the text box */
		void DrawText(olc::PixelGameEngine* pge, float fElapsedTime) override
		{
			adjustFlasher(fElapsedTime);
			Rect pos = GetClientRect();
			ControlStyle cs = Theme.GetStyle(Enabled, Hovering, Active);
			if (!focused)
			{
				olc::vi2d negsize = pge->GetTextSize("WW"); // use this to determine when we have less than 2 characters 
				negsize = { (int)(negsize.x * cs.TextScale.x),(int)(negsize.y * cs.TextScale.y) };

				std::string displaytext = Text;
				olc::vi2d size = pge->GetTextSize(displaytext);
				size = { (int)(size.x * cs.TextScale.x), (int)(size.y * cs.TextScale.y) };
				if (size.x >= pos.Size.x)
				{
					int amt = std::max(1, 2 * (pos.Size.x / negsize.x));
					displaytext = displaytext.substr(0, amt);
				}

				olc::vf2d textpos = UI::AlignTextIn(pge, displaytext, pos, cs.TextAlign, cs.TextScale);
				pge->DrawStringDecal(textpos, displaytext, cs.ForegroundColor, cs.TextScale);
			}
			else
			{
				bool changed = appendInput(pge);
				if (changed && OnTextChanged != NULL)
					OnTextChanged(this, Text);

				if (Text.length() == 0)
				{
					pge->FillRectDecal({ (float)pos.Position.x + 3, (float)pos.Position.y + 2 }, { (float)2,(float)pos.Size.y - 4 }, olc::Pixel(CursorColor.r, CursorColor.g, CursorColor.b, Alpha));
				}
				else
				{
					olc::vi2d negsize = pge->GetTextSize("WW"); // use this to determine when we have less than 2 characters 
					negsize = { (int)(negsize.x * cs.TextScale.x),(int)(negsize.y * cs.TextScale.y) };
					olc::vi2d size = pge->GetTextSize(Text);
					size = { (int)(size.x * cs.TextScale.x), (int)(size.y * cs.TextScale.y) };
					std::string displaytext = Text;
					int maxsize = pos.Size.x - negsize.x;

					while (size.x >= maxsize)
					{
						// trim characters ..
						int diff = maxsize - size.x;
						int amt = std::max(1, 2 * (diff / negsize.x));
						displaytext = displaytext.substr(amt);
						size = pge->GetTextSize(displaytext);
						size = { (int)(size.x * cs.TextScale.x), (int)(size.y * cs.TextScale.y) };
					}
					olc::vf2d textpos = UI::AlignTextIn(pge, displaytext, pos, cs.TextAlign, cs.TextScale);
					pge->DrawStringDecal(textpos, displaytext, cs.ForegroundColor, cs.TextScale);
					pge->FillRectDecal({ (float)pos.Position.x + size.x + 1, (float)pos.Position.y + 2 }, { (float)2,(float)pos.Size.y - 4 }, olc::Pixel(CursorColor.r, CursorColor.g, CursorColor.b, Alpha));
				}
			}
		}

		/* appendInput: check for any keys and append them to Text if there are any pressed that we care about, returns a bool indicating if any input caused Text to change */
		bool appendInput(olc::PixelGameEngine *pge)
		{
			bool changed = false;
			bool shifted = pge->GetKey(olc::SHIFT).bPressed || pge->GetKey(olc::SHIFT).bHeld;
			for (int i = 0; i < 41; i++)
			{
				if (pge->GetKey(keyList[i]).bPressed)
				{
					if (keyValues[i] == 0x08)
					{
						if (Text.length() > 0)
						{
							Text = Text.substr(0, Text.length() - 1);
							changed = true;
						}
					}
					else
					{
						changed = true;
						if (shifted)
							Text += keyValues[i];
						else
							Text += keyValues[i + 41];
					}
				}
			}
			return changed;
		}

		/* adjustFlasher: updates the pulse of the cursor */
		void adjustFlasher(float fElapsedTime)
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
		}

		/* Focus: overridden to handle focus */
		void Focus() override
		{
			focused = true;
		}

		/* Blur: overridden to handle focus */
		void Blur() override
		{
			focused = false;
		}
	};

	/* Horizontal Slider: for normal use, later for scroll bar use */
	class Slider : public ControlBase
	{
	private:
		int value = 0;
		int minimum = 0;
		int maximum = 255;
	public:

		int GetValue() { return value; }
		void SetValue(int val)
		{
			if (value != val)
			{
				value = val;
				SetPosition();
				OnValue.Invoke(this, value);
			}
		}

		int GetMinimum() { return minimum; }
		void SetMinimum(int min) { minimum = min; SetPosition(); }

		int GetMaximum() { return maximum; }
		void SetMazimum(int max) { maximum = max; SetPosition(); }

		EventHandler1Param<int> OnValue;

		ControlBase* track;
		DragHandle* dragger;
		Slider(Rect location, ControlBase* parent, int min, int max, int current, olc::PixelGameEngine *pge)
			: ControlBase(location, parent)
		{
			Theme.Default.BorderType = Enums::BorderType::Sunken;
			minimum = min;
			maximum = max;
			value = current;
			track = new ControlBase({ {2,2}, {location.Size.y - 4,location.Size.y - 4 } }, this);
			track->Theme.Default.BorderType = Enums::BorderType::Raised;

			dragger = new DragHandle({ {0,0},track->Size() }, track, pge);
			dragger->DragControl = track;
			dragger->BoundingRectangle = { {2,2},{ Size().x - 3, location.Size.y - 3} };
			SetPosition();

			dragger->OnDrag.Subscribe([&](RasputinUI::ControlBase* sender) {
				int nx = track->Position().x - 2;
				int totsize = Size().x - (dragger->Size().x + 4);

				int per = (nx * 100) / totsize;
				value = (((maximum - minimum) * per) / 100) + minimum;
				OnValue.Invoke(this, value);
				Text = std::to_string(value);
				});
		}

	private:
		void SetPosition()
		{
			float per = (((float)value) * 100) / ((float)maximum);
			track->SetPosition({ (int)((Location.Size.x * per) / 100) - (track->Size().x / 2), track->Position().y });
		}

	};

}

#endif
