#include "system_service_WindowsRegistry.h"
#include "ExceptionJNI.hpp"
#include "StringConverter.h"
#include "JniUtil.h"

#include <Windows.h>

jlong Java_com_infomaximum_system_registry_WindowsRegistry_create(JNIEnv* env, jclass jcls, jlong jhkey, jstring jkey, jint jaccess)
{
	const std::wstring key = StringConverter::UTF8toUTF16(JniUtil::ToStdString(env, jkey));

	HKEY hSubKey;
	DWORD error = RegCreateKeyExW(reinterpret_cast<HKEY>(jhkey), key.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, static_cast<REGSAM>(jaccess), NULL, &hSubKey, NULL);
	if (error != ERROR_SUCCESS)
	{
		SystemExceptionJni::ThrowNew(env, error);
		return 0;
	}

	return reinterpret_cast<jlong>(hSubKey);
}

jlong Java_com_infomaximum_system_registry_WindowsRegistry_open(JNIEnv* env, jclass jcls, jlong jhkey, jstring jkey, jint jaccess)
{
	const std::wstring key = StringConverter::UTF8toUTF16(JniUtil::ToStdString(env, jkey));

	HKEY hSubKey;
	DWORD error = RegOpenKeyExW(reinterpret_cast<HKEY>(jhkey), key.c_str(), 0, static_cast<REGSAM>(jaccess), &hSubKey);
	if (error != ERROR_SUCCESS)
	{
		SystemExceptionJni::ThrowNew(env, error);
		return 0;
	}

	return reinterpret_cast<jlong>(hSubKey);
}

void Java_com_infomaximum_system_registry_WindowsRegistry_remove(JNIEnv* env, jclass jcls, jlong jhkey, jstring jkey)
{
	const std::wstring key = StringConverter::UTF8toUTF16(JniUtil::ToStdString(env, jkey));
	DWORD error = RegDeleteTreeW(reinterpret_cast<HKEY>(jhkey), key.c_str());
	switch (error)
	{
	case ERROR_SUCCESS:
	case ERROR_FILE_NOT_FOUND:
		// do nothing
		break;
	default:
		SystemExceptionJni::ThrowNew(env, error);
		break;
	}
}

void Java_com_infomaximum_system_registry_WindowsRegistry_setValue(JNIEnv* env, jobject jobj, jlong jnativePointer, jstring jname, jint jtype, jbyteArray jvalue)
{
	const std::wstring name = StringConverter::UTF8toUTF16(JniUtil::ToStdString(env, jname));
	const DWORD valueSize = static_cast<DWORD>(env->GetArrayLength(jvalue));
	
	std::string value;
	value.reserve(valueSize);
	env->GetByteArrayRegion(jvalue, 0, valueSize, (jbyte*)const_cast<char*>(value.data()));

	DWORD error = RegSetValueExW(reinterpret_cast<HKEY>(jnativePointer), name.c_str(), 0, static_cast<DWORD>(jtype), (BYTE*)value.data(), valueSize);
	if (error != ERROR_SUCCESS)
	{
		SystemExceptionJni::ThrowNew(env, error);
	}
}

void Java_com_infomaximum_system_registry_WindowsRegistry_destroy(JNIEnv* env, jobject jobj, jlong jnativePointer)
{
	RegCloseKey(reinterpret_cast<HKEY>(jnativePointer));
}