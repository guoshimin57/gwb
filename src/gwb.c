#include <stdio.h>
#include <stdbool.h>
#include <webkit2/webkit2.h>
#include <gtk/gtk.h>

WebKitWebView *request_new_client(WebKitWebView *web_view, WebKitNavigationAction *act, gpointer data);
WebKitWebView *create_client(char *url, WebKitWebView *related_view, bool show);

WebKitWebView *request_new_client(WebKitWebView *web_view, WebKitNavigationAction *act, gpointer data)
{
    return create_client(NULL, web_view, false);
}

WebKitWebView *create_client(char *url, WebKitWebView *related_view, bool show)
{
    GtkWidget *web_view, *win;
    if(related_view)
        web_view=webkit_web_view_new_with_related_view(related_view);
    else
        web_view=webkit_web_view_new();
    g_signal_connect(web_view, "create", G_CALLBACK(request_new_client), NULL);
    if(url)
        webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), url);

    win=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(win), 1024, 768);
    gtk_container_add(GTK_CONTAINER(win), web_view);
    g_signal_connect_swapped(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    if(show)
        gtk_widget_show_all(win);
    else
        g_signal_connect_swapped(web_view, "ready-to-show", G_CALLBACK(gtk_widget_show_all), win);
    return WEBKIT_WEB_VIEW(web_view);
}

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);
    create_client(argc==2 ? argv[1] : "http://www.bing.com", NULL, true);
    gtk_main();
    return 0;
}
