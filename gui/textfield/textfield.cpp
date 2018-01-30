#include "textfield.hpp"
#include "../contextmenu/contextmenu.hpp"
extern void *focusedElement;

// very primitive text input, quite sucking atm
TextInput::TextInput(int _x, int _y, int _max, string name, bool _multiline, int _lineMax, int width) : x(_x), y(_y), lineMax(_lineMax), multiline(_multiline)
, bg(Vector2f(_max * 8 + 8, 20 * _lineMax)), text("", font, charSize), title(name, font, charSize), cursorBlink(0), cursor(Vector2f(2, 20)), editing(false), charCountLine(_lineMax), vmax(_max),
mouseLock(0)
{
	if (name != "")
		bg.setPosition(x + title.getLocalBounds().width + 4, y);
	else
		bg.setPosition(x, y);

	if (width != -1)
		bg.setSize(Vector2f(width, 20 * _lineMax));

	bg.setOutlineThickness(1.f);
	bg.setOutlineColor(colors[TEXTINPUTOUTLINE]);
	text.setPosition(x + title.getLocalBounds().width + 8, y);
	text.setColor(colors[TEXTINPUTTEXT]);
	title.setPosition(x, y);
	title.setColor(colors[TITLE]);
	cursor.setFillColor(colors[TEXTINPUTCURSOR]);
	cursor.setPosition(x + title.getLocalBounds().width + 8, y);
	bg.setFillColor(colors[TEXTINPUTBG]);
	cursor.setScale(Vector2f(0, 0));
	selectionBegin = currentLine = 0;
	charCountLine.push_back(0);
	Color selectionColor = colors[TEXTINPUTCURSOR];
	selectionColor.a *= 0.5;
	selection.setFillColor(selectionColor);
	selected = 0;
}


int TextInput::getSelectionStart()
{
	if (pos2 < selectionBegin)
		return max(0, selectionBegin - selectionCount);
	else
		return selectionBegin;
}
void TextInput::cutSelection()
{
	selectionCount = min(text.getString().toAnsiString().size(), selectionCount);

	selectionBegin = getSelectionStart();

	text.setString(string(text.getString()).erase(selectionBegin, selectionCount));
	selection.setSize(Vector2f(0, 20));
	charCountLine[currentLine] -= selectionCount;
	selectionCount = 0;
}

int TextInput::getCursorIndex(int &stringPosLastLine)
{
	stringPosLastLine = 0;
	for (int i = 0; i < currentLine; i++)
	{
		stringPosLastLine += charCountLine[i];
	}

	int pos22 = stringPosLastLine;
	int pos = 0;
	while (pos < mouse.pos.x - 8 && pos22 < stringPosLastLine + charCountLine[currentLine] && (int)round((text.findCharacterPos(pos22).y - y) / font.getLineSpacing(charSize)) == currentLine && text.getString()[pos22] != '\n')
	{
		pos = text.findCharacterPos(pos22).x;
		pos22++;
	}
	return pos22;
}

void TextInput::recalcCursorPos()
{
	cursorBlink = 0;
	cursor.setPosition(text.findCharacterPos(selectionBegin).x, text.findCharacterPos(selectionBegin).y);
}

void TextInput::removeLine()
{
	charCountLine[currentLine - 1] += charCountLine[currentLine] - 1;
	charCountLine.erase(charCountLine.begin() + currentLine);
	currentLine--;
}

void TextInput::newLine()
{
	currentLine++;
	charCountLine.insert(charCountLine.begin() + currentLine, 0);
	text.setString(string(text.getString()).insert(selectionBegin, "\n"));
	selectionBegin++;
	charCountLine[currentLine]++;
}

