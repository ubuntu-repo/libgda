/*
 * Copyright (C) 2008 - 2014 Vivien Malerba <malerba@gnome-db.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "html-doc.h"
#include <glib/gi18n-lib.h>

HtmlDoc *
html_doc_new (const gchar *title)
{
	HtmlDoc *hdoc;
	xmlNodePtr topnode, head, node, div1, div2, div3;

	hdoc = g_new0 (HtmlDoc, 1);
	hdoc->doc = xmlNewDoc (BAD_CAST "1.0");
	topnode = xmlNewDocNode (hdoc->doc, NULL, BAD_CAST "html", NULL);
	xmlDocSetRootElement (hdoc->doc, topnode);

	/* head */
	head = xmlNewChild (topnode, NULL, BAD_CAST "head", NULL);
	hdoc->head = head;

	node = xmlNewChild (head, NULL, BAD_CAST "meta", BAD_CAST "");
	xmlSetProp(node, BAD_CAST "http-equiv", BAD_CAST "Content-Type");
	xmlSetProp(node, BAD_CAST "content", BAD_CAST "text/html; charset=UTF-8");

	/*
	node = xmlNewChild (head, NULL, BAD_CAST "meta", NULL);
	xmlSetProp(node, BAD_CAST "http-equiv", BAD_CAST "refresh");
	xmlSetProp(node, BAD_CAST "content", BAD_CAST "30");*/ /* refresh the page every 30 seconds */

	node = xmlNewChild (head, NULL, BAD_CAST "title", BAD_CAST title);

	node = xmlNewChild (head, NULL, BAD_CAST "link", BAD_CAST "");
	xmlSetProp(node, BAD_CAST "href", BAD_CAST "/gda.css");
	xmlSetProp(node, BAD_CAST "rel", BAD_CAST "stylesheet");
	xmlSetProp(node, BAD_CAST "type", BAD_CAST "text/css");
	xmlSetProp(node, BAD_CAST "media", BAD_CAST "screen");

	node = xmlNewChild (head, NULL, BAD_CAST "link", BAD_CAST "");
	xmlSetProp(node, BAD_CAST "href", BAD_CAST "/gda-print.css");
	xmlSetProp(node, BAD_CAST "rel", BAD_CAST "stylesheet");
	xmlSetProp(node, BAD_CAST "type", BAD_CAST "text/css");
	xmlSetProp(node, BAD_CAST "media", BAD_CAST "print");

	/* body */
	node = xmlNewChild (topnode, NULL, BAD_CAST "body", NULL);
	hdoc->body = node;

	/* top */
	div1 = xmlNewChild (hdoc->body, NULL, BAD_CAST "div", NULL);
	xmlSetProp (div1, BAD_CAST "id", BAD_CAST "header");
	xmlNewChild (div1, NULL, BAD_CAST "h1", BAD_CAST title);

	/* main part */
	div1 = xmlNewChild (hdoc->body, NULL, BAD_CAST "div", NULL);
	xmlSetProp (div1, BAD_CAST "class", BAD_CAST "colmask leftmenu");

	div2 = xmlNewChild (div1, NULL, BAD_CAST "div", NULL);
	xmlSetProp (div2, BAD_CAST "class", BAD_CAST "colleft");

	div3 = xmlNewChild (div2, NULL, BAD_CAST "div", BAD_CAST "");
	xmlSetProp (div3, BAD_CAST "class", BAD_CAST "col1");
	hdoc->content = div3;

	div3 = xmlNewChild (div2, NULL, BAD_CAST "div", BAD_CAST "");
	xmlSetProp (div3, BAD_CAST "class", BAD_CAST "col2");
	hdoc->sidebar = div3;

	/* footer */
	div1 = xmlNewChild (hdoc->body, NULL, BAD_CAST "div", NULL);
	xmlSetProp (div1, BAD_CAST "id", BAD_CAST "footer");
	xmlNewChild (div1, NULL, BAD_CAST "p", BAD_CAST _("Generated by the GDA SQL console"));
	hdoc->footer = div1;

	return hdoc;
}

void
html_doc_free  (HtmlDoc *hdoc)
{
	xmlFreeDoc (hdoc->doc);
	g_free (hdoc);
}

xmlChar *
html_doc_to_string (HtmlDoc *hdoc, gsize *out_size)
{
	xmlChar *retval = NULL;
	int size;
	xmlNodePtr li, a, node;

	node = xmlNewChild (hdoc->sidebar, NULL, BAD_CAST "ul", BAD_CAST "Misc");
	li = xmlNewChild (node, NULL, BAD_CAST "li", NULL);
	a = xmlNewChild (li, NULL, BAD_CAST "a", BAD_CAST _("Console"));
	xmlSetProp (a, BAD_CAST "href", BAD_CAST "/~console");

	xmlDocDumpFormatMemory (hdoc->doc, &retval, &size, 1);
	if (out_size)
		*out_size = (gsize) size;

	return retval;
}