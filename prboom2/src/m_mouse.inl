//
// Copyright(C) 2026 by Andrik Powell
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	DSDA Menu Mouse Functions
//

#define MENU_MOUSE_LEFT        1
#define MENU_MOUSE_RIGHT       2
#define MENU_MOUSE_MIDDLE      4
#define MENU_MOUSE_HEIGHT      200
#define MENU_MOUSE_TAB_Y_PAD   4
#define MENU_MOUSE_TAB_X_PAD   6
#define MENU_MOUSE_MAIN_X_PAD  4
#define MENU_MOUSE_MAIN_Y_PAD  2
#define MENU_MOUSE_SETUP_X_PAD 3
#define MENU_MOUSE_SETUP_Y_PAD 2

typedef struct
{
  int left;
  int top;
  int right;
  int bottom;
} menu_mouse_rect_t;

static int menu_mouse_buttons;
static int menu_mouse_x = BASE_WIDTH / 2;
static int menu_mouse_y = MENU_MOUSE_HEIGHT / 2;
static dboolean menu_mouse_in_viewport;
static int menu_mouse_drag_setup = -1;
static int menu_mouse_drag_main = -1;
static int menu_mouse_hover_main = -1;
static int menu_mouse_hover_tab = -1;

static dboolean M_MouseSoundSliderAtPointer(int *index);

dboolean M_MouseHovered(int index)
{
  return index == menu_mouse_hover_main &&
         currentMenu &&
         index >= 0 &&
         index < currentMenu->numitems &&
         currentMenu->menuitems[index].status != M_ITEM_SKIP;
}

static dboolean M_MouseTabHovered(int page)
{
  return page == menu_mouse_hover_tab;
}

static int M_MouseClamp(int value, int low, int high)
{
  return BETWEEN(low, high, value);
}

static void M_MouseBeginSetupNavigation(void)
{
  setup_menu_layout_t layout;

  if (!setup_active || !current_setup_menu || menu_mouse_setup_scroll != KEYBOARD_NAV)
    return;

  M_GetSetupMenuLayout(current_setup_menu, DEFAULT_LIST_Y, &layout);
  menu_mouse_setup_scroll = layout.scroll_i;
}

static int M_MouseMenuRowHeight(void)
{
  return raven ? 20 : LINEHEIGHT;
}

static void M_MouseClearMainHover(void)
{
  menu_mouse_hover_main = -1;
}

static void M_MouseClearTabHover(void)
{
  menu_mouse_hover_tab = -1;
}

static void M_MouseResetButtons(void)
{
  menu_mouse_buttons = 0;
  menu_mouse_drag_setup = -1;
  menu_mouse_drag_main = -1;
}

static void M_MouseSetLogicalPosition(int x, int y)
{
  int viewport_w = viewport_rect.w > 0 ? viewport_rect.w :
                   (SCREENWIDTH > 0 ? SCREENWIDTH : BASE_WIDTH);
  int viewport_h = viewport_rect.h > 0 ? viewport_rect.h :
                   (SCREENHEIGHT > 0 ? SCREENHEIGHT : MENU_MOUSE_HEIGHT);
  int screen_w = SCREENWIDTH > 0 ? SCREENWIDTH : BASE_WIDTH;
  int screen_h = SCREENHEIGHT > 0 ? SCREENHEIGHT : MENU_MOUSE_HEIGHT;
  stretch_param_t *stretch = dsda_StretchParams(VPT_STRETCH);

  menu_mouse_in_viewport = x >= 0 && y >= 0 && x < viewport_w && y < viewport_h;

  x = M_MouseClamp(x, 0, viewport_w - 1);
  y = M_MouseClamp(y, 0, viewport_h - 1);

  x = x * screen_w / viewport_w;
  y = y * screen_h / viewport_h;

  if (stretch && stretch->video &&
      stretch->video->width > 0 && stretch->video->height > 0)
  {
    x = (x - stretch->deltax1) * BASE_WIDTH / stretch->video->width;
    y = (y - stretch->deltay1) * MENU_MOUSE_HEIGHT / stretch->video->height;
  }
  else
  {
    x = x * BASE_WIDTH / screen_w;
    y = y * MENU_MOUSE_HEIGHT / screen_h;
  }

  menu_mouse_x = M_MouseClamp(x, 0, BASE_WIDTH - 1);
  menu_mouse_y = M_MouseClamp(y, 0, MENU_MOUSE_HEIGHT - 1);
}