bool TextInput::modified()
{
	if (contextMenu)
		return 0;

	if (hover())
	{
		mouse.cursor = CURSOR_TEXT;
		if (mouse.clickg) selected = true;
	}

	if (mouse.clickgReleased)
		selected = false;

	if (mouseLock && !Mouse::isButtonPressed(Mouse::Left))
	{
		mouseLock = 0;
	}
	if (selected)
	{

		if (Mouse::isButtonPressed(Mouse::Left))
		{
			if (!mouseLock)
			{
				focusedElement = this;
				selection.setSize(Vector2f(0, 20));
				textEnteredCount = 0;
				editing = true;
				cursorBlink = 0;
				currentLine = min(charCountLine.size() - 1, (mouse.pos.y - bg.getPosition().y) / font.getLineSpacing(charSize));

				int stringPos = 0;

				sIndex = getCursorIndex(stringPos);

				selectionBegin = mouse.pos.x < x + title.getLocalBounds().width + 8 ? stringPos : sIndex;
				recalcCursorPos();
				mouseLock = 1;
			}
			int stringPosLastLine;
			pos2 = getCursorIndex(stringPosLastLine);

			if (pos2 < selectionBegin)
			{
				selectionCount = selectionBegin - pos2;
			}
			else if (pos2 > selectionBegin)
			{
				selectionCount = pos2 - selectionBegin;
			}
			else
			{
				selectionCount = 0;
			}

			if (mouse.pos.x <= text.getPosition().x)
			{

				selection.setSize(Vector2f(selectionBegin == stringPosLastLine ? 0 : text.getPosition().x - text.findCharacterPos(sIndex).x, 20));

				cursor.setPosition(text.getPosition().x, text.findCharacterPos(selectionBegin).y);

				selectionCount++;
				if (currentLine == 0)
					pos2 = 0;
			}
			else
			{
				cursor.setPosition(text.findCharacterPos(pos2).x, text.findCharacterPos(selectionBegin).y);
				if (charCountLine[currentLine] > 1)
					selection.setSize(Vector2f(cursor.getPosition().x - text.findCharacterPos(sIndex - 1 * (selectionBegin == 0)).x, 20));
				else
					selection.setSize(Vector2f(0, 20));
			}
		}


		if (!editing)
		{
			bg.setFillColor(colors[TEXTINPUTBGHOVER]);
			bg.setOutlineColor(colors[TEXTINPUTOUTLINEHOVER]);
		}
	}
	else
	{
		bg.setOutlineColor(colors[TEXTINPUTOUTLINE]);
		if (mouse.clickg)
		{
			unselect();
		}
		bg.setFillColor(colors[TEXTINPUTBG]);
	}


	if (editing)
	{

		cursor.setScale(Vector2f(cursorBlink < 32, 1));

		cursorBlink += frameTime60;

		if (cursorBlink >= 60)
			cursorBlink = 0;

		bg.setFillColor(colors[TEXTINPUTBGFOCUS]);
		bg.setOutlineColor(colors[FOCUSOUTLINE]);

		// suppr
		if (keyboard.del && (selectionBegin < text.getString().toAnsiString().size() || ((int)round(selection.getSize().x) != 0)))
		{

			if ((int)selection.getSize().x == 0)
			{
				if (text.getString()[selectionBegin] == '\n' && charCountLine.size()>1)
				{
					charCountLine[currentLine] += charCountLine[currentLine + 1];
					charCountLine.erase(charCountLine.begin() + currentLine + 1);

				}
				charCountLine[currentLine]--;
				text.setString(string(text.getString()).erase(selectionBegin, 1));
			}
			else
			{
				cutSelection();
			}
			recalcCursorPos();
		}
		if (textEnteredCount > 0)
		{
			textEnteredCount--;
			if (!Mouse::isButtonPressed(Mouse::Left) && !keyboard.ctrl)
			{

				if (textEntered[textEnteredCount] == 8)
				{ // backspace

					if (selectionBegin > 0)
					{

						if ((int)round(selection.getSize().x) == 0)
						{
							if (text.getString().toAnsiString()[selectionBegin - 1] == '\n')
							{
								removeLine();
							}
							else
							{
								charCountLine[currentLine]--;
							}
							selectionBegin--;
							text.setString(string(text.getString()).erase(selectionBegin, 1));

						}
						else
						{
							cutSelection();
						}
					}
					recalcCursorPos();
					return true;
				}
				else if (textEntered[textEnteredCount] != 9)
				{ // don't allow tab
					if (textEntered[textEnteredCount] == 13)
					{ // enter
						if (multiline && charCountLine.size() < lineMax)
						{
							selection.setSize(Vector2f(0, 20));
							cutSelection();
							if (text.getString()[selectionBegin] != '\n')
							{

								int cpt = selectionBegin;
								while (cpt < text.getString().toAnsiString().size() && text.getString()[cpt] != '\n')
								{
									cpt++;
								}
								cpt -= selectionBegin;

								charCountLine[currentLine] -= cpt;
								charCountLine[currentLine]++;
								currentLine++;
								charCountLine.insert(charCountLine.begin() + currentLine, cpt);
								text.setString(string(text.getString()).insert(selectionBegin, "\n"));
								selectionBegin++;
							}
							else
							{
								newLine();
							}
							pos2++;
							recalcCursorPos();
						}
					}
					else if (textEntered[textEnteredCount] != 0)
					{ // input characters
						if ((int)selection.getSize().x != 0)
							cutSelection();
						if (text.getString().toAnsiString().length() < vmax)
						{

							if (text.findCharacterPos(selectionBegin).x >= bg.getPosition().x + bg.getSize().x - 8)
							{
								newLine();
							}
							text.setString(string(text.getString()).insert(selectionBegin, string(1, (textEntered[textEnteredCount]))));
							charCountLine[currentLine]++;
							selectionBegin++;
							pos2++;
							recalcCursorPos();
						}
						return true;
					}
				}
			}
		}
		if (keyboard.up)
		{
			int old = selectionBegin;
			selectionBegin = max(0, selectionBegin - 1);
			bool canChangeLine=true;
			while (text.getString()[selectionBegin] != '\n' && selectionBegin > 0)
			{
				selectionBegin--;
			}

			if (text.getString()[selectionBegin] == '\n')
			{
				canChangeLine=false;
			}
			
			while (text.findCharacterPos(selectionBegin).x > text.findCharacterPos(old).x)
			{
				
				selectionBegin--;
				if (text.getString()[selectionBegin] == '\n' && !canChangeLine)
					break;
			}
			

			currentLine = max(0, currentLine - 1);
			cursorBlink = 0;
			recalcCursorPos();
			selection.setSize(Vector2f(0, 20));
		}
		if (keyboard.down)
		{
			int old = selectionBegin;

			while (text.getString()[selectionBegin] != '\n' && selectionBegin < text.getString().toAnsiString().size())
			{
				selectionBegin++;
			}

			selectionBegin = min(text.getString().toAnsiString().size(), selectionBegin + 1);

			if (selectionBegin < text.getString().toAnsiString().size() && text.getString()[selectionBegin] != '\n')
			{
				while (text.findCharacterPos(selectionBegin).x < text.findCharacterPos(old).x)
				{
					selectionBegin++;
				}
			}

			currentLine = min(charCountLine.size() - 1, currentLine + 1);
			recalcCursorPos();
			selection.setSize(Vector2f(0, 20));
		}
		if (keyboard.left)
		{
			moveCursorX(-1);
		}
		if (keyboard.right)
		{
			moveCursorX(1);
		}
		if (keyboard.ctrl)
		{
			if (keyboard.c)
			{
				sf::Clipboard::setString(text.getString().toAnsiString().substr(getSelectionStart(), selectionCount));
			}
			else if (keyboard.x)
			{
				sf::Clipboard::setString(text.getString().toAnsiString().substr(getSelectionStart(), selectionCount));
				cutSelection();
				selectionBegin -= abs(selectionCount);
				recalcCursorPos();
				return true;
			}
			else if (keyboard.v)
			{
				cutSelection();
				string current = text.getString().toAnsiString();
				string toInsert = sf::Clipboard::getString().toAnsiString();
				if (current.length() + toInsert.length() > vmax)
				{

					toInsert = toInsert.substr(0, toInsert.length() - (current.length() + toInsert.length() - vmax));
				}
				if (current.length() + toInsert.length() <= vmax)
				{
					current.insert(selectionBegin, toInsert);
					selectionBegin += toInsert.length();
					setText(current);

					return true;
				}

			}
		}
	}
	selectionBegin == 0 ? selection.setPosition(x + title.getLocalBounds().width + 8, y + currentLine*font.getLineSpacing(charSize)) : selection.setPosition(text.findCharacterPos(selectionBegin).x, y + currentLine*font.getLineSpacing(charSize));
	return false;
}

