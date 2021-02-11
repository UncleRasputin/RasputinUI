#define OLC_PGE_APPLICATION

#include "RasputinUI.h"

using namespace RasputinUI;

class Win3Window : public ControlBase
{
protected:
	ControlBase* closeButton;
	ResizeHandle* resize;
	DragHandle* CountDraggo;
	ControlBase* titleBar;

public:
	ControlBase* clientArea;
	ControlBase* windowTitle;
	TextEdit* testEdit;
	ListControl* listControl;
	ControlBase* testButton;

	Win3Window(Rect location, ControlBase* parent, olc::PixelGameEngine* pge)
		:ControlBase(location, parent)
	{
		Theme.Default.Background = new SolidBackground(olc::Pixel(r, g, b));
		Theme.Default.BorderType = RUI::BorderType::Raised;

		clientArea = new ControlBase({ {3,24},{Location.Size.x - 6,Location.Size.y - 27} }, this);
		clientArea->Theme.Default.BorderType = RUI::BorderType::Sunken;

		titleBar = new ControlBase({ {3,3},{Location.Size.x - 6,20} }, this);
		titleBar->Theme.Default.Padding = { 3,3,3,3 };
		titleBar->Theme.Default.BorderType = RUI::BorderType::Solid;
		titleBar->Theme.Default.Background = new SolidBackground(olc::Pixel(0, 0, 128));
				
		windowTitle = new DragHandle({ {3,3},{Location.Size.x - 32,14} }, titleBar, pge);
		windowTitle->Theme.Default.TextAlign = { RUI::Alignment::Near, RUI::Alignment::Center };
		windowTitle->SetText("Test Window");

		CountDraggo = new DragHandle({ {3,3},{Location.Size.x - 32,14} }, titleBar, pge);
		CountDraggo->DragControl = this;


		closeButton = new ControlBase({ { Location.Size.x - 24,3 }, { 14,14 } }, titleBar);
		closeButton->Theme.Default.Background = new SolidBackground(olc::Pixel(192,192,192));
		closeButton->Theme.Default.ForegroundColor = olc::Pixel(32,32,32);
		closeButton->Theme.Hover.empty = false;
		closeButton->Theme.Hover.Background = new SolidBackground(olc::GREY);
		closeButton->Theme.Hover.ForegroundColor = olc::BLACK;
		closeButton->Theme.Default.BorderType = RUI::BorderType::Raised;
		closeButton->SetText("X");

		closeButton->OnClick.Subscribe([&](ControlBase* sender, int mButton) {Visible = false; });

		resize = new ResizeHandle({ {Location.Size.x - 5,location.Size.y - 5},{5,5} }, this, pge);
		resize->ResizeControl = this;

		this->OnResize.Subscribe([&](ControlBase* sender) {FixSize(sender); });


		Slider* rslider = new Slider({ {10,10}, {100,16} }, clientArea, 0, 255, r, pge);
		Slider* gslider = new Slider({ {10,30}, {100,16} }, clientArea, 0, 255, g, pge);
		Slider* bslider = new Slider({ {10,50}, {100,16} }, clientArea, 0, 255, b, pge);
		Slider* aslider = new Slider({ {10,70}, {100,16} }, clientArea, 0, 255, b, pge);
		//aslider->Visible = false; // when alpha is fixed!

		rslider->OnValue.Subscribe([&](ControlBase* sender, int val) {
			r = val;
			((SolidBackground*)Theme.Default.Background)->Color = olc::Pixel(r, g, b, a);
			Invalidate();
		});
		gslider->OnValue.Subscribe([&](ControlBase* sender, int val) {
			g = val;
			((SolidBackground*)Theme.Default.Background)->Color = olc::Pixel(r, g, b, a);
			Invalidate();
		});
		bslider->OnValue.Subscribe([&](ControlBase* sender, int val) {
			b = val;
			((SolidBackground*)Theme.Default.Background)->Color = olc::Pixel(r, g, b, a);
			Invalidate();
		});
		aslider->OnValue.Subscribe([&](ControlBase* sender, int val) {
			a = val;
			((SolidBackground*)Theme.Default.Background)->Color = olc::Pixel(r, g, b,a);
			Invalidate();
		});

		testEdit = new TextEdit({ {130,10},{150,20} }, clientArea);
		testEdit->Theme.Default.Background = new SolidBackground(olc::WHITE);
		testEdit->Theme.Default.ForegroundColor = olc::BLACK;
		testEdit->Theme.Default.Padding = { 2,2,2,2 };
		testEdit->Theme.Default.TextAlign = { RUI::Alignment::Near,RUI::Alignment::Center };
		testEdit->Theme.Default.BorderType = RUI::BorderType::Sunken;
		testEdit->SetText("asdf");

		
		ControlTheme ct;
		ct.Default.Background = new SolidBackground(olc::WHITE);
		ct.Default.ForegroundColor = olc::BLACK;
		ct.Hover.empty = false;
		ct.Hover.Background = new SolidBackground(olc::Pixel(227, 227, 227)); // old windows wasn't nice enough to have hover, but it looks nice
		ct.Hover.ForegroundColor = olc::BLACK;
		ct.Active.empty = false;
		ct.Active.Background = new SolidBackground(olc::BLUE);
		ct.Active.ForegroundColor = olc::WHITE;

		listControl = new ListControl({ {130,40},{150,100} }, clientArea, ct);
		listControl->Theme.Default.BorderType = RUI::BorderType::Sunken;
		listControl->Theme.Default.Padding = { 2,2,2,2 };

		std::vector<std::string> list_items = { "Some Item","Some Other Item", "Yet Another Item", "Extra Item", "Bonus Item" };
		listControl->SetItems(list_items);

		testButton = new ControlBase({ {10,120},{50,20} }, clientArea);
		testButton->Theme.Default.Background = new SolidBackground(olc::DARK_GREY);
		testButton->Theme.Default.BorderType = RUI::BorderType::Raised;
		testButton->Theme.Default.TextAlign = { RUI::Alignment::Center,RUI::Alignment::Center };
		testButton->Theme.Active = testButton->Theme.Default.DeepCopy();
		testButton->Theme.Active.BorderType = RUI::BorderType::Sunken;

		testButton->OnMouseDown.Subscribe([&](ControlBase* sender, int mButton) 
		{
			sender->Active = true; 
		});
		testButton->OnMouseUp.Subscribe([&](ControlBase* sender, int mButton) {sender->Active = false; });
		testButton->SetText("OK");
	}

