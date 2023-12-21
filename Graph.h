#pragma once

#include <iostream>
#include <set>
#include <cmath>
#include <raylib.h>

namespace plot
{
	class Point
	{
	public:
		double x, y;
		Point(double x, double y) : x(x), y(y) {}
		//~Point() {}
		friend std::ostream& operator<<(std::ostream& os, const Point& p)
		{
			os << "(" << p.x << ", " << p.y << ")";
			return os;
		}
		bool operator<(const Point& other) const {
			if (x != other.x) return x < other.x;
			return y < other.y;
		}
	};


	class Graph
	{
	public:

		Graph(int x, int y, int width, int height, int x_start, int x_end, int y_start, int y_end)
		{
			loc.x = x;
			loc.y = y;
			dim.width = width;
			dim.height = height;
			x_axis.start = x_start;
			x_axis.end = x_end;
			y_axis.start = y_start;
			y_axis.end = y_end;
			bgcolor = BLACK;
			fgcolor = GREEN;
			gridcolor = LIGHTGRAY;
			axiscolor = RED;
			//points.reserve(abs(x_start) + abs(x_end) + abs(y_start) + abs(y_end));
		}
		Graph(){}
		~Graph()
		{
			points.clear();
			//normalized_points.clear();
		}

		void add_point(Point p)
		{
			if (invalid_point(p)) return;
			points.insert(p);
			//normalized_points.push_back(normalize_point(p));
		}

		void add_point(double x, double y)
		{
			add_point(Point(x, y));
		}


		void add_point(std::vector<Point> _points)
		{
			for (int i = 0; i < _points.size(); i++)
			{
				add_point(_points[i]);
			}
		}


		void clear_points()
		{
			points.clear();
			//normalized_points.clear();
		}

		Point normalize_point(Point p)
		{
			return Point((p.x - x_axis.start) / (x_axis.end - x_axis.start), (p.y - y_axis.start) / (y_axis.end - y_axis.start));
		}

		//std::vector<Point> normalize_points()
		//{
		//	std::vector<Point> normalizedPoints;
		//	for (int i = 0; i < points.size(); i++)
		//	{
		//		normalizedPoints.push_back(normalize_point(points[i]));
		//	}
		//	return normalizedPoints;
		//}

		void draw_axis()
		{
			
			int x_axis_y = loc.y + dim.height * (1.0 - (0.0 - y_axis.start) / (y_axis.end - y_axis.start));
			int y_axis_x = loc.x + dim.width * ((0.0 - x_axis.start) / (x_axis.end - x_axis.start));
			DrawLine(loc.x, x_axis_y, loc.x + dim.width, x_axis_y, axiscolor);
			DrawLine(y_axis_x, loc.y, y_axis_x, loc.y + dim.height, axiscolor);
		}

		void draw_ticks()
		{
			int x_axis_y = loc.y + dim.height * (1.0 - (0.0 - y_axis.start) / (y_axis.end - y_axis.start));
			int y_axis_x = loc.x + dim.width * ((0.0 - x_axis.start) / (x_axis.end - x_axis.start));
			// Draw x-axis ticks
			for (int i = x_axis.start; i < x_axis.end; ++i) {
				int tickX = loc.x + (i - x_axis.start) * dim.width / (x_axis.end - x_axis.start);
				DrawLine(tickX, x_axis_y - 5, tickX, x_axis_y + 5, fgcolor);
			}

			// Draw y-axis ticks
			for (int i = y_axis.start; i <= y_axis.end; ++i) {
				int tickY = loc.y + (1.0 - (i - y_axis.start) / (double)(y_axis.end - y_axis.start)) * dim.height;
				DrawLine(y_axis_x - 5, tickY, y_axis_x + 5, tickY, fgcolor);
			}
		}

		void draw_grid()
		{
			// Draw x-axis grid lines
			for (int i = x_axis.start; i < x_axis.end; ++i) {
				int x = loc.x + (i - x_axis.start) * dim.width / (x_axis.end - x_axis.start);
				DrawLine(x, loc.y, x, loc.y + dim.height, gridcolor);
			}

			// Draw y-axis grid lines
			for (int i = y_axis.start; i <= y_axis.end; ++i) {
				int y = loc.y + (1.0 - (i - y_axis.start) / (double)(y_axis.end - y_axis.start)) * dim.height;;
				DrawLine(loc.x, y, loc.x + dim.width, y, gridcolor);
			}
		}

		void draw_bg()
		{
			DrawRectangle(loc.x, loc.y, dim.width, dim.height, bgcolor);
		}

		void draw()
		{
			draw_bg();
			draw_grid();
			draw_axis();
			draw_ticks();
			plot();
		}


		void plot()
		{
			#pragma omp parallel for
			for (auto point : points)
			{
				Point p = normalize_point(point);
				double inverted_y = 1.0 - p.y;
				int x = loc.x + p.x * dim.width;
				int y = loc.y + inverted_y * dim.height;
				DrawCircle(x, y, 2, fgcolor);
			}
		}

		void set_bgcolor(Color color)
		{
			this->bgcolor = color;
		}

		void set_fgcolor(Color color)
		{
			this->fgcolor = color;
		}

		std::set<Point> get_points()
		{
			return points;
		}

		//std::vector<Point> get_normalized_points()
		//{
		//	return normalized_points;
		//}

		int get_width()
		{
			return dim.width;
		}
		
		int get_height()
		{
			return dim.height;
		}

		int get_x()
		{
			return loc.x;
		}

		int get_y()
		{
			return loc.y;
		}

		int get_x_start()
		{
			return x_axis.start;
		}

		int get_x_end()
		{
			return x_axis.end;
		}

		int get_y_start()
		{
			return y_axis.start;
		}

		int get_y_end()
		{
			return y_axis.end;
		}

		void set_gridcolor(Color color)
		{
			this->gridcolor = color;
		}
		void set_axiscolor(Color color)
		{
			this->axiscolor = color;
		}


	private:
		double distanceBetweenPoints(const Point& a, const Point& b) {
			return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
		}
		bool invalid_point(Point p) {
			// Check for NaN
			if (std::isnan(p.x) || std::isnan(p.y)) return true;

			// Check for infinity or negative infinity
			if (std::isinf(p.x) || std::isinf(p.y)) return true;

			// Check for subnormal (denormal) numbers
			if (p.x != 0.0 && std::abs(p.x) < std::numeric_limits<cmn::value>::min()) return true;
			if (p.y != 0.0 && std::abs(p.y) < std::numeric_limits<cmn::value>::min()) return true;

			if (p.x < get_x_start() || p.x > get_x_end()) return true;
			if (p.y < get_y_start() || p.y > get_y_end()) return true;

			return false;
		}
		struct
		{
			int x, y;
		}loc;
		struct
		{
			int width, height;
		}dim;
		struct
		{
			int start, end;
		} x_axis, y_axis;
		std::set<Point> points;
		//std::vector<Point> normalized_points;
		Color bgcolor;
		Color fgcolor;
		Color gridcolor;
		Color axiscolor;

	};

} // namespace plot