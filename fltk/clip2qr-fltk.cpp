/*
 * clip2qr (FLTK multi-platform variant)
 *
 * Copyright (c) Peter Lawrence. (MIT License)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * - The Software is provided "as is", without warranty of any kind, express or
 *   implied, including but not limited to the warranties of merchantability,
 *   fitness for a particular purpose and noninfringement. In no event shall the
 *   authors or copyright holders be liable for any claim, damages or other
 *   liability, whether in an action of contract, tort or otherwise, arising from,
 *   out of or in connection with the Software or the use or other dealings in the
 *   Software.
 */

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Menu.H>
#include <stdint.h>
#include <stdbool.h>
#include "qrcodegen.h"

class QRwindow : public Fl_Window
{
public:
	QRwindow(int w, int h, const char *title = 0) : Fl_Window(w, h, title) {};
	int handle(int e);
	void draw(void);
	static void clipboard_notify(int source, void *data);
	enum qrcodegen_Ecc ecc_level = qrcodegen_Ecc_MEDIUM;
	static void set_ecc(Fl_Widget *, void *);
private:
	char *string = NULL;
};

static QRwindow *win;

void QRwindow :: clipboard_notify(int source, void *data)
{
	/* reject all but clipboard events */
	if (1 != source) return;

	/* reject clipboards not containing text */
	if (!Fl::clipboard_contains(Fl::clipboard_plain_text)) return;

	/* if the above tests pass, generate FL_PASTE event for window */
	Fl::paste(*win, 1, Fl::clipboard_plain_text);
}

void QRwindow :: set_ecc(Fl_Widget *wpnt, void *dpnt)
{
	/* verify provided pointers are not null */
	if ( !wpnt || !dpnt ) return;
	
	enum qrcodegen_Ecc new_level = *(enum qrcodegen_Ecc *)dpnt;
	QRwindow *self = (QRwindow *)wpnt;

	/* reject invalid ECC levels */
	if ( (new_level < qrcodegen_Ecc_LOW) || (new_level > qrcodegen_Ecc_HIGH) ) return;

	/* set new level */
	self->ecc_level = new_level;

	/* ask for QR to be re-rendered */
	self->redraw();
}

void QRwindow :: draw(void)
{
	if (!string) return;

	static uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
	static uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];

	bool ok = qrcodegen_encodeText(
			string, tempBuffer, qrcode, 
			ecc_level, qrcodegen_VERSION_MIN, 
			qrcodegen_VERSION_MAX, qrcodegen_Mask_0, true);

	if (!ok) return;

	const int size = qrcodegen_getSize(qrcode);
	const int span = (h() < w()) ? h() : w();
	const int tile_width = span / (size + 2);
	const int width = tile_width * (size + 2);
	const int x_offset = (w() - width) / 2;
	const int y_offset = (h() - width) / 2;

	fl_draw_box(FL_FLAT_BOX, 0, 0, w(), h(), FL_BACKGROUND_COLOR);
	fl_draw_box(FL_FLAT_BOX, x_offset, y_offset,
		(size + 2) * tile_width, (size + 2) * tile_width, FL_WHITE);

	for (int y = 0; y < size; y++)
		for (int x = 0; x < size; x++)
		{
			if (!qrcodegen_getModule(qrcode, x, y)) continue;
			fl_draw_box(FL_FLAT_BOX, 
				x_offset + tile_width * (x + 1), 
				y_offset + tile_width * (y + 1),
				tile_width, tile_width, 
				FL_BLACK);
		}
}

int QRwindow :: handle(int e)
{
	static qrcodegen_Ecc ecc_levels[] =
	{
		[0] = qrcodegen_Ecc_LOW,
		[1] = qrcodegen_Ecc_MEDIUM,
		[2] = qrcodegen_Ecc_QUARTILE,
		[3] = qrcodegen_Ecc_HIGH,
	};
	
	switch (e)
	{
	case FL_PUSH:

		/* provide right-click menu to change ECC level */
		if (FL_RIGHT_MOUSE == Fl::event_button())
		{
			Fl_Menu_Item rclick_menu[] = {
				{ "ECC &Low",      0, set_ecc, ecc_levels + 0, FL_MENU_TOGGLE | ((0 == ecc_level) ? FL_MENU_VALUE : 0) },
				{ "ECC &Medium",   0, set_ecc, ecc_levels + 1, FL_MENU_TOGGLE | ((1 == ecc_level) ? FL_MENU_VALUE : 0) },
				{ "ECC &Quartile", 0, set_ecc, ecc_levels + 2, FL_MENU_TOGGLE | ((2 == ecc_level) ? FL_MENU_VALUE : 0) },
				{ "ECC &High",     0, set_ecc, ecc_levels + 3, FL_MENU_TOGGLE | ((3 == ecc_level) ? FL_MENU_VALUE : 0) },
				{ 0 },
			};
			const Fl_Menu_Item *m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, 0);
			if (m)
				if (m->callback_)
					m->do_callback(this, m->user_data());
			return 1;
		}
		break;
	case FL_PASTE:

		if (string) free(string);
		const char *str = Fl::event_text();
		int len = strlen(str) + 1;
		string = (char *)malloc(len);
		if (string)
		{
			strncpy(string, str, len);
			draw();
		}
		return 1;
	}
	
	return 0;
}

int main (int argc, char *argv[])
{
	win = new QRwindow(300, 300, "Clip2QR");

	if (win)
	{
		win->size_range(32, 32);

	 	Fl::add_clipboard_notify(win->clipboard_notify);

		win->show();
		Fl::run();

		Fl::remove_clipboard_notify(win->clipboard_notify);
	}

	return 0;
}
