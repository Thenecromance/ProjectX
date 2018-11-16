/* MenuClass.cpp */
//#include "stdafx.h"
#include "../internal_native.h"
#include "../../../SDK/inc/enums.h"
#include "../../Utility/General.h"

#include "menu.h"
#include "menucontrols.h"
#include "menuutils.h"
#include "Scaleform.h"
#include <locale>
#include <map>
#pragma region Set UTF-8
#pragma once
#pragma execution_character_set("utf-8")
#pragma endregion



// TODO: Fixes: Origin
//      - Reduce code duplication (titles, OptionPlus title)
//      - Handle Chalet London scaling

// TODO: Improvements: maybe I can do
//      - Mouse support
//      - Badges?


int gGlareHandle;
int scaleform;
float gGlareDir;
float GlareX = 1.10f;
float GlareY = 0.41f;
float Glarewidth = 0.71f;
float Glareheight = 0.73f;

namespace NativeMenu {

	Menu::Menu() { }

	Menu::~Menu() { }

	void Menu::SetFiles(const std::string &fileName) {
		settings.SetFiles(fileName);
	}
	void Menu::SaveSettings()
	{
		LOG_MENU("Save Setting");
		settings.SaveSettings(&controls, this);
	}
	void Menu::ReadSettings() {
		settings.ReadSettings(&controls, this);
		//menuX = menuX; //+ menuWidth / 2.0f;
	}

	void Menu::RegisterOnMain(std::function<void()> onMain) {
		this->onMain = onMain;
	}

	void Menu::RegisterOnExit(std::function<void()> onExit) {
		this->onExit = onExit;
	}

	bool Menu::CurrentMenu(std::string menuname) {
		if (menuname == actualmenu) return true;
		return false;
	}

	void Menu::fitTitle(std::string &title, float &newSize, float titleSize) {
		float width = getStringWidth(title, titleSize, titleFont);
		float maxWidth = menuWidth - 2.0f*menuTextMargin;
		int maxTries = 50;
		int tries = 0;
		newSize = titleSize;
		while (width > maxWidth && newSize > titleTextSize * 0.5f && tries < maxTries) {
			newSize -= 0.01f;
			width = getStringWidth(title, newSize, titleFont);
			tries++;
		}
		if (width > maxWidth) {
			auto titleLines = splitString(maxWidth, title, newSize, titleFont);
			title = "";
			for (auto line : titleLines) {
				if (line != titleLines.back())
					title += line + '\n';
				else
					title += line;
			}
		}
	}

	void Menu::Title(std::string title) {
		float newSize;
		fitTitle(title, newSize, titleTextSize);
		Title(title, textureDicts[titleTextureIndex], textureNames[titleTextureIndex], newSize);
	}

	void Menu::Title(std::string title, float customSize) {
		Title(title, textureDicts[titleTextureIndex], textureNames[titleTextureIndex], customSize);
	}

	void Menu::Title(std::string title, std::string dict, std::string texture) {
		float newSize;
		fitTitle(title, newSize, titleTextSize);
		Title(title, dict, texture, newSize);
	}

	void Menu::Title(std::string title, std::string dict, std::string texture, float customSize) {
		optioncount = 0;
		totalHeight = 0.0f;
		if (Custom)
		{
			dict = "ProjectX";
			texture = "Title";
		}
		float titletexty = menuY + totalHeight + titleTextOffset + titleTextOffset * 2.0f * (titleTextSize - customSize);
		float titley = menuY + totalHeight + titleTextureOffset;
		int newlines = 0;
		for (auto c : title) {
			if (c == '\n')
				newlines++;
		}
		for (int i = 0; i < newlines; i++) {
			titletexty -= titleHeight / 5.0f;
		}
		textDraws.push_back(
			std::bind(&Menu::drawText, this, title, titleFont, menuX, titletexty, customSize, customSize, titleTextColor, 0));
		/*	DrawGlare(menuX, titley, 0.71f, 0.73f, titleBackgroundColor);*/
		backgroundSpriteDraws.push_back(
			std::bind(&Menu::drawSprite, this, dict, texture,
				menuX, titley, menuWidth, titleHeight, 0.0f, titleBackgroundColor));
		if (!glareBool)
			GlareDraws.push_back(
				std::bind(&Menu::DrawGlare, this, menuX + glareX, titley + glareY, menuWidth + glareWidth, titleHeight + glareHeight, titleBackgroundColor)
			);
		totalHeight = titleHeight;
		headerHeight = titleHeight;
	}

	void Menu::Title(std::string title, int textureHandle) {
		float newSize;
		fitTitle(title, newSize, titleTextSize);
		Title(title, textureHandle, newSize);
	}

	void Menu::Title(std::string title, int textureHandle, float customSize) {
		optioncount = 0;
		totalHeight = 0.0f;

		float titletexty = menuY + totalHeight + titleTextOffset + titleTextOffset * 2.0f * (titleTextSize - customSize);
		float titley = menuY + totalHeight + titleTextureOffset;
		int newlines = 0;
		for (auto c : title) {
			if (c == '\n')
				newlines++;
		}
		for (int i = 0; i < newlines; i++) {
			titletexty -= titleHeight / 5.0f;
		}

		textDraws.push_back(
			std::bind(&Menu::drawText, this, title, titleFont, menuX, titletexty, customSize, customSize, titleTextColor, 0));

		float safeZone = GRAPHICS::GET_SAFE_ZONE_SIZE();
		float safeOffset = (1.0f - safeZone) * 0.5f;
		float safeOffsetX = safeOffset;


		//float ar =  GRAPHICS::_GET_ASPECT_RATIO(FALSE);
		//float ar_true =  GRAPHICS::_GET_ASPECT_RATIO(TRUE);

		//// game allows max 16/9 ratio for UI elements
		//if (ar > 16.0f / 9.0f) {
		//	titleX += (ar - 16.0f / 9.0f) / (2.0f * ar);
		//}

		//float drawWidth = menuWidth;
		//// handle multi-monitor setups
		//if (ar_true > ar) {
		//	if (ar > 16.0f / 9.0f) {
		//		titleX -= (ar - 16.0f / 9.0f) / (2.0f * ar);
		//	}

		//	titleX /= ar_true / ar;
		//	titleX += ar / ar_true;

		//	drawWidth *= ar / ar_true;
		//	safeOffsetX *= ar / ar_true;
		//}

		if (!glareBool)
			GlareDraws.push_back(
				std::bind(&Menu::DrawGlare, this, menuX + glareX, titley + glareY, menuWidth + glareWidth, titleHeight + glareHeight, titleBackgroundColor)
			);
		// We don't worry about depth since SHV draws these on top of the game anyway
		//g_D3DHook.DrawTexture(textureHandle, 0, -9999, 60,									 // handle, index, depth, time
		//	menuWidth + glareWidth + Customwidth, titleHeight / GRAPHICS::_GET_ASPECT_RATIO(FALSE), 0.5f, Customheight, // width, height, origin x, origin y
		//	titleX + safeOffsetX + CustomX, titley + safeOffset + CustomY, 0.0f, GRAPHICS::_GET_ASPECT_RATIO(FALSE), 1.0f, 1.0f, 1.0f, 1.0f);

		totalHeight = titleHeight;
		headerHeight = titleHeight;
	}