static void M_MouseReadPosition(void)
{
  int window_x, window_y;
  int renderer_x, renderer_y;

  SDL_GetMouseState(&window_x, &window_y);

  if (window_rect.w > 0 && window_rect.h > 0 &&
      renderer_rect.w > 0 && renderer_rect.h > 0)
  {
    renderer_x = window_x * renderer_rect.w / window_rect.w;
    renderer_y = window_y * renderer_rect.h / window_rect.h;
  }
  else
  {
    renderer_x = window_x;
    renderer_y = window_y;
  }

  M_MouseSetLogicalPosition(renderer_x - viewport_rect.x,
                            renderer_y - viewport_rect.y);
}

static int M_MouseWheelAction(event_t *ev)
{
  if (ev->type != ev_keydown && ev->type != ev_keyup)
    return MENU_NULL;

  switch (ev->data1.i)
  {
    case KEYD_MWHEELUP:
      return MENU_UP;
    case KEYD_MWHEELDOWN:
      return MENU_DOWN;
    case KEYD_MWHEELLEFT:
      return MENU_LEFT;
    case KEYD_MWHEELRIGHT:
      return MENU_RIGHT;
    default:
      return MENU_NULL;
  }
}

static dboolean M_MouseBindingCaptureActive(void)
{
  return setup_active && set_keybnd_active && setup_select &&
         current_setup_menu &&
         (current_setup_menu[set_menu_itemon].m_flags & S_INPUT);
}

static dboolean M_MouseMainItemRect(const menuitem_t *item, int index,
                                    dboolean lumps_missing,
                                    menu_mouse_rect_t *rect)
{
  int x = currentMenu->x;
  int y = currentMenu->y + index * LINEHEIGHT;

  if (raven)
  {
    if (!item->alttext)
      return false;

    rect->left = x;
    rect->top = currentMenu->y + index * M_MouseMenuRowHeight();
    rect->right = x + MN_TextBWidth(item->alttext);
    rect->bottom = rect->top + M_MouseMenuRowHeight();
    return rect->right > rect->left;
  }

  if (!lumps_missing && item->name[0] && W_LumpNameExists(item->name))
  {
    const rpatch_t *patch = R_PatchByName(item->name);

    rect->left = x - patch->leftoffset;
    rect->top = y - patch->topoffset;
    rect->right = rect->left + patch->width;
    rect->bottom = rect->top + patch->height;
    return true;
  }

  if (!item->alttext)
    return false;

  rect->left = x;
  rect->top = y + 8 - (M_StringHeight(item->alttext) / 2);
  rect->right = x + M_StringWidth(item->alttext);
  rect->bottom = rect->top + M_StringHeight(item->alttext);

  return rect->right > rect->left;
}

static int M_MousePixelWidth(const char *text)
{
  int width;

  if (!text || !text[0])
    return 0;

  width = M_GetPixelWidth(text);

  return width > 0 ? width : 0;
}

static int M_MousePixelWidthN(const char *text, size_t len)
{
  size_t i;
  int width = 0;

  if (!text || !len)
    return 0;

  for (i = 0; i < len; i++)
  {
    int c = toupper(text[i]) - HU_FONTSTART;

    if (c < 0 || c > HU_FONTSIZE)
      width += menu_font->space_width;
    else
      width += menu_font->font[c].width;

    width += g_menu_font_spacing;
  }

  width -= g_menu_font_spacing;

  return width > 0 ? width : 0;
}

static dboolean M_MousePointInPaddedRect(const menu_mouse_rect_t *rect,
                                         int x_pad, int y_pad)
{
  if (!rect || rect->right <= rect->left || rect->bottom <= rect->top)
    return false;

  return menu_mouse_x >= rect->left - x_pad &&
         menu_mouse_x < rect->right + x_pad &&
         menu_mouse_y >= rect->top - y_pad &&
         menu_mouse_y < rect->bottom + y_pad;
}

static dboolean M_MousePointInRect(const menu_mouse_rect_t *rect)
{
  return M_MousePointInPaddedRect(rect,
                                  MENU_MOUSE_MAIN_X_PAD,
                                  MENU_MOUSE_MAIN_Y_PAD);
}

static dboolean M_MouseMainItemAtPointer(int *index)
{
  int i;
  dboolean lumps_missing;

  if (!menu_mouse_in_viewport || !currentMenu || currentMenu->numitems <= 0)
    return false;

  lumps_missing = M_MenuHasMissingRequiredLumps(currentMenu);

  for (i = 0; i < currentMenu->numitems; i++)
  {
    menu_mouse_rect_t rect;

    if (currentMenu->menuitems[i].status == M_ITEM_SKIP)
      continue;

    if (M_MouseMainItemRect(&currentMenu->menuitems[i], i, lumps_missing, &rect) &&
        M_MousePointInRect(&rect))
    {
      *index = i;
      return true;
    }
  }

  return false;
}