	int r = 192;
	int g = 192;
	int b = 192;
	int a = 255;
	void FixSize(ControlBase* sender)
	{
		olc::vi2d size = sender->Size();
		resize->SetPosition({ std::max(0, size.x - 5), std::max(0, size.y - 5) });
		clientArea->SetSize(std::max(0, size.x - 6), std::max(0, size.y - 28));
		titleBar->SetSize(std::max(0, size.x - 6), 20);
		closeButton->SetPosition({ std::max(0,size.x - 24), 3 });
		windowTitle->SetSize(std::max(0,size.x - 32), 14);
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

	olc::Sprite* pattern;
	bool OnUserCreate() override
	{
		uiManager = new RasputinUI::UIManager();
		olc::Pixel gridcolor = olc::RED;
		gridcolor.a = 40;
		uiManager->mainControl->Theme.Default.Background = new SolidBackground(olc::Pixel(0,160,160));

		Win3Window* win1 = new Win3Window({ {10,10},{300,200} }, uiManager->mainControl, this);
		Win3Window* win2 = new Win3Window({ {320,10},{300,200} }, uiManager->mainControl, this);
		Win3Window* win3 = new Win3Window({ {10,220},{300,200} }, uiManager->mainControl, this);
		Win3Window* win4 = new Win3Window({ {320,220},{300,200} }, uiManager->mainControl, this);

		Clear(olc::BLACK);
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
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
