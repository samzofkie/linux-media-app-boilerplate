#include <vector>
#include <iostream>
#include <cstring>

#include <X11/Xlib.h>

#include <cairo.h>
#include <cairo-xlib.h>

#include <pulse/simple.h>

using namespace std;

void err_n_exit(const char* message, int err = 1) {
  cout << message << endl;
  exit(err);
}

class CairoXWindow {
  public:
    double ww, wh;
    Display *display;
    int screen;
    Drawable window;
    cairo_surface_t *surface;
    cairo_t *cr;

    CairoXWindow(double ww_arg, double wh_arg) :
      ww(ww_arg), wh(wh_arg)
    {
      if (!(display = XOpenDisplay(NULL)))
        err_n_exit("XOpenDisplay failed");
      screen = DefaultScreen(display);
      window = XCreateSimpleWindow(display, DefaultRootWindow(display),
          0, 0, ww, wh, 0, 0, 0);
      XSelectInput(display, window, 
          ButtonPressMask | KeyPressMask | ExposureMask); // Nec?
      XMapWindow(display, window);
      surface = cairo_xlib_surface_create(display, window,
          DefaultVisual(display, screen),
          DisplayWidth(display, screen),
          DisplayHeight(display, screen));
      cr = cairo_create(surface);
    }
};

enum PulseStreamType {record, playback};

class PulseStream {
  public:
    pa_simple *s;
    const pa_sample_spec ss;
    int error;

    PulseStream(const pa_sample_spec &ss_arg, PulseStreamType type) :
      ss(ss_arg)
    {
      pa_stream_direction_t dir =
        type == record? PA_STREAM_RECORD : PA_STREAM_PLAYBACK;
      const char *stream_name = 
        type == record? "app_record_stream" : "app_playback_stream";
      if (!(s = pa_simple_new(NULL, "app_name", dir, NULL,
              stream_name, &ss, NULL, NULL, &error))) {
        char * err_mess = "pa_simple_new failed for ";
        err_n_exit(strcat(err_mess, stream_name));
      }
    } 
};

int main() {
  CairoXWindow X(1500, 750);
  static const pa_sample_spec ss = {
    .format = PA_SAMPLE_S16BE,
    .rate = 44100,
    .channels = 2 
  };
  PulseStream record_stream(ss, record);
  PulseStream playback_stream(ss, playback);
 
  for (;;) {
    // Read in data from mic
    const int bufsize = 4096;
    uint8_t buf[bufsize];
    if (pa_simple_read(record_stream.s, buf, sizeof(buf), 
          &record_stream.error) < 0)
      err_n_exit("pa_simple_read failed");

    // Draw Stuff
    cairo_set_source_rgb(X.cr, 0, 1, 1); 
    cairo_move_to(X.cr, 0, X.wh / 2);
    cairo_line_to(X.cr, X.ww, X.wh / 2);
    cairo_stroke(X.cr);
    
    // Respond mouse, keyboard inputs, or window resizes
    if (XPending(X.display)) {
      XEvent e;
      XNextEvent(X.display, &e);
      switch (e.type) {
        case ButtonPress:
          cout << "click @ ";
          cout << '(' << e.xbutton.x << ',';
          cout << e.xbutton.y << ')' << endl;
          break;
        case KeyPress:
          cout << "keypress: " << e.xkey.keycode << endl;
          break;
        case Expose:
          X.ww = e.xexpose.width;
          X.wh = e.xexpose.height;
          break;
      }
    }
  }
}
