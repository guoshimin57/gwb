#include <stdio.h>
#include <stdbool.h>
#include <webkit2/webkit2.h>
#include <gtk/gtk.h>
#include "gwb.h"
#include "config.h"

GtkBuilder *builder=NULL;

GtkBuilder *load_ui(void)
{
    GtkBuilder *builder=gtk_builder_new();
    return (gtk_builder_add_from_file(builder, "../res/gwb.ui", NULL)
        || gtk_builder_add_from_file(builder, STD_UI_FILE, NULL))
        ? builder : NULL;
}

void setup_main_win(void)
{
    GObject *win, *notebook, *add_button, *search_entry, *back_button,
            *forward_button, *reload_button, *home_button;
    win=gtk_builder_get_object(builder, "main_win");
    notebook=gtk_builder_get_object(builder, "notebook");
    add_button=gtk_builder_get_object(builder, "add_tab_page_button");
    search_entry=gtk_builder_get_object(builder, "search_entry");
    back_button=gtk_builder_get_object(builder, "go_back_button");
    forward_button=gtk_builder_get_object(builder, "go_forward_button");
    reload_button=gtk_builder_get_object(builder, "reload_button");
    home_button=gtk_builder_get_object(builder, "go_home_button");
    gtk_window_set_default_size(GTK_WINDOW(win), MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);
    gtk_entry_set_width_chars(GTK_ENTRY(search_entry), SEARCH_ENTRY_WIDTH_CHARS);
    g_signal_connect_swapped(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(notebook, "switch-page", G_CALLBACK(notebook_switch_page), win);
    g_signal_connect(add_button, "clicked", G_CALLBACK(new_tab_page), NULL);
    g_signal_connect(search_entry, "activate", G_CALLBACK(search), notebook);
    g_signal_connect(back_button, "clicked", G_CALLBACK(go_back), notebook);
    g_signal_connect(forward_button, "clicked", G_CALLBACK(go_forward), notebook);
    g_signal_connect(reload_button, "clicked", G_CALLBACK(reload), notebook);
    g_signal_connect(home_button, "clicked", G_CALLBACK(go_home), notebook);
    gtk_widget_show_all(GTK_WIDGET(win));
}

void notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, guint page_n, GtkWindow *win)
{
    GtkWidget *view=gtk_notebook_get_nth_page(notebook, page_n);
    const gchar *s=webkit_web_view_get_title(WEBKIT_WEB_VIEW(view));
    gtk_window_set_title(GTK_WINDOW(win), s ? s : "未知標題");
    update_search_entry(page_n);
}

void update_search_entry(guint page_n)
{
    GObject *notebook=gtk_builder_get_object(builder, "notebook");
    GObject *search_entry=gtk_builder_get_object(builder, "search_entry");
    GtkWidget *view=gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page_n);
    const gchar *text=webkit_web_view_get_uri(WEBKIT_WEB_VIEW(view));
    gtk_entry_set_text(GTK_ENTRY(search_entry), text ? text : "");
}

void new_tab_page(GtkButton *button, gpointer data)
{
    create_client(DEFAULT_URI, NULL, true);
}

void search(GtkEntry* entry, GtkNotebook* notebook)
{
    const char *s=gtk_entry_buffer_get_text(gtk_entry_get_buffer(entry));
    load_uri(WEBKIT_WEB_VIEW(WEBKIT_WEB_VIEW(get_current_page(notebook))), s);
}

void go_back(GtkButton *button, GtkNotebook* notebook)
{
    webkit_web_view_go_back(WEBKIT_WEB_VIEW(get_current_page(notebook)));
}

void go_forward(GtkButton *button, GtkNotebook* notebook)
{
    webkit_web_view_go_forward(WEBKIT_WEB_VIEW(get_current_page(notebook)));
}

void reload(GtkButton *button, GtkNotebook* notebook)
{
    webkit_web_view_reload(WEBKIT_WEB_VIEW(get_current_page(notebook)));
}

void go_home(GtkButton *button, GtkNotebook* notebook)
{
    load_uri(WEBKIT_WEB_VIEW(get_current_page(notebook)), HOME_PAGE);
}

GtkWidget* get_current_page(GtkNotebook* notebook)
{
    gint n=gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    return gtk_notebook_get_nth_page(notebook, n);
}

WebKitWebView *request_new_client(WebKitWebView *view, WebKitNavigationAction *act, gpointer data)
{
    return create_client(NULL, view, false);
}

