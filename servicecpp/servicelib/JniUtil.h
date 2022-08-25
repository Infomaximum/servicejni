#pragma once
#include <jni.h>

#include <string>

struct JniUtil
{
	static std::string ToStdString(JNIEnv* env, jstring jvalue)
	{
		if (jvalue == nullptr)
		{
			return std::string();
		}

		const char* value = env->GetStringUTFChars(jvalue, nullptr);
		std::string result(value);
		env->ReleaseStringUTFChars(jvalue, value);
		return result;
	}

	static jboolean ToJboolen(bool value)
	{
		return value ? JNI_TRUE : JNI_FALSE;
	}

	static bool ToBool(jboolean value)
	{
		return value == JNI_TRUE;
	}
};