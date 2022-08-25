#pragma once

#include <jni.h>

extern "C" {

	JNIEXPORT jlong JNICALL Java_com_infomaximum_system_registry_WindowsRegistry_create(JNIEnv* env, jclass jcls, jlong hkey, jstring key, jint access);

	JNIEXPORT jlong JNICALL Java_com_infomaximum_system_registry_WindowsRegistry_open(JNIEnv* env, jclass jcls, jlong hkey, jstring key, jint access);

	JNIEXPORT void JNICALL Java_com_infomaximum_system_registry_WindowsRegistry_remove(JNIEnv* env, jclass jcls, jlong hkey, jstring key);

	JNIEXPORT void JNICALL Java_com_infomaximum_system_registry_WindowsRegistry_setValue(JNIEnv* env, jobject jobj, jlong nativePointer, jstring name, jint type, jbyteArray value);

	JNIEXPORT void JNICALL Java_com_infomaximum_system_registry_WindowsRegistry_destroy(JNIEnv* env, jobject jobj, jlong nativePointer);
}