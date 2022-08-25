#pragma once

#include <jni.h>
#include <iostream>

class VoidCallbackJni
{
private:

	class EnvironmentGuard
	{
	private:

		JavaVM* jvm = nullptr;
		JNIEnv* env = nullptr;

	public:

		explicit EnvironmentGuard(JavaVM* jvm)
		{
			if (jvm == nullptr)
			{
				return;
			}

			jint res = jvm->GetEnv((void**)&env, JNI_VERSION_1_8);
			if (res != JNI_OK)
			{
				res = jvm->AttachCurrentThread((void**)&env, NULL);
				if (res == JNI_OK)
				{
					this->jvm = jvm;
				}
				else
				{
					std::cerr << "Failed attach to current thread, result = " << res << std::endl;
				}
			}
		}

		~EnvironmentGuard()
		{
			if (jvm != nullptr)
			{
				jvm->DetachCurrentThread();
			}
		}

		bool isNull() const
		{
			return env == nullptr;
		}

		JNIEnv* Get()
		{
			return env;
		}
	};

private:

	jobject		jGlobalHandler = nullptr;
	jmethodID	jmethod = nullptr;
	JavaVM*		jvm = nullptr;

public:

	VoidCallbackJni(JNIEnv* env, jobject jhandler)
	{
		jGlobalHandler = env->NewGlobalRef(jhandler);

		jclass handlerClass = env->GetObjectClass(jGlobalHandler);
		jmethod = env->GetMethodID(handlerClass, "apply", "()V");
		if (jmethod != 0) 
		{
			env->GetJavaVM(&jvm);
		}
		else
		{
			std::cerr << "Could not get method 'apply'!" << std::endl;
		}
	}

	VoidCallbackJni(VoidCallbackJni&& source)
	{
		this->jGlobalHandler = source.jGlobalHandler;
		this->jmethod = source.jmethod;
		this->jvm = source.jvm;

		source.Reset();
	}

	VoidCallbackJni& operator=(VoidCallbackJni&& source)
	{
		this->Clear();

		this->jGlobalHandler = source.jGlobalHandler;
		this->jmethod = source.jmethod;
		this->jvm = source.jvm;

		source.Reset();
		return *this;
	}

	VoidCallbackJni(const VoidCallbackJni&)
	{
		std::cerr << "VoidCallbackJni(const VoidCallbackJni&) should not be called!";
	}

	VoidCallbackJni& operator=(const VoidCallbackJni&) = delete;

	~VoidCallbackJni()
	{
		Clear();
	}

	void Clear()
	{
		if (jGlobalHandler != nullptr)
		{
			CallIntoEnvironment([this](JNIEnv* env) {
				env->DeleteGlobalRef(jGlobalHandler);
			});
		}

		Reset();
	}

	void Call()
	{
		CallIntoEnvironment([this](JNIEnv* env) {
			env->CallVoidMethod(jGlobalHandler, jmethod); 
		});
	}

private:

	void Reset()
	{
		this->jGlobalHandler = nullptr;
		this->jmethod = nullptr;
		this->jvm = nullptr;
	}

	template<typename TMethod>
	void CallIntoEnvironment(TMethod method)
	{
		EnvironmentGuard env(jvm);
		if (!env.isNull())
		{
			method(env.Get());
		}
	}
};