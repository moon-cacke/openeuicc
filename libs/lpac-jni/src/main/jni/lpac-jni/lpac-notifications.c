#include "lpac-notifications.h"
#include <euicc/es9p.h>
#include <euicc/es10b.h>
#include <malloc.h>
#include <syslog.h>

jclass local_profile_notification_class;
jmethodID local_profile_notification_constructor;

jclass local_profile_notification_operation_class;
jmethodID local_profile_notification_operation_from_string;

void lpac_notifications_init() {
    LPAC_JNI_SETUP_ENV;

    local_profile_notification_class =
            (*env)->FindClass(env, "net/typeblog/lpac_jni/LocalProfileNotification");
    local_profile_notification_class =
            (*env)->NewGlobalRef(env, local_profile_notification_class);
    local_profile_notification_constructor =
            (*env)->GetMethodID(env, local_profile_notification_class, "<init>",
                                      "(JLnet/typeblog/lpac_jni/LocalProfileNotification$Operation;Ljava/lang/String;Ljava/lang/String;)V");

    local_profile_notification_operation_class =
            (*env)->FindClass(env, "net/typeblog/lpac_jni/LocalProfileNotification$Operation");
    local_profile_notification_operation_class =
            (*env)->NewGlobalRef(env, local_profile_notification_operation_class);
    local_profile_notification_operation_from_string =
            (*env)->GetStaticMethodID(env, local_profile_notification_operation_class, "fromString",
                                      "(Ljava/lang/String;)Lnet/typeblog/lpac_jni/LocalProfileNotification$Operation;");
}

JNIEXPORT jobject JNICALL
Java_net_typeblog_lpac_1jni_LpacJni_es10bListNotification(JNIEnv *env, jobject thiz, jlong handle) {
    struct euicc_ctx *ctx = (struct euicc_ctx *) handle;
    struct es10b_notification_metadata *info = NULL;
    struct es10b_notification_metadata *curr = NULL;
    jobject notification = NULL;
    jobject operation = NULL;
    jobjectArray ret = NULL;
    int count = 0;

    if (es10b_list_notification(ctx, &info) < 0)
        return NULL;

    count = LPAC_JNI_LINKED_LIST_COUNT(info, curr);

    ret = (*env)->NewObjectArray(env, count, local_profile_notification_class, NULL);

    LPAC_JNI_LINKED_LIST_FOREACH(info, curr, {
        operation =
                (*env)->CallStaticObjectMethod(env, local_profile_notification_operation_class,
                                               local_profile_notification_operation_from_string,
                                               toJString(env, curr->profileManagementOperation));

        notification =
                (*env)->NewObject(env, local_profile_notification_class,
                                  local_profile_notification_constructor, curr->seqNumber, operation,
                                  toJString(env, curr->notificationAddress),
                                  toJString(env, curr->iccid));

        (*env)->SetObjectArrayElement(env, ret, i, notification);

        (*env)->DeleteLocalRef(env, operation);
        (*env)->DeleteLocalRef(env, notification);
    });

    es10b_notification_metadata_free_all(info);
    return ret;
}

JNIEXPORT jint JNICALL
Java_net_typeblog_lpac_1jni_LpacJni_handleNotification(JNIEnv *env, jobject thiz, jlong handle,
                                                       jlong seq_number) {
    struct euicc_ctx *ctx = (struct euicc_ctx *) handle;
    struct es10b_notification notification;
    int res;

    res = es10b_retrieve_notification(ctx, &notification, (unsigned long) seq_number);
    syslog(LOG_DEBUG, "es10b_retrieve_notification = %d", res);
    if (res < 0)
        goto out;

    res = es9p_handle_notification(ctx, notification.receiver, notification.b64_payload);
    syslog(LOG_DEBUG, "es9p_handle_notification = %d", res);
    if (res < 0)
        goto out;

    out:
    return res;
}

JNIEXPORT jint JNICALL
Java_net_typeblog_lpac_1jni_LpacJni_es10bDeleteNotification(JNIEnv *env, jobject thiz, jlong handle,
                                                            jlong seq_number) {
    struct euicc_ctx *ctx = (struct euicc_ctx *) handle;
    return es10b_remove_notification_from_list(ctx, (unsigned long) seq_number);
}