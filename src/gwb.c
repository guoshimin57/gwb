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
    return (gtk_builder_add_from_file(builder, STD_UI_FILE, NULL))
        || gtk_builder_add_from_file(builder, "../res/gwb.ui", NULL)
        ? builder : NULL;
}

void setup_main_win(void)
{
    GObject *win, *notebook, *search_entry, *add_button;
    win=gtk_builder_get_object(builder, "main_win");
    notebook=gtk_builder_get_object(builder, "notebook");
    add_button=gtk_builder_get_object(builder, "add_tab_page");
    search_entry=gtk_builder_get_object(builder, "search_entry");
    gtk_window_set_default_size(GTK_WINDOW(win), MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);
    gtk_entry_set_width_chars(GTK_ENTRY(search_entry), SEARCH_ENTRY_WIDTH_CHARS);
    g_signal_connect_swapped(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(notebook, "switch-page", G_CALLBACK(notebook_switch_page), win);
    g_signal_connect(add_button, "clicked", G_CALLBACK(new_tab_page), NULL);
    g_signal_connect(search_entry, "activate", G_CALLBACK(search), NULL);
    gtk_widget_show_all(GTK_WIDGET(win));
}

void notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, guint page_n, GtkWindow *win)
{
    GtkWidget *child, *tab_label, *label;
    child=gtk_notebook_get_nth_page(notebook, page_n);
    tab_label=gtk_notebook_get_tab_label(notebook, child);
    label = (GtkWidget *)g_object_get_data(G_OBJECT(tab_label), "tab_label");
    gtk_window_set_title(win, gtk_label_get_text(GTK_LABEL(label)));
}

void new_tab_page(GtkButton *button, gpointer data)
{
    create_client(DEFAULT_URI, NULL, true);
}

void search(GtkEntry* entry, gpointer data)
{
    const char *s=gtk_entry_buffer_get_text(gtk_entry_get_buffer(entry));
    create_client(s, NULL, true);
    gtk_entry_set_text(entry, "");
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

    tab_label=gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    label=gtk_label_new("新標籤頁");
    close_button=gtk_button_new_with_label("×");
    gtk_button_set_relief(GTK_BUTTON(close_button), GTK_RELIEF_NONE);
    gtk_box_pack_start(GTK_BOX(tab_label), label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(tab_label), close_button, TRUE, TRUE, 0);
    g_object_set_data(G_OBJECT(tab_label), "tab_label", label);
    gtk_widget_show_all(tab_label);

    if(related_view)
        view=webkit_web_view_new_with_related_view(related_view);
    else
        view=webkit_web_view_new();
    g_signal_connect(view, "create", G_CALLBACK(request_new_client), (gpointer)builder);
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

void remove_tab_page(GtkButton *button, GtkWidget *page)
{
    GtkNotebook *notebook=GTK_NOTEBOOK(gtk_builder_get_object(builder, "notebook"));
    int n=gtk_notebook_page_num(notebook, page);
    gtk_notebook_remove_page(notebook, n);
}

void update_title(WebKitWebView *view, GParamSpec *ps, GtkWidget *label)
{
    GObject *win=gtk_builder_get_object(builder, "main_win");
    gtk_window_set_title(GTK_WINDOW(win), webkit_web_view_get_title(view));
    gtk_label_set_label(GTK_LABEL(label), webkit_web_view_get_title(view));

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
