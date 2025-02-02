// Aseprite
// Copyright (C) 2022  Igara Studio S.A.
//
// This program is distributed under the terms of
// the End-User License Agreement for Aseprite.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/script/graphics_context.h"

#include "app/color.h"
#include "app/color_utils.h"
#include "app/modules/palettes.h"
#include "app/script/engine.h"
#include "app/script/luacpp.h"
#include "app/ui/skin/skin_theme.h"
#include "app/util/conversion_to_surface.h"
#include "os/draw_text.h"

#ifdef ENABLE_UI

namespace app {
namespace script {

void GraphicsContext::fillText(const std::string& text, int x, int y)
{
  os::draw_text(m_surface.get(), m_font.get(),
                text, m_paint.color(), 0, x, y, nullptr);
}

gfx::Size GraphicsContext::measureText(const std::string& text) const
{
  return os::draw_text(nullptr, m_font.get(), text,
                       0, 0, 0, 0, nullptr).size();
}

void GraphicsContext::drawImage(const doc::Image* img, int x, int y)
{
  convert_image_to_surface(
    img,
    get_current_palette(),
    m_surface.get(),
    0, 0,
    x, y,
    img->width(), img->height());
}

void GraphicsContext::drawThemeImage(const std::string& partId, const gfx::Point& pt)
{
  if (auto theme = skin::SkinTheme::instance()) {
    skin::SkinPartPtr part = theme->getPartById(partId);
    if (part && part->bitmap(0))
      m_surface->drawRgbaSurface(part->bitmap(0), pt.x, pt.y);
  }
}

void GraphicsContext::drawThemeRect(const std::string& partId, const gfx::Rect& rc)
{
  if (auto theme = skin::SkinTheme::instance()) {
    skin::SkinPartPtr part = theme->getPartById(partId);
    if (part && part->bitmap(0)) {
      ui::Graphics g(nullptr, m_surface, 0, 0);
      theme->drawRect(&g, rc, part.get(), true);
    }
  }
}

void GraphicsContext::stroke()
{
  m_paint.style(os::Paint::Stroke);
  m_surface->drawPath(m_path, m_paint);
}

void GraphicsContext::fill()
{
  m_paint.style(os::Paint::Fill);
  m_surface->drawPath(m_path, m_paint);
}

namespace {

int GraphicsContext_gc(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  gc->~GraphicsContext();
  return 0;
}

int GraphicsContext_save(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  gc->save();
  return 0;
}

int GraphicsContext_restore(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  gc->restore();
  return 0;
}

int GraphicsContext_strokeRect(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  const gfx::Rect rc = convert_args_into_rect(L, 2);
  gc->strokeRect(rc);
  return 0;
}

int GraphicsContext_fillRect(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  const gfx::Rect rc = convert_args_into_rect(L, 2);
  gc->fillRect(rc);
  return 0;
}

int GraphicsContext_fillText(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  if (const char* text = lua_tostring(L, 2)) {
    int x = lua_tointeger(L, 3);
    int y = lua_tointeger(L, 4);
    gc->fillText(text, x, y);
  }
  return 0;
}

int GraphicsContext_measureText(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  if (const char* text = lua_tostring(L, 2)) {
    push_obj(L, gc->measureText(text));
    return 1;
  }
  return 0;
}

int GraphicsContext_drawImage(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  if (const doc::Image* img = may_get_image_from_arg(L, 2)) {
    int x = lua_tointeger(L, 3);
    int y = lua_tointeger(L, 4);
    gc->drawImage(img, x, y);
  }
  return 0;
}

int GraphicsContext_drawThemeImage(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  if (const char* id = lua_tostring(L, 2)) {
    const gfx::Point pt = convert_args_into_point(L, 3);
    gc->drawThemeImage(id, pt);
  }
  return 0;
}

int GraphicsContext_drawThemeRect(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  if (const char* id = lua_tostring(L, 2)) {
    const gfx::Rect rc = convert_args_into_rect(L, 3);
    gc->drawThemeRect(id, rc);
  }
  return 0;
}

int GraphicsContext_beginPath(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  gc->beginPath();
  lua_pushvalue(L, 1);
  return 1;
}

int GraphicsContext_closePath(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  gc->closePath();
  lua_pushvalue(L, 1);
  return 1;
}

int GraphicsContext_moveTo(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  float x = lua_tonumber(L, 2);
  float y = lua_tonumber(L, 3);
  gc->moveTo(x, y);
  lua_pushvalue(L, 1);
  return 1;
}

int GraphicsContext_lineTo(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  float x = lua_tonumber(L, 2);
  float y = lua_tonumber(L, 3);
  gc->lineTo(x, y);
  lua_pushvalue(L, 1);
  return 1;
}

int GraphicsContext_cubicTo(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  float cp1x = lua_tonumber(L, 2);
  float cp1y = lua_tonumber(L, 3);
  float cp2x = lua_tonumber(L, 4);
  float cp2y = lua_tonumber(L, 5);
  float x = lua_tonumber(L, 6);
  float y = lua_tonumber(L, 7);
  gc->cubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
  lua_pushvalue(L, 1);
  return 1;
}

int GraphicsContext_stroke(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  gc->stroke();
  lua_pushvalue(L, 1);
  return 1;
}

int GraphicsContext_fill(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  gc->fill();
  lua_pushvalue(L, 1);
  return 1;
}

int GraphicsContext_get_width(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  lua_pushinteger(L, gc->width());
  return 1;
}

int GraphicsContext_get_height(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  lua_pushinteger(L, gc->height());
  return 1;
}

int GraphicsContext_get_antialias(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  lua_pushboolean(L, gc->antialias());
  return 1;
}

int GraphicsContext_set_antialias(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  const bool antialias = lua_toboolean(L, 2);
  gc->antialias(antialias);
  return 1;
}

int GraphicsContext_get_color(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  push_obj(L, color_utils::color_from_ui(gc->color()));
  return 1;
}

int GraphicsContext_set_color(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  const app::Color color = convert_args_into_color(L, 2);
  gc->color(color_utils::color_for_ui(color));
  return 1;
}

int GraphicsContext_get_strokeWidth(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  lua_pushnumber(L, gc->strokeWidth());
  return 1;
}

int GraphicsContext_set_strokeWidth(lua_State* L)
{
  auto gc = get_obj<GraphicsContext>(L, 1);
  const float strokeWidth = lua_tonumber(L, 2);
  gc->strokeWidth(strokeWidth);
  return 1;
}

const luaL_Reg GraphicsContext_methods[] = {
  { "__gc", GraphicsContext_gc },
  { "save", GraphicsContext_save },
  { "restore", GraphicsContext_restore },
  { "strokeRect", GraphicsContext_strokeRect },
  { "fillRect", GraphicsContext_fillRect },
  { "fillText", GraphicsContext_fillText },
  { "measureText", GraphicsContext_measureText },
  { "drawImage", GraphicsContext_drawImage },
  { "drawThemeImage", GraphicsContext_drawThemeImage },
  { "drawThemeRect", GraphicsContext_drawThemeRect },
  { "beginPath", GraphicsContext_beginPath },
  { "closePath", GraphicsContext_closePath },
  { "moveTo", GraphicsContext_moveTo },
  { "lineTo", GraphicsContext_lineTo },
  { "cubicTo", GraphicsContext_cubicTo },
  { "stroke", GraphicsContext_stroke },
  { "fill", GraphicsContext_fill },
  { nullptr, nullptr }
};

const Property GraphicsContext_properties[] = {
  { "width", GraphicsContext_get_width, nullptr },
  { "height", GraphicsContext_get_height, nullptr },
  { "antialias", GraphicsContext_get_antialias, GraphicsContext_set_antialias },
  { "color", GraphicsContext_get_color, GraphicsContext_set_color },
  { "strokeWidth", GraphicsContext_get_strokeWidth, GraphicsContext_set_strokeWidth },
  { nullptr, nullptr, nullptr }
};

} // anonymous namespace

DEF_MTNAME(GraphicsContext);

void register_graphics_context_class(lua_State* L)
{
  REG_CLASS(L, GraphicsContext);
  REG_CLASS_PROPERTIES(L, GraphicsContext);
}

} // namespace script
} // namespace app

#endif // ENABLE_UI
