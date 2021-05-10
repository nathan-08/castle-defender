#include "TextManager.hpp"
#include <stdio.h>
#include <iostream>

static std::string getCharlist()
{
  std::string charlist;
  for(int i = 0; i < 255; i++) {
    if (isgraph(i)) charlist += i;
  }
  charlist += ' ';
  return charlist;
}

TextManager::GlyphCache::GlyphCache(SDL_Renderer *r, TTF_Font *f):
  renderer(r),
  font(f)
{
  std::string charlist = getCharlist();
  int xoffset = 0;
  for(const char& ch : charlist)
  {
    int minx, maxx, miny, maxy, advance;
    TTF_GlyphMetrics( font, ch, &minx, &maxx, &miny, &maxy, &advance );
    glyph_map[ch] = new SDL_Rect { xoffset, 0, advance, 8 };
    xoffset += advance;
  }
  // generate glyphset texture
  SDL_Surface *s = TTF_RenderText_Solid( font, charlist.c_str(), SDL_Color{255,255,255,255} );
  glyphset = SDL_CreateTextureFromSurface( renderer, s );
  SDL_FreeSurface( s );
}

int TextManager::GlyphCache::sizeText (
    const char *str, int *w, int *h) const {
  return TTF_SizeText(font, str, w, h);
}

TextManager::GlyphCache::~GlyphCache()
{
  if(glyphset != NULL)
  {
    SDL_DestroyTexture(glyphset);
    glyphset = NULL;
  }
}

TextManager::TextArea::TextArea( SDL_Renderer *r, int x, int y, int w, int h ):
  renderer(r), _x(x), _y(y), _w(w), _h(h) { }

void TextManager::TextArea::drawRect()
{

  SDL_Rect border { _x, _y, _w, _h };
  SDL_SetRenderDrawColor(renderer,
      0x0,0x0,0x0,0xff);
  SDL_RenderFillRect(renderer, &border);
  SDL_SetRenderDrawColor(renderer,
      0xff,0xff,0xff,0xff);
  SDL_RenderDrawRect( renderer, &border );
}

void TextManager::TextArea::updateRect(int x, int y, int w, int h) {
  _x = x; _y = y; _w = w; _h = h;
}

void TextManager::TextArea::printCenter(const TextManager::GlyphCache &gc, const char *text,
  int width, int height
) {
  int x, y;
  gc.sizeText(text, &x, &y);
  int textAndBorderWidth = x + 2;
  int heightToPrint = (height*8 / 2) - 5;
  int widthToPrint = (width*8 / 2) - (textAndBorderWidth / 2);
  updateRect(widthToPrint, heightToPrint, textAndBorderWidth, 10);
  drawRect();
  renderPrint(gc, text);
}

void TextManager::TextArea::printMultilineCenter(
  const TextManager::GlyphCache &gc, const std::vector<std::string>& lines,
  int maxX, int maxY
) {
  int greatestX(0);
  int x, y;
  std::string result;
  for (const auto& line: lines) {
    gc.sizeText(line.c_str(), &x, &y);
    if (x > greatestX) greatestX = x;
    result += line;
    result += '\n';
  }
  int textAndBorderWidth = greatestX + 2;
  int textAndBorderHeight = (lines.size() * 8 + 2);
  int heightToPrint = (maxY * 8 / 2) - (textAndBorderHeight / 2);
  int widthToPrint = (maxX*8 / 2) - (textAndBorderWidth / 2);
  updateRect(widthToPrint, heightToPrint, textAndBorderWidth, textAndBorderHeight);
  drawRect();

  renderPrint(gc, result.c_str());
}

void TextManager::TextArea::renderPrint( const TextManager::GlyphCache &gc, const char *text )
{
  SDL_SetRenderDrawColor(renderer,
      0xff,0xff,0xff,0xff);
  int base_x_offset = _x + 2;
  int base_y_offset = _y + 1;
  int xoffset = base_x_offset;
  int yoffset = base_y_offset;
  for (int i = 0; text[i] != '\0'; i++) {

    if (text[i] == '\n')
    {
      yoffset += 8;
      xoffset = base_x_offset;
      continue;
    }

    SDL_Rect *rect = gc.glyph_map.at(text[i]);
    SDL_Rect destrect = { xoffset, yoffset, rect->w, rect->h };
    SDL_RenderCopy( renderer, gc.glyphset, rect, &destrect );
    xoffset += rect->w;
  } 
  //SDL_Rect cursor = { xoffset, yoffset, 4, 8, };
  //SDL_RenderFillRect( renderer, &cursor );
}