static dboolean M_MouseSaveItemRect(int index, menu_mouse_rect_t *rect)
{
  if (index < 0 || index >= currentMenu->numitems)
    return false;

  if (raven)
  {
    const rpatch_t *patch = R_PatchByName("M_FSLOT");

    rect->left = currentMenu->x - patch->leftoffset;
    rect->top = currentMenu->y + index * M_MouseMenuRowHeight() + 5;
    rect->right = rect->left + patch->width; // [AR] Get full width of slot
    rect->bottom = rect->top + MN_TextAHeight(savegamestrings[index]);
  }
  else
  {
    rect->left = currentMenu->x - 8;
    rect->top = currentMenu->y + index * LINEHEIGHT;
    rect->right = currentMenu->x + (24 + 1) * 8; // [AR] Get full width of slot
    rect->bottom = rect->top + M_StringHeight(savegamestrings[index]);
  }

  return rect->right > rect->left;
}

static dboolean M_MouseSaveItemAtPointer(int *index)
{
  int i;

  if (!menu_mouse_in_viewport ||
      (currentMenu != &LoadDef && currentMenu != &SaveDef))
    return false;

  for (i = 0; i < currentMenu->numitems; i++)
  {
    menu_mouse_rect_t rect;

    if (M_MouseSaveItemRect(i, &rect) && M_MousePointInRect(&rect))
    {
      *index = i;
      return true;
    }
  }

  return false;
}

static dboolean M_MouseUpdateMainHover(void)
{
  int index;

  if (!M_MouseMainItemAtPointer(&index) &&
      !M_MouseSaveItemAtPointer(&index) &&
      !M_MouseSoundSliderAtPointer(&index))
  {
    M_MouseClearMainHover();
    return false;
  }

  if (menu_mouse_hover_main != index)
  {
    menu_mouse_hover_main = index;
    itemOn = index;
    S_StartOptionalSound(g_sfx_mnumov, g_sfx_menu, true);
  }

  return true;
}

static dboolean M_MouseSelectMainItem(void)
{
  int index;

  if (!M_MouseMainItemAtPointer(&index) &&
      !M_MouseSaveItemAtPointer(&index) &&
      !M_MouseSoundSliderAtPointer(&index))
    return false;

  menu_mouse_hover_main = index;
  itemOn = index;

  return true;
}

static int M_MouseSliderValue(int x, int slider_x, int width, int low, int high)
{
  int range = high - low + 1;
  int rel = x - slider_x - 8;
  int span = width * 8;
  int value;

  if (range <= 1 || width <= 0)
    return low;

  rel = M_MouseClamp(rel, 0, span - 1);
  value = low + rel * range / span;

  return M_MouseClamp(value, low, high);
}

static int M_MouseThermoValue(int x, int slider_x, int width, int low, int high)
{
  if (raven && width > 3)
  {
    // MN_DrawSlider() shortens the bar and positions the knob from x + 20.
    return M_MouseSliderValue(x, slider_x + 12, width - 3, low, high);
  }

  return M_MouseSliderValue(x, slider_x, width, low, high);
}

static dboolean M_MouseSetSoundSlider(int index)
{
  dsda_config_identifier_t id;
  int value;

  if (currentMenu != &SoundDef || index < 0 || index >= currentMenu->numitems ||
      currentMenu->menuitems[index].status != M_ITEM_THERMO)
    return false;

  id = index == sfx_vol ? dsda_config_sfx_volume : dsda_config_music_volume;
  value = M_MouseThermoValue(menu_mouse_x, raven ? SoundDef.x - 8 : SoundDef.x,
                             16, 0, 15);

  if (dsda_IntConfig(id) != value)
  {
    dsda_UpdateIntConfig(id, value, true);

    if (index == sfx_vol && dsda_IntConfig(dsda_config_mute_sfx))
      dsda_ToggleConfig(dsda_config_mute_sfx, true);
    else if (index == music_vol && dsda_IntConfig(dsda_config_mute_music))
      dsda_ToggleConfig(dsda_config_mute_music, true);

    S_StartOptionalSound(g_sfx_mnusli, g_sfx_stnmov, false);
  }

  itemOn = index;
  return true;
}

static dboolean M_MouseSoundSliderAtPointer(int *index)
{
  int i;
  int row_height;
  int left;
  int right;

  if (!menu_mouse_in_viewport || currentMenu != &SoundDef)
    return false;

  row_height = M_MouseMenuRowHeight();
  left = raven ? SoundDef.x - 20 : SoundDef.x - 8;
  right = raven ? SoundDef.x + 140 : SoundDef.x + 16 * 8 + 16;

  for (i = 0; i < currentMenu->numitems; i++)
  {
    int y;

    if (currentMenu->menuitems[i].status != M_ITEM_THERMO)
      continue;

    y = SoundDef.y + row_height * (i + 1);
    if (menu_mouse_y >= y - 4 && menu_mouse_y < y + row_height &&
        menu_mouse_x >= left && menu_mouse_x <= right)
    {
      *index = i;
      return true;
    }
  }

  return false;
}