	void Menu::Subtitle(std::string subtitle) {

		float subtitleY = subtitleTextureOffset + menuY + totalHeight;
		float subtitleTextY = menuY + totalHeight;
		std::transform(subtitle.begin(), subtitle.end(), subtitle.begin(), ::toupper);
		subtitle = "~HUD_COLOUR_GOLD~" + subtitle;
		textDraws.push_back(
			std::bind(&Menu::drawText, this, subtitle, optionsFont, (menuX - menuWidth / 2.0f) + menuTextMargin, subtitleTextY, subtitleTextSize, subtitleTextSize, titleTextColor, 1)
		);

		backgroundSpriteDraws.push_back(
			std::bind(&Menu::drawSprite, this,
				Custom ? "ProjectX" : textureDicts[backgTextureIndex],
				Custom ? "Footer" : textureNames[backgTextureIndex],
				menuX, subtitleY, menuWidth, subtitleHeight, 0.0f, subtitlecolor)
		);

		totalHeight += subtitleHeight;
		headerHeight += subtitleHeight;
	}

	void Menu::Footer(Color color) {
		footerType = FooterType::Color;
		footerColor = color;
	}

	void Menu::Footer(std::string dict, std::string texture) {
		footerType = FooterType::Sprite;
		footerSprite.Dictionary = dict;
		footerSprite.Name = texture;
	}
	bool Menu::Option(std::string option, std::vector<std::string> details, bool isTranslate, bool isprivate) {
		//TODO: complete Translate 

		std::string optiontext = /*isTranslate ? getTranslateion(option) :*/ option;
		if (isprivate)optiontext = "~HUD_COLOUR_GOLD~[VIP]~w~ " + optiontext;
		//std::string optiondetail = isTranslate? 
		return Option(optiontext, optionsBackgroundSelectColor, details, isprivate);
	}

	bool Menu::Option(std::string option, Color highlight, std::vector<std::string> details, bool isprivate) {

		optioncount++;

		bool highlighted = currentoption == optioncount;


		bool visible = false;
		float optiony;
		float optiontexty;

		if (currentoption <= maxDisplay && optioncount <= maxDisplay) {
			visible = true;
			optiontexty = menuY + totalHeight;
			optiony = optiontexty + optionTextureOffset;
		}
		else if (optioncount > currentoption - maxDisplay && optioncount <= currentoption) {
			visible = true;
			optiontexty = menuY + headerHeight + (optioncount - (currentoption - maxDisplay + 1)) * optionHeight;
			optiony = optiontexty + optionTextureOffset;
		}

		if (visible) {
			const float big_ass_Chalet_London_mult = optionsFont == 0 ? 0.75f : 1.0f;
			bool appendDots = false;
			while (getStringWidth(option, optionTextSize * big_ass_Chalet_London_mult, optionsFont) > (menuWidth - 2.0f*menuTextMargin)) {
				option.pop_back();
				appendDots = true;
			}
			if (appendDots) {
				option.pop_back();
				option.pop_back();
				option.pop_back();
				option += "...";
			}
			textDraws.push_back(
				std::bind(&Menu::drawText, this,
					option, optionsFont, (menuX - menuWidth / 2.0f) + menuTextMargin, optiontexty, optionTextSize, optionTextSize, highlighted ? optionsTextSelectColor : optionsTextColor, 1
				)
			);

			if (highlighted) {
				highlightsSpriteDraws.push_back(
					std::bind(&Menu::drawSprite, this,
						Custom ? "ProjectX" : textureDicts[optionselectnum],
						Custom ? "OptionRect" : textureNames[optionselectnum],
						menuX, optiony, menuWidth, optionHeight, 0.0f, highlight)
				);

				if (details.size() > 0) {
					this->details = details;
				}
			}
		}

		totalHeight += optionHeight;
		return !isprivate&&optionpress && currentoption == optioncount;
	}

	bool Menu::MenuOption(std::string option, std::string menu, std::vector<std::string> details, bool isTranslate, bool isprivate) {
		Option(option, details, isTranslate, isprivate);
		float indicatorHeight = totalHeight - optionHeight; // why the hell was this menu designed like *this*?
		bool highlighted = currentoption == optioncount;

		if (currentoption <= maxDisplay && optioncount <= maxDisplay) {
			/*	foregroundSpriteCalls.push_back(
					std::bind(&Menu::drawSprite, this,
						Custom ? "ProjectX" : "commonmenu",
						Custom ? "Menu" : "arrowright",
						menuX + menuWidth / 2.0f - optionRightMargin,
						indicatorHeight + menuY + 0.018f,
						0.020f, 0.020f, 0.0f, highlighted ? menuOptionSelect : menuOption));*/
			textDraws.push_back(
				std::bind(&Menu::drawText, this,
					"~b~>~w~>", optionsFont,
					menuX + menuWidth / 2.0f - optionRightMargin,
					indicatorHeight + menuY,
					optionTextSize, optionTextSize,
					highlighted ? optionsTextSelectColor : optionsTextColor, 2
				));
		}
		else if ((optioncount > (currentoption - maxDisplay)) && optioncount <= currentoption) {
			//foregroundSpriteCalls.push_back(
				//std::bind(&Menu::drawSprite, this,
				//	Custom ? "ProjectX" : "commonmenu",
				//	Custom ? "Menu" : "arrowright",
				//	menuX + menuWidth / 2.0f - optionRightMargin,
				//	menuY + headerHeight + (optioncount - (currentoption - maxDisplay + 1)) * optionHeight + 0.018f,
				//	0.020f, 0.020f, 0.0f, highlighted ? menuOptionSelect : menuOption));
			textDraws.push_back(
				std::bind(&Menu::drawText, this,
					"~b~>~w~>", optionsFont,
					menuX + menuWidth / 2.0f - optionRightMargin,
					menuY + headerHeight + (optioncount - (currentoption - maxDisplay + 1)) * optionHeight,
					optionTextSize, optionTextSize,
					highlighted ? optionsTextSelectColor : optionsTextColor, 2
				));
		}

		if (isprivate&&optionpress)return false;
		if (optionpress && currentoption == optioncount) {
			optionpress = false;
			changeMenu(menu);
			return true;
		}
		return false;
	}

