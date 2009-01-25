/*
 * dhcpcd-gtk
 * Copyright 2009 Roy Marples <roy@marples.name>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "dhcpcd-gtk.h"
#include "menu.h"

static const char *copyright = "Copyright (c) 2009 Roy Marples";

static const char *authors[] = {
	"Roy Marples <roy@marples.name>",
	NULL
};
static const char *license =
	"Licensed under the 2 clause BSD license.\n"
	"\n"
	"Redistribution and use in source and binary forms, with or without\n"
	"modification, are permitted provided that the following conditions\n"
	"are met:\n"
	"1. Redistributions of source code must retain the above copyright\n"
	"   notice, this list of conditions and the following disclaimer.\n"
	"2. Redistributions in binary form must reproduce the above copyright\n"
	"   notice, this list of conditions and the following disclaimer in the\n"
	"   documentation and/or other materials provided with the distribution.\n"
	"\n"
	"THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND\n"
	"ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n"
	"IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n"
	"ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE\n"
	"FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n"
	"DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS\n"
	"OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\n"
	"HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT\n"
	"LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY\n"
	"OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\n"
	"SUCH DAMAGE.";

static void
on_quit(_unused GtkMenuItem *item, _unused gpointer data)
{
	gtk_main_quit();
}

static void
on_help(_unused GtkMenuItem *item, _unused gpointer data)
{
}

static void
url_show(GtkAboutDialog *dialog, const char *url)
{
	GdkScreen *screen;

	screen = gtk_widget_get_screen(GTK_WIDGET(dialog));
	gtk_show_uri(screen, url, GDK_CURRENT_TIME, NULL);
}

static void
email_hook(GtkAboutDialog *dialog, const char *url, _unused gpointer p)
{
	char *address;

	address = g_strdup_printf("mailto:%s", url);
	url_show(dialog, address);
	g_free(address);
}

static void
url_hook(GtkAboutDialog *dialog, const char *url, _unused gpointer p)
{
	url_show(dialog, url);
}

static void
on_about(_unused GtkMenuItem *item, _unused gpointer data)
{
	gtk_window_set_default_icon_name(GTK_STOCK_NETWORK);
	gtk_about_dialog_set_email_hook(email_hook, NULL, NULL);
	gtk_about_dialog_set_url_hook(url_hook, NULL, NULL);
	gtk_show_about_dialog(NULL,
			      "version", VERSION,
			      "copyright", copyright,
			      "license", license,
			      "website-label", "dhcpcd Website",
			      "website", "http://roy.marples.name/projects/dhcpcd",
			      "authors", authors,
			      "wrap-license", TRUE,
			      "logo-icon-name", GTK_STOCK_NETWORK,
			      NULL);
}

static void
add_scan_results(GtkMenu *menu, const struct if_msg *ifm)
{
	GList *gl;
	const struct if_ap *ifa;
	GtkWidget *item, *image;

	for (gl = ifm->scan_results; gl; gl = gl->next) {
		ifa = (const struct if_ap *)gl->data;
		item = gtk_image_menu_item_new_with_label(ifa->ssid);
		image = gtk_image_new_from_icon_name("network-wireless",
						     GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	}
}

static void
on_activate(GtkStatusIcon *icon, _unused guint button, _unused guint32 atime, _unused gpointer data)
{
	GtkMenu *menu;
	const struct if_msg *ifm;
	GList *gl;
	size_t n;

	notify_close();

	n = 0;
	for (gl = interfaces; gl; gl = gl->next) {
		ifm = (const struct if_msg *)gl->data;
		if (ifm->wireless)
			if (++n > 1)
				break;
	}
	if (n == 0)
		return;

	menu = (GtkMenu *)gtk_menu_new();

	for (gl = interfaces; gl; gl = gl->next) {
		ifm = (const struct if_msg *)gl->data;
		if (!ifm->wireless)
			continue;
		if (n == 1) {
			add_scan_results(menu, ifm);
			break;
		}
#if 0
		item = gtk_image_menu_item_new_with_label(ifm->name);
		image = gtk_image_new_from_icon_name("network-wireless",
						     GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);

		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
#endif
	}
	gtk_widget_show_all(GTK_WIDGET(menu));
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL,
		       gtk_status_icon_position_menu, icon,
		       1, gtk_get_current_event_time());
}

static void
on_popup(GtkStatusIcon *icon, guint button, guint32 atime, gpointer data)
{
	GtkMenu *menu;
	GtkWidget *item, *image;

	notify_close();

	menu = (GtkMenu *)gtk_menu_new();

	item = gtk_image_menu_item_new_with_mnemonic("_Quit");
	image = gtk_image_new_from_icon_name(GTK_STOCK_QUIT,
					     GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(on_quit), icon);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	item = gtk_image_menu_item_new_with_mnemonic("_Help");
	image = gtk_image_new_from_icon_name(GTK_STOCK_HELP,
					     GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(on_help), icon);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	item = gtk_image_menu_item_new_with_mnemonic("_About");
	image = gtk_image_new_from_icon_name(GTK_STOCK_ABOUT,
					     GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(on_about), icon);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	gtk_widget_show_all(GTK_WIDGET(menu));
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL,
		       gtk_status_icon_position_menu, data, button, atime);
	if (button == 0)
		gtk_menu_shell_select_first(GTK_MENU_SHELL(menu), FALSE);
}

void
menu_init(GtkStatusIcon *icon)
{
	g_signal_connect_object(G_OBJECT(icon), "activate",
				G_CALLBACK(on_activate), icon, 0);
	g_signal_connect_object(G_OBJECT(icon), "popup_menu",
				G_CALLBACK(on_popup), icon, 0);
}