static dboolean M_MouseSetupTextRangeAtPointer(const char *text, size_t len,
                                               int x, int y)
{
  menu_mouse_rect_t rect;
  int width = M_MousePixelWidthN(text, len);

  if (width <= 0)
    return false;

  rect.left = x;
  rect.top = y;
  rect.right = x + width;
  rect.bottom = y + menu_font->height;

  return M_MousePointInPaddedRect(&rect,
                                  MENU_MOUSE_SETUP_X_PAD,
                                  MENU_MOUSE_SETUP_Y_PAD);
}

static dboolean M_MouseSetupTextAtPointer(const char *text, int x, int y)
{
  return M_MouseSetupTextRangeAtPointer(text, strlen(text), x, y);
}

static dboolean M_MouseSetupDescriptionAtPointer(const setup_menu_t *item,
                                                 int y)
{
  const char *line;
  int x = item->m_x;
  int flags = item->m_flags;

  if (!(flags & S_SHOWDESC) || !item->m_text)
    return false;

  for (line = item->m_text; *line; y += 8)
  {
    const char *end;
    size_t len;
    int width;
    int left = x;

    while (*line == '\n')
      line++;

    if (!*line)
      break;

    end = strchr(line, '\n');
    len = end ? (size_t)(end - line) : strlen(line);
    width = M_MousePixelWidthN(line, len);

    if (flags & S_CENTER)
      left = (BASE_WIDTH - width) / 2;
    else if (!(flags & S_LEFTJUST))
      left -= width + 4;

    if (M_MouseSetupTextRangeAtPointer(line, len, left, y))
      return true;

    if (!end)
      break;

    line = end + 1;
  }

  return false;
}

static dboolean M_MouseSetupSettingAtPointer(const setup_menu_t *item, int y)
{
  char text[MENU_BUFFER_SIZE];
  menu_mouse_rect_t rect;
  int width;

  if (!(item->m_flags & S_SHOWSET))
    return false;

  if (item->m_flags & S_COLOR)
  {
    rect.left = item->m_x;
    rect.top = y - 1;
    rect.right = item->m_x + 8;
    rect.bottom = y + menu_font->height;

    if (M_ShowBlinkingArrowRight(item))
      rect.right += M_MousePixelWidth(" <");

    return M_MousePointInPaddedRect(&rect,
                                    MENU_MOUSE_SETUP_X_PAD,
                                    MENU_MOUSE_SETUP_Y_PAD);
  }

  if (!M_SetupSettingText(item, text, sizeof(text), false))
    return false;

  width = M_MousePixelWidth(text);

  if (item->m_flags & S_THERMO)
  {
    rect.left = item->m_x;
    rect.top = y;
    rect.right = item->m_x + 80 + width;
    rect.bottom = y + 3 + menu_font->height;
  }
  else
  {
    rect.left = item->m_x;
    rect.top = y;
    rect.right = item->m_x + width;
    rect.bottom = y + menu_font->height;
  }

  return M_MousePointInPaddedRect(&rect,
                                  MENU_MOUSE_SETUP_X_PAD,
                                  MENU_MOUSE_SETUP_Y_PAD);
}

static dboolean M_MouseSetupItemAtVisibleText(const setup_menu_t *item,
                                              int desc_y, int set_y)
{
  return M_MouseSetupDescriptionAtPointer(item, desc_y) ||
         M_MouseSetupSettingAtPointer(item, set_y);
}

static dboolean M_MouseSetupItemSelectable(const setup_menu_t *item)
{
  return !(item->m_flags & (S_SKIP | S_END | S_PREV | S_NEXT |
                            S_RESET_Y | S_NOSELECT));
}

#define MENU_LEVELTABLE_LEFT 8
#define MENU_LEVELTABLE_RIGHT 309

// [AR] Allow mouse to highlight full row for level table
static dboolean M_MouseLevelTableRowAtPointer(int y)
{
  menu_mouse_rect_t rect;

  if (!level_table_active)
    return false;

  rect.left = MENU_LEVELTABLE_LEFT;
  rect.top = y;
  rect.right = MENU_LEVELTABLE_RIGHT;
  rect.bottom = y + menu_font->height;

  return M_MousePointInPaddedRect(&rect, 0, MENU_MOUSE_SETUP_Y_PAD);
}