	bool Menu::OptionPlus(std::string option, std::vector<std::string> &extra, bool *_highlighted,
		std::function<void() > onRight, std::function<void() > onLeft,
		std::string title, std::vector<std::string> details) {
		Option(option, details);
		size_t infoLines = extra.size();
		bool highlighted = currentoption == optioncount;
		if (_highlighted != nullptr) {
			*_highlighted = highlighted;
		}

		if (currentoption == optioncount) {
			if (onLeft && leftpress) {
				onLeft();
				leftpress = false;
				return false;
			}
			if (onRight && rightpress) {
				onRight();
				rightpress = false;
				return false;
			}
		}

		if (highlighted && infoLines > 0 &&
			((currentoption <= maxDisplay && optioncount <= maxDisplay) ||
			((optioncount > (currentoption - maxDisplay)) && optioncount <= currentoption))) {
			drawOptionPlusExtras(extra, title);
		}

		return optionpress && currentoption == optioncount;
	}

	void Menu::OptionPlusPlus(std::vector<std::string> &extra, std::string title) {
		drawOptionPlusExtras(extra, title);
	}

	bool Menu::IntOption(std::string option, int &var, int min, int max, int step, std::vector<std::string> details, bool isTranslate, bool isprivate) {
		std::string printVar = std::to_string(var);

		Option(option, details, isTranslate, isprivate);
		bool highlighted = currentoption == optioncount;

		drawOptionValue(printVar, highlighted, max - min);
		return processOptionItemControls(var, min, max, step,isprivate);
	}

	bool Menu::FloatOption(std::string option, float &var, float min, float max, float step, std::vector<std::string> details, bool isTranslate, bool isprivate) {
		unsigned precision = behindDec(step);
		if (precision < 2) precision = 2;
		if (precision > 6) precision = 6;

		char buf[100];
		_snprintf_s(buf, sizeof(buf), "%.*f", precision, var);
		std::string printVar = buf;
		int items = min != max ? 1 : 0;

		Option(option, details, isTranslate, isprivate);
		bool highlighted = currentoption == optioncount;

		drawOptionValue(printVar, highlighted, items);
		return processOptionItemControls(var, min, max, step);
	}

	bool Menu::BoolOption(std::string option, bool &var, std::vector<std::string> details, bool isTranslate, bool isprivate) {
		Option(option, details, isTranslate, isprivate);
		float indicatorHeight = totalHeight - optionHeight;
		bool highlighted = currentoption == optioncount;

		char *tickBoxTexture;
		Color optionColors = optionsTextColor;
		float boxSz = 0.03f;

		if (highlighted) {
			if (Custom) { tickBoxTexture = var ? "on" : "off"; }
			else
			{
				tickBoxTexture = var ? "shop_box_tick" : "shop_box_blank";
			}
		}
		else {
			if (Custom) { tickBoxTexture = var ? "on" : "off"; }
			else
			{
				tickBoxTexture = var ? "shop_box_tick" : "shop_box_blank";
			}
		}

		bool doDraw = false;
		float textureY;

		if (currentoption <= maxDisplay && optioncount <= maxDisplay) {
			doDraw = true;
			textureY = (indicatorHeight + (menuY + 0.016f));
		}
		else if ((optioncount > (currentoption - maxDisplay)) && optioncount <= currentoption) {
			doDraw = true;
			textureY = menuY + headerHeight + (optioncount - (currentoption - maxDisplay + 1)) * optionHeight + 0.016f;
		}

		if (doDraw) {
			int resX, resY;
			GRAPHICS::_GET_ACTIVE_SCREEN_RESOLUTION(&resX, &resY);
			float ratio = (float)resX / (float)resY;
			foregroundSpriteCalls.push_back(
				std::bind(&Menu::drawSprite, this, Custom ? "ProjectX" : "commonmenu", tickBoxTexture,
					menuX + menuWidth / 2.0f - optionRightMargin, textureY, boxSz / ratio, boxSz, 0.0f, optionColors)
			);
		}
		if (isprivate&&optionpress)
		{
			showNotification("this is my personal Options");
			return false;
		}
		if (optionpress && currentoption == optioncount) {
			var ^= 1;
			return true;
		}
		return false;
	}

	bool Menu::BoolSpriteOption(std::string option, bool enabled, std::string category, std::string spriteOn, std::string spriteOff, std::vector<std::string> details) {
		Option(option, details);
		float indicatorHeight = totalHeight - optionHeight;
		bool highlighted = currentoption == optioncount;

		if (currentoption <= maxDisplay && optioncount <= maxDisplay) {
			foregroundSpriteCalls.push_back(
				std::bind(&Menu::drawSprite, this, category, enabled ? spriteOn : spriteOff,
					menuX + menuWidth / 2.0f - optionRightMargin,
					(indicatorHeight + (menuY + 0.016f)),
					0.03f, 0.05f, 0.0f, highlighted ? optionsTextSelectColor : optionsTextColor));
		}
		else if ((optioncount > (currentoption - maxDisplay)) && optioncount <= currentoption) {
			foregroundSpriteCalls.push_back(
				std::bind(&Menu::drawSprite, this, category, enabled ? spriteOn : spriteOff,
					menuX + menuWidth / 2.0f - optionRightMargin,
					menuY + headerHeight + (optioncount - (currentoption - maxDisplay + 1)) * optionHeight + 0.016f,
					0.03f, 0.05f, 0.0f, highlighted ? optionsTextSelectColor : optionsTextColor));
		}

		return optionpress && currentoption == optioncount;
	}

	bool Menu::IntArray(std::string option, std::vector<int> display, int &iterator, std::vector<std::string> details, bool isTranslate, bool isprivate) {
		std::string printVar = std::to_string(display[iterator]);

		Option(option, details, isTranslate, isprivate);
		bool highlighted = currentoption == optioncount;

		int min = 0;
		int max = static_cast<int>(display.size()) - 1;

		drawOptionValue(printVar, highlighted, max);
		return processOptionItemControls(iterator, min, max, 1);
	}

	bool Menu::FloatArray(std::string option, std::vector<float> display, int &iterator, std::vector<std::string> details,  bool isTranslate, bool isprivate) {
		Option(option, details, isTranslate, isprivate);
		bool highlighted = currentoption == optioncount;
		int min = 0;
		int max = static_cast<int>(display.size()) - 1;

		if (iterator > display.size() || iterator < 0) {
			drawOptionValue("error", highlighted, max);
			return false;
		}

		unsigned precision = behindDec(display[iterator]);
		if (precision < 2) precision = 2;
		if (precision > 6) precision = 6;

		char buf[100];
		_snprintf_s(buf, sizeof(buf), "%.*f", precision, display[iterator]);
		std::string printVar = buf;

		drawOptionValue(printVar, highlighted, max);
		return processOptionItemControls(iterator, min, max, 1);
	}

