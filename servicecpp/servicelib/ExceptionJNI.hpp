#pragma once
#include <jni.h>
#include "ErrorConverter.h"
#include <iostream>

template<class T> class JavaException
{
protected:

	JavaException()
	{}
	
	static bool ThrowNew(JNIEnv* env, const std::string& msg) 
	{
		jclass jclazz = T::getJClass(env);
		if (jclazz == nullptr) {
			std::cerr << "JavaException::ThrowNew - Error: occurred accessing class!" << std::endl;
			return env->ExceptionCheck() == JNI_TRUE;
		}

		const jint rs = env->ThrowNew(jclazz, msg.c_str());
		if (rs != JNI_OK) {
			std::cerr << "JavaException::ThrowNew - Fatal: could not throw exception!" << std::endl;
			return env->ExceptionCheck() == JNI_TRUE;
		}

		return true;
	}
};

class SystemExceptionJni : public JavaException<SystemExceptionJni>
{
public:

	static jclass getJClass(JNIEnv* env)
	{
		return env->FindClass("com/infomaximum/system/SystemException");
	}

	static bool ThrowNew(JNIEnv* env, const std::string& msg)
	{
		return JavaException::ThrowNew(env, msg);
	}

	static bool ThrowNew(JNIEnv* env, uint32_t err)
	{
		return JavaException::ThrowNew(env, ErrorConverter::toString(err));
	}
};
