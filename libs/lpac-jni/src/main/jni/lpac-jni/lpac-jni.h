#pragma once
#include <euicc/euicc.h>
#include <pthread.h>
#include <jni.h>

struct lpac_jni_ctx {
    jobject apdu_interface;
    jobject http_interface;
};

#define LPAC_JNI_CTX(ctx) ((struct lpac_jni_ctx *) ctx->userdata)
#define LPAC_JNI_SETUP_ENV \
    JNIEnv *env; \
    (*jvm)->AttachCurrentThread(jvm, &env, NULL)

#define __LPAC_JNI_LINKED_LIST_FOREACH(list, curr, body, after) { \
    int i = 0;                                                    \
    curr = list;                                                  \
    while (curr != NULL) {                                        \
        body;                                                     \
        curr = curr->next;                                        \
        i++;                                                      \
    };                                                             \
    after;                                                        \
}
#define LPAC_JNI_LINKED_LIST_FOREACH(list, curr, body) \
    __LPAC_JNI_LINKED_LIST_FOREACH(list, curr, body, {})
#define LPAC_JNI_LINKED_LIST_COUNT(list, curr) \
    (__LPAC_JNI_LINKED_LIST_FOREACH(list, curr, {}, i))

extern JavaVM *jvm;
extern jclass string_class;

jstring toJString(JNIEnv *env, const char *pat);