	bool Menu::StringArray(std::string option, std::vector<std::string>display, int &iterator, std::vector<std::string> details,  bool isTranslate, bool isprivate) {
		Option(option, details, isTranslate, isprivate);
		bool highlighted = currentoption == optioncount;
		int min = 0;
		int max = static_cast<int>(display.size()) - 1;

		if (iterator > display.size() || iterator < 0) {
			drawOptionValue("error", highlighted, max);
			return false;
		}

		std::string printVar;
		printVar = UI::_GET_LABEL_TEXT((char*)display[iterator].c_str());
		if (printVar == "NULL")
			printVar = display[iterator];
		drawOptionValue(printVar, highlighted, max);
		return processOptionItemControls(iterator, min, max, 1);
	}

	void Menu::drawInstructionalButtons() {
		//std::vector<InstructionalButton> instructionalButtons;
		Scaleform instructionalButtonsScaleform("instructional_buttons");

		instructionalButtonsScaleform.CallFunction("CLEAR_ALL");
		instructionalButtonsScaleform.CallFunction("TOGGLE_MOUSE_BUTTONS", { 0 });
		instructionalButtonsScaleform.CallFunction("CREATE_CONTAINER");
		instructionalButtonsScaleform.CallFunction("SET_DATA_SLOT", { 0, std::string(CONTROLS::GET_CONTROL_INSTRUCTIONAL_BUTTON(2, ControlHUDSpecial, 0)), std::string(UI::_GET_LABEL_TEXT("Help")) });
		instructionalButtonsScaleform.CallFunction("SET_DATA_SLOT", { 0, std::string(CONTROLS::GET_CONTROL_INSTRUCTIONAL_BUTTON(2, INPUT_CELLPHONE_DOWN, 0)), std::string(UI::_GET_LABEL_TEXT("HUD_INPUT2")) });
		instructionalButtonsScaleform.CallFunction("SET_DATA_SLOT", { 1, std::string(CONTROLS::GET_CONTROL_INSTRUCTIONAL_BUTTON(2, ControlPhoneCancel, 0)), std::string(UI::_GET_LABEL_TEXT("HUD_INPUT3")) });
		instructionalButtonsScaleform.CallFunction("SET_DATA_SLOT", { 2, std::string(CONTROLS::GET_CONTROL_INSTRUCTIONAL_BUTTON(2, VehicleColorHunterGreen, 0)), std::string("Next Option") });
		instructionalButtonsScaleform.CallFunction("SET_DATA_SLOT", { 3, std::string(CONTROLS::GET_CONTROL_INSTRUCTIONAL_BUTTON(2, VehicleColorOrange, 0)), std::string("Previous option") });

		//int count = 2;
		//for (auto button : instructionalButtons) {
		//    if (button.ItemBind == null || MenuItems[CurrentSelection] == button.ItemBind) {
		//        instructionalButtonsScaleform.CallFunction("SET_DATA_SLOT", { count, button.GetButtonId(), button.Text });
		//        count++;
		//    }
		//}

		instructionalButtonsScaleform.CallFunction("DRAW_INSTRUCTIONAL_BUTTONS", { -1 });
		instructionalButtonsScaleform.Render2D();
	}

	void Menu::drawMenuDetails() {
		int maxOptionCount = optioncount > maxDisplay ? maxDisplay : optioncount;

		float footerTextY;
		float footerBackY;

		if (optioncount > maxDisplay) {
			footerTextY = maxDisplay * optionHeight + menuY + headerHeight;
		}
		else {
			footerTextY = totalHeight + menuY;
		}
		footerBackY = footerTextY + optionTextureOffset;

		switch (footerType)
		{
		case FooterType::Color:
		{
			backgroundRectDraws.push_back(
				std::bind(&Menu::drawRect, this,
					menuX, footerBackY, menuWidth, optionHeight, footerColor)
			);
			break;
		}
		case FooterType::Sprite:
		{

			backgroundSpriteDraws.push_back(
				std::bind(&Menu::drawSprite, this, Custom ? "ProjectX" : textureDicts[backgTextureIndex], Custom ? "Footer" : textureNames[backgTextureIndex],
					menuX, footerBackY, menuWidth, optionHeight, 0.0f, titleBackgroundColor)
			);
			break;
		}
		default: {
			backgroundRectDraws.push_back(
				std::bind(&Menu::drawRect, this,
					menuX, footerBackY, menuWidth, optionHeight, solidBlack)
			);
			break;
		}
		}

		textDraws.push_back(
			std::bind(&Menu::drawText, this, std::to_string(currentoption) + " / " + std::to_string(optioncount),
				optionsFont, menuX - 0.1f, footerTextY, optionTextSize, optionTextSize, titleTextColor, 2)
		);

		// Options background
		backgroundSpriteDraws.push_back(
			std::bind(&Menu::drawSprite, this,
				Custom ? "ProjectX" : textureDicts[backgTextureIndex],
				Custom ? "option" : textureNames[backgTextureIndex],
				menuX,
				menuY + headerHeight + maxOptionCount * optionHeight / 2,
				menuWidth,
				optionHeight * maxOptionCount, 0.0f, optionsBackgroundColor)
		);

		// Menu detail box
		if (details.size() > 0) {
			drawMenuDetails(details, footerBackY + optionHeight / 1.5f);
		}

		// Indicators
		if (currentoption == 1) {
			foregroundSpriteCalls.push_back(
				std::bind(&Menu::drawSprite, this, "commonmenu", "arrowright",
					menuX,
					(footerTextY + 0.0175f),
					0.02f, 0.02f, 90.0f, titleTextColor)
			);
		}
		else if (currentoption == optioncount) {
			foregroundSpriteCalls.push_back(
				std::bind(&Menu::drawSprite, this, "commonmenu", "arrowright",
					menuX,
					(footerTextY + 0.0175f),
					0.02f, 0.02f, 270.0f, titleTextColor)
			);
		}
		else {
			foregroundSpriteCalls.push_back(
				std::bind(&Menu::drawSprite, this, "commonmenu", "arrowright",
					menuX,
					(footerTextY + 0.0125f),
					0.02f, 0.02f, 270.0f, titleTextColor)
			);
			foregroundSpriteCalls.push_back(
				std::bind(&Menu::drawSprite, this, "commonmenu", "arrowright",
					menuX,
					(footerTextY + 0.0225f),
					0.02f, 0.02f, 90.0f, titleTextColor)
			);
		}
	}

