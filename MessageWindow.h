#pragma once
#include <string>
#include <vector>
#include <raylib.h>
#include <sstream>
#include <iostream>

namespace mw
{ 
	class MessageWindow
	{
	public:
		std::vector<std::string> messages;
		Font font;
		int spacing = 5;
		Color cursor_color = SKYBLUE;
		Color text_color = WHITE;
		Color bg_color = BLACK;

		// Delete copy constructor and assignment operator
		MessageWindow(const MessageWindow&) = delete;
		MessageWindow& operator=(const MessageWindow&) = delete;

		static MessageWindow& getInstance() {
			static MessageWindow instance; // Guaranteed to be destroyed and instantiated on first use
			return instance;
		}

		void set_loc(int loc_x, int loc_y)
		{
			locx = loc_x;
			locy = loc_y;
		}

		void set_width(int _width)
		{
			width = _width;
		}

		void set_height(int _height)
		{
			height = _height;
		}

		void add_message(std::string message) {
			std::istringstream stream(message);
			std::string line;
			while (std::getline(stream, line)) {
				messages.push_back(line);
			}
		}


		void print(std::string message)
		{
			add_message(message);
			add_message(" ");
			//add_message(" ");
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

			int currentY = locy + height;
			std::vector<std::string>::reverse_iterator it = messages.rbegin();
			std::vector<std::ptrdiff_t> indicesToRemove;
//#pragma omp parallel for ordered schedule(dynamic)
			while (it != messages.rend()) {
				std::string message = *it;
				if (message.empty()) message = " ";

				int fontSize = font_size(message);
				Vector2 textSize = MeasureTextEx(font, message.c_str(), (float)fontSize, (float)spacing);

				currentY -= (int)textSize.y + 2;

				if (currentY < locy) {
					indicesToRemove.push_back(std::distance(it, messages.rend()) - 1);
					++it;
					continue;
				}

				Vector2 textPosition = { (float)locx, (float)currentY };
				DrawTextEx(font, message.c_str(), textPosition, (float)fontSize, (float)spacing, text_color);

				if (it == messages.rbegin() && cursor >= 0 && cursor <= message.length()) {
					Vector2 cursorPosition = { locx + MeasureTextEx(font, message.substr(0, cursor).c_str(), (float) fontSize, (float)spacing).x, (float)currentY };
					DrawLineV(cursorPosition, { cursorPosition.x, cursorPosition.y + textSize.y }, cursor_color);
				}

				++it;
			}

			// Remove messages that are outside the drawing area
			for (ptrdiff_t index : indicesToRemove) {
				messages.erase(messages.begin() + ((size_t)index));
			}
			DrawLine(locx + width, locy, locx + width, locy + height, cursor_color);
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

		void load_font(std::string font_path, int font_size)
		{
			UnloadFont(font);
			font = LoadFontEx(font_path.c_str(), font_size, 0, 250);
		}

		void load_arial(int font_size)
		{
			UnloadFont(font);
			font = LoadFontEx("C:\\Windows\\Fonts\\arial.ttf", font_size, 0, 250);
		}

		void load_monospace(int font_size)
		{
			UnloadFont(font);
			font = LoadFontEx("C:\\Windows\\Fonts\\consola.ttf", font_size, 0, 250);
		}

	private:
		// Make constructor private
		MessageWindow(int loc_x, int loc_y, int _width, int _height) : locx(loc_x), locy(loc_y), width(_width), height(_height)
		{
			font = GetFontDefault();
		}
		MessageWindow() : locx(0), locy(0), width(0), height(0) {
			font = GetFontDefault();
		}
		~MessageWindow() { UnloadFont(font); }

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
			if (text.empty()) return 0;
			int fontSize = 50; // Start with a reasonable font size
			Vector2 textSize = MeasureTextEx(font, &text.front(), (float)fontSize, (float)spacing);

			// Reduce font size until the text width fits within the window width
			while (textSize.x > width && fontSize > 1) {
				fontSize--;
				textSize = MeasureTextEx(font, text.c_str(), (float)fontSize,(float) spacing);
			}

			return fontSize;
		}
	};


}
