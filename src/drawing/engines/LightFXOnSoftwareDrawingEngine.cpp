#pragma region Copyright (c) 2014-2016 OpenRCT2 Developers
/*****************************************************************************
* OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
*
* OpenRCT2 is the work of many authors, a full list can be found in contributors.md
* For more information, visit https://github.com/OpenRCT2/OpenRCT2
*
* OpenRCT2 is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* A full copy of the GNU General Public License can be found in licence.txt
*****************************************************************************/
#pragma endregion

#include "opengl/GLSLTypes.h"
#include "opengl/OpenGLAPI.h"
#include "opengl/OpenGLFramebuffer.h"

#include "../../core/Math.hpp"
#include "../../core/Memory.hpp"
#include "../IDrawingContext.h"
#include "../IDrawingEngine.h"
#include "../Rain.h"

#include "../../core/Exception.hpp"

extern "C"
{
#include "../../config.h"
#include "../../game.h"
#include "../../interface/screenshot.h"
#include "../../interface/window.h"
#include "../../intro.h"
#include "../drawing.h"
}

class LightFXOnSoftwareDrawingEngine;

struct DirtyGrid
{
	uint32  BlockShiftX;
	uint32  BlockShiftY;
	uint32  BlockWidth;
	uint32  BlockHeight;
	uint32  BlockColumns;
	uint32  BlockRows;
	uint8 * Blocks;
};

class LightFXOnSoftwareDrawingContext : public IDrawingContext
{
private:
	LightFXOnSoftwareDrawingEngine	* _engine;
	rct_drawpixelinfo				* _dpi;

public:
	LightFXOnSoftwareDrawingContext(LightFXOnSoftwareDrawingEngine * engine);
	~LightFXOnSoftwareDrawingContext() override;

	IDrawingEngine * GetEngine() override;

	void Clear(uint32 colour) override;
	void FillRect(uint32 colour, sint32 x, sint32 y, sint32 w, sint32 h) override;
	void DrawLine(uint32 colour, sint32 x1, sint32 y1, sint32 x2, sint32 y2) override;
	void DrawSprite(uint32 image, sint32 x, sint32 y, uint32 tertiaryColour) override;
	void DrawSpriteRawMasked(sint32 x, sint32 y, uint32 maskImage, uint32 colourImage) override;
	void DrawSpriteSolid(uint32 image, sint32 x, sint32 y, uint8 colour) override;
	void DrawGlyph(uint32 image, sint32 x, sint32 y, uint8 * palette) override;

	void SetDPI(rct_drawpixelinfo * dpi);
};

class LightFXOnSoftwareDrawingEngine : public IDrawingEngine
{
private:
	SDL_Window							*_window = nullptr;
	SDL_GLContext						_context;

	OpenGLFramebuffer					*_screenFramebuffer = nullptr;

	LightFXOnSoftwareDrawingContext		*_drawingContext;

	uint32								_width = 0;
	uint32								_height = 0;
	uint32								_pitch = 0;
	size_t								_bitsSize = 0;
	uint8								* _bits = nullptr;

	DirtyGrid							_dirtyGrid = { 0 };
	rct_drawpixelinfo					_bitsDPI = { 0 };
public:
	LightFXOnSoftwareDrawingEngine()
	{
		_drawingContext = new LightFXOnSoftwareDrawingContext(this);
	}

	~LightFXOnSoftwareDrawingEngine() override
	{
		delete _drawingContext;
	}

