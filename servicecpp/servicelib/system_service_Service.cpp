#include "system_service_Service.h"

#include "Service.h"
#include "JniUtil.h"
#include "ExceptionJNI.hpp"

jlong Java_com_infomaximum_system_service_Service_create(JNIEnv* env, jclass jcls, jstring jname, jstring jexecutablePath, jint jaccountType)
{
	const std::string name = JniUtil::ToStdString(env, jname);
	const std::string executablePath = JniUtil::ToStdString(env, jexecutablePath);
	uint32_t err;
	std::unique_ptr<Service> pService = Service::Create(name, executablePath, static_cast<Service::AccountType>(jaccountType), err);
	if (err != 0)
	{
		SystemExceptionJni::ThrowNew(env, err);
		return 0;
	}
	return reinterpret_cast<jlong>(pService.release());
}

jlong Java_com_infomaximum_system_service_Service_open(JNIEnv* env, jclass jcls, jstring jname, jint jaccessRights)
{
	const std::string name = JniUtil::ToStdString(env, jname);
	uint32_t err;
	std::unique_ptr<Service> pService = Service::Open(name, static_cast<Service::OpenAccessRights>(jaccessRights), err);
	if (err != 0)
	{
		SystemExceptionJni::ThrowNew(env, err);
		return 0;
	}
	return reinterpret_cast<jlong>(pService.release());
}

jboolean Java_com_infomaximum_system_service_Service_existsService(JNIEnv* env, jclass jcls, jstring jname)
{
	const std::string name = JniUtil::ToStdString(env, jname);
	uint32_t err;
	bool exists = Service::Exists(name, err);
	if (err != 0)
	{
		SystemExceptionJni::ThrowNew(env, err);
		return JNI_FALSE;
	}
	return JniUtil::ToJboolen(exists);
}

void Java_com_infomaximum_system_service_Service_removeService(JNIEnv* env, jclass jcls, jstring jname)
{
	const std::string name = JniUtil::ToStdString(env, jname);
	uint32_t err;
	Service::Remove(name, err);
	if (err != 0)
	{
		SystemExceptionJni::ThrowNew(env, err);
	}
}

void Java_com_infomaximum_system_service_Service_start(JNIEnv* env, jobject jobj, jlong jnativePointer)
{
	Service* pService = reinterpret_cast<Service*>(jnativePointer);

	uint32_t err;
	pService->Start(err);
	if (err != 0)
	{
		SystemExceptionJni::ThrowNew(env, err);
	}
}

void Java_com_infomaximum_system_service_Service_startAsync(JNIEnv* env, jobject jobj, jlong jnativePointer)
{
	Service* pService = reinterpret_cast<Service*>(jnativePointer);

	uint32_t err;
	pService->StartAsync(err);
	if (err != 0)
	{
		SystemExceptionJni::ThrowNew(env, err);
	}
}

void Java_com_infomaximum_system_service_Service_stop(JNIEnv* env, jobject jobj, jlong jnativePointer)
{
	Service* pService = reinterpret_cast<Service*>(jnativePointer);

	uint32_t err;
	pService->Stop(err);
	if (err != 0)
	{
		SystemExceptionJni::ThrowNew(env, err);
	}
}

void Java_com_infomaximum_system_service_Service_getStatus(JNIEnv* env, jobject jobj, jlong jnativePointer, jbooleanArray jisRunning, jintArray jexitCode)
{
	Service* pService = reinterpret_cast<Service*>(jnativePointer);

	uint32_t err;
	auto status = pService->GetStatus(err);
	if (err != 0)
	{
		SystemExceptionJni::ThrowNew(env, err);
		return;
	}

	jboolean isRunning = JniUtil::ToJboolen(status.isRunning);
	env->SetBooleanArrayRegion(jisRunning, 0, 1, &isRunning);

	jint exitCode = static_cast<jint>(status.exitCode);
	env->SetIntArrayRegion(jexitCode, 0, 1, &exitCode);
}

void Java_com_infomaximum_system_service_Service_setAutoRestart(JNIEnv* env, jobject jobj, jlong jnativePointer, jboolean jautoRestart, jstring jpostFailCommand)
{
	Service* pService = reinterpret_cast<Service*>(jnativePointer);

	uint32_t err;
	pService->SetAutoRestart(JniUtil::ToBool(jautoRestart), JniUtil::ToStdString(env, jpostFailCommand), err);
	if (err != 0)
	{
		SystemExceptionJni::ThrowNew(env, err);
	}
}

jstring Java_com_infomaximum_system_service_Service_getDescription(JNIEnv* env, jobject jobj, jlong jnativePointer)
{
	Service* pService = reinterpret_cast<Service*>(jnativePointer);

	uint32_t err;
	std::string desc = pService->GetDescription(err);
	if (err != 0)
	{
		SystemExceptionJni::ThrowNew(env, err);
		return nullptr;
	}

	return env->NewStringUTF(desc.c_str());
}

void Java_com_infomaximum_system_service_Service_setDescription(JNIEnv* env, jobject jobj, jlong jnativePointer, jstring jdescription)
{
	Service* pService = reinterpret_cast<Service*>(jnativePointer);

	uint32_t err;
	pService->SetDescription(JniUtil::ToStdString(env, jdescription), err);
	if (err != 0)
	{
		SystemExceptionJni::ThrowNew(env, err);
	}
}

void Java_com_infomaximum_system_service_Service_sendCode(JNIEnv* env, jobject jobj, jlong jnativePointer, jint userCode)
{
	Service* pService = reinterpret_cast<Service*>(jnativePointer);

	uint32_t err;
	pService->SendCode(static_cast<uint32_t>(userCode), err);
	if (err != 0)
	{
		SystemExceptionJni::ThrowNew(env, err);
	}
}

jlong Java_com_infomaximum_system_service_Service_getPid(JNIEnv* env, jobject jobj, jlong jnativePointer)
{
	Service* pService = reinterpret_cast<Service*>(jnativePointer);

	uint32_t err;
	uint32_t pid = pService->GetPid(err);
	if (err != 0)
	{
		SystemExceptionJni::ThrowNew(env, err);
		return 0;
	}

	return static_cast<jlong>(pid);
}

jstring Java_com_infomaximum_system_service_Service_getExecutablePath(JNIEnv* env, jobject jobj, jlong jnativePointer)
{
	Service* pService = reinterpret_cast<Service*>(jnativePointer);

	uint32_t err;
	std::string path = pService->GetExecutablePath(err);
	if (err != 0)
	{
		SystemExceptionJni::ThrowNew(env, err);
		return nullptr;
	}

	return env->NewStringUTF(path.c_str());
}

void Java_com_infomaximum_system_service_Service_destroy(JNIEnv* env, jobject jobj, jlong jnativePointer)
{
	Service* pService = reinterpret_cast<Service*>(jnativePointer);

	delete pService;
}