diff --git a/cleanwm.c b/cleanwm.c
index b17216a..48a86a5 100644
--- a/cleanwm.c
+++ b/cleanwm.c
@@ -55,19 +55,6 @@ typedef struct {
 } Desktop;
 
 typedef struct {
-	/* note number of desktops MUST be DESKTOPS+1
-	 * becouse of activate_left_view() and activate_right_view()
-	 */
-	Desktop ld[11];		/* left desktops */
-	Desktop rd[11];		/* right desktops */
-	int curr_left_id;
-	int prev_left_id;
-	int curr_right_id;
-	int prev_right_id;
-	int curr_desk;		/* current view, LEFT/RIGHT */
-} View;
-
-typedef struct {
 	GC gc;
 	XColor color;
 	Colormap colormap;
@@ -130,6 +117,17 @@ static void maprequest(XEvent *e);
 /** Include config **/
 #include "config.h"
 
+/* view struct */
+typedef struct {
+	Desktop ld[DESKTOPS + 1];	/* left desktops */
+	Desktop rd[DESKTOPS + 1];	/* right desktops */
+	int curr_left_id;
+	int prev_left_id;
+	int curr_right_id;
+	int prev_right_id;
+	int curr_desk;		/* current view, LEFT/RIGHT */
+} View;
+
 /** Variables **/
 static int bar_height;
 static int sh;
@@ -470,6 +468,12 @@ void client_to_view(const Arg *arg)
 	/* DBG */	fprintf(stderr, "client_to_view(): OUT\n");
 }
 
+void change_view(const Arg *arg) {
+	/* DBG */	fprintf(stderr, "change_view(): IN\n");
+
+	/* DBG */	fprintf(stderr, "change_views(): OUT\n");
+}
+
 void change_left_desktop(const Arg *arg)
 {
 	/* DBG */	fprintf(stderr, "change_left_desktop(): IN\n");
@@ -478,7 +482,7 @@ void change_left_desktop(const Arg *arg)
 	Desktop *d = &views[cv_id].ld[views[cv_id].curr_left_id];
 	Desktop *n = &views[cv_id].ld[arg->i];
 
-	if (arg->i == views[cv_id].curr_left_id)
+	if (arg->i == views[cv_id].curr_left_id || arg->i > DESKTOPS)
 		return;
 	if ((c = d->head)) {
 	/* DBG */	fprintf(stderr, "change_left_desktop(): unmapping\n");
@@ -508,7 +512,7 @@ void change_right_desktop(const Arg *arg)
 	Desktop *d = &views[cv_id].rd[views[cv_id].curr_right_id];
 	Desktop *n = &views[cv_id].rd[arg->i];
 
-	if (arg->i == views[cv_id].curr_right_id)
+	if (arg->i == views[cv_id].curr_right_id || arg->i > DESKTOPS)
 		return;
 	if ((c = d->head)) {
 	/* DBG */	fprintf(stderr, "change_right_desktop(): unmapping\n");
@@ -717,7 +721,8 @@ void draw_title(void)
 	XColor color;
 	XAllocNamedColor(dpy, cmap, bar_fg_color, &color, &color);
 	XSetForeground(dpy, gc, color.pixel);
-	XDrawString(dpy, buf, gc, (sw / 2) - (font_struct->per_char->width * d->curr->title_len / 2), font_height, d->curr->title, d->curr->title_len);
+	XDrawString(dpy, buf, gc, (sw / 2) - (font_struct->per_char->width * d->curr->title_len / 2), \
+		    font_height, d->curr->title, d->curr->title_len);
 	XCopyArea(dpy, buf, bar, gc, 0, 0, sw, bar_height, 0, 0);
 	XFlush(dpy);
 	/* DBG */	fprintf(stderr, "draw_title(): OUT\n");
@@ -734,11 +739,10 @@ void draw_tags(void)
 	XColor color_left;
 	XColor color_right;
 
-	for (i = 0; i < DESKTOPS; i++) {
+	for (i = 1; i <= DESKTOPS; i++) {
 		sprintf(c, "%d", i);
-
 		if (views[cv_id].ld[i].head || views[cv_id].curr_left_id == i) {
-			if (views[cv_id].curr_left_id == i) {
+			if (views[cv_id].curr_left_id == i && views[cv_id].curr_desk == LEFT) {
 				XAllocNamedColor(dpy, cmap, bar_fg_focus_color, &color_left, &color_left);
 			} else {
 				XAllocNamedColor(dpy, cmap, bar_fg_color, &color_left, &color_left);
@@ -750,7 +754,7 @@ void draw_tags(void)
 			x_left += 20;
 		}
 		if (views[cv_id].rd[i].head || views[cv_id].curr_right_id == i) {
-			if (views[cv_id].curr_right_id == i) {
+			if (views[cv_id].curr_right_id == i && views[cv_id].curr_desk == RIGHT) {
 				XAllocNamedColor(dpy, cmap, bar_fg_focus_color, &color_right, &color_right);
 			} else {
 				XAllocNamedColor(dpy, cmap, bar_fg_color, &color_right, &color_right);
diff --git a/config.h b/config.h
index 8e67c53..b1a7e66 100644
--- a/config.h
+++ b/config.h
@@ -15,7 +15,7 @@ static const char font[] = "-misc-fixed-medium-r-normal--13-120-75-75-c-70-*-*";
 #define CURSOR		XC_left_ptr	/* default cursor */
 
 /** Settings **/
-#define DESKTOPS		10		/* number of desktops */	
+#define DESKTOPS		5		/* number of desktops */	
 #define DEFAULT_LEFT_DESKTOP	1		/* defualt left desktop */
 #define DEFAULT_RIGHT_DESKTOP	1		/* defualt right desktop */
 #define VIEWS			10		/* number of views */
@@ -75,7 +75,6 @@ static Key keys[] = {
 	{  MOD1,             XK_p,          spawn,                  { .com = spawn_dmenu }},
 	{  MOD4,             XK_p,          spawn,                  { .com = spawn_dmenu }},
 	
-	{  MOD4,             XK_0,          change_left_desktop,    { .i = 0 }},
 	{  MOD4,             XK_1,          change_left_desktop,    { .i = 1 }},
 	{  MOD4,             XK_2,          change_left_desktop,    { .i = 2 }},
 	{  MOD4,             XK_3,          change_left_desktop,    { .i = 3 }},
@@ -86,7 +85,6 @@ static Key keys[] = {
 	{  MOD4,             XK_8,          change_left_desktop,    { .i = 8 }},
 	{  MOD4,             XK_9,          change_left_desktop,    { .i = 9 }},
 
-	{  MOD1,             XK_0,          change_right_desktop,   { .i = 0 }},
 	{  MOD1,             XK_1,          change_right_desktop,   { .i = 1 }},
 	{  MOD1,             XK_2,          change_right_desktop,   { .i = 2 }},
 	{  MOD1,             XK_3,          change_right_desktop,   { .i = 3 }},
@@ -97,7 +95,6 @@ static Key keys[] = {
 	{  MOD1,             XK_8,          change_right_desktop,   { .i = 8 }},
 	{  MOD1,             XK_9,          change_right_desktop,   { .i = 9 }},
 	
-	CLIENT_TO_DESKTOP(   XK_0,                                    0)
 	CLIENT_TO_DESKTOP(   XK_1,                                    1)
 	CLIENT_TO_DESKTOP(   XK_2,                                    2)
 	CLIENT_TO_DESKTOP(   XK_3,                                    3)
