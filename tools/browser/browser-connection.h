/* 
 * Copyright (C) 2009 Vivien Malerba
 *
 * This Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#ifndef __BROWSER_CONNECTION_H_
#define __BROWSER_CONNECTION_H_

#include <libgda/libgda.h>
#include "browser-favorites.h"
#include "decl.h"

G_BEGIN_DECLS

#define BROWSER_TYPE_CONNECTION          (browser_connection_get_type())
#define BROWSER_CONNECTION(obj)          G_TYPE_CHECK_INSTANCE_CAST (obj, browser_connection_get_type(), BrowserConnection)
#define BROWSER_CONNECTION_CLASS(klass)  G_TYPE_CHECK_CLASS_CAST (klass, browser_connection_get_type (), BrowserConnectionClass)
#define BROWSER_IS_CONNECTION(obj)       G_TYPE_CHECK_INSTANCE_TYPE (obj, browser_connection_get_type ())

typedef struct _BrowserConnectionClass BrowserConnectionClass;
typedef struct _BrowserConnectionPrivate BrowserConnectionPrivate;

/* struct for the object's data */
struct _BrowserConnection
{
	GObject                   object;
	BrowserConnectionPrivate *priv;
};

/* struct for the object's class */
struct _BrowserConnectionClass
{
	GObjectClass              parent_class;

	/* signals */
	void                    (*busy) (BrowserConnection *bcnc, gboolean is_busy, const gchar *reason);
	void                    (*meta_changed) (BrowserConnection *bcnc, GdaMetaStruct *mstruct);
	void                    (*favorites_changed) (BrowserConnection *bcnc);
	void                    (*transaction_status_changed) (BrowserConnection *bcnc);
	void                    (*table_column_pref_changed) (BrowserConnection *bcnc, GdaMetaTable *table,
							      GdaMetaTableColumn *column,
							      const gchar *attr_name, const gchar *value);
};

GType               browser_connection_get_type               (void) G_GNUC_CONST;

BrowserConnection  *browser_connection_new                    (GdaConnection *cnc);
const gchar        *browser_connection_get_name               (BrowserConnection *bcnc);
const GdaDsnInfo   *browser_connection_get_information        (BrowserConnection *bcnc);

gboolean            browser_connection_is_busy                (BrowserConnection *bcnc, gchar **out_reason);
void                browser_connection_update_meta_data       (BrowserConnection *bcnc);
GdaMetaStruct      *browser_connection_get_meta_struct        (BrowserConnection *bcnc);
GdaMetaStore       *browser_connection_get_meta_store         (BrowserConnection *bcnc);
const gchar        *browser_connection_get_dictionary_file    (BrowserConnection *bcnc);

BrowserFavorites   *browser_connection_get_favorites          (BrowserConnection *bcnc);

gchar             **browser_connection_get_completions        (BrowserConnection *bcnc, const gchar *sql,
							       gint start, gint end);

/*
 * statements's manipulations
 */
GdaSqlParser       *browser_connection_create_parser          (BrowserConnection *bcnc);
gchar              *browser_connection_render_pretty_sql      (BrowserConnection *bcnc,
							       GdaStatement *stmt);
guint               browser_connection_execute_statement      (BrowserConnection *bcnc,
							       GdaStatement *stmt,
							       GdaSet *params,
							       GdaStatementModelUsage model_usage,
							       gboolean need_last_insert_row,
							       GError **error);
guint               browser_connection_rerun_select           (BrowserConnection *bcnc,
							       GdaDataModel *model,
							       GError **error);
GObject            *browser_connection_execution_get_result   (BrowserConnection *bcnc,
							       guint exec_id,
							       GdaSet **last_insert_row, GError **error);
gboolean            browser_connection_normalize_sql_statement(BrowserConnection *bcnc,
							       GdaSqlStatement *sqlst, GError **error);
gboolean            browser_connection_check_sql_statement_validify (BrowserConnection *bcnc,
								     GdaSqlStatement *sqlst, GError **error);
/**
 * BrowserConnectionExecuteCallback
 *
 * Callback function called by browser_connection_execute_statement_cb(). If you need to keep
 * some of the arguments for a later usage, you need to ref/copy them.
 */
typedef void (*BrowserConnectionExecuteCallback) (BrowserConnection *bcnc,
						  guint exec_id,
						  GObject *out_result,
						  GdaSet *out_last_inserted_row, GError *error,
						  gpointer data);

guint               browser_connection_execute_statement_cb   (BrowserConnection *bcnc,
							       GdaStatement *stmt,
							       GdaSet *params,
							       GdaStatementModelUsage model_usage,
							       gboolean need_last_insert_row,
							       BrowserConnectionExecuteCallback callback,
							       gpointer data,
							       GError **error);
guint               browser_connection_rerun_select_cb        (BrowserConnection *bcnc,
							       GdaDataModel *model,
							       BrowserConnectionExecuteCallback callback,
							       gpointer data,
							       GError **error);


/*
 * transactions
 */
GdaTransactionStatus *browser_connection_get_transaction_status (BrowserConnection *bcnc);
gboolean              browser_connection_begin (BrowserConnection *bcnc, GError **error);
gboolean              browser_connection_commit (BrowserConnection *bcnc, GError **error);
gboolean              browser_connection_rollback (BrowserConnection *bcnc, GError **error);

/*
 * preferences
 */
#define BROWSER_CONNECTION_COLUMN_PLUGIN "PLUGIN"
gboolean             browser_connection_set_table_column_attribute (BrowserConnection *bcnc,
								    GdaMetaTable *table,
								    GdaMetaTableColumn *column,
								    const gchar *attr_name,
								    const gchar *value, GError **error);
gchar               *browser_connection_get_table_column_attribute (BrowserConnection *bcnc,
								    GdaMetaTable *table,
								    GdaMetaTableColumn *column,
								    const gchar *attr_name,
								    GError **error);

void                 browser_connection_define_ui_plugins_for_batch(BrowserConnection *bcnc,
								    GdaBatch *batch, GdaSet *params);
void                 browser_connection_define_ui_plugins_for_stmt (BrowserConnection *bcnc,
								    GdaStatement *stmt, GdaSet *params);

/*
 * Variables used at various places and for which a copy of the last recent value
 * is stored in the BrowserConnection object
 */
void                 browser_connection_keep_variables (BrowserConnection *bcnc, GdaSet *set);
void                 browser_connection_load_variables (BrowserConnection *bcnc, GdaSet *set);

G_END_DECLS

#endif