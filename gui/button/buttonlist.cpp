#include "buttonlist.hpp"
#include "../../fmengine/fmlib.h"
#include "../contextmenu/contextmenu.hpp"

extern fmsynth *phanoo;
extern ListMenu *contextMenu;

ButtonList::ButtonList(int _x, int _y) :selected(0), maxId(0)
{
	x = _x;
	y = _y;
	scroll = 0;
	bgfocus.setOutlineThickness(2);
	bgfocus.setOutlineColor(colors[FOCUSOUTLINE]);
	bgfocus.setFillColor(colors[FOCUSOUTLINE]);
	bgfocus.setPosition(_x, _y);
}

void ButtonList::select(int index)
{
	selectedIndex = index;
}

void ButtonList::draw()
{

	scroll = clamp(selectedIndex - 23, 0, (int)buttons.size());

	int xpos = buttons[scroll].x;

	for (unsigned i = scroll; i < min<int>(buttons.size(), scroll + 46); ++i)
	{
		buttons[i].selected = (selectedIndex == i);
		buttons[i].setPosition(xpos - (buttons[scroll].x - buttons[0].x), y);
		xpos += buttons[i].w + 1;
	}

	bgfocus.setSize(Vector2f(xpos - buttons[0].x, 19));

	if (selected)
	{
		window->draw(bgfocus);
	}

	for (unsigned i = scroll; i < min<int>(buttons.size(), scroll + 46); ++i)
	{
		buttons[i].draw();
	}
}

void ButtonList::add(string text)
{

	buttons.push_back(Button(x + buttons.size() * 24, y, std::to_string(maxId), maxId < 100 ? 19 : 25));

	maxId++;
}

int ButtonList::getElementHovered()
{
	int elem = -1;
	for (unsigned i = max<int>(0, scroll); i < min<int>(buttons.size(), scroll + 46); ++i)
	{

		buttons[i].clicked();
		if (buttons[i].hover())
		{
			elem = i;
			break;
		}
	}

	return elem;
}

void ButtonList::update()
{

	if (mouse.clickg || mouse.clickd)
	{
		if (mouse.pos.x >= x && mouse.pos.y >= y && mouse.pos.x < x + buttons.size() * 24 && mouse.pos.y < y + 19)
		{
			focusedElement = this;
			selected = 1;
		}
		else if (!contextMenu || !contextMenu->hover())
		{
			selected = 0;
		}
	}
}

unsigned ButtonList::elementCount()
{
	return buttons.size();
}

void ButtonList::erase(int index)
{
	buttons.erase(buttons.begin() + index);
	for (unsigned i = 0; i<buttons.size(); ++i)
		buttons[i].setPosition(x + i * 24, buttons[i].y);
}

void ButtonList::move(int indexA, int indexB)
{
	buttons.insert(buttons.begin() + indexB + (indexB>indexA), buttons[indexA]);
	buttons.erase(buttons.begin() + indexA + (indexB <= indexA));
	int xpos=0;
	for (unsigned i = 0; i < buttons.size(); i++)
	{
		buttons[i].setPosition(x+xpos,y);
		xpos += buttons[i].w + 1;
	}

}

void ButtonList::clear()
{
	buttons.clear();
	maxId = 0;
}

int ButtonList::isElementHovered(int index)
{
	for (unsigned i = scroll; i < min<int>(buttons.size(), scroll + 46); ++i)
	{
		buttons[i].selected = (selectedIndex == i);
		buttons[i].setPosition(x + (i - scroll) * 24, y);
	}
	if (index < 0 || index >= buttons.size())
		return 0;
	return buttons[index].hover();
}

void ButtonList::insert(int index, string text)
{
	buttons.insert(buttons.begin() + index, Button(x + index * 24, y, std::to_string(maxId), maxId < 100 ? 19 : 25));
	maxId++;
}

void ButtonList::selectAll()
{
	for (int i = 0; i < fm->patternCount; i++)
	{
		buttons[i].selected = 1;
	}
}