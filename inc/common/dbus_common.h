/*
 *Author: Terry.Rong
 *Email: <terry_rong_3@sina.com>
 *  
 */

#ifndef DBUS_COMMON_H
#define DBUS_COMMON_H

#include "cppni.h"

#ifdef DBUS_SRV_SUPPORT
#include <dbus/dbus.h>
#endif

namespace dbus_service{
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
                                     ...);
    
    DBusMessage * dbus_func_args(JNIEnv *env,
                                 DBusConnection *conn,
                                 const char *path,
                                 const char *ifc,
                                 const char *func,
                                 int first_arg_type,
                                 ...);
    
    DBusMessage * dbus_func_args_error(JNIEnv *env,
                                       DBusConnection *conn,
                                       DBusError *err,
                                       const char *path,
                                       const char *ifc,
                                       const char *func,
                                       int first_arg_type,
                                       ...);

    DBusMessage * dbus_func_args_timeout_valist(JNIEnv *env,
                                                DBusConnection *conn,
                                                int timeout_ms,
                                                DBusError *err,
                                                const char *path,
                                                const char *ifc,
                                                const char *func,
                                                int first_arg_type,
                                                va_list args);

    
    DBusMessage * dbus_func_args_timeout(JNIEnv *env,
                                         DBusConnection *conn,
                                         int timeout_ms,
                                         const char *path,
                                         const char *ifc,
                                         const char *func,
                                         int first_arg_type,
                                         ...);
    

    
    int dbus_returns_int32(DBusMessage *reply);
    int dbus_returns_uint32( DBusMessage *reply);
    int dbus_returns_unixfd(DBusMessage *reply);
    string dbus_returns_string( DBusMessage *reply);
    int dbus_returns_boolean(DBusMessage *reply);
    vector<string> *dbus_returns_array_of_strings(DBusMessage *reply);
    vector<string> *dbus_returns_array_of_object_path( DBusMessage *reply);
    vector<char> *dbus_returns_array_of_bytes( DBusMessage *reply);

}

#endif