	void Menu::EndMenu() {
		if (menulevel < 1)
			return;

		drawMenuDetails();

		GRAPHICS::_SCREEN_DRAW_POSITION_BEGIN(76, 84);
		GRAPHICS::_SCREEN_DRAW_POSITION_RATIO(0, 0, 0, 0);
		//for (auto f : backgroundglareDraws) { f(); }

		for (auto f : backgroundSpriteDraws) { f(); }
		for (auto f : backgroundRectDraws) { f(); }
		for (auto f : highlightsSpriteDraws) { f(); }
		for (auto f : foregroundSpriteCalls) { f(); }
		for (auto f : spriteDraws) { f(); }
		for (auto f : textDraws) { f(); }
		for (auto f : GlareDraws) { f(); }

		GRAPHICS::_SCREEN_DRAW_POSITION_END();
		GlareDraws.clear();
		backgroundSpriteDraws.clear();
		backgroundRectDraws.clear();
		spriteDraws.clear();
		highlightsSpriteDraws.clear();
		foregroundSpriteCalls.clear();
		textDraws.clear();
		details.clear();
		footerType = FooterType::Sprite;

		//hideHUDComponents();
		disableKeys();
		drawInstructionalButtons();
	}

	void Menu::CheckKeys() {
		if (cheatString != "") {
			if (GAMEPLAY::_HAS_CHEAT_STRING_JUST_BEEN_ENTERED(GAMEPLAY::GET_HASH_KEY((char*)cheatString.c_str()))) {
				OpenMenu();
			}
		}
		controls.Update();
		optionpress = false;

		if (GetTickCount() - delay > menuTime ||
			controls.IsKeyJustPressed(MenuControls::MenuKey) ||
			controls.IsKeyJustPressed(MenuControls::MenuSelect) || useNative && CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(0, ControlPhoneSelect) ||
			controls.IsKeyJustPressed(MenuControls::MenuCancel) || useNative && CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(0, ControlPhoneCancel) ||
			controls.IsKeyJustPressed(MenuControls::MenuUp) || useNative && CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(0, ControlFrontendUp) ||
			controls.IsKeyJustPressed(MenuControls::MenuDown) || useNative && CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(0, ControlFrontendDown) ||
			controls.IsKeyJustPressed(MenuControls::MenuLeft) || useNative && CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(0, ControlPhoneLeft) ||
			controls.IsKeyJustPressed(MenuControls::MenuRight) || useNative && CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(0, ControlPhoneRight)) {

			processMenuNav(onMain, onExit);
		}

		if (controls.IsKeyJustReleased(MenuControls::MenuKey) || controls.IsKeyJustPressed(MenuControls::MenuKey) ||
			controls.IsKeyJustReleased(MenuControls::MenuSelect) || controls.IsKeyJustPressed(MenuControls::MenuSelect) ||
			controls.IsKeyJustReleased(MenuControls::MenuCancel) || controls.IsKeyJustPressed(MenuControls::MenuCancel) ||
			controls.IsKeyJustReleased(MenuControls::MenuUp) || controls.IsKeyJustPressed(MenuControls::MenuUp) ||
			controls.IsKeyJustReleased(MenuControls::MenuDown) || controls.IsKeyJustPressed(MenuControls::MenuDown) ||
			controls.IsKeyJustReleased(MenuControls::MenuLeft) || controls.IsKeyJustPressed(MenuControls::MenuLeft) ||
			controls.IsKeyJustReleased(MenuControls::MenuRight) || controls.IsKeyJustPressed(MenuControls::MenuRight) ||
			CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlPhoneSelect) || CONTROLS::IS_DISABLED_CONTROL_JUST_RELEASED(0, ControlPhoneSelect) ||
			CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlPhoneCancel) || CONTROLS::IS_DISABLED_CONTROL_JUST_RELEASED(0, ControlPhoneCancel) ||
			CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendUp) || CONTROLS::IS_DISABLED_CONTROL_JUST_RELEASED(0, ControlFrontendUp) ||
			CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendDown) || CONTROLS::IS_DISABLED_CONTROL_JUST_RELEASED(0, ControlFrontendDown) ||
			CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlPhoneLeft) || CONTROLS::IS_DISABLED_CONTROL_JUST_RELEASED(0, ControlPhoneLeft) ||
			CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlPhoneRight) || CONTROLS::IS_DISABLED_CONTROL_JUST_RELEASED(0, ControlPhoneRight)) {
			menuTime = menuTimeRepeat;
		}

		for (int i = 1; i < menuTimeDelays.size(); i++) {
			if (controls.IsKeyDownFor(MenuControls::MenuUp, (i + 1) * menuTimeRepeat) || controls.IsControlDownFor(ControlFrontendUp, (i + 1) * menuTimeRepeat) ||
				controls.IsKeyDownFor(MenuControls::MenuDown, (i + 1) * menuTimeRepeat) || controls.IsControlDownFor(ControlFrontendDown, (i + 1) * menuTimeRepeat) ||
				controls.IsKeyDownFor(MenuControls::MenuLeft, (i + 1) * menuTimeRepeat) || controls.IsControlDownFor(ControlFrontendLeft, (i + 1) * menuTimeRepeat) ||
				controls.IsKeyDownFor(MenuControls::MenuRight, (i + 1) * menuTimeRepeat) || controls.IsControlDownFor(ControlFrontendRight, (i + 1) * menuTimeRepeat)) {
				menuTime = menuTimeDelays[i];
			}
		}


		if (!useNative && GetTickCount() - delay > 1000) {
			useNative = true;
		}

	}

	void Menu::OpenMenu() {
		if (menulevel == 0) {
			changeMenu("mainmenu");
			updateScreenSize();
			if (onMain) {
				onMain();
			}
			//instructionalButtonsScaleform.Request("instructional_buttons");
			//while (!instructionalButtonsScaleform.IsLoaded()) WAIT(0);
		}
	}

	void Menu::CloseMenu() {
		while (menulevel > 0) {
			backMenu();
		}
		if (onExit) onExit();
	}

	const MenuControls &Menu::GetControls() {
		return controls;
	}

	bool Menu::IsThisOpen() {
		return menulevel > 0;
	}

	float Menu::getStringWidth(std::string text, float scale, int font) {
		UI::_BEGIN_TEXT_COMMAND_WIDTH("STRING");
		UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME((char *)text.c_str());
		UI::SET_TEXT_FONT(font);
		UI::SET_TEXT_SCALE(scale, scale);
		return  UI::_END_TEXT_COMMAND_GET_WIDTH(true);
	}

	std::vector<std::string> Menu::splitString(float maxWidth, std::string &details, float scale, int font) {
		std::vector<std::string> splitLines;

		std::vector<std::string> words = split(details, ' ');

		std::string line;
		for (std::string word : words) {
			float lineWidth = getStringWidth(line, scale, font);
			float wordWidth = getStringWidth(word, scale, font);
			if (lineWidth + wordWidth > maxWidth) {
				splitLines.push_back(line);
				line.clear();
			}
			line += word + ' ';
			if (word == words.back()) {
				splitLines.push_back(line);
			}
		}

		return splitLines;
	}

	void Menu::drawText(const std::string text, int font, float x, float y, float pUnknown, float scale, Color color, int justify) {
		// justify: 0 - center, 1 - left, 2 - right
		if (justify == 2) {
			UI::SET_TEXT_WRAP(menuX - menuWidth / 2, menuX + menuWidth / 2 - optionRightMargin / 2.0f);
		}
		UI::SET_TEXT_JUSTIFICATION(justify);
		UI::SET_TEXT_FONT(font);
		if (font == 0) { // big-ass Chalet London
			scale *= 0.75f;
			y += 0.003f;
		}
		UI::SET_TEXT_SCALE(0.0f, scale);
		UI::SET_TEXT_COLOUR(color.R, color.G, color.B, color.A);
		UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
		UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME((char *)text.c_str());
		UI::END_TEXT_COMMAND_DISPLAY_TEXT(x, y);
	}

	void Menu::drawRect(float x, float y, float width, float height, Color color) {
		GRAPHICS::DRAW_RECT(x, y, width, height, color.R, color.G, color.B, color.A);
	}



	void Menu::DrawGlare(float posX, float posY, float sizeX, float sizeY, Color Color)
	{

		gGlareHandle = GRAPHICS::REQUEST_SCALEFORM_MOVIE("MP_MENU_GLARE");
		// Get Player Camera Rotation
		Vector3 rot = CAM::_GET_GAMEPLAY_CAM_ROT(2);
		// Calculate Glare Rotation
		float dir = Utility::conv360(rot.z, 0, 360);
		// Check if custom rotation is necessary for the rotation component.
		if ((gGlareDir == 0 || gGlareDir - dir > 0.5) || gGlareDir - dir < -0.5)
		{
			// Set global variable to current direction
			gGlareDir = dir;
			// Open Data Slot Function
			GRAPHICS::BEGIN_SCALEFORM_MOVIE_METHOD(gGlareHandle, "SET_DATA_SLOT");
			// Set Data Slot Value/Parameter
			GRAPHICS::_ADD_SCALEFORM_MOVIE_METHOD_PARAMETER_FLOAT(gGlareDir);
			// Close Data Slot Function
			GRAPHICS::END_SCALEFORM_MOVIE_METHOD();
		}
		// Draw Scaleform
		//GRAPHICS::_SET_SCREEN_DRAW_POSITION(76, 84);
		//GRAPHICS::_0x76C641E4(-0.0755f, -0.0755f, 0.f, 0.f);
		GRAPHICS::DRAW_SCALEFORM_MOVIE(
			gGlareHandle, posX + 0.36f, posY + 0.44f, sizeX + 0.91f, sizeY + 0.99f, 255, 255, 255, 255, 0
			/*gGlareHandle, GlareX=1.10f, GlareY=0.41f, Glarewidth=0.71f, Glareheight=0.73f, r, g, b, 255, 0*/
		);
		GRAPHICS::_SCREEN_DRAW_POSITION_END();

		//GRAPHICS::smile:RAW_SCALEFORM_MOVIE_FULLSCREEN(gGlareHandle, 255, 255, 255, 255, 0);
	}

	void Menu::drawSprite(std::string textureDict, std::string textureName, float x, float y, float width, float height, float rotation, Color color) {
		if (!GRAPHICS::HAS_STREAMED_TEXTURE_DICT_LOADED((char *)textureDict.c_str())) {
			GRAPHICS::REQUEST_STREAMED_TEXTURE_DICT((char *)textureDict.c_str(), false);
		}
		else {
			GRAPHICS::DRAW_SPRITE((char *)textureDict.c_str(), (char *)textureName.c_str(), x, y, width, height, rotation, color.R, color.G, color.B, color.A);
		}

	}

	void Menu::drawOptionPlusTitle(std::string title) {
		float newSize;
		fitTitle(title, newSize, titleTextSize);

		float extrax = menuX + menuWidth;

		float titletexty = menuY + titleTextOffset;
		float titley = menuY + titleTextureOffset;

		textDraws.push_back(
			std::bind(&Menu::drawText, this, title, titleFont, extrax, titletexty, newSize, newSize, titleTextColor, 0)
		);

		backgroundSpriteDraws.push_back(
			std::bind(&Menu::drawSprite, this, Custom ? "ProjectX" : textureDicts[titleTextureIndex], Custom ? "Title" : textureNames[titleTextureIndex],
				extrax, titley, menuWidth, titleHeight, 0.0f, titleBackgroundColor)
		);

	}

	void Menu::drawOptionPlusImage(std::string &extra, float &finalHeight) {
		int imgHandle;
		int imgWidth;
		int imgHeight;
		std::string scanFormat = ImagePrefix + "%dW%dH%d";
		int nParams = sscanf_s(extra.c_str(), scanFormat.c_str(), &imgHandle, &imgWidth, &imgHeight);
		if (nParams != 3) {
			std::string errTxt = "Format error: " + extra;
			textDraws.push_back(
				std::bind(&Menu::drawText, this,
					errTxt, optionsFont, menuX + menuWidth / 2.0f + menuTextMargin, finalHeight + (menuY + headerHeight), optionTextSize, optionTextSize, optionsTextColor, 1));
			finalHeight += optionHeight;
			return;
		}
		float drawWidth = menuWidth - 2.0f * menuTextMargin;
		float drawHeight = (float)imgHeight * (drawWidth / (float)imgWidth);
		float imgXpos = (menuX + menuWidth / 2.0f + menuTextMargin);
		float imgYpos = finalHeight + (menuY + headerHeight) + menuTextMargin;

		float safeZone = GRAPHICS::GET_SAFE_ZONE_SIZE();
		float safeOffset = (1.0f - safeZone) * 0.5f;
		float safeOffsetX = safeOffset;

		float ar = GRAPHICS::_GET_ASPECT_RATIO(FALSE);

		// game allows max 16/9 ratio for UI elements
		if (ar > 16.0f / 9.0f) {
			imgXpos += (ar - 16.0f / 9.0f) / (2.0f * ar);
		}


		// handle multi-monitor setups
		float ar_true = GRAPHICS::_GET_ASPECT_RATIO(TRUE);
		if (ar_true > ar) {
			if (ar > 16.0f / 9.0f) {
				imgXpos -= (ar - 16.0f / 9.0f) / (2.0f * ar);
			}

			imgXpos /= ar_true / ar;
			imgXpos += ar / ar_true;

			drawWidth *= ar / ar_true;
			safeOffsetX *= ar / ar_true;
		}

		//drawTexture(imgHandle, 0, -9999, 60,                            // handle, index, depth, time
		//            drawWidth, drawHeight, 0.0f, 0.0f,                  // width, height, origin x, origin y
		//            imgXpos + safeOffsetX, imgYpos + safeOffset, 0.0f,   // pos x, pos y, rot
		//            ar, 1.0f, 1.0f, 1.0f, 1.0f);                        // screen correct, rgba
		finalHeight += drawHeight * ar + 2.0f * menuTextMargin;
	}

	void Menu::drawOptionPlusSprite(std::string &extra, float &finalHeight) {
		const unsigned max_sz = 128;
		char dict[max_sz];
		char name[max_sz];
		int imgWidth;
		int imgHeight;
		std::string scanFormat = SpritePrefix + "%s %s W%dH%d";
		int nParams = sscanf_s(extra.c_str(), scanFormat.c_str(), dict, max_sz, name, max_sz, &imgWidth, &imgHeight);
		if (nParams != 4) {
			std::string errTxt = "Format error: " + extra;
			textDraws.push_back(
				std::bind(&Menu::drawText, this,
					errTxt, optionsFont, menuX + menuWidth / 2.0f + menuTextMargin, finalHeight + (menuY + headerHeight), optionTextSize, optionTextSize, optionsTextColor, 1));
			finalHeight += optionHeight;
			return;
		}
		float drawWidth = menuWidth - 2.0f * menuTextMargin;
		float drawHeight = (float)imgHeight * (drawWidth / (float)imgWidth) * GRAPHICS::_GET_ASPECT_RATIO(FALSE);
		float imgXpos = menuX + menuWidth / 2.0f + drawWidth / 2.0f + menuTextMargin;
		float imgYpos = finalHeight + drawHeight / 2.0f + (menuY + headerHeight) + menuTextMargin;

		foregroundSpriteCalls.push_back(
			std::bind(&Menu::drawSprite, this, std::string(dict), std::string(name),
				imgXpos, imgYpos, drawWidth, drawHeight, 0.0f, titleTextColor)
		);

		finalHeight += drawHeight + 2.0f * menuTextMargin;
	}

	void Menu::drawOptionPlusText(std::string &extra, float &finalHeight) {
		std::vector<std::string> splitExtra;
		const float big_ass_Chalet_London_mult = optionsFont == 0 ? 0.75f : 1.0f;
		auto splitLines = splitString(menuWidth, extra, optionTextSize * big_ass_Chalet_London_mult, optionsFont);
		splitExtra.insert(std::end(splitExtra), std::begin(splitLines), std::end(splitLines));

		for (auto line = 0; line < splitExtra.size(); line++) {
			textDraws.push_back(
				std::bind(&Menu::drawText, this,
					splitExtra[line], optionsFont,
					menuX + menuWidth / 2.0f + menuTextMargin, finalHeight + (menuY + headerHeight) + line * optionHeight,
					optionTextSize, optionTextSize,
					optionsTextColor, 1));
		}
		finalHeight += splitExtra.size() * optionHeight;
	}

	void Menu::drawOptionPlusExtras(std::vector<std::string> &extras, std::string title) {
		float extrax = menuX + menuWidth;
		drawOptionPlusTitle(title);

		if (headerHeight == titleHeight + subtitleHeight) {
			float subtitleY = subtitleTextureOffset + menuY + titleHeight;
			backgroundRectDraws.push_back(
				std::bind(&Menu::drawRect, this, extrax, subtitleY, menuWidth, subtitleHeight, solidBlack)
			);
		}

		float finalHeight = 0;

		for (auto extra : extras) {
			if (extra.compare(0, ImagePrefix.size(), ImagePrefix) == 0) {
				drawOptionPlusImage(extra, finalHeight);
			}
			else if (extra.compare(0, SpritePrefix.size(), SpritePrefix) == 0) {
				drawOptionPlusSprite(extra, finalHeight);
			}
			else {
				drawOptionPlusText(extra, finalHeight);
			}
		}

		backgroundSpriteDraws.push_back(
			std::bind(&Menu::drawSprite, this, textureDicts[backgTextureIndex], textureNames[backgTextureIndex],
				extrax, menuY + headerHeight + finalHeight / 2.0f, menuWidth, finalHeight, 0.0f, optionsBackgroundColor)
		);
	}

	void Menu::drawMenuDetails(std::vector<std::string> details, float y) {
		std::vector<std::string> splitDetails;
		const float big_ass_Chalet_London_mult = optionsFont == 0 ? 0.75f : 1.0f;
		for (auto detailLine : details) {
			auto splitLines = splitString(menuWidth, detailLine, optionTextSize * big_ass_Chalet_London_mult, optionsFont);
			splitDetails.insert(std::end(splitDetails), std::begin(splitLines), std::end(splitLines));
		}

		for (auto i = 0; i < splitDetails.size(); i++) {
			textDraws.push_back(
				std::bind(&Menu::drawText, this,
					splitDetails[i], optionsFont, (menuX - menuWidth / 2.0f) + menuTextMargin, i * detailLineHeight + y, optionTextSize, optionTextSize, optionsTextColor, 1));
		}

		// The thin line
		backgroundRectDraws.push_back(std::bind(&Menu::drawRect, this,
			menuX, y, menuWidth, optionHeight / 16.0f, solidBlack));

		float boxHeight = (splitDetails.size() * detailLineHeight) + (optionHeight - detailLineHeight);

		backgroundSpriteDraws.push_back(
			std::bind(&Menu::drawSprite, this, textureDicts[backgTextureIndex], textureNames[backgTextureIndex],
				menuX, y + boxHeight / 2, menuWidth, boxHeight, 0.0f, optionsBackgroundColor));
	}

	void Menu::drawOptionValue(std::string printVar, bool highlighted, int items) {
		float indicatorHeight = totalHeight - optionHeight;

		std::string leftArrow = "<";
		std::string rightArrow = ">";
		if (items == 0) {
			leftArrow = rightArrow = "";
		}
		if (currentoption <= maxDisplay && optioncount <= maxDisplay) {
			textDraws.push_back(std::bind(&Menu::drawText, this, leftArrow + printVar + rightArrow, optionsFont,
				menuX + menuWidth / 2.0f - optionRightMargin,
				indicatorHeight + menuY,
				optionTextSize, optionTextSize,
				highlighted ? optionsTextSelectColor : optionsTextColor, 2));
		}



		else if ((optioncount > (currentoption - maxDisplay)) && optioncount <= currentoption) {
			textDraws.push_back(std::bind(&Menu::drawText, this, leftArrow + printVar + rightArrow, optionsFont,
				menuX + menuWidth / 2.0f - optionRightMargin,
				menuY + headerHeight + (optioncount - (currentoption - maxDisplay + 1)) * optionHeight,
				optionTextSize, optionTextSize,
				highlighted ? optionsTextSelectColor : optionsTextColor, 2));
		}

	}

	void Menu::changeMenu(std::string menuname) {
		currentmenu[menulevel] = actualmenu;
		lastoption[actualmenu] = currentoption;
		menulevel++;
		actualmenu = menuname;
		currentoption = getWithDef(lastoption, actualmenu, 1);
		menuBeep();
		resetButtonStates();
	}

	void Menu::nextOption() {
		if (currentoption < optioncount)
			currentoption++;
		else
			currentoption = 1;
		if (menulevel > 0) {
			menuBeep();
		}
		resetButtonStates();
	}

	void Menu::previousOption() {
		if (currentoption > 1)
			currentoption--;
		else
			currentoption = optioncount;
		if (menulevel > 0) {
			menuBeep();
		}
		resetButtonStates();
	}

	void Menu::backMenu() {
		if (menulevel > 0) {
			menuBeep();
		}
		lastoption[actualmenu] = currentoption;
		menulevel--;
		actualmenu = currentmenu[menulevel];
		currentoption = getWithDef(lastoption, actualmenu, 1);

	}

	void Menu::menuBeep() {
		AUDIO::PLAY_SOUND_FRONTEND(-1, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", 0);
	}

	void Menu::resetButtonStates() {
		optionpress = false;
		leftpress = false;
		rightpress = false;
		uppress = false;
		downpress = false;
	}

	void Menu::disableKeysOnce() {
		CAM::SET_CINEMATIC_BUTTON_ACTIVE(0);

	}

	void Menu::enableKeysOnce() {
		CAM::SET_CINEMATIC_BUTTON_ACTIVE(1);
	}

	void Menu::hideHUDComponents() {
		UI::HIDE_HELP_TEXT_THIS_FRAME();
		UI::HIDE_HUD_COMPONENT_THIS_FRAME(HudComponentVehicleName);
		UI::HIDE_HUD_COMPONENT_THIS_FRAME(HudComponentAreaName);
		UI::HIDE_HUD_COMPONENT_THIS_FRAME(HudComponentUnused);
		UI::HIDE_HUD_COMPONENT_THIS_FRAME(HudComponentStreetName);
		UI::HIDE_HUD_COMPONENT_THIS_FRAME(HudComponentHelpText);
	}

	void Menu::disableKeys() {
		disableKeysOnce();

		// sjaak327
		// http://gtaforums.com/topic/796908-simple-trainer-for-gtav/?view=findpost&p=1069587144
		CONTROLS::DISABLE_CONTROL_ACTION(2, ControlPhone, true);
		CONTROLS::DISABLE_CONTROL_ACTION(2, ControlTalk, true);
		CONTROLS::DISABLE_CONTROL_ACTION(2, ControlVehicleHeadlight, true);
		CONTROLS::DISABLE_CONTROL_ACTION(2, ControlVehicleCinCam, true);
		CONTROLS::DISABLE_CONTROL_ACTION(2, ControlVehicleRadioWheel, true);
		CONTROLS::DISABLE_CONTROL_ACTION(2, ControlMeleeAttackLight, true);
		CONTROLS::DISABLE_CONTROL_ACTION(2, ControlMeleeAttackHeavy, true);
		CONTROLS::DISABLE_CONTROL_ACTION(2, ControlMeleeAttackAlternate, true);
		CONTROLS::DISABLE_CONTROL_ACTION(2, ControlMeleeBlock, true);
		CONTROLS::DISABLE_CONTROL_ACTION(2, ControlHUDSpecial, true);
		CONTROLS::DISABLE_CONTROL_ACTION(2, ControlCharacterWheel, true);
	}

	void Menu::processMenuNav(std::function<void()> onMain, std::function<void()> onExit) {
		if (controls.IsKeyJustPressed(MenuControls::MenuSelect) ||
			controls.IsKeyJustPressed(MenuControls::MenuCancel) ||
			controls.IsKeyJustPressed(MenuControls::MenuUp) ||
			controls.IsKeyJustPressed(MenuControls::MenuDown) ||
			controls.IsKeyJustPressed(MenuControls::MenuLeft) ||
			controls.IsKeyJustPressed(MenuControls::MenuRight)) {
			useNative = false;
		}

		if (controls.IsKeyJustPressed(MenuControls::MenuKey) || useNative &&
			CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, controls.ControllerButton1) &&
			CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(0, controls.ControllerButton2)) {
			if (menulevel == 0) {
				OpenMenu();
			}
			else {
				CloseMenu();
				enableKeysOnce();
				if (onExit) {
					onExit();
				}
			}
			delay = GetTickCount();
			return;
		}
		if (controls.IsKeyJustPressed(MenuControls::MenuCancel) || useNative && CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendCancel)) {
			if (menulevel > 0) {
				if (menulevel == 1) {
					enableKeysOnce();
					if (onExit) {
						onExit();
					}
				}
				backMenu();

			}
			delay = GetTickCount();
		}
		if (controls.IsKeyJustPressed(MenuControls::MenuSelect) || useNative && CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
			if (menulevel > 0) {
				menuBeep();
			}
			optionpress = true;
			delay = GetTickCount();
		}
		if (controls.IsKeyPressed(MenuControls::MenuDown) || useNative && CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendDown)) {
			nextOption();
			delay = GetTickCount();
			downpress = true;
		}
		if (controls.IsKeyPressed(MenuControls::MenuUp) || useNative && CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendUp)) {
			previousOption();
			delay = GetTickCount();
			uppress = true;
		}
		if (controls.IsKeyPressed(MenuControls::MenuLeft) || useNative && CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlPhoneLeft)) {
			if (menulevel > 0) {
				menuBeep();
			}
			leftpress = true;
			delay = GetTickCount();
		}
		if (controls.IsKeyPressed(MenuControls::MenuRight) || useNative && CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlPhoneRight)) {
			if (menulevel > 0) {
				menuBeep();
			}
			rightpress = true;
			delay = GetTickCount();
		}
	}


	void Menu::updateScreenSize() {
		aspectR = (16.0f / 9.0f) / GRAPHICS::_GET_ASPECT_RATIO(FALSE);
		menuWidth = menuWidthOriginal * aspectR;
	}

	void Menu::Setkeys(std::string keyname, MenuControls::ControlType type)
	{
		settings.CustomKeys(keyname, type, &controls);
	}

	void Menu::DrawMarker(Vector3 Pos)
	{
		GRAPHICS::DRAW_MARKER(markerType,
			Pos.x,
			Pos.y,
			Pos.z + markerOffset,
			mDir.x,
			mDir.y,
			mDir.z,
			mRot.x,
			mRot.y,
			mRot.z,
			mScale.x,
			mScale.y,
			mScale.z,
			mRgb.R,
			mRgb.G,
			mRgb.B,
			mRgb.A,
			bobUpAndDown,
			faceCamera,
			2,
			rotate,
			0, 0,
			false
		);
	}
}
