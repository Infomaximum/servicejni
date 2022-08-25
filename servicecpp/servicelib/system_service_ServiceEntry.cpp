#include "system_service_ServiceEntry.h"
#include "ExceptionJNI.hpp"
#include "JniUtil.h"
#include "CallbackJNI.hpp"

#include "ServiceEntry.h"

jlong Java_com_infomaximum_system_service_ServiceEntry_create(JNIEnv* env, jclass jcls, jstring jserviceName)
{
	const std::string serviceName = JniUtil::ToStdString(env, jserviceName);

	return reinterpret_cast<jlong>(new ServiceEntry(serviceName));
}

void Java_com_infomaximum_system_service_ServiceEntry_registerEventHandler(JNIEnv* env, jobject jobj, jlong jnativePointer, jint jevent, jobject jhandler)
{
	ServiceEntry* pServiceEntry = reinterpret_cast<ServiceEntry*>(jnativePointer);

	pServiceEntry->RegisterEventHandler(static_cast<ServiceEntry::Event>(jevent), std::bind([](VoidCallbackJni& callback) {
		callback.Call();
	}, VoidCallbackJni(env, jhandler)));
}

void Java_com_infomaximum_system_service_ServiceEntry_registerNativeEventHandler(JNIEnv* env, jobject jobj, jlong jnativePointer, jint jnativeEvent, jobject jhandler)
{
	ServiceEntry* pServiceEntry = reinterpret_cast<ServiceEntry*>(jnativePointer);
	
	pServiceEntry->RegisterEventHandler(static_cast<uint32_t>(jnativeEvent), std::bind([]
#if defined(WINDOWS)
	(uint32_t /*eventType*/, void* /*pEventData*/, VoidCallbackJni& callback)
#else
	(VoidCallbackJni& callback)
#endif
	{
		callback.Call();
	}, std::placeholders::_1, std::placeholders::_2, VoidCallbackJni(env, jhandler)));
}

void Java_com_infomaximum_system_service_ServiceEntry_setExitCode(JNIEnv* env, jobject jobj, jlong nativePointer, jint exitCode)
{
	ServiceEntry* pServiceEntry = reinterpret_cast<ServiceEntry*>(nativePointer);

	pServiceEntry->SetExitCode(static_cast<uint32_t>(exitCode));
}

void Java_com_infomaximum_system_service_ServiceEntry_execute(JNIEnv* env, jobject jobj, jlong nativePointer)
{
	ServiceEntry* pServiceEntry = reinterpret_cast<ServiceEntry*>(nativePointer);

	uint32_t err;
	pServiceEntry->Execute(err);
	if (err != 0)
	{
		SystemExceptionJni::ThrowNew(env, err);
	}
}

void Java_com_infomaximum_system_service_ServiceEntry_exit(JNIEnv* env, jobject jobj, jlong nativePointer)
{
	ServiceEntry* pServiceEntry = reinterpret_cast<ServiceEntry*>(nativePointer);

	pServiceEntry->Exit();
}

void Java_com_infomaximum_system_service_ServiceEntry_destroy(JNIEnv* env, jobject jobj, jlong nativePointer)
{
	ServiceEntry* pServiceEntry = reinterpret_cast<ServiceEntry*>(nativePointer);

	delete pServiceEntry;
}
