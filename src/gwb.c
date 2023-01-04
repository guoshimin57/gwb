#include <stdio.h>
#include <stdbool.h>
#include <webkit2/webkit2.h>
#include <gtk/gtk.h>
#include "gwb.h"
#include "config.h"

WebKitWebView *request_new_client(WebKitWebView *view, WebKitNavigationAction *act, gpointer data)
{
    return create_client(NULL, view, false);
}

WebKitWebView *create_client(char *uri, WebKitWebView *related_view, bool show)
{
    GtkWidget *view, *win;
    if(related_view)
        view=webkit_web_view_new_with_related_view(related_view);
    else
        view=webkit_web_view_new();
    g_signal_connect(view, "create", G_CALLBACK(request_new_client), NULL);
    if(uri)
        load_uri(WEBKIT_WEB_VIEW(view), uri);

    win=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(win), WIN_WIDTH, WIN_HEIGHT);
    gtk_container_add(GTK_CONTAINER(win), view);
    g_signal_connect(view, "notify::title", G_CALLBACK(update_title), win);
    g_signal_connect_swapped(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    if(show)
        gtk_widget_show_all(win);
    else
        g_signal_connect_swapped(view, "ready-to-show", G_CALLBACK(gtk_widget_show_all), win);
    return WEBKIT_WEB_VIEW(view);
}

void update_title(WebKitWebView *view, GParamSpec *ps, GtkWidget *win)
{
    gtk_window_set_title(GTK_WINDOW(win), webkit_web_view_get_title(view));
}

void load_uri(WebKitWebView *view, const char *uri)
{
    const char *prefix[]={"http://", "https://", "file://", "about:", NULL};
    for(const char **p=prefix; *p; p++)
        if(strstr(uri, *prefix))
            { webkit_web_view_load_uri(WEBKIT_WEB_VIEW(view), uri); return;}

    /* 應先檢測uri是不是文件，但這依賴於系統，暫時不搞，粗暴地用http嘗試打開 */
    char full_uri[strlen(prefix[0])+strlen(uri)+1];
    sprintf(full_uri, "%s%s", prefix[0], uri);
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(view), full_uri);
}

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);
    for(size_t i=1; i<argc; i++)
        create_client(argv[i], NULL, true);
    if(argc == 1)
        create_client(HOME_PAGE, NULL, true);
    gtk_main();
    return 0;
}
