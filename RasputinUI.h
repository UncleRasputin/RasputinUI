#pragma once
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
/*
Totally free UI from Rasputin (aka UncleTime)

License Type: Relief

Please take this code and do not return it, it's site offends me and torments my soul.
You may do whatever you want with it, including remove this comment if you feel the need to.
If you make changes or derive from it to add functionality, consider sharing it as well.

Requires olcPixelGameEngine.h
*/

namespace RasputinUI
{
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

	enum Alignment
	{
		Near, Far, Center
	};

	struct Spacing
	{
		int top;
		int right;
		int bottom;
		int left;

		int vert() { return top + bottom; }
		int horiz() { return left + right; }
	};

	struct FullAlignment
	{
		Alignment Horizontal;
		Alignment Vertical;
	};

	class ControlBase;

	typedef std::list<ControlBase*> ControlList;

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

		ControlStyle Dupe()
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

		ControlTheme Dupe()
		{
			ControlTheme result;
			result.Disabled = Disabled.Dupe();
			result.Default = Default.Dupe();
			result.Hover = Hover.Dupe();
			result.Active = Active.Dupe();
			return result;
		}
	};

	class ControlBase
	{
	public:
		std::function<void(ControlBase*, int)> OnClick;
		Rect Location;
		ControlBase *Parent = NULL;
		ControlTheme Theme;
		std::string Text;
		bool Active = false;
		bool Enabled = true;
		std::string Name = "";
		ControlList Controls;
		bool Hovering = false;

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

		ControlBase() { Theme.Default.ForegroundColor = olc::WHITE; } // just so by default objects show text

		virtual void Render(olc::PixelGameEngine *pge)
		{
			DrawBackground(pge);
			DrawCustom(pge);
			DrawText(pge);

			for (ControlList::iterator iter = Controls.begin(); iter != Controls.end(); ++iter)
			{
				(*iter)->Render(pge);
			}

		}

		virtual ControlBase *MouseOver(olc::vi2d mpos)
		{
			Rect sRect;
			sRect.Position = ScreenPos();
			sRect.Size = Location.Size;
			if (Enabled && sRect.Contains(mpos))
			{
				for (ControlList::iterator iter = Controls.begin(); iter != Controls.end(); ++iter)
				{
					ControlBase *res = (*iter)->MouseOver(mpos);
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

		void ApplyTheme(ControlTheme theme)
		{
			Theme = theme.Dupe();
		}

	protected:
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

		virtual void DrawCustom(olc::PixelGameEngine *pge) { }

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

	class BorderedControl : public ControlBase
	{
	public:
		olc::Pixel BorderColor = olc::BLANK;
		olc::Pixel BorderHoverColor = olc::BLANK;
		int BorderWidth = 2;
		BorderedControl(Rect location, ControlBase* parent, olc::Pixel borderColor, int borderWidth = 2)
			: ControlBase(location, parent)
		{
			BorderColor = borderColor;
			BorderWidth = borderWidth;
		}

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

	class ListControl : public BorderedControl
	{
	public:
		std::function<void(ControlBase*)> SelectionChanged;
		ControlTheme ItemTheme;
		ControlTheme EmptyTheme;
		ListControl(Rect location, ControlBase* parent, ControlTheme itemTheme)
			: BorderedControl(location, parent, olc::BLANK, 0) 
		{ 
			ItemTheme = itemTheme; 
		}

		ListControl(Rect location, ControlBase* parent, ControlTheme itemTheme, olc::Pixel borderColor, int borderWidth = 0)
			: BorderedControl(location, parent, borderColor, borderWidth) 
		{ 
			ItemTheme = itemTheme;
		}

		int ItemHeight = 16;

		void SetItems(std::vector<std::string> items)
		{
			for (ControlList::iterator iter = Controls.begin(); iter != Controls.end(); ++iter)
			{
				try { delete (*iter); }
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

		std::string GetSelection()
		{
			if (SelectedIndex == -1)
				return "";
			else
				return Items.at(SelectedIndex);
		}

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
		void Scroll(int amt)
		{
			TopIndex = std::min(std::max(0, TopIndex + amt), ((int)Items.size()) - 1);
			setItemText();
		}

		bool CanScroll(bool up /*else down*/)
		{
			if (up)
				return TopIndex >= 0;
			else
				return TopIndex < (Items.size() - 1);
		}

	private:
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
		std::vector<std::string> Items;
		int TopIndex = 0; // our scroll position

		std::vector<ControlBase *> ListItems;
		void createListItems()
		{
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
		int SelectedIndex = -1;
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
		int ItemAt(olc::vi2d location) 
		{
			Rect cr = GetClientRect();
			if (location.x < cr.Position.x || location.y < cr.Position.y || location.x > cr.right() || location.y > cr.bottom())
				return -1;
			int visidx = ((location.y - cr.Position.y)/ItemHeight)+TopIndex;
		}
	};
	
	class UIManager : public ControlBase
	{
	private:
		olc::PixelGameEngine *PGE;
		ControlBase* curControl = NULL;
		ControlList m_controls;
	public:

		UIManager(olc::PixelGameEngine *pge)
		{
			Location.Size = { pge->ScreenWidth(),pge->ScreenHeight() };
			PGE = pge;
		}

		~UIManager()
		{  // if we have a control in our control list, lets make sure its deleted
			for (ControlList::iterator iter = m_controls.begin(); iter != m_controls.end(); ++iter)
			{
				try
				{
					delete (*iter);
				}
				catch(...) {}
			}
		}

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

		ControlBase *CreateControl(Rect location, ControlBase* parent = NULL)
		{
			ControlBase* result = new ControlBase(location, parent);
			m_controls.push_back(result);
			return result;
		}

		ControlBase *CreateControl(Rect location, ControlTheme theme, ControlBase* parent = NULL)
		{
			ControlBase* result = CreateControl(location, parent);
			result->ApplyTheme(theme);
			m_controls.push_back(result);
			return result;
		}

		void AddControl(ControlBase* control)
		{
			m_controls.push_back(control);
		}

	};
}