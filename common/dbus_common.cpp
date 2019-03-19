/*
 *Author: Terry.Rong
 *Email: <terry_rong_3@sina.com>
 *  
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#ifdef DBUS_SRV_SUPPORT
#include "dbus_common.h"
#endif

namespace dbus_service{
#ifdef DBUS_SRV_SUPPORT
/*enum define*/

/*global var define*/
	static Properties remote_device_properties[] = {
		{"Address",  DBUS_TYPE_STRING},
		{"Name", DBUS_TYPE_STRING},
		{"Icon", DBUS_TYPE_STRING},
		{"Class", DBUS_TYPE_UINT32},
		{"UUIDs", DBUS_TYPE_ARRAY},
		{"Services", DBUS_TYPE_ARRAY},
		{"Paired", DBUS_TYPE_BOOLEAN},
		{"Connected", DBUS_TYPE_BOOLEAN},
		{"Trusted", DBUS_TYPE_BOOLEAN},
		{"Blocked", DBUS_TYPE_BOOLEAN},
		{"Alias", DBUS_TYPE_STRING},
		{"Nodes", DBUS_TYPE_ARRAY},
		{"Adapter", DBUS_TYPE_OBJECT_PATH},
		{"LegacyPairing", DBUS_TYPE_BOOLEAN},
		{"RSSI", DBUS_TYPE_INT16},
		{"TX", DBUS_TYPE_UINT32},
		{"Broadcaster", DBUS_TYPE_BOOLEAN}
	};

	static Properties adapter_properties[] = {
		{"Address", DBUS_TYPE_STRING},
		{"Name", DBUS_TYPE_STRING},
		{"Class", DBUS_TYPE_UINT32},
		{"Powered", DBUS_TYPE_BOOLEAN},
		{"Discoverable", DBUS_TYPE_BOOLEAN},
		{"DiscoverableTimeout", DBUS_TYPE_UINT32},
		{"Pairable", DBUS_TYPE_BOOLEAN},
		{"PairableTimeout", DBUS_TYPE_UINT32},
		{"Discovering", DBUS_TYPE_BOOLEAN},
		{"Devices", DBUS_TYPE_ARRAY},
		{"UUIDs", DBUS_TYPE_ARRAY},
	};

	static Properties input_properties[] = {
		{"Connected", DBUS_TYPE_BOOLEAN},
	};

	static Properties pan_properties[] = {
		{"Connected", DBUS_TYPE_BOOLEAN},
		{"Interface", DBUS_TYPE_STRING},
		{"UUID", DBUS_TYPE_STRING},
	};

	static Properties health_device_properties[] = {
		{"MainChannel", DBUS_TYPE_OBJECT_PATH},
	};

	static Properties health_channel_properties[] = {
		{"Type", DBUS_TYPE_STRING},
		{"Device", DBUS_TYPE_OBJECT_PATH},
		{"Application", DBUS_TYPE_OBJECT_PATH},
	};

/*struct define*/
	typedef struct {
		void (*user_cb)(DBusMessage *, void *, void *);
		void *user;
		void *priv_data;
	} dbus_async_call_t;

/*static functions*/
static void dbus_func_args_async_callback(DBusPendingCall *call, void *data) {

    dbus_async_call_t *req = (dbus_async_call_t *)data;
    DBusMessage *msg;

    /* This is guaranteed to be non-NULL, because this function is called only
       when once the remote method invokation returns. */
    msg = dbus_pending_call_steal_reply(call);

    if (msg) {
        if (req->user_cb) {
            // The user may not deref the message object.
            req->user_cb(msg, req->user, req->nat);
        }
        dbus_message_unref(msg);
    }

    //dbus_message_unref(req->method);
    dbus_pending_call_cancel(call);
    dbus_pending_call_unref(call);
    free(req);
}


static dbus_bool_t dbus_func_args_async_valist(JNIEnv *env,
                                        DBusConnection *conn,
                                        int timeout_ms,
                                        void (*user_cb)(DBusMessage *,
                                                        void *,
                                                        void*),
                                        void *user,
                                        void *nat,
                                        const char *path,
                                        const char *ifc,
                                        const char *func,
                                        int first_arg_type,
                                        va_list args) {
    DBusMessage *msg = NULL;
    const char *name;
    dbus_async_call_t *pending;
    dbus_bool_t reply = FALSE;

    /* Compose the command */
    msg = dbus_message_new_method_call(BLUEZ_DBUS_BASE_IFC, path, ifc, func);

    if (msg == NULL) {
        LOGE("Could not allocate D-Bus message object!");
        goto done;
    }

    /* append arguments */
    if (!dbus_message_append_args_valist(msg, first_arg_type, args)) {
        LOGE("Could not append argument to method call!");
        goto done;
    }

    /* Make the call. */
    pending = (dbus_async_call_t *)malloc(sizeof(dbus_async_call_t));
    if (pending) {
        DBusPendingCall *call;

        pending->env = env;
        pending->user_cb = user_cb;
        pending->user = user;
        pending->nat = nat;
        //pending->method = msg;

        reply = dbus_connection_send_with_reply(conn, msg,
                                                &call,
                                                timeout_ms);
        if (reply == TRUE) {
            dbus_pending_call_set_notify(call,
                                         dbus_func_args_async_callback,
                                         pending,
                                         NULL);
        }
    }

done:
    if (msg) dbus_message_unref(msg);
    return reply;
}