	void Initialise(SDL_Window * window) override
	{
		_window = window;

		_context = SDL_GL_CreateContext(_window);
		SDL_GL_MakeCurrent(_window, _context);

		if (!OpenGLAPI::Initialise())
		{
			throw Exception("Unable to initialise OpenGL.");
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Do not draw the unseen side of the primitives
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	}

	void ConfigureCanvas()
	{
		// Re-create screen framebuffer
		delete _screenFramebuffer;
		_screenFramebuffer = new OpenGLFramebuffer(_window);

	//	// Re-create canvas framebuffer
	//	delete _swapFramebuffer;
	//	_swapFramebuffer = new SwapFramebuffer(_width, _height);

	//	_copyFramebufferShader->Use();
	//	_copyFramebufferShader->SetScreenSize(_width, _height);
	//	_copyFramebufferShader->SetBounds(0, 0, _width, _height);
	//	_copyFramebufferShader->SetTextureCoordinates(0, 1, 1, 0);
	}

	void Resize(uint32 width, uint32 height) override
	{
		ConfigureBits(width, height, width);
		ConfigureCanvas();
	}

	void SetPalette(SDL_Color * palette) override
	{
		// Todo
	}

	void Invalidate(sint32 left, sint32 top, sint32 right, sint32 bottom) override
	{
	}

	void Draw() override
	{
		assert(_screenFramebuffer != nullptr);
	//	assert(_swapFramebuffer != nullptr);

	//	_swapFramebuffer->Bind();

		if (gIntroState != INTRO_STATE_NONE) {
			intro_draw(&_bitsDPI);
		}
		else {
		//	_rainDrawer.SetDPI(&_bitsDPI);
		//	_rainDrawer.Restore();

			// Redraw dirty regions before updating the viewports, otherwise
			// when viewports get panned, they copy dirty pixels
			DrawAllDirtyBlocks();

			window_update_all_viewports();
			DrawAllDirtyBlocks();
			window_update_all();

			gfx_draw_pickedup_peep(&_bitsDPI);
			gfx_invalidate_pickedup_peep();

		//	DrawRain(&_bitsDPI, &_rainDrawer);

			rct2_draw(&_bitsDPI);
		}

		// Scale up to window
		_screenFramebuffer->Bind();

		sint32 width = _screenFramebuffer->GetWidth();
		sint32 height = _screenFramebuffer->GetHeight();
	//	_copyFramebufferShader->Use();
	//	_copyFramebufferShader->SetTexture(_swapFramebuffer->GetTargetFramebuffer()
	//		->GetTexture());
	//	_copyFramebufferShader->Draw();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

		Display();
	}

	void Display()
	{
		SDL_GL_SwapWindow(_window);
	}

	void CopyRect(sint32 x, sint32 y, sint32 width, sint32 height, sint32 dx, sint32 dy) override
	{
		if (dx == 0 && dy == 0) return;

		// Originally 0x00683359
		// Adjust for move off screen
		// NOTE: when zooming, there can be x, y, dx, dy combinations that go off the 
		// screen; hence the checks. This code should ultimately not be called when
		// zooming because this function is specific to updating the screen on move
		int lmargin = Math::Min(x - dx, 0);
		int rmargin = Math::Min((sint32)_width - (x - dx + width), 0);
		int tmargin = Math::Min(y - dy, 0);
		int bmargin = Math::Min((sint32)_height - (y - dy + height), 0);
		x -= lmargin;
		y -= tmargin;
		width += lmargin + rmargin;
		height += tmargin + bmargin;

		sint32  stride = _bitsDPI.width + _bitsDPI.pitch;
		uint8 * to = _bitsDPI.bits + y * stride + x;
		uint8 * from = _bitsDPI.bits + (y - dy) * stride + x - dx;

		if (dy > 0)
		{
			// If positive dy, reverse directions
			to += (height - 1) * stride;
			from += (height - 1) * stride;
			stride = -stride;
		}

		// Move bytes
		for (int i = 0; i < height; i++)
		{
			memmove(to, from, width);
			to += stride;
			from += stride;
		}
	}


	void ConfigureDirtyGrid()
	{
		_dirtyGrid.BlockShiftX = 7;
		_dirtyGrid.BlockShiftY = 6;
		_dirtyGrid.BlockWidth = 1 << _dirtyGrid.BlockShiftX;
		_dirtyGrid.BlockHeight = 1 << _dirtyGrid.BlockShiftY;
		_dirtyGrid.BlockColumns = (_width >> _dirtyGrid.BlockShiftX) + 1;
		_dirtyGrid.BlockRows = (_height >> _dirtyGrid.BlockShiftY) + 1;

		delete[] _dirtyGrid.Blocks;
		_dirtyGrid.Blocks = new uint8[_dirtyGrid.BlockColumns * _dirtyGrid.BlockRows];
	}

	void DrawAllDirtyBlocks()
	{
		uint32  dirtyBlockColumns = _dirtyGrid.BlockColumns;
		uint32  dirtyBlockRows = _dirtyGrid.BlockRows;
		uint8 * dirtyBlocks = _dirtyGrid.Blocks;

		for (uint32 x = 0; x < dirtyBlockColumns; x++)
		{
			for (uint32 y = 0; y < dirtyBlockRows; y++)
			{
				uint32 yOffset = y * dirtyBlockColumns;
				if (dirtyBlocks[yOffset + x] == 0)
				{
					continue;
				}

				// Determine columns
				uint32 xx;
				for (xx = x; xx < dirtyBlockColumns; xx++)
				{
					if (dirtyBlocks[yOffset + xx] == 0)
					{
						break;
					}
				}
				uint32 columns = xx - x;

				// Check rows
				uint32 yy;
				for (yy = y; yy < dirtyBlockRows; yy++)
				{
					uint32 yyOffset = yy * dirtyBlockColumns;
					for (xx = x; xx < x + columns; xx++)
					{
						if (dirtyBlocks[yyOffset + xx] == 0)
						{
							goto endRowCheck;
						}
					}
				}

			endRowCheck:
				uint32 rows = yy - y;
				DrawDirtyBlocks(x, y, columns, rows);
			}
		}
	}

	void DrawDirtyBlocks(uint32 x, uint32 y, uint32 columns, uint32 rows)
	{
		uint32  dirtyBlockColumns = _dirtyGrid.BlockColumns;
		uint8 * screenDirtyBlocks = _dirtyGrid.Blocks;

		// Unset dirty blocks
		for (uint32 top = y; top < y + (uint32)rows; top++)
		{
			uint32 topOffset = top * dirtyBlockColumns;
			for (uint32 left = x; left < x + (uint32)columns; left++)
			{
				screenDirtyBlocks[topOffset + left] = 0;
			}
		}

		// Determine region in pixels
		uint32 left = Math::Max<uint32>(0, x * _dirtyGrid.BlockWidth);
		uint32 top = Math::Max<uint32>(0, y * _dirtyGrid.BlockHeight);
		uint32 right = Math::Min((uint32)gScreenWidth, left + (columns * _dirtyGrid.BlockWidth));
		uint32 bottom = Math::Min((uint32)gScreenHeight, top + (rows * _dirtyGrid.BlockHeight));
		if (right <= left || bottom <= top)
		{
			return;
		}

		// Draw region
		window_draw_all(&_bitsDPI, left, top, right, bottom);
	}

	sint32 Screenshot() override
	{
		return screenshot_dump_png(&_bitsDPI);
	}

	IDrawingContext * GetDrawingContext(rct_drawpixelinfo * dpi) override
	{
		_drawingContext->SetDPI(dpi);
		return _drawingContext;
	}

	rct_drawpixelinfo * GetDrawingPixelInfo() override
	{
		return &_bitsDPI;
	}

	DRAWING_ENGINE_FLAGS GetFlags() override
	{
		return DEF_DIRTY_OPTIMISATIONS;
	}

	void InvalidateImage(uint32 image) override
	{
		// Not applicable for this engine
	}

	rct_drawpixelinfo * GetDPI()
	{
		return &_bitsDPI;
	}

private:

	void ConfigureBits(uint32 width, uint32 height, uint32 pitch)
	{
		size_t  newBitsSize = pitch * height;
		uint8 * newBits = new uint8[newBitsSize];
		if (_bits == nullptr)
		{
			Memory::Set(newBits, 0, newBitsSize);
		}
		else
		{
			if (_pitch == pitch)
			{
				Memory::Copy(newBits, _bits, Math::Min(_bitsSize, newBitsSize));
			}
			else
			{
				uint8 * src = _bits;
				uint8 * dst = newBits;

				uint32 minWidth = Math::Min(_width, width);
				uint32 minHeight = Math::Min(_height, height);
				for (uint32 y = 0; y < minHeight; y++)
				{
					Memory::Copy(dst, src, minWidth);
					if (pitch - minWidth > 0)
					{
						Memory::Set(dst + minWidth, 0, pitch - minWidth);
					}
					src += _pitch;
					dst += pitch;
				}
			}
			delete[] _bits;
		}

		_bits = newBits;
		_bitsSize = newBitsSize;
		_width = width;
		_height = height;
		_pitch = pitch;

		rct_drawpixelinfo * dpi = &_bitsDPI;
		dpi->bits = _bits;
		dpi->x = 0;
		dpi->y = 0;
		dpi->width = width;
		dpi->height = height;
		dpi->pitch = _pitch - width;

		ConfigureDirtyGrid();
	}

	//void ReadCentrePixel(uint32 * pixel)
	//{
	//	SDL_Rect centrePixelRegion = { (sint32)(_width / 2), (sint32)(_height / 2), 1, 1 };
	//	SDL_RenderReadPixels(_sdlRenderer, &centrePixelRegion, SDL_PIXELFORMAT_RGBA8888, pixel, sizeof(uint32));
	//}

	//// Should be called before SDL_RenderPresent to capture frame buffer before Steam overlay is drawn.
	//void OverlayPreRenderCheck()
	//{
	//	ReadCentrePixel(&_pixelBeforeOverlay);
	//}

	//// Should be called after SDL_RenderPresent, when Steam overlay has had the chance to be drawn.
	//void OverlayPostRenderCheck()
	//{
	//	ReadCentrePixel(&_pixelAfterOverlay);

	//	// Detect an active Steam overlay by checking if the center pixel is changed by the gray fade.
	//	// Will not be triggered by applications rendering to corners, like FRAPS, MSI Afterburner and Friends popups.
	//	bool newOverlayActive = _pixelBeforeOverlay != _pixelAfterOverlay;

	//	// Toggle game pause state consistently with base pause state
	//	if (!_overlayActive && newOverlayActive)
	//	{
	//		_pausedBeforeOverlay = gGamePaused & GAME_PAUSED_NORMAL;
	//		if (!_pausedBeforeOverlay)
	//		{
	//			pause_toggle();
	//		}
	//	}
	//	else if (_overlayActive && !newOverlayActive && !_pausedBeforeOverlay)
	//	{
	//		pause_toggle();
	//	}

	//	_overlayActive = newOverlayActive;
	//}
};

IDrawingEngine * DrawingEngineFactory::CreateLightFXOnSoftware()
{
	return new LightFXOnSoftwareDrawingEngine();
}

LightFXOnSoftwareDrawingContext::LightFXOnSoftwareDrawingContext(LightFXOnSoftwareDrawingEngine * engine)
{
	_engine = engine;
}

LightFXOnSoftwareDrawingContext::~LightFXOnSoftwareDrawingContext()
{

}

IDrawingEngine * LightFXOnSoftwareDrawingContext::GetEngine()
{
	return _engine;
}

void LightFXOnSoftwareDrawingContext::Clear(uint32 colour)
{
	rct_drawpixelinfo * dpi = _dpi;

	int w = dpi->width >> dpi->zoom_level;
	int h = dpi->height >> dpi->zoom_level;
	uint8 * ptr = dpi->bits;

	for (int y = 0; y < h; y++)
	{
		Memory::Set(ptr, colour, w);
		ptr += w + dpi->pitch;
	}
}

void LightFXOnSoftwareDrawingContext::FillRect(uint32 colour, sint32 left, sint32 top, sint32 right, sint32 bottom)
{
	rct_drawpixelinfo * dpi = _dpi;

	if (left > right) return;
	if (top > bottom) return;
	if (dpi->x > right) return;
	if (left >= dpi->x + dpi->width) return;
	if (bottom < dpi->y) return;
	if (top >= dpi->y + dpi->height) return;

	colour |= RCT2_GLOBAL(0x009ABD9C, uint32);

	uint16 crossPattern = 0;

	int startX = left - dpi->x;
	if (startX < 0)
	{
		crossPattern ^= startX;
		startX = 0;
	}

	int endX = right - dpi->x + 1;
	if (endX > dpi->width)
	{
		endX = dpi->width;
	}

	int startY = top - dpi->y;
	if (startY < 0)
	{
		crossPattern ^= startY;
		startY = 0;
	}

	int endY = bottom - dpi->y + 1;
	if (endY > dpi->height)
	{
		endY = dpi->height;
	}

	int width = endX - startX;
	int height = endY - startY;

	if (colour & 0x1000000)
	{
		// Cross hatching
		uint8 * dst = (startY * (dpi->width + dpi->pitch)) + startX + dpi->bits;
		for (int i = 0; i < height; i++)
		{
			uint8 * nextdst = dst + dpi->width + dpi->pitch;
			uint32  p = ror32(crossPattern, 1);
			p = (p & 0xFFFF0000) | width;

			// Fill every other pixel with the colour
			for (; (p & 0xFFFF) != 0; p--)
			{
				p = p ^ 0x80000000;
				if (p & 0x80000000)
				{
					*dst = colour & 0xFF;
				}
				dst++;
			}
			crossPattern ^= 1;
			dst = nextdst;
		}
	}
	else if (colour & 0x2000000)
	{
		//0x2000000
		// 00678B7E   00678C83
		// Location in screen buffer?
		uint8 * dst = dpi->bits + (uint32)((startY >> (dpi->zoom_level)) * ((dpi->width >> dpi->zoom_level) + dpi->pitch) + (startX >> dpi->zoom_level));

		// Find colour in colour table?
		uint16           g1Index = palette_to_g1_offset[colour & 0xFF];
		rct_g1_element * g1Element = &g1Elements[g1Index];
		uint8 *          g1Bits = g1Element->offset;

		// Fill the rectangle with the colours from the colour table
		for (int i = 0; i < height >> dpi->zoom_level; i++)
		{
			uint8 * nextdst = dst + (dpi->width >> dpi->zoom_level) + dpi->pitch;
			for (int j = 0; j < (width >> dpi->zoom_level); j++)
			{
				*dst = g1Bits[*dst];
				dst++;
			}
			dst = nextdst;
		}
	}
	else if (colour & 0x4000000)
	{
		uint8 * dst = startY * (dpi->width + dpi->pitch) + startX + dpi->bits;

		// The pattern loops every 15 lines this is which
		// part the pattern is on.
		int patternY = (startY + dpi->y) % 16;

		// The pattern loops every 15 pixels this is which
		// part the pattern is on.
		int startPatternX = (startX + dpi->x) % 16;
		int patternX = startPatternX;

		uint16 * patternsrc = RCT2_ADDRESS(0x0097FEFC, uint16*)[colour >> 28]; // or possibly uint8)[esi*4] ?

		for (int numLines = height; numLines > 0; numLines--)
		{
			uint8 * nextdst = dst + dpi->width + dpi->pitch;
			uint16  pattern = patternsrc[patternY];

			for (int numPixels = width; numPixels > 0; numPixels--)
			{
				if (pattern & (1 << patternX))
				{
					*dst = colour & 0xFF;
				}
				patternX = (patternX + 1) % 16;
				dst++;
			}
			patternX = startPatternX;
			patternY = (patternY + 1) % 16;
			dst = nextdst;
		}
	}
	else if (colour & 0x8000000)
	{
		uintptr_t esi = left - RCT2_GLOBAL(0x1420070, sint16);
		RCT2_GLOBAL(0xEDF824, uint32) = esi;
		esi = top - RCT2_GLOBAL(0x1420072, sint16);
		RCT2_GLOBAL(0xEDF828, uint32) = esi;
		left -= dpi->x;
		if (left < 0)
		{
			RCT2_GLOBAL(0xEDF824, sint32) -= left;
			left = 0;
		}
		right -= dpi->x;
		right++;
		if (right > dpi->width)
		{
			right = dpi->width;
		}
		right -= left;
		top -= dpi->y;
		if (top < 0)
		{
			RCT2_GLOBAL(0xEDF828, sint32) -= top;
			top = 0;
		}
		bottom -= dpi->y;
		bottom++;
		if (bottom > dpi->height)
		{
			bottom = dpi->height;
		}
		bottom -= top;
		RCT2_GLOBAL(0xEDF824, sint32) &= 0x3F;
		RCT2_GLOBAL(0xEDF828, sint32) &= 0x3F;
		esi = dpi->width;
		esi += dpi->pitch;
		esi *= top;
		esi += left;
		esi += (uintptr_t)dpi->bits;
		RCT2_GLOBAL(0xEDF82C, sint32) = right;
		RCT2_GLOBAL(0xEDF830, sint32) = bottom;
		left = dpi->width;
		left += dpi->pitch;
		left -= right;
		RCT2_GLOBAL(0xEDF834, sint32) = left;
		colour &= 0xFF;
		colour--;
		right = colour;
		colour <<= 8;
		right |= colour;
		RCT2_GLOBAL(0xEDF838, sint32) = right;
		//right <<= 4;
		esi = RCT2_GLOBAL(0xEDF828, sint32);
		esi *= 0x40;
		left = 0;
		esi += (uintptr_t)g1Elements[right].offset;//???
												   //Not finished
												   //Start of loop
	}
	else
	{
		uint8 * dst = startY * (dpi->width + dpi->pitch) + startX + dpi->bits;
		for (int i = 0; i < height; i++)
		{
			Memory::Set(dst, colour & 0xFF, width);
			dst += dpi->width + dpi->pitch;
		}
	}
}

void LightFXOnSoftwareDrawingContext::DrawLine(uint32 colour, sint32 x1, sint32 y1, sint32 x2, sint32 y2)
{
	gfx_draw_line_software(_dpi, x1, y1, x2, y2, colour);
}

void LightFXOnSoftwareDrawingContext::DrawSprite(uint32 image, sint32 x, sint32 y, uint32 tertiaryColour)
{
	gfx_draw_sprite_software(_dpi, image, x, y, tertiaryColour);
}

void LightFXOnSoftwareDrawingContext::DrawSpriteRawMasked(sint32 x, sint32 y, uint32 maskImage, uint32 colourImage)
{
	gfx_draw_sprite_raw_masked_software(_dpi, x, y, maskImage, colourImage);
}

void LightFXOnSoftwareDrawingContext::DrawSpriteSolid(uint32 image, sint32 x, sint32 y, uint8 colour)
{
	uint8 palette[256];
	memset(palette, colour, 256);
	palette[0] = 0;

	RCT2_GLOBAL(0x00EDF81C, uint32) = 0x20000000;
	image &= 0x7FFFF;
	gfx_draw_sprite_palette_set_software(_dpi, image | 0x20000000, x, y, palette, nullptr);
}

void LightFXOnSoftwareDrawingContext::DrawGlyph(uint32 image, sint32 x, sint32 y, uint8 * palette)
{
	gfx_draw_sprite_palette_set_software(_dpi, image, x, y, palette, nullptr);
}

void LightFXOnSoftwareDrawingContext::SetDPI(rct_drawpixelinfo * dpi)
{
	_dpi = dpi;
}