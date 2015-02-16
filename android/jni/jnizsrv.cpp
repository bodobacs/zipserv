#ifndef JNIZSRV_H
#define JNIZSRV_H

#include "zsrv.h"
#include <jni.h>
#include <android/log.h>

const std::string TAG("jnizsrv.cpp");

class czsrv_jni : public czsrv
{
public:
	czsrv_jni(std::string fn, int p) : czsrv::czsrv(fn, p)
	{
	}

protected:

private:

};

extern "C" {


static void myJNIFunc(JNIEnv* env, jclass clazz)
{
	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "START myJNIFunc");
}

static void myJNICallJavaFunc(JNIEnv* env, jclass clazz)
{
	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "START myJNICallJavaFunc");

	 std::string s = "Csak kezd kialakulni";
	jmethodID jmid = env->GetStaticMethodID(clazz, "funcFromC", "(Ljava/lang/String;)V"); //ReleaseStringUTFChars
	if(jmid != 0)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "NewString");

		jstring jstr = env->NewStringUTF(s.c_str()); //ez utan nem kell Release?! Itt tudja, hogy miután visszatért a függvény, már nem kell. Akkor mi van, ha mashol is használni akarom, he?

		__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "CallStaticMethod");
		env->CallStaticVoidMethod(clazz, jmid, jstr);

		__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "ReleaseString not called");
	}
}

void cf_init_zipserver(JNIEnv *env, jclass clazz, jstring jstr_fn, jint ji)
{
	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "Zip-Server initialize");

	if(NULL != jstr_fn)
	{
		const char *cstr = env->GetStringUTFChars(jstr_fn, NULL); //Ez utan kell Release!? Mert itt nem tudja mikor lehet felszabadítani.
		env->ReleaseStringUTFChars(jstr_fn, cstr);

		__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "Zip selected: %s", cstr);

		czsrv_jni server(cstr, 19000);

		if(server.open_zipfile())
		{
			server.run_server();

			server.close_zipfile();
		}
	}
}

const int method_table_size = 3;

//these can be called from java code + need declaration in java code
static JNINativeMethod method_table[] = {
	{ "myJNIFunc", "()V", (void *)myJNIFunc },
	{ "myJNICallJavaFunc", "()V", (void *)myJNICallJavaFunc },
	{ "cf_init_zipserver", "(Ljava/lang/String;I)V", (void *)cf_init_zipserver} //(String zip_fn, int nport);
};

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
    {
//		LOGI("JNI INIT");
		return JNI_ERR;
	}else{
		jclass clazz = (env)->FindClass("com/example/hellojni/MyService");
		if (clazz) {
			jint ret = (env)->RegisterNatives(clazz, method_table, method_table_size);
			(env)->DeleteLocalRef(clazz);
			return ret == 0 ? JNI_VERSION_1_6 : JNI_ERR;
		}else{
			return JNI_ERR;
    	}
	}
    return JNI_VERSION_1_6;
}

}//extern "C"

#endif