/*APIs define*/
dbus_bool_t dbus_func_args_async(JNIEnv *env,
								 DBusConnection *conn,
								 int timeout_ms,
								 void (*reply)(DBusMessage *, void *, void *),
								 void *user,
								 void *nat,
								 const char *path,
								 const char *ifc,
								 const char *func,
								 int first_arg_type,
								 ...){
    dbus_bool_t ret;
    va_list lst;
    va_start(lst, first_arg_type);

    ret = dbus_func_args_async_valist(env, conn,
                                      timeout_ms,
                                      reply, user, nat,
                                      path, ifc, func,
                                      first_arg_type, lst);
    va_end(lst);
    return ret;

}

DBusMessage * dbus_func_args(JNIEnv *env,
							 DBusConnection *conn,
							 const char *path,
							 const char *ifc,
							 const char *func,
							 int first_arg_type,
							 ...){
    DBusMessage *ret;
    va_list lst;
    va_start(lst, first_arg_type);
    ret = dbus_func_args_timeout_valist(env, conn, -1, NULL,
                                        path, ifc, func,
                                        first_arg_type, lst);
    va_end(lst);
    return ret;
}

DBusMessage * dbus_func_args_error(JNIEnv *env,
								   DBusConnection *conn,
								   DBusError *err,
								   const char *path,
								   const char *ifc,
								   const char *func,
								   int first_arg_type,
								   ...){
    DBusMessage *ret;
    va_list lst;
    va_start(lst, first_arg_type);
    ret = dbus_func_args_timeout_valist(env, conn, -1, err,
                                        path, ifc, func,
                                        first_arg_type, lst);
    va_end(lst);
    return ret;
}

DBusMessage * dbus_func_args_timeout_valist(JNIEnv *env,
											DBusConnection *conn,
											int timeout_ms,
											DBusError *err,
											const char *path,
											const char *ifc,
											const char *func,
											int first_arg_type,
											va_list args){

    DBusMessage *msg = NULL, *reply = NULL;
    const char *name;
    bool return_error = (err != NULL);

    if (!return_error) {
        err = (DBusError*)malloc(sizeof(DBusError));
        dbus_error_init(err);
    }

    /* Compose the command */
    msg = dbus_message_new_method_call(BLUEZ_DBUS_BASE_IFC, path, ifc, func);

    if (msg == NULL) {
        LOGE("Could not allocate D-Bus message object!");
        goto done;
    }

    /* append arguments */
    if (!dbus_message_append_args_valist(msg, first_arg_type, args)) {
        LOGE("Could not append argument to method call!");
        goto done;
    }

    /* Make the call. */
    reply = dbus_connection_send_with_reply_and_block(conn, msg, timeout_ms, err);
    if (!return_error && dbus_error_is_set(err)) {
        LOG_AND_FREE_DBUS_ERROR_WITH_MSG(err, msg);
    }

done:
    if (!return_error) {
        free(err);
    }
    if (msg) dbus_message_unref(msg);
    return reply;
}

DBusMessage * dbus_func_args_timeout(JNIEnv *env,
									 DBusConnection *conn,
									 int timeout_ms,
									 const char *path,
									 const char *ifc,
									 const char *func,
									 int first_arg_type,
									 ...){
    DBusMessage *ret;
    va_list lst;
    va_start(lst, first_arg_type);
    ret = dbus_func_args_timeout_valist(env, conn, timeout_ms, NULL,
                                        path, ifc, func,
                                        first_arg_type, lst);
    va_end(lst);
    return ret;
}


int dbus_returns_int32(DBusMessage *reply){
    DBusError err;
    int ret = -1;

    dbus_error_init(&err);
    if (!dbus_message_get_args(reply, &err,
                               DBUS_TYPE_INT32, &ret,
                               DBUS_TYPE_INVALID)) {
        LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, reply);
    }
    dbus_message_unref(reply);
    return ret;

}

int dbus_returns_uint32( DBusMessage *reply){

    DBusError err;
    int ret = -1;

    dbus_error_init(&err);
    if (!dbus_message_get_args(reply, &err,
                               DBUS_TYPE_UINT32, &ret,
                               DBUS_TYPE_INVALID)) {
        LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, reply);
    }
    dbus_message_unref(reply);
    return ret;
}

int dbus_returns_unixfd(DBusMessage *reply){
    DBusError err;
    int ret = -1;

    dbus_error_init(&err);
    if (!dbus_message_get_args(reply, &err,
                               DBUS_TYPE_UNIX_FD, &ret,
                               DBUS_TYPE_INVALID)) {
        LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, reply);
    }
    dbus_message_unref(reply);
    return ret;
}

string dbus_returns_string( DBusMessage *reply){
    DBusError err;
    char * ret = NULL;
    const char *name;

    dbus_error_init(&err);
    if (dbus_message_get_args(reply, &err,
                               DBUS_TYPE_STRING, &name,
                               DBUS_TYPE_INVALID)) {
        ret = strdup(name);
    } else {
        LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, reply);
    }
    dbus_message_unref(reply);

    return ret;
}

int dbus_returns_boolean(DBusMessage *reply){
    DBusError err;
    int ret = 0;
    dbus_bool_t val = 0;

    dbus_error_init(&err);

    /* Check the return value. */
    if (dbus_message_get_args(reply, &err,
                               DBUS_TYPE_BOOLEAN, &val,
                               DBUS_TYPE_INVALID)) {
        ret = val ? 1 : 0;
    } else {
        LOG_AND_FREE_DBUS_ERROR_WITH_MSG(&err, reply);
    }

    dbus_message_unref(reply);
    return ret;
}

vector<string> *dbus_returns_array_of_strings(DBusMessage *reply){

}

vector<string> *dbus_returns_array_of_object_path( DBusMessage *reply){

}

vector<char> *dbus_returns_array_of_bytes( DBusMessage *reply){

}

#endif
}


