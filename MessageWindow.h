#pragma once
#include <string>
#include <vector>
#include <raylib.h>

namespace mw
{ 
static class MessageWindow
{
public:
	std::vector<std::string> messages;
	Font font;
	int spacing = 5;
	Color cursor_color = RED;
	Color text_color = WHITE;
	Color bg_color = BLACK;
	MessageWindow() : locx(0), locy(0), width(0), height(0)
	{
		font = GetFontDefault();//LoadFontEx("C:\\Windows\\Fonts\\arial.ttf", 20, 0, 250);
	}
	MessageWindow(int loc_x, int loc_y, int _width, int _height) : locx(loc_x), locy(loc_y), width(_width), height(_height)
	{
		font = LoadFontEx("C:\\Windows\\Fonts\\arial.ttf", 100, 0, 0);
	}
	~MessageWindow() { UnloadFont(font); }
	void add_message(std::string message)
	{
		messages.push_back(message);
	}
	void append_message(std::string message)
	{
		if (messages.size() > 0)
		{
			messages.back() += message;
		}
		else
		{
			messages.push_back(message);
		}
	}
	void draw() {
		DrawRectangle(locx, locy, width, height, bg_color);

		// Start Y position from the bottom of the window
		int currentY = height;

		for (auto it = messages.rbegin(); it != messages.rend(); ++it) {
			const std::string& message = *it;

			int fontSize = font_size(message);
			Vector2 textSize = MeasureTextEx(font, message.c_str(), fontSize, 1);

			// Update Y position to draw the text (subtract text height)
			currentY -= (int)textSize.y;

			// Check if the message fits vertically in the window
			if (currentY < 0) {
				break; // Stop drawing if there is no more vertical space
			}

			Vector2 textPosition = { locx, (float)currentY };
			DrawTextEx(font, message.c_str(), textPosition, fontSize, spacing, text_color);
			// draw the cursor as a line at the index of the cursor
			// if not last string in vector
			if (it == messages.rbegin())
				if (cursor >= 0 && cursor <= message.length())
				{
					Vector2 cursorPosition = { locx + MeasureTextEx(font, message.substr(0, cursor).c_str(), fontSize, spacing).x, (float)currentY };
					DrawLineV(cursorPosition, { cursorPosition.x, cursorPosition.y + textSize.y }, cursor_color);
				}

		}
	}

	void replace_back(std::string message)
	{
		if (messages.size() > 0)
		{
			messages.back() = message;
		}
		else
		{
			messages.push_back(message);
		}
	}

	void set_cursor(int _cursor)
	{
		cursor = _cursor;
	}

private:
	int locx, locy;
	int width, height;
	int cursor = 0;

	int font_size1(std::string text) {
		int fontSize = width; // Start with the maximum possible font size
		int textWidth = MeasureText(text.c_str(), fontSize);

		// Reduce font size until the text fits within the window width
		while (textWidth > width && fontSize > 1) {
			fontSize--;
			textWidth = MeasureText(text.c_str(), fontSize);
		}

		return fontSize;
	}

	int font_size(std::string text) {
		int fontSize = 50; // Start with a reasonable font size
		Vector2 textSize = MeasureTextEx(font, &text.front(), fontSize, spacing);

		// Reduce font size until the text width fits within the window width
		while (textSize.x > width && fontSize > 1) {
			fontSize--;
			textSize = MeasureTextEx(font, text.c_str(), fontSize, spacing);
		}

		return fontSize;
	}
};

}