// [AR] Allow mouse select for color picker
static dboolean M_MouseColorChipAtPointer(void)
{
  int x;
  int y;

  if (!menu_mouse_in_viewport || !setup_active || !setup_select ||
      !colorbox_active)
    return false;

  x = menu_mouse_x - COLORPALXORIG;
  y = menu_mouse_y - COLORPALYORIG;

  if (x < 0 || x >= 16 * CHIP_SIZE ||
      y < 0 || y >= 16 * CHIP_SIZE)
    return false;

  x /= CHIP_SIZE;
  y /= CHIP_SIZE;

  if (x != color_palette_x || y != color_palette_y)
  {
    color_palette_x = x;
    color_palette_y = y;
    S_StartOptionalSound(g_sfx_mnusel, g_sfx_itemup, false);
  }

  return true;
}

static dboolean M_MouseSetupItemAtPointer(int *index)
{
  int i = 0;
  int carry_y = 0; // Larger items add an offset that carries over to following settings.
  dboolean found = false;
  setup_menu_layout_t layout;
  setup_menu_t *src;

  if (!menu_mouse_in_viewport || !current_setup_menu)
    return false;

  M_GetSetupMenuLayout(current_setup_menu, DEFAULT_LIST_Y, &layout);

  i = 0;
  for (src = current_setup_menu; !(src->m_flags & S_END); src++)
  {
    int desc_y;
    int item_y;

    if (!M_GetSetupItemPosition(src, DEFAULT_LIST_Y, &layout, &i, &carry_y, &desc_y, &item_y))
      continue;

    if (!M_MouseSetupItemSelectable(src))
      continue;

    if (M_MouseLevelTableRowAtPointer(desc_y) ||
        M_MouseSetupItemAtVisibleText(src, desc_y, item_y))
    {
      // Later entries are drawn later, so they win if padded hitboxes overlap.
      *index = (int)(src - current_setup_menu);
      found = true;
    }
  }

  return found;
}

static dboolean M_MouseSetupThermoAtPointer(const setup_menu_t *item)
{
  return menu_mouse_x >= item->m_x &&
         menu_mouse_x < item->m_x + 80;
}

static const char **M_MouseCurrentTabs(int *visible_tabs, int *y,
                                       setup_menu_t ***setup_pages)
{
  *visible_tabs = 0;
  *y = 0;
  *setup_pages = NULL;

  if (currentMenu == &LoadDef || currentMenu == &SaveDef)
  {
    *visible_tabs = 5;
    *y = raven ? 135 : 145;
    return saves_pages;
  }

  if (!setup_active)
    return NULL;

  *y = TABS_Y;
  *visible_tabs = setup_page_context.visible_tabs;
  *setup_pages = setup_page_context.pages;

  return setup_page_context.labels;
}

static dboolean M_MouseTabAtPointer(const char **pages, int visible_tabs,
                                    int y, int *target_page)
{
  int x;
  int i;
  menu_tab_layout_t layout;

  M_GetTabLayout(pages, visible_tabs, &layout);

  if (!menu_mouse_in_viewport || !layout.page_count ||
      menu_mouse_y < y - MENU_MOUSE_TAB_Y_PAD ||
      menu_mouse_y >= y + DEFAULT_LIST_Y - TABS_Y - 1)
    return false;

  x = layout.x;

  if (layout.start_i > 0)
  {
    int arrow_w = M_GetPixelWidth("<-");
    int arrow_x = x - arrow_w - 2;

    if (menu_mouse_x >= arrow_x - MENU_TAB_GAP / 2 &&
        menu_mouse_x < arrow_x + arrow_w + MENU_TAB_GAP / 2)
    {
      *target_page = current_page - 1;
      return true;
    }
  }

  if (layout.end_i + 1 < layout.page_count)
  {
    int arrow_w = M_GetPixelWidth("->");
    int arrow_x = BASE_WIDTH - x + 2;

    if (menu_mouse_x >= arrow_x - MENU_TAB_GAP / 2 &&
        menu_mouse_x < arrow_x + arrow_w + MENU_TAB_GAP / 2)
    {
      *target_page = current_page + 1;
      return true;
    }
  }

  for (i = layout.start_i; i <= layout.end_i; i++)
  {
    int w = M_GetPixelWidth(pages[i]);
    int left;
    int right;

    left = x - MENU_TAB_GAP / 2;
    right = x + w + MENU_TAB_GAP / 2;

    if (i == layout.start_i && layout.start_i == 0)
      left -= MENU_MOUSE_TAB_X_PAD;
    if (i == layout.end_i && layout.end_i + 1 == layout.page_count)
      right += MENU_MOUSE_TAB_X_PAD;

    if (menu_mouse_x >= left && menu_mouse_x < right)
    {
      *target_page = i;
      return true;
    }

    x += w + MENU_TAB_GAP;
  }

  return false;
}

