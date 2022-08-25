#pragma once

#include <jni.h>

extern "C" {

	JNIEXPORT jlong JNICALL Java_com_infomaximum_system_service_ServiceEntry_create(JNIEnv* env, jclass jcls, jstring serviceName);


	JNIEXPORT void JNICALL Java_com_infomaximum_system_service_ServiceEntry_registerEventHandler(JNIEnv* env, jobject jobj, jlong nativePointer, jint event, jobject handler);

	JNIEXPORT void JNICALL Java_com_infomaximum_system_service_ServiceEntry_registerNativeEventHandler(JNIEnv* env, jobject jobj, jlong nativePointer, jint nativeEvent, jobject handler);

	JNIEXPORT void JNICALL Java_com_infomaximum_system_service_ServiceEntry_setExitCode(JNIEnv* env, jobject jobj, jlong nativePointer, jint exitCode);

	JNIEXPORT void JNICALL Java_com_infomaximum_system_service_ServiceEntry_execute(JNIEnv* env, jobject jobj, jlong nativePointer);

	JNIEXPORT void JNICALL Java_com_infomaximum_system_service_ServiceEntry_exit(JNIEnv* env, jobject jobj, jlong nativePointer);

	JNIEXPORT void JNICALL Java_com_infomaximum_system_service_ServiceEntry_destroy(JNIEnv* env, jobject jobj, jlong nativePointer);

}