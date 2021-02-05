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
	/* Rectangle class, not strictly needed, but makes some calculations more convenient*/
	struct Rect
	{
		olc::vi2d Position;
		olc::vi2d Size;

		bool Contains(olc::vi2d point)
		{
			return (point.x >= Position.x && point.x < (Position.x + Size.x) && point.y >= Position.y && point.y < (Position.y + Size.y));
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

		bool Empty()
		{
			return (Size.x == 0 || Size.y == 0); // either dimension being 0 is an empty rectangle
		}
	};

	/* Alignment: a basic Near/Center/Far alignment system */
	enum Alignment
	{
		Near, Far, Center
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

	/* FullAlignment: a structure containing both Horizontal and Vertical alignment */
	struct FullAlignment
	{
		Alignment Horizontal;
		Alignment Vertical;
	};

	/* ControlStyle: determines the appearance of a control */
	struct ControlStyle
	{
		bool empty = false; // set this to true to make it be ignored
		olc::Pixel ForegroundColor = olc::BLANK;
		olc::Pixel BackgroundColor = olc::BLANK;
		FullAlignment TextAlign = { Center,Center };
		olc::vf2d TextScale = { 1,1 };
		Spacing Padding = { 0,0,0,0 };
		olc::Decal* BackgroundDecal = NULL;
		FullAlignment BackgroundDecalAlign = { Center,Center };
		olc::vf2d BackgroundDecalScale = { 1,1 };
		static ControlStyle Empty()
		{
			ControlStyle result;
			result.empty = true;
			return result;
		}

		ControlStyle DeepCopy()
		{
			ControlStyle result;
			result.empty = empty;
			result.ForegroundColor = ForegroundColor;
			result.BackgroundColor = BackgroundColor;
			result.TextAlign = TextAlign;
			result.TextScale = TextScale;
			result.Padding = Padding;
			result.BackgroundDecal = BackgroundDecal;
			result.BackgroundDecalAlign = BackgroundDecalAlign;
			result.BackgroundDecalScale = BackgroundDecalScale;
			return result;
		}
	};

	/* ControlTheme: allows a control to have varied ControlStyle based on it's state (Enabled, Default, Active, Hover) */
	struct ControlTheme
	{
		ControlStyle Disabled=ControlStyle::Empty();
		ControlStyle Default;
		ControlStyle Hover= ControlStyle::Empty();
		ControlStyle Active= ControlStyle::Empty();

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

	/* ControlBase: the base control for the UI system. This can be used as a label, a panel, a button, an image, just about anything.  
		You can also derive from it and create addition custom controls, as well as build composite controls.
	*/
	class ControlBase
	{
	public:
		/* OnClick: Event handler, invoked when any (left,right,middle) mouse buttons are pressed while over this control, AND it was enabled. */
		std::function<void(ControlBase*, int)> OnClick;

		/* Location: The controls location and size, relative to its parent */
		Rect Location;

		/* Parent: The control's parent if any */
		ControlBase *Parent = NULL;

		/* Theme: the ControlTheme for this control */
		ControlTheme Theme;

		/* Text: Any text to display on this control.  Text Scale and Text Align will be used to determine its size and location */
		std::string Text;

		/* Active: wether the control is currently "Active" is used for an additional state to be used differently as needed.  In list items, it can serve as the selected item color, for example. */
		bool Active = false;

		/* Enabled: wether the control uses its Disabled state (if not .empty), and wether it responds to hover and click state transitions */
		bool Enabled = true;

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
			Rect sRect;
			sRect.Position = ScreenPos();
			sRect.Size = Location.Size;
			if (Enabled && sRect.Contains(mpos))
			{
				for (auto control : Controls)
				{
					ControlBase *res = control->MouseOver(mpos);
					if (res != NULL)
					{
						Hovering = false;
						return res;
					}
				}
				Hovering = true;
				return this;
			}
			Hovering = false;
			return NULL;
		}

		/* ApplyTheme: Change the theme of a control.  individual aspects of a theme can also be changed at any time. */
		void ApplyTheme(ControlTheme theme)
		{
			Theme = theme.DeepCopy();
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
		virtual void Render(olc::PixelGameEngine* pge)
		{
			DrawBackground(pge);
			DrawCustom(pge);
			DrawText(pge);

			for (auto control : Controls)
			{
				control->Render(pge);
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
		virtual void DrawBackground(olc::PixelGameEngine *pge)
		{
			ControlStyle cs = Theme.GetStyle(Enabled, Hovering, Active);
			pge->FillRectDecal(ScreenPos(), Location.Size, cs.BackgroundColor);

			if (cs.BackgroundDecal)
			{
				olc::vf2d spos = ScreenPos();
				spos.x += cs.Padding.left;
				spos.y += cs.Padding.top;
				olc::vi2d tsize = { cs.BackgroundDecal->sprite->width, cs.BackgroundDecal->sprite->height };
				tsize *= cs.BackgroundDecalScale;

				if (cs.BackgroundDecalAlign.Horizontal == Far)
					spos.x = (Location.right() - cs.Padding.right) - tsize.x;
				else if (cs.BackgroundDecalAlign.Horizontal == Center)
					spos.x += ((Location.Size.x - cs.Padding.horiz()) - tsize.x) / 2;

				if (cs.BackgroundDecalAlign.Vertical == Far)
					spos.y = (Location.bottom() - cs.Padding.bottom) - tsize.y;
				else if (cs.BackgroundDecalAlign.Vertical == Center)
					spos.y += ((Location.Size.y - cs.Padding.vert()) - tsize.y) / 2;

				pge->DrawDecal(spos, cs.BackgroundDecal, cs.BackgroundDecalScale);

			}
		}

		/* DrawCustom: A layer to draw anything needed for custom controls, sits between the foreground and background. */
		virtual void DrawCustom(olc::PixelGameEngine *pge) { }

		/* DrawText: The text positioning and drawing layer, handles proper color, scaling, position. */
		virtual void DrawText(olc::PixelGameEngine *pge)
		{
			if (Text.length() > 0)
			{
				ControlStyle cs = Theme.GetStyle(Enabled, Hovering, Active);
				if (Hovering)
				{
					int a = 0;
				}
				olc::vf2d spos = AlignTextIn(pge, Text, GetClientRect(), cs.TextAlign, cs.TextScale);
				pge->DrawStringDecal(spos, Text, cs.ForegroundColor, cs.TextScale);
			}
		}

		/* AlignTextIn: A convenience function to calculate position within a rectangle given an alignment, size, and scale. */
		olc::vf2d AlignTextIn(olc::PixelGameEngine* pge, std::string text, Rect destination, FullAlignment textAlign, olc::vf2d scale)
		{
			olc::vf2d spos = destination.Position;
			olc::vi2d tsize = pge->GetTextSize(Text);
			tsize *= scale;

			if (textAlign.Horizontal == Far)
				spos.x += destination.Size.x - tsize.x;
			else if (textAlign.Horizontal == Center)
				spos.x += (destination.Size.x - tsize.x) / 2;

			if (textAlign.Vertical == Far)
				spos.y = destination.bottom() - tsize.y;
			else if (textAlign.Vertical == Center)
				spos.y += (destination.Size.y - tsize.y) / 2;

			return spos;
		}
	};

	/* BorderedControl: a derived ControlBase that adds a border definable by color and width. */
	/* Currently, since border isnt a part of a them, BorderedControl only supports two state differences, hovering or normal. */
	class BorderedControl : public ControlBase
	{
	public:
		/* BorderColor: Default Border Color */
		olc::Pixel BorderColor = olc::BLANK;

		/* BorderHoverColor: Hover Border Color */
		olc::Pixel BorderHoverColor = olc::BLANK;

		/* BorderWidth: the width, and height depending on the edge, of the border */
		int BorderWidth = 2;

		/* Constructor: sets position and parent, as well as bordercolor and width */
		BorderedControl(Rect location, ControlBase* parent, olc::Pixel borderColor, int borderWidth = 2)
			: ControlBase(location, parent)
		{
			BorderColor = borderColor;
			BorderWidth = borderWidth;
		}

		/* DrawCustom: this class overrides only the DrawCustom portion of the Control Render process. */
		void DrawCustom(olc::PixelGameEngine* pge) override
		{
			Rect bnds = { ScreenPos(), {Location.Size.x, Location.Size.y} };
			olc::Pixel bc = (Hovering && BorderHoverColor != olc::BLANK) ? BorderHoverColor : BorderColor;
			if (BorderWidth > 0 && bc != olc::BLANK)
			{
				pge->FillRectDecal(bnds.Position, { (float)bnds.Size.x, (float)BorderWidth }, bc);
				pge->FillRectDecal(bnds.Position, { (float)BorderWidth, (float)bnds.Size.y }, bc);
				pge->FillRectDecal({ (float)bnds.right() - BorderWidth, (float)bnds.Position.y }, { (float)BorderWidth, (float)bnds.Size.y }, bc);
				pge->FillRectDecal({ (float)bnds.Position.x, (float)bnds.bottom() - BorderWidth}, { (float)bnds.Size.x, (float)BorderWidth }, bc);

			}
		}
	};

	/* ListControl: a derived and composite ControlBase that allows listbox like functionality. */
	
	class ListControl : public BorderedControl
	{
	public:
		/* SelectionChanged: A callback function to indicate that the selection of the list has changed */
		std::function<void(ControlBase*)> SelectionChanged;

		/* ItemTheme: The theme for the items in the list (independant of the theme of the list). */
		ControlTheme ItemTheme;

		/* EmptyTheme: EmptyTheme is a default theme that is used to draw any region of list that doesnt have an item available to draw */
		ControlTheme EmptyTheme;

		/* Constructor: Constructor to create and set ItemTheme */
		ListControl(Rect location, ControlBase* parent, ControlTheme itemTheme)
			: BorderedControl(location, parent, olc::BLANK, 0) 
		{ 
			ItemTheme = itemTheme; 
		}

		/* Constructor: Constructor to create and set ItemTheme and configure a border. */
		ListControl(Rect location, ControlBase* parent, ControlTheme itemTheme, olc::Pixel borderColor, int borderWidth = 0)
			: BorderedControl(location, parent, borderColor, borderWidth) 
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
			if (SelectionChanged !=NULL)
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
			for (int i = 0; i < Items.size(); i++)
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
				return TopIndex < (Items.size() - 1);
		}

	private:
		/* setItemText: this redraws the text of each control created to match the current position in the list, as well as show the current selection */
		void setItemText()
		{
			for (int i = 0; i < ListItems.size(); i++)
			{
				int ridx = i + TopIndex;
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
				listItem->OnClick = std::bind(&ListControl::ItemClicked, this, std::placeholders::_1);
			}
			setItemText(); 
		}

		/* SelectedIndex: the current index of the selection, IN Items, or -1 if no selection. */
		int SelectedIndex = -1;

		/* ItemClicked: Callback function to handle item selection */
		void ItemClicked(ControlBase* control)
		{
			// find the index of the string
			std::string tofind = control->Text;
			for (int i=0;i<Items.size();i++)
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

	protected:

		/* ItemAt: converts screen coordinates to an index into Items */
		int ItemAt(olc::vi2d location) 
		{
			Rect cr = GetClientRect();
			if (location.x < cr.Position.x || location.y < cr.Position.y || location.x > cr.right() || location.y > cr.bottom())
				return -1;
			int visidx = ((location.y - cr.Position.y)/ItemHeight)+TopIndex;
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
	public:

		/* Constructor: it nees an engine! */
		UIManager(olc::PixelGameEngine *pge)
		{
			Location.Size = { pge->ScreenWidth(),pge->ScreenHeight() };
			PGE = pge;
		}

		/* Descructor: clean up (or try) all of the controls you were managing. */
		~UIManager()
		{  /
			for (auto control : Controls)
			{
				try
				{
					delete control;
				}
				catch(...) {}
			}
		}

		/* UpdateUI: the main update loop for the ui, to be called by OnUserUpdate in the game loop. */
		void UpdateUI(float fElapsedTime)
		{
			olc::vi2d mpos = PGE->GetMousePos();
			if (curControl && curControl->Hovering)
				curControl->Hovering = false; // we will set it in the next call anyway, just makes sure we dont get "stuck" hovers
			curControl = MouseOver(mpos);
			Render(PGE);
			
			if (curControl != NULL && curControl->OnClick != NULL)
			{
				for (int i=0;i<3;i++)
					if (PGE->GetMouse(i).bPressed)
						curControl->OnClick(curControl, i);
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

	};
}

#endif