static dboolean M_MouseUpdateTabHover(void)
{
  const char **pages;
  setup_menu_t **setup_pages;
  int visible_tabs;
  int y;
  int target_page;

  pages = M_MouseCurrentTabs(&visible_tabs, &y, &setup_pages);
  if (!M_MouseTabAtPointer(pages, visible_tabs, y, &target_page))
  {
    M_MouseClearTabHover();
    return false;
  }

  if (menu_mouse_hover_tab != target_page)
  {
    menu_mouse_hover_tab = target_page;
    S_StartOptionalSound(g_sfx_mnusel, g_sfx_itemup, false);
  }

  return true;
}

static dboolean M_MouseSwitchSetupPage(setup_menu_t **pages, int target_page)
{
  if (!pages || target_page < 0 || !pages[target_page])
    return false;

  if (target_page == current_page)
    return true;

  M_MouseClearTabHover();
  current_setup_menu[set_menu_itemon].m_flags &= ~(S_HILITE | S_SELECT);
  M_SetSetupMenuItemOn(set_menu_itemon);

  previous_page = current_page;
  current_page = target_page;
  M_UpdateSetupMenu(pages[target_page]);
  M_MouseBeginSetupNavigation();
  setup_select = false;
  setup_gather = false;
  colorbox_active = false;
  S_StartOptionalSound(g_sfx_mnumov, g_sfx_menu, true);

  return true;
}

static dboolean M_MouseSwitchSavePage(int target_page)
{
  if (target_page < 0 || target_page >= save_page_limit)
    return false;

  if (target_page == current_page)
    return true;

  M_MouseClearTabHover();
  current_page = target_page;
  M_ReadSaveStrings();
  S_StartOptionalSound(g_sfx_mnumov, g_sfx_menu, true);

  return true;
}

static dboolean M_MouseActivateTab(void)
{
  const char **pages;
  setup_menu_t **setup_pages;
  int visible_tabs;
  int y;
  int target_page;

  pages = M_MouseCurrentTabs(&visible_tabs, &y, &setup_pages);
  if (!M_MouseTabAtPointer(pages, visible_tabs, y, &target_page))
    return false;

  if (setup_pages)
    return M_MouseSwitchSetupPage(setup_pages, target_page);

  return M_MouseSwitchSavePage(target_page);
}

static void M_MouseSelectSetupItem(int index)
{
  setup_menu_t *old_item;
  setup_menu_t *new_item;

  if (!current_setup_menu || index == set_menu_itemon)
    return;

  old_item = current_setup_menu + set_menu_itemon;
  new_item = current_setup_menu + index;

  old_item->m_flags &= ~(S_HILITE | S_SELECT);
  set_menu_itemon = index;
  M_SetSetupMenuItemOn(set_menu_itemon);
  new_item->m_flags |= S_HILITE;
  setup_select = false;
  setup_gather = false;
  colorbox_active = false;
  S_StartOptionalSound(g_sfx_mnusel, g_sfx_itemup, false);
}

static void M_MouseUpdateSetupHover(void)
{
  int index;

  if (M_MouseSetupItemAtPointer(&index))
    M_MouseSelectSetupItem(index);
}

static dboolean M_MouseScrollSetup(int action)
{
  setup_menu_layout_t layout;
  int scroll;

  if (!setup_active || setup_select || !current_setup_menu ||
      (action != MENU_UP && action != MENU_DOWN))
    return false;

  M_MouseBeginSetupNavigation();
  M_GetSetupMenuLayout(current_setup_menu, DEFAULT_LIST_Y, &layout);

  if (!layout.excess_i)
    return false;

  scroll = menu_mouse_setup_scroll + (action == MENU_DOWN ? 1 : -1);
  menu_mouse_setup_scroll = M_MouseClamp(scroll, 0, layout.excess_i);
  M_MouseUpdateSetupHover();

  return true;
}

static dboolean M_MouseSetSetupThermo(int index)
{
  setup_menu_t *item;
  int value;

  if (!current_setup_menu || index < 0)
    return false;

  item = current_setup_menu + index;
  if (!(item->m_flags & S_THERMO))
    return false;

  value = M_MouseThermoValue(menu_mouse_x, item->m_x, 8, 0, 15);
  if (dsda_IntConfig(item->config_id) != value)
  {
    dsda_UpdateIntConfig(item->config_id, value, true);
    S_StartOptionalSound(g_sfx_mnusli, g_sfx_stnmov, false);
  }

  return true;
}

