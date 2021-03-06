/*
 * Copyright (C) 2008 - 2011 Vivien Malerba <malerba@gnome-db.org>
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
#include <stdio.h>
#include <glib.h>
#include <glib-object.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <gmodule.h>
#include <libgda/libgda.h>
#include <libgda/gda-util.h>
#include <sql-parser/gda-sql-parser.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

GdaConnection *cnc;
static gint do_test (const xmlChar *id, const xmlChar *sql, gboolean valid_expected);

int 
main (G_GNUC_UNUSED int argc, G_GNUC_UNUSED char** argv)
{
	xmlDocPtr doc;
        xmlNodePtr root, node;
	gint failures = 0;
	gint ntests = 0;
	gchar *fname;

	gda_init ();

	/* open connection */
	gchar *cnc_string;
	fname = g_build_filename (ROOT_DIR, "data", NULL);
	cnc_string = g_strdup_printf ("DB_DIR=%s;DB_NAME=sales_test", fname);
	g_free (fname);
	cnc = gda_connection_open_from_string ("SQLite", cnc_string, NULL,
					       GDA_CONNECTION_OPTIONS_READ_ONLY, NULL);
	if (!cnc) {
		g_print ("Failed to open connection, cnc_string = %s\n", cnc_string);
		exit (1);
	}
	if (!gda_connection_update_meta_store (cnc, NULL, NULL)) {
		g_print ("Failed to update meta store, cnc_string = %s\n", cnc_string);
		exit (1);
	}
	g_free (cnc_string);

	/* load file */
	fname = g_build_filename (ROOT_DIR, "tests", "parser", "testvalid.xml", NULL);
	if (! g_file_test (fname, G_FILE_TEST_EXISTS)) {
                g_print ("File '%s' does not exist\n", fname);
                exit (1);
        }

	/* use test data */
	doc = xmlParseFile (fname);
	g_free (fname);
	g_assert (doc);
	root = xmlDocGetRootElement (doc);
	g_assert (!strcmp ((gchar*) root->name, "testdata"));
	for (node = root->children; node; node = node->next) {
		if (strcmp ((gchar*) node->name, "test"))
			continue;
		xmlNodePtr snode;
		xmlChar *sql = NULL;
		xmlChar *id;
		gboolean valid = FALSE;

		id = xmlGetProp (node, BAD_CAST "id");
		for (snode = node->children; snode; snode = snode->next) {
			if (!strcmp ((gchar*) snode->name, "sql")) {
				sql = xmlNodeGetContent (snode);
				xmlChar *prop;
				prop = xmlGetProp (snode, (const xmlChar*) "valid");
				if (prop) {
					if ((*prop == 't') || (*prop == 'T') || (*prop == '1'))
						valid = TRUE;
					xmlFree (prop);
				}
			}
		}
		if (sql) {
			if (!do_test (id, sql, valid))
				failures++;
			ntests++;
		}

		/* mem free */
		if (sql) xmlFree (sql);
		if (id)	xmlFree (id);
	}
	xmlFreeDoc (doc);

	g_print ("TESTS COUNT: %d\n", ntests);
	g_print ("FAILURES: %d\n", failures);
  
	return failures != 0 ? 1 : 0;
}

/*
 * Returns: the number of failures
 */
static gint
do_test (const xmlChar *id, const xmlChar *sql, gboolean valid_expected) 
{
	static GdaSqlParser *parser = NULL;
	GdaStatement *stmt;
	gboolean is_valid;
	GError *error = NULL;

	if (!parser) {
		parser = gda_connection_create_parser (cnc);
		if (!parser)
			parser = gda_sql_parser_new ();
	}

#ifdef GDA_DEBUG
	g_print ("===== TEST %s SQL: @%s@\n", id, sql);
#endif

	stmt = gda_sql_parser_parse_string (parser, (const gchar*) sql, NULL, NULL);
	if (!stmt) {
		g_print ("ERROR for test '%s': could not parse statement\n", id);
		return FALSE;
	}
	is_valid = gda_statement_check_validity (stmt, cnc, &error);
	if (is_valid && !valid_expected) {
		g_print ("ERROR for test '%s': statement is valid but test expected it invalid\n", id);
		g_object_unref (stmt);
		return FALSE;
	}
	if (!is_valid && valid_expected) {
		g_print ("ERROR for test '%s': statement is invalid but test expected it valid: %s\n", id,
			 error && error->message ? error->message : "No detail");
		g_object_unref (stmt);
		return FALSE;
	}
	/*g_print ("EXP %d, got %d\n", valid_expected, is_valid);*/
	/*g_print ("PARSED: %s\n", gda_statement_serialize (stmt));*/

	g_object_unref (stmt);
	return TRUE;
}
