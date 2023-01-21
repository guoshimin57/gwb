/* *************************************************************************
 *     gwb.h：與gwb.c相應的頭文件。
 *     版權 (C) 2023 gsm <406643764@qq.com>
 *     本程序為自由軟件：你可以依據自由軟件基金會所發布的第三版或更高版本的
 * GNU通用公共許可證重新發布、修改本程序。
 *     雖然基于使用目的而發布本程序，但不負任何擔保責任，亦不包含適銷性或特
 * 定目標之適用性的暗示性擔保。詳見GNU通用公共許可證。
 *     你應該已經收到一份附隨此程序的GNU通用公共許可證副本。否則，請參閱
 * <http://www.gnu.org/licenses/>。
 * ************************************************************************/

#ifndef GWB_H
#define GWB_H

GtkBuilder *load_ui(void);
void setup_main_win(void);
void setup_favor_box(void);
void add_favor_button(GtkBox *box, const char *uri, const char *name);
void load_favor_uri(GtkButton *button, char *uri);
void favor(GtkEntry *entry, GtkEntryIconPosition pos, GdkEvent *event, GtkNotebook *notebook);
void add_favor(const char *uri, const char *filename, const char *file_contents, size_t n, GtkNotebook *notebook);
void del_favor(const char *uri, const char *filename, const char *file_contents, size_t n, const char *file_uri_start);
gchar *get_favor_uri_filename(void);
bool is_favor_uri(const char *uri);
void notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, guint page_n, GtkWindow *win);
void update_search_entry(guint page_n);
void new_tab_page(GtkButton *button, gpointer data);
void search(GtkEntry* entry, GtkNotebook *notebook);
void go_back(GtkButton *button, GtkNotebook *notebook);
void go_forward(GtkButton *button, GtkNotebook *notebook);
void reload(GtkButton *button, GtkNotebook *notebook);
void go_home(GtkButton *button, GtkNotebook *notebook);
GtkWidget* get_current_page(GtkNotebook *notebook);
WebKitWebView *request_new_client(WebKitWebView *view, WebKitNavigationAction *act, gpointer data);
WebKitWebView *create_client(const char *uri, WebKitWebView *related_view, bool show);
void setup_tab_label(GtkWidget **label, GtkWidget **close_button, GtkWidget **tab_label);
GtkWidget *create_web_view(WebKitWebView *related_view);
void insert_web_view(GtkNotebook *notebook, GtkWidget *view, GtkWidget *tab_label);
void remove_tab_page(GtkButton *button, GtkWidget *page);
void update_title(WebKitWebView *web_view, GParamSpec *ps, GtkWidget *label);
void fix_label_width(GtkLabel *label, const char *text);
void load_uri(WebKitWebView *web_view, const char *uri);
void show_web_view(WebKitWebView *view, GtkNotebook *notebook);

#endif
