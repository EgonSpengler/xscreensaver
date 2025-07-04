/* fps, Copyright © 2001-2025 Jamie Zawinski <jwz@jwz.org>
 * Draw a frames-per-second display (Xlib and OpenGL).
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#include "xlockmoreI.h"
#include "fpsI.h"
#include "texfont.h"

/* These are in xlock-gl.c */
extern void clear_gl_error (void);
extern void check_gl_error (const char *type);

typedef struct {
  texture_font_data *texfont;
  int line_height;
  Bool top_p;
} gl_fps_data;


static void
xlockmore_gl_fps_init (fps_state *st)
{
  gl_fps_data *data = (gl_fps_data *) calloc (1, sizeof(*data));
  int ascent, descent;
  data->top_p = get_boolean_resource (st->dpy, "fpsTop", "FPSTop");
  data->texfont = load_texture_font (st->dpy, "fpsFont");
  texture_string_metrics (data->texfont, "M", 0, &ascent, &descent);
  data->line_height = ascent + descent;
  st->gl_fps_data = data;
}


/* Callback in xscreensaver_function_table, via xlockmore.c.
 */
void
xlockmore_gl_compute_fps (Display *dpy, Window w, fps_state *fpst, 
                          void *closure)
{
  ModeInfo *mi = (ModeInfo *) closure;
  if (! mi->fpst)
    {
      mi->fpst = fpst;
      xlockmore_gl_fps_init (fpst);
    }

  fps_compute (fpst, mi->polygon_count, mi->recursion_depth);
}


/* Called directly from GL programs (as `do_fps') before swapping buffers.
 */
void
xlockmore_gl_draw_fps (ModeInfo *mi)
{
  fps_state *st = mi->fpst;
  if (st)   /* might be too early */
    {
      gl_fps_data *data = (gl_fps_data *) st->gl_fps_data;
      XWindowAttributes xgwa;
      XGetWindowAttributes (st->dpy, st->window, &xgwa);

# ifndef HAVE_ANDROID
      /* This crashes in the Android emulator, but not on real hardware. */
      glColor3f (1, 1, 1);
# endif

      print_texture_label (st->dpy, data->texfont,
                           xgwa.width, xgwa.height,
                           (data->top_p ? 1 : 2),
                           st->string);
    }
}

void
xlockmore_gl_free_fps (fps_state *st)
{
  gl_fps_data *data = (gl_fps_data *) st->gl_fps_data;
  if (data)
    {
      if (data->texfont) free_texture_font (data->texfont);
      free (data);
      st->gl_fps_data = 0;
    }
  fps_free (st);
}
