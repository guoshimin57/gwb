#include <stdio.h>
#include <stdbool.h>
#include <webkit2/webkit2.h>
#include <gtk/gtk.h>
#include "gwb.h"
#include "config.h"

GtkBuilder *builder=NULL;

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
    setup_favor_box();
    g_signal_connect_swapped(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(notebook, "switch-page", G_CALLBACK(notebook_switch_page), win);
    g_signal_connect(add_button, "clicked", G_CALLBACK(new_tab_page), NULL);
    g_signal_connect(search_entry, "activate", G_CALLBACK(search), notebook);
    g_signal_connect(search_entry, "icon-press", G_CALLBACK(favor), notebook);
    g_signal_connect(back_button, "clicked", G_CALLBACK(go_back), notebook);
    g_signal_connect(forward_button, "clicked", G_CALLBACK(go_forward), notebook);
    g_signal_connect(reload_button, "clicked", G_CALLBACK(reload), notebook);
    g_signal_connect(home_button, "clicked", G_CALLBACK(go_home), notebook);
    gtk_widget_show_all(GTK_WIDGET(win));
}

void setup_favor_box(void)
{
    FILE *fp=fopen(get_favor_uri_filename(), "r");
    if(fp)
    {
        GObject *box=gtk_builder_get_object(builder, "favor_box");
        for(char *name, *uri; (uri=g_malloc(BUFSIZ)) && fgets(uri, BUFSIZ, fp);)
            if((uri=strtok(uri, " \t\n")) && (name=strtok(NULL, " \t\n")))
                add_favor_button(GTK_BOX(box), uri, name);
        fclose(fp);
    }
}

void add_favor_button(GtkBox *box, const char *uri, const char *name)
{
    GtkWidget *button=gtk_button_new_with_label(name);
    GtkWidget *label=gtk_bin_get_child(GTK_BIN(button));
    gtk_label_set_max_width_chars(GTK_LABEL(label), FAVOR_LABEL_WIDTH_CHARS);
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text(button, uri);
    g_object_set_data(G_OBJECT(box), uri, button);
    g_signal_connect(button, "clicked", G_CALLBACK(load_favor_uri), g_strdup(uri));
    gtk_box_pack_start(box, button, TRUE, TRUE, 0);
    gtk_widget_show_all(GTK_WIDGET(box));
}

void del_favor_button(GtkBox *box, const char *uri)
{
    GObject *button=g_object_get_data(G_OBJECT(box), uri);
    gtk_container_remove(GTK_CONTAINER(box), GTK_WIDGET(button));
}

void load_favor_uri(GtkButton *button, char *uri)
{
    GObject *notebook=gtk_builder_get_object(builder, "notebook");
    GtkWidget *view=get_current_page(GTK_NOTEBOOK(notebook));
    load_uri(WEBKIT_WEB_VIEW(view), uri);
}

void favor(GtkEntry *entry, GtkEntryIconPosition pos, GdkEvent *event, GtkNotebook *notebook)
{
    if(pos == GTK_ENTRY_ICON_SECONDARY)
    {
        gsize n;
        gchar *p=NULL, *c=NULL, *filename=get_favor_uri_filename();
        const gchar *uri=gtk_entry_get_text(entry);

        if(!g_file_test(g_get_user_config_dir(), G_FILE_TEST_IS_DIR))
            g_mkdir_with_parents(filename, 0755);
        if(g_file_get_contents(filename, &c, &n, NULL) && (p=strstr(c, uri)))
            del_favor(uri, filename, c, n, p);
        else
            add_favor(uri, filename, c, n, notebook);
        g_free(c), g_free(filename);
        gtk_entry_set_icon_from_icon_name(entry, GTK_ENTRY_ICON_SECONDARY,
            p ? "non-starred" : "starred");
    }
}

// 書籤的保存格式：uri 書籤名稱\n
void add_favor(const gchar *uri, const gchar *filename, const gchar *file_contents, size_t n, GtkNotebook *notebook)
{
    GObject *box=gtk_builder_get_object(builder, "favor_box");
    GtkWidget *view=get_current_page(notebook);
    const gchar *name=webkit_web_view_get_title(WEBKIT_WEB_VIEW(view));
    gchar s[n+strlen(uri)+strlen(name)+3]; // 考慮空格、\n、\0

    sprintf(s, "%s%s %s\n", file_contents ? file_contents : "", uri, name);
    g_file_set_contents(filename, s, -1, NULL);
    add_favor_button(GTK_BOX(box), uri, name);
}

void del_favor(const gchar *uri, const gchar *filename, const gchar *file_contents, size_t n, const gchar *file_uri_start)
{
    gchar *end=NULL, s[n];
    GObject *box=gtk_builder_get_object(builder, "favor_box");

    strncpy(s, file_contents, n=file_uri_start-file_contents), s[n]='\0';
    if((end=strchr(file_uri_start, '\n')))
        strcat(s, ++end);
    g_file_set_contents(filename, s, -1, NULL);
    del_favor_button(GTK_BOX(box), uri);
}

gchar *get_favor_uri_filename(void)
{
    return g_build_filename(g_get_user_config_dir(), "gwb", "favor_uri", NULL);
}

