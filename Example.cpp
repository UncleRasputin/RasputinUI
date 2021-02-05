#include "RasputinUI.h"

class Example : public olc::PixelGameEngine
{
private:
	RasputinUI::UIManager *uiManager;
	void LoadSprites()
	{
		sprButton = new olc::Sprite("button.png");
		decButton = new olc::Decal(sprButton);
		sprButtonHover = new olc::Sprite("button_hover.png");
		decButtonHover = new olc::Decal(sprButtonHover);
		sprButtonDisabled = new olc::Sprite("button_disabled.png");
		decButtonDisabled = new olc::Decal(sprButtonDisabled);
	}
	void CreateThemes()
	{
		ButtonTheme.Default.BackgroundColor = olc::BLANK;
		ButtonTheme.Default.BackgroundDecal = decButton;
		ButtonTheme.Default.ForegroundColor = olc::GREEN;
		ButtonTheme.Default.TextScale = { 1,1 };

		ButtonTheme.Hover = ButtonTheme.Default.Dupe();
		ButtonTheme.Hover.BackgroundColor = olc::BLANK;
		ButtonTheme.Hover.BackgroundDecal = decButtonHover;
		ButtonTheme.Hover.ForegroundColor = olc::BLACK;

		ButtonTheme.Disabled = ButtonTheme.Default.Dupe();
		ButtonTheme.Disabled.BackgroundColor = olc::BLANK;
		ButtonTheme.Disabled.BackgroundDecal = decButtonDisabled;
		ButtonTheme.Disabled.ForegroundColor = olc::VERY_DARK_GREY;


		TextBarTheme.Default.BackgroundColor = olc::DARK_GREY;
		TextBarTheme.Default.Padding = { 4,4,4,4 };
		TextBarTheme.Default.ForegroundColor = olc::WHITE;

		TextBarTheme.Hover = TextBarTheme.Default.Dupe();
		TextBarTheme.Hover.BackgroundColor = olc::GREY;

		ListItemTheme.Default.BackgroundColor = olc::Pixel(0, 0, 0, 20);;
		ListItemTheme.Default.Padding = { 4,4,4,4 };
		ListItemTheme.Default.ForegroundColor = olc::WHITE;

		ListItemTheme.Hover = ListItemTheme.Default.Dupe();
		ListItemTheme.Hover.BackgroundColor = olc::Pixel(0, 0, 0, 50);

		ListItemTheme.Active = ListItemTheme.Default.Dupe();
		ListItemTheme.Active.BackgroundColor = olc::RED;

	}
public:
	Example()
	{
		sAppName = "Rasputin UI Test App";
	}


	olc::Sprite* sprButton;
	olc::Sprite* sprButtonHover;
	olc::Decal* decButton;
	olc::Decal* decButtonHover;

	olc::Sprite* sprButtonDisabled;
	olc::Decal* decButtonDisabled;
public:
	RasputinUI::ControlTheme ListItemTheme;
	RasputinUI::ControlTheme SelectedListItemTheme;
	RasputinUI::ControlTheme ButtonTheme;
	RasputinUI::ControlTheme TextBarTheme;
	RasputinUI::BorderedControl* UIWindow;
	RasputinUI::ControlBase* Heading;
	RasputinUI::ListControl* ListBox1;
	bool OnUserCreate() override
	{
		LoadSprites();
		CreateThemes();
		uiManager = new RasputinUI::UIManager(this);
		CreateUI();
		Clear(olc::BLACK);
		return true;
	}

	void CreateUI()
	{
		UIWindow = new RasputinUI::BorderedControl({ {20,10},{600,450} }, uiManager, olc::Pixel(151, 115, 106), 3);
		uiManager->AddControl(UIWindow);
		UIWindow->Theme.Default.BackgroundColor = olc::Pixel(51, 115, 106);
		UIWindow->Theme.Default.ForegroundColor = olc::WHITE;
		UIWindow->Theme.Default.Padding = { 4,4,4,4 };
		UIWindow->Theme.Default.TextAlign = { RasputinUI::Far, RasputinUI::Far };

		Heading = uiManager->CreateControl({ {10,10}, {580,80} }, UIWindow);
		Heading->Theme.Default.BackgroundColor = olc::BLANK;
		Heading->Theme.Default.ForegroundColor = olc::RED;
		Heading->Theme.Default.TextAlign = { RasputinUI::Center,RasputinUI::Center };
		Heading->Text = "Sample Heading!";
		Heading->Theme.Default.TextScale = { 4,4 };

		ListBox1 = new RasputinUI::ListControl({ {10,250}, {250, 100} }, UIWindow, ListItemTheme);
		ListBox1->BorderColor = olc::BLACK;
		ListBox1->BorderWidth = 3;
		ListBox1->Theme.Default.Padding = { 5,5,5,5 };
		
		ListBox1->Theme.Default.BackgroundColor = olc::Pixel(0,0,0,20);
		ListBox1->ItemHeight = 20;
		uiManager->AddControl(ListBox1);
		std::vector<std::string> items;
		items.push_back("item 1");
		items.push_back("item 2");
		items.push_back("item 3");
		items.push_back("item 4");
		ListBox1->SetItems(items);

		ListBox1->SetSelection("item 3");
		
		for (int i = 0; i < 3; i++)
		{
			RasputinUI::ControlBase* textbar = new RasputinUI::ControlBase({ {10,100 + (i * 30)}, {580,30} }, TextBarTheme, UIWindow);
			textbar->Text = "This is TextBar" + std::to_string(i + 1) + "!";
			textbar->Theme.Default.TextAlign.Horizontal = textbar->Theme.Hover.TextAlign.Horizontal = (RasputinUI::Alignment)i;
		}

		for (int i = 0; i < 4; i++)
		{
			RasputinUI::ControlBase* button = uiManager->CreateControl({ { 80 + (110 * i), 400 }, {100,30} }, ButtonTheme, UIWindow);
			button->Text = std::to_string(i + 1);
			button->OnClick = std::bind(&Example::OnClick, this, std::placeholders::_1, std::placeholders::_2);
			if (i == 2)
				button->Enabled = false;
		}
	}

	void OnClick(RasputinUI::ControlBase* sender, int mbutton)
	{
		if (sender->Text.compare("1") == 0)
		{
			ListBox1->Scroll(1);
		}
		if (sender->Text.compare("2") == 0)
		{
			ListBox1->Scroll(-1);
		}
		Heading->Text = "";
		if (mbutton == 0)
			Heading->Text += "Left ";
		else if (mbutton == 1)
			Heading->Text += "Right ";
		else if (mbutton == 2)
			Heading->Text += "Middle ";
		Heading->Text += "click on ";
		Heading->Text += sender->Text;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		uiManager->UpdateUI(fElapsedTime);
		return true;
	}
};

int main()
{
	Example demo;
	if (demo.Construct(640,480,1,1))
		demo.Start();
	return 0;
}

