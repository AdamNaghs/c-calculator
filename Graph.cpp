#include "Graph.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <raylib.h>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000

namespace plot {

	static Graph internal_graph;
	static std::string internal_graph_name;

	bool is_plotting = false;

	void plotter(Graph graph, std::string name)
	{
		InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, name.c_str());
		SetTargetFPS(60);
		is_plotting = true;

		internal_graph = graph;
		internal_graph_name = name;
		while (!WindowShouldClose())
		{
			BeginDrawing();
			ClearBackground(WHITE);

			internal_graph.draw_axis();
			internal_graph.plot();
			// Draw points

			EndDrawing();
		}
		CloseWindow();
	}

	void plot(Graph graph, std::string name)
	{
		if (is_plotting)
		{
			internal_graph = graph;
			internal_graph_name = name;
			return;
		}
		else
		{
			plotter(graph, name);
		}
	}

} // namespace plot