WebKitWebView *create_client(const char *uri, WebKitWebView *related_view, bool show)
{
    GtkWidget *label, *close_button, *view, *tab_label;
    GObject *win, *notebook;

    win=gtk_builder_get_object(builder, "main_win");
    notebook=gtk_builder_get_object(builder, "notebook");
    setup_tab_label(&label, &close_button, &tab_label);

    if(related_view)
        view=webkit_web_view_new_with_related_view(related_view);
    else
        view=webkit_web_view_new();
    g_signal_connect(view, "create", G_CALLBACK(request_new_client), NULL);
    g_signal_connect(view, "notify::title", G_CALLBACK(update_title), label);
    g_signal_connect(close_button, "clicked", G_CALLBACK(remove_tab_page), view);
    if(uri)
        load_uri(WEBKIT_WEB_VIEW(view), uri);

    gint n=gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))+1;
    gtk_notebook_insert_page(GTK_NOTEBOOK(notebook), view, tab_label, n);
    gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(notebook), view, TRUE);

    gtk_widget_show_all(GTK_WIDGET(win));
    if(show)
        show_web_view(WEBKIT_WEB_VIEW(view), GTK_NOTEBOOK(notebook));
    else
        g_signal_connect(view, "ready-to-show", G_CALLBACK(show_web_view), notebook);

    return WEBKIT_WEB_VIEW(view);
}

void setup_tab_label(GtkWidget **label, GtkWidget **close_button, GtkWidget **tab_label)
{
    *tab_label=gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    *label=gtk_label_new("新標籤頁");
    gtk_label_set_width_chars(GTK_LABEL(*label), TAB_LABEL_WIDTH_CHARS);
    gtk_label_set_ellipsize(GTK_LABEL(*label), PANGO_ELLIPSIZE_MIDDLE);
    *close_button=gtk_button_new_from_icon_name("window-close", GTK_ICON_SIZE_BUTTON);
    gtk_style_context_add_class(gtk_widget_get_style_context(*close_button), "circular");
    gtk_button_set_relief(GTK_BUTTON(*close_button), GTK_RELIEF_NONE);
    gtk_box_pack_start(GTK_BOX(*tab_label), *label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(*tab_label), *close_button, TRUE, TRUE, 0);
    gtk_widget_show_all(*tab_label);
}

void remove_tab_page(GtkButton *button, GtkWidget *page)
{
    GtkNotebook *notebook=GTK_NOTEBOOK(gtk_builder_get_object(builder, "notebook"));
    int n=gtk_notebook_page_num(notebook, page);
    gtk_notebook_remove_page(notebook, n);
}

void update_title(WebKitWebView *view, GParamSpec *ps, GtkWidget *label)
{
    GObject *win=gtk_builder_get_object(builder, "main_win");
    GObject *notebook=gtk_builder_get_object(builder, "notebook");
    gtk_window_set_title(GTK_WINDOW(win), webkit_web_view_get_title(view));
    gtk_label_set_label(GTK_LABEL(label), webkit_web_view_get_title(view));
    update_search_entry(gtk_notebook_page_num(GTK_NOTEBOOK(notebook), GTK_WIDGET(view)));
}

void load_uri(WebKitWebView *view, const char *uri)
{
    const char *prefix[]={"http://", "https://", "file://", "about:", NULL};
    for(const char **p=prefix; *p; p++)
        if(strstr(uri, *p))
            { webkit_web_view_load_uri(WEBKIT_WEB_VIEW(view), uri); return;}

    /* 應先檢測uri是不是文件，但這依賴於系統，暫時不搞，粗暴地用http嘗試打開 */
    char full_uri[strlen(prefix[0])+strlen(uri)+1];
    sprintf(full_uri, "%s%s", prefix[0], uri);
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(view), full_uri);
}

void show_web_view(WebKitWebView *view, GtkNotebook *notebook)
{
    if(FOCUS_NEW_TAB_PAGE)
    {
        gint n=gtk_notebook_page_num(notebook, GTK_WIDGET(view));
        if(n != -1)
        {
            gtk_notebook_set_current_page(notebook, n);
            gtk_widget_grab_focus(GTK_WIDGET(view));
        }
    }
}

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);
    builder=load_ui();
    setup_main_win();
    for(size_t i=1; i<argc; i++)
        create_client(argv[i], NULL, true);
    if(argc == 1)
        create_client(DEFAULT_URI, NULL, true);
    gtk_main();
    g_object_unref(builder);
    return 0;
}