static int M_MouseNextSetupChoice(const char **choices, int current)
{
  int next;

  if (!choices)
    return current;

  next = current + 1;
  while (choices[next] && choices[next][0] == '~')
    next++;

  if (!choices[next])
  {
    next = 0;
    while (choices[next] && choices[next][0] == '~')
      next++;
  }

  return choices[next] ? next : current;
}

static dboolean M_MouseCycleSetupChoice(setup_menu_t *item)
{
  int current;
  int next;

  if (!(item->m_flags & S_CHOICE) || !item->selectstrings)
    return false;

  if (dsda_StrictMode() && dsda_IsStrictConfig(item->config_id))
    return true;

  if (item->m_flags & S_STR)
  {
    current = M_IndexInChoices(dsda_StringConfig(item->config_id),
                               item->selectstrings);
    next = M_MouseNextSetupChoice(item->selectstrings, current);

    if (next != current)
      dsda_UpdateStringConfig(item->config_id, item->selectstrings[next], true);
  }
  else
  {
    current = dsda_IntConfig(item->config_id);
    next = M_MouseNextSetupChoice(item->selectstrings, current);

    if (next != current)
      dsda_UpdateIntConfig(item->config_id, next, true);
  }

  if (next != current)
    S_StartOptionalSound(g_sfx_mnusel, g_sfx_itemup, false);

  return true;
}

static dboolean M_MouseActivateSetupItem(event_t *ev)
{
  int index;
  setup_menu_t *item;

  if (setup_select)
  {
    item = current_setup_menu + set_menu_itemon;

    if (item->m_flags & S_COLOR)
    {
      if (!M_MouseColorChipAtPointer())
        return true;

      return M_SetupResponder(MENU_NULL, MENU_ENTER, ev);
    }

    if (item->m_flags & S_THERMO)
    {
      if (!M_MouseSetupThermoAtPointer(item))
        return true;

      menu_mouse_drag_setup = set_menu_itemon;
      return M_MouseSetSetupThermo(set_menu_itemon);
    }

    if (item->m_flags & S_CHOICE)
      return true;

    return M_SetupResponder(MENU_NULL, MENU_ENTER, ev);
  }

  if (!M_MouseSetupItemAtPointer(&index))
    return true;

  M_MouseSelectSetupItem(index);
  item = current_setup_menu + set_menu_itemon;

  if (item->m_flags & S_THERMO)
  {
    if (!M_MouseSetupThermoAtPointer(item))
      return true;

    menu_mouse_drag_setup = set_menu_itemon;
    return M_MouseSetSetupThermo(set_menu_itemon);
  }

  if (item->m_flags & S_YESNO)
  {
    if (!M_SetupResponder(MENU_NULL, MENU_ENTER, ev))
      return false;

    return M_SetupResponder(MENU_NULL, MENU_ENTER, ev);
  }

  if (item->m_flags & S_CHOICE)
    return M_MouseCycleSetupChoice(item);

  return M_SetupResponder(MENU_NULL, MENU_ENTER, ev);
}

static dboolean M_MouseCancelSetupSelection(event_t *ev)
{
  if (!setup_active || !setup_select || !current_setup_menu)
    return false;

  menu_mouse_drag_setup = -1;
  menu_mouse_drag_main = -1;

  return M_SetupResponder(MENU_NULL, MENU_ESCAPE, ev);
}

static dboolean M_MouseMenuAction(int action, event_t *ev)
{
  if (action == MENU_NULL)
    return false;

  if (messageToPrint)
    return M_MessageResponder(MENU_NULL, action, ev);

  if (!menuactive)
    return false;

  if (currentMenu == &LoadDef || currentMenu == &SaveDef)
    if (M_SaveResponder(MENU_NULL, action, ev))
      return true;

  if (setup_active)
    if (M_SetupResponder(MENU_NULL, action, ev))
      return true;

  if (M_MainNavigationResponder(MENU_NULL, action, ev))
    return true;

  return false;
}

static dboolean M_MouseWheelResponder(event_t *ev, int action)
{
  if (action == MENU_NULL)
    return false;

  if (ev->type == ev_keyup)
    return !M_MouseBindingCaptureActive();

  if (M_MouseBindingCaptureActive())
    return false;

  M_MouseReadPosition();
  M_MouseClearMainHover();
  M_MouseClearTabHover();

  if (M_MouseScrollSetup(action))
    return true;

  return M_MouseMenuAction(action, ev);
}

static dboolean M_MouseBindingCaptureResponder(event_t *ev)
{
  if (ev->type == ev_mouse)
  {
    int buttons = ev->data1.i;

    menu_mouse_buttons = buttons;

    return buttons == 0;
  }

  return true;
}

