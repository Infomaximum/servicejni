#pragma once

#include <jni.h>

extern "C" {

	JNIEXPORT jlong JNICALL Java_com_infomaximum_system_service_Service_create(JNIEnv* env, jclass jcls, jstring name, jstring executablePath, jint accountType);
	
	JNIEXPORT jlong JNICALL Java_com_infomaximum_system_service_Service_open(JNIEnv* env, jclass jcls, jstring name, jint accessRights);
	
	JNIEXPORT jboolean JNICALL Java_com_infomaximum_system_service_Service_existsService(JNIEnv* env, jclass jcls, jstring name);
	
	JNIEXPORT void JNICALL Java_com_infomaximum_system_service_Service_removeService(JNIEnv* env, jclass jcls, jstring name);


	JNIEXPORT void JNICALL Java_com_infomaximum_system_service_Service_start(JNIEnv* env, jobject jobj, jlong nativePointer);
	
	JNIEXPORT void JNICALL Java_com_infomaximum_system_service_Service_startAsync(JNIEnv* env, jobject jobj, jlong nativePointer);
	
	JNIEXPORT void JNICALL Java_com_infomaximum_system_service_Service_stop(JNIEnv* env, jobject jobj, jlong nativePointer);
	
	JNIEXPORT void JNICALL Java_com_infomaximum_system_service_Service_getStatus(JNIEnv* env, jobject jobj, jlong nativePointer, jbooleanArray isRunning, jintArray exitCode);
	
	JNIEXPORT void JNICALL Java_com_infomaximum_system_service_Service_setAutoRestart(JNIEnv* env, jobject jobj, jlong nativePointer, jboolean autoRestart, jstring postFailCommand);
	
	JNIEXPORT jstring JNICALL Java_com_infomaximum_system_service_Service_getDescription(JNIEnv* env, jobject jobj, jlong nativePointer);
	
	JNIEXPORT void JNICALL Java_com_infomaximum_system_service_Service_setDescription(JNIEnv* env, jobject jobj, jlong nativePointer, jstring description);
	
	JNIEXPORT void JNICALL Java_com_infomaximum_system_service_Service_sendCode(JNIEnv* env, jobject jobj, jlong nativePointer, jint userCode);
	
	JNIEXPORT jlong JNICALL Java_com_infomaximum_system_service_Service_getPid(JNIEnv* env, jobject jobj, jlong nativePointer);
	
	JNIEXPORT jstring JNICALL Java_com_infomaximum_system_service_Service_getExecutablePath(JNIEnv* env, jobject jobj, jlong nativePointer);
	
	JNIEXPORT void JNICALL Java_com_infomaximum_system_service_Service_destroy(JNIEnv* env, jobject jobj, jlong nativePointer);
}