void TextInput::moveCursorX(int delta)
{

	if (pos2<selectionBegin)
		selectionBegin -= selectionCount - delta;
	else if (pos2>selectionBegin)
		selectionBegin += selectionCount + delta;
	else
		selectionBegin += delta;

	selectionBegin = clamp(selectionBegin, 0, text.getString().toAnsiString().size());

	pos2 = selectionBegin;

	recalcCursorPos();
	selection.setSize(Vector2f(0, 20));
}

void TextInput::draw()
{
	window->draw(bg);
	window->draw(text);
	window->draw(title);
	window->draw(selection);
	window->draw(cursor);

}

void TextInput::setText(string _text)
{
	charCountLine.clear();
	int charCounter = 0, totalChar = 0;



	text.setString(_text);

	for (int i = 0; i < _text.length(); i++)
	{
		if (text.findCharacterPos(i).x > bg.getPosition().x + bg.getSize().x - 8)
		{
			if (_text[i] != '\n')
			{
				_text.insert(_text.begin() + i, '\n');
				text.setString(_text);
			}

		}
		if (_text[i] == '\n')
		{
			if (charCountLine.size() + 1 >= lineMax)
			{
				_text = _text.substr(0, i);
				text.setString(_text);
				break;
			}
			charCountLine.push_back(charCounter + 1);
			charCounter = 0;
		}
		else
		{
			charCounter++;
		}
		totalChar++;
	}



	charCountLine.push_back(charCounter);
	selectionBegin = clamp(selectionBegin, 0, _text.size());
	recalcCursorPos();
}

bool TextInput::hover()
{
	return (mouse.pos.x >= x && mouse.pos.x < x + title.getLocalBounds().width + bg.getSize().x + 4 && mouse.pos.y >= y - 1 && mouse.pos.y < y + bg.getLocalBounds().height - 1);
}

void TextInput::unselect()
{
	cursor.setScale(Vector2f(0, 0));
	editing = false;
	bg.setOutlineColor(colors[TEXTINPUTOUTLINE]);
	selection.setSize(Vector2f(0, 20));
}