static dboolean M_MouseSoundSliderTitleAtPointer(void)
{
  return currentMenu->menuitems[itemOn].status == M_ITEM_THERMO;
}

static dboolean M_MouseMotionResponder(void)
{
  if (menu_mouse_drag_setup >= 0 && (menu_mouse_buttons & MENU_MOUSE_LEFT))
    return M_MouseSetSetupThermo(menu_mouse_drag_setup);

  if (menu_mouse_drag_main >= 0 && (menu_mouse_buttons & MENU_MOUSE_LEFT))
    return M_MouseSetSoundSlider(menu_mouse_drag_main);

  if (M_MouseColorChipAtPointer())
    return true;

  if (menuactive && M_MouseUpdateTabHover())
  {
    M_MouseClearMainHover();
    return true;
  }

  if (menuactive && setup_active && !setup_select)
  {
    M_MouseUpdateSetupHover();
    return true;
  }

  if (menuactive)
  {
    M_MouseUpdateMainHover();
    return true;
  }

  return messageToPrint;
}

static dboolean M_MouseLeftPressResponder(event_t *ev)
{
  int slider_index;

  if (messageToPrint)
    return M_MouseMenuAction(MENU_ENTER, ev);

  if (!menuactive)
    return false;

  if (inhelpscreens)
    return M_MouseMenuAction(MENU_ENTER, ev);

  if (M_MouseActivateTab())
    return true;

  if (setup_active)
    return M_MouseActivateSetupItem(ev);

  if (M_MouseSoundSliderAtPointer(&slider_index))
  {
    menu_mouse_drag_main = slider_index;
    return M_MouseSetSoundSlider(slider_index);
  }

  if (M_MouseSoundSliderTitleAtPointer())
    return true;

  if (!M_MouseSelectMainItem())
    return true;

  return M_MouseMenuAction(MENU_ENTER, ev);
}

static dboolean M_MouseButtonResponder(event_t *ev)
{
  int buttons = ev->data1.i;
  int pressed = buttons & ~menu_mouse_buttons;
  int released = menu_mouse_buttons & ~buttons;

  menu_mouse_buttons = buttons;

  if (released & MENU_MOUSE_LEFT)
  {
    menu_mouse_drag_setup = -1;
    menu_mouse_drag_main = -1;
  }

  if ((buttons & MENU_MOUSE_LEFT) && menu_mouse_drag_setup >= 0)
    return M_MouseSetSetupThermo(menu_mouse_drag_setup);

  if ((buttons & MENU_MOUSE_LEFT) && menu_mouse_drag_main >= 0)
    return M_MouseSetSoundSlider(menu_mouse_drag_main);

  if ((pressed & MENU_MOUSE_RIGHT) && M_MouseCancelSetupSelection(ev))
    return true;

  if ((pressed & MENU_MOUSE_RIGHT) && saveStringEnter &&
      currentMenu == &SaveDef)
    return M_MouseMenuAction(MENU_ESCAPE, ev);

  if (pressed & MENU_MOUSE_RIGHT)
    return M_MouseMenuAction(MENU_BACKSPACE, ev);

  if (pressed & MENU_MOUSE_MIDDLE)
    return M_MouseMenuAction(MENU_CLEAR, ev);

  if (pressed & MENU_MOUSE_LEFT)
    return M_MouseLeftPressResponder(ev);

  return false;
}

static dboolean M_MouseResponder(event_t *ev)
{
  int action = M_MouseWheelAction(ev);

  if (!menuactive && !messageToPrint)
  {
    dboolean click_to_open_menu = (gamestate == GS_DEMOSCREEN ||
                                  demoplayback);
  
    if (click_to_open_menu && ev->type == ev_mouse &&
        (ev->data1.i & MENU_MOUSE_LEFT))
    {
      M_MouseReadPosition();

      // [AR] Bring up the menu if mouse is not on demo progressbar
      if (menu_mouse_in_viewport &&
          (!demoplayback || !HU_MouseOnDemoProgressBar(NULL)))
      {
        M_StartControlPanel();
        S_StartOptionalSound(g_sfx_mnuopn, g_sfx_swtchn, true);
        return true;
      }
    }

    return false;    
  }

  if (M_MouseWheelResponder(ev, action))
    return true;

  if (action != MENU_NULL)
    return false;

  if (ev->type != ev_mouse && ev->type != ev_mousemotion)
    return false;

  M_MouseReadPosition();

  if (setup_active && !setup_select)
    M_MouseBeginSetupNavigation();

  if (M_MouseBindingCaptureActive())
    return M_MouseBindingCaptureResponder(ev);

  if (ev->type == ev_mousemotion)
    return M_MouseMotionResponder();

  return M_MouseButtonResponder(ev);
}
