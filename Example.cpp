#define OLC_PGE_APPLICATION

#include "RasputinUI.h"

using namespace RasputinUI;

class GridBackground : public UIBackground
{
public:
	olc::Pixel BackgroundColor = olc::BLANK;
	olc::Pixel GridColor = olc::RED;
	olc::vi2d GridSpacing = { 5,5 };
	GridBackground(olc::Pixel backgroundColor = olc::BLANK, olc::Pixel gridColor = olc::RED, olc::vi2d gridSpacing = { 5,5 })
	{
		BackgroundColor = backgroundColor;
		GridColor = gridColor;
		GridSpacing = gridSpacing;
	}

	void Render(olc::PixelGameEngine* pge, Rect area, float fElapsedTime) override
	{
		pge->FillRectDecal(area.Position, area.Size, BackgroundColor);

		for (int y = area.Position.y; y <= area.Position.y + area.Size.y; y += GridSpacing.y)
		{
			// some moving bits on the grid, a bit brighter if possible,than the original color
			pge->FillRectDecal({ (float)area.Position.x,(float)y }, { (float)area.Size.x,1 }, GridColor);
			int bits = rand() % 2;
			for (int i = 0; i < bits; i++)
			{
				if (rand() % 3 < 2)
				{
					int xspot = (rand() % area.Size.x) + area.Position.x;
					pge->FillRectDecal({ (float)xspot,(float)y }, { (float)(1 + (rand() % 4)),1 }, olc::Pixel(GridColor.r, GridColor.g, GridColor.b, 50 + (rand() % 100)));
				}
			}
		}
		for (int x = area.Position.x; x <= area.Position.x + area.Size.x; x += GridSpacing.x)
		{
			pge->FillRectDecal({ (float)x,(float)area.Position.y }, { 1,(float)area.Size.y }, GridColor);
			int bits = rand() % 2;
			for (int i = 0; i < bits; i++)
			{
				if (rand() % 3 < 2)
				{
					int yspot = (rand() % area.Size.y) + area.Position.y;
					pge->FillRectDecal({ (float)x,(float)yspot }, { 1,(float)(1 + (rand() % 4)) }, olc::Pixel(GridColor.r, GridColor.g, GridColor.b, 50 + (rand() % 100)));
				}
			}
		}
	}
};

class Win3Window : public ControlBase
{
protected:
	ControlBase* bc2;
	ControlBase* closeButton;
	ResizeHandle* resize;

public:
	DragHandle* windowTitle;

	Win3Window(Rect location, ControlBase* parent, olc::PixelGameEngine *pge)
		:ControlBase(location, parent)
	{
		Theme.Default.Background = new SolidBackground(olc::Pixel(r,g,b));
		Theme.Default.BorderType = Enums::BorderType::Raised;

		bc2 = new ControlBase({ {3,25},{Location.Size.x-6,Location.Size.y - 28} }, this);
		bc2->Theme.Default.BorderType = Enums::BorderType::Sunken;

		windowTitle = new DragHandle({ {3,3},{Location.Size.x - 6,20} }, this, pge);
		windowTitle->Theme.Default.Padding = { 2,2,2,2 };
		windowTitle->Theme.Default.Background = new SolidBackground(olc::DARK_BLUE);
		windowTitle->Theme.Default.TextAlign = { Enums::Alignment::Near, Enums::Alignment::Center };
		windowTitle->DragControl = this;
		windowTitle->Text = "Test Window";


		closeButton = new ControlBase({ { Location.Size.x-23,3 }, { 14,14 } }, windowTitle);
		closeButton->Theme.Default.Background = new SolidBackground(olc::DARK_GREY);
		closeButton->Theme.Hover.empty = false;
		closeButton->Theme.Hover.Background = new SolidBackground(olc::GREY);
		closeButton->Theme.Hover.ForegroundColor = olc::BLACK;
		closeButton->Theme.Default.BorderType = Enums::BorderType::Raised;
		closeButton->Text = "X";

		auto OnClose = [&](ControlBase* sender, int mButton)
		{
			this->Visible = false;
		};
		closeButton->OnClick.Subscribe([&](ControlBase* sender, int mButton) {Visible = false;});

		resize = new ResizeHandle({ {Location.Size.x-5,location.Size.y-5},{5,5} }, this, pge);
		resize->ResizeControl = this;

		this->OnResize.Subscribe([&](ControlBase* sender) {FixSize(sender); });


		Slider* rslider = new Slider({ {10,50}, {200,20} }, this, 0, 255, r,pge);
		Slider* gslider = new Slider({ {10,75}, {200,20} }, this, 0, 255, g, pge);
		Slider* bslider = new Slider({ {10,100}, {200,20} }, this, 0, 255, b, pge);
		
		rslider->OnValue.Subscribe([&](ControlBase * sender, int val) {
			r = val;
			((SolidBackground*)Theme.Default.Background)->Color = olc::Pixel(r, g, b);
		});
		gslider->OnValue.Subscribe([&](ControlBase* sender, int val) {
			g = val;
			((SolidBackground*)Theme.Default.Background)->Color = olc::Pixel(r, g, b);
		});
		bslider->OnValue.Subscribe([&](ControlBase* sender, int val) {
			b = val;
			((SolidBackground*)Theme.Default.Background)->Color = olc::Pixel(r, g, b);
		});
	}

	int r = 192;
	int g = 192;
	int b = 192;
	void FixSize(ControlBase* sender)
	{
		olc::vi2d size = sender->Size();
		bc2->SetSize(size.x - 6, size.y - 28);
		windowTitle->SetSize(size.x - 6, 20);
		resize->SetPosition({ size.x - 5, size.y - 5 });
		closeButton->SetPosition({size.x - 23, 3});
	}
};

class UIDemo : public olc::PixelGameEngine
{
private:
	RasputinUI::UIManager* uiManager;
public:
	UIDemo()
	{
		sAppName = "RasputinUI Demo App";
	}
	
	bool OnUserCreate() override
	{
		uiManager = new RasputinUI::UIManager(this);
		olc::Pixel gridcolor = olc::RED;
		gridcolor.a = 40;
		uiManager->Theme.Default.Background = new GridBackground(olc::BLACK, gridcolor, { 20,20 });
		
		Win3Window* win1 = new Win3Window({ {10,10},{300,200} }, uiManager, this);
		win1->Theme.Default.Background = new SolidBackground(olc::RED);
		Win3Window* win2 = new Win3Window({ {320,10},{300,200} }, uiManager, this);

		Clear(olc::BLACK);
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		uiManager->UpdateUI(fElapsedTime);
		return true;
	}
};

int main()
{
	srand((unsigned int)time(NULL));
	
	UIDemo demo;
	if (demo.Construct(640, 480, 1, 1))
		demo.Start();

	return 0;
}