bool is_favor_uri(const char *uri)
{
    gchar *data=NULL, *filename=get_favor_uri_filename();
    bool result = (g_file_test(filename, G_FILE_TEST_IS_REGULAR)
        && g_file_get_contents(filename, &data, NULL, NULL)
        && strstr(data, uri));
    g_free(data), g_free(filename);
    return result;
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
    const gchar *s=webkit_web_view_get_uri(WEBKIT_WEB_VIEW(view));

    if(!s)
        return;
    gtk_entry_set_text(GTK_ENTRY(search_entry), s);
    gtk_entry_set_icon_from_icon_name(GTK_ENTRY(search_entry),
        GTK_ENTRY_ICON_SECONDARY, is_favor_uri(s) ? "starred" : "non-starred");
}

void new_tab_page(GtkButton *button, gpointer data)
{
    create_client(DEFAULT_URI, NULL, true);
}

void search(GtkEntry *entry, GtkNotebook *notebook)
{
    const char *s=gtk_entry_buffer_get_text(gtk_entry_get_buffer(entry));
    load_uri(WEBKIT_WEB_VIEW(get_current_page(notebook)), s);
}

void go_back(GtkButton *button, GtkNotebook *notebook)
{
    webkit_web_view_go_back(WEBKIT_WEB_VIEW(get_current_page(notebook)));
}

void go_forward(GtkButton *button, GtkNotebook *notebook)
{
    webkit_web_view_go_forward(WEBKIT_WEB_VIEW(get_current_page(notebook)));
}

void reload(GtkButton *button, GtkNotebook *notebook)
{
    webkit_web_view_reload(WEBKIT_WEB_VIEW(get_current_page(notebook)));
}

void go_home(GtkButton *button, GtkNotebook *notebook)
{
    load_uri(WEBKIT_WEB_VIEW(get_current_page(notebook)), HOME_PAGE);
}

GtkWidget* get_current_page(GtkNotebook *notebook)
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
    GObject *notebook=gtk_builder_get_object(builder, "notebook");

    setup_tab_label(&label, &close_button, &tab_label);
    view=create_web_view(related_view);
    g_signal_connect(view, "create", G_CALLBACK(request_new_client), NULL);
    g_signal_connect(view, "notify::title", G_CALLBACK(update_title), label);
    g_signal_connect(close_button, "clicked", G_CALLBACK(remove_tab_page), view);
    if(uri)
        load_uri(WEBKIT_WEB_VIEW(view), uri);
    insert_web_view(GTK_NOTEBOOK(notebook), view, tab_label);
    if(show)
        show_web_view(WEBKIT_WEB_VIEW(view), GTK_NOTEBOOK(notebook));
    else
        g_signal_connect(view, "ready-to-show", G_CALLBACK(show_web_view), notebook);

    return WEBKIT_WEB_VIEW(view);
}

GtkWidget *create_web_view(WebKitWebView *related_view)
{
    return related_view ? webkit_web_view_new_with_related_view(related_view) :
        webkit_web_view_new();
}

void insert_web_view(GtkNotebook *notebook, GtkWidget *view, GtkWidget *tab_label)
{
    GObject *win=gtk_builder_get_object(builder, "main_win");
    gint n=gtk_notebook_get_current_page(notebook)+1;
    gtk_notebook_insert_page(notebook, view, tab_label, n);
    gtk_notebook_set_tab_reorderable(notebook, view, TRUE);
    gtk_widget_show_all(GTK_WIDGET(win));
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
    if(gtk_notebook_get_n_pages(notebook) == 1)
        gtk_main_quit();
    gtk_notebook_remove_page(notebook, gtk_notebook_page_num(notebook, page));
}

void update_title(WebKitWebView *view, GParamSpec *ps, GtkWidget *label)
{
    GObject *win=gtk_builder_get_object(builder, "main_win");
    GObject *notebook=gtk_builder_get_object(builder, "notebook");
    const gchar *text=webkit_web_view_get_title(view);
    gtk_window_set_title(GTK_WINDOW(win), text);
    gtk_label_set_label(GTK_LABEL(label), text);
    fix_label_width(GTK_LABEL(label), text);
    update_search_entry(gtk_notebook_page_num(GTK_NOTEBOOK(notebook), GTK_WIDGET(view)));
}

void fix_label_width(GtkLabel *label, const char *text)
{
    int w, w0;
    PangoLayout *layout=gtk_widget_create_pango_layout(GTK_WIDGET(label), text);
    pango_layout_get_pixel_size(layout, &w, NULL);
    pango_layout_set_text(layout, "a", -1);
    pango_layout_get_pixel_size(layout, &w0, NULL);
    int n=(w+w0-1)/w0;
    if(n > TAB_LABEL_WIDTH_CHARS)
        n=TAB_LABEL_WIDTH_CHARS;
    gtk_label_set_width_chars(label, n);
    gtk_label_set_ellipsize(label, PANGO_ELLIPSIZE_MIDDLE);
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
    GtkWidget *v=GTK_WIDGET(view);
    gint n=gtk_notebook_page_num(notebook, v);
    if(FOCUS_NEW_TAB_PAGE && n!=-1)
        gtk_notebook_set_current_page(notebook, n), gtk_widget_grab_focus(v);
}
