#include <iostream>
#include <streambuf>
#include <cstdio>
#include <jni.h>
#include <android/log.h>

#include "../../src/zsrv.h"

//cd android/src
//javah -classpath ~/android-develop/sdk/platforms/android-11/android.jar:../bin/classes com.example.hellojni.MyService

const std::string TAG("jnizsrv");

class cmybuf : public std::streambuf //redirect cout,clog and cerr
{
	static const int buffersize = 50;
	char buffer[buffersize+1]; //+1 for 0 string closing character

public:
	cmybuf()
	{
		setp(buffer, buffer+(buffersize-1));
	}

	virtual ~cmybuf()
	{
		sync();
	}

protected:
	int flushbuffer(void)
	{
		*pptr() = 0;
		int num = pptr() - pbase();
//		if(num != printf("%s", pbase())) return EOF; // IDE KELL IRNI A KIMENETET!
//		if(write(1, buffer, num) != num) return EOF; //irashiba
		__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "%s", pbase());
		pbump(-num);
		return num;
	}

	virtual int_type overflow(int_type c)
	{
		if(EOF != c)
		{
			*pptr() = c; //gondolom azt nézi, hogy ezzel megtelik a buffer
			pbump(1);
		}

		if(EOF == flushbuffer()) return EOF;

		return c;
	}

	virtual int sync(void)
	{
		if(EOF == flushbuffer()) return -1;
		return 0;	
	}
};

/*
JNIEXPORT void myJNICallJavaFunc(JNIEnv* env, jobject obj)
{
    jclass cla = env->GetObjectClass(obj);

	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "START myJNICallJavaFunc");

	std::string s = "Csak kezd kialakulni";
	jmethodID jmid = env->GetMethodID(cla, "funcFromC", "(Ljava/lang/String;)V"); //ReleaseStringUTFChars
	if(jmid != 0)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "NewString");

		jstring jstr = env->NewStringUTF(s.c_str()); //ez utan nem kell Release?! Itt tudja, hogy miután visszatért a függvény, már nem kell. Akkor mi van, ha mashol is használni akarom, he?

		__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "CallStaticMethod");
		env->CallVoidMethod(obj, jmid, jstr);

		__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "ReleaseString not called");
	}
}

int JNI_call_java_IntFunc(JNIEnv* env, jobject obj, const std::string func_name, int param_i)
{
	jint ret = 0;
    jclass cla = env->GetObjectClass(obj);

	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "START myJNICallJavaFunc");

	std::string s = "Csak kezd kialakulni";
	jmethodID jmid = env->GetMethodID(cla, "fromjni_status", "(I)I"); //ReleaseStringUTFChars
	if(jmid != 0)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "NewString");

		jstring jstr = env->NewStringUTF(s.c_str()); //ez utan nem kell Release?! Itt tudja, hogy miután visszatért a függvény, már nem kell. Akkor mi van, ha mashol is használni akarom, he?

		__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "CallStaticMethod");
		ret = env->CallIntMethod(obj, jmid, (jint)param_i);

		__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "ReleaseString not called");
	}
	return (int)ret;
}
*/

//rerouting cout, cerr, clog
cmybuf mb;
std::streambuf *pcerrbuf = std::cerr.rdbuf(), *pcoutbuf = std::cout.rdbuf(), *pclogbuf = std::clog.rdbuf();

czsrv server;

JNIEXPORT jboolean cf_init_server(JNIEnv *env, jobject obj, jint ji)
{
	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "server INIT");

	int portnumber = (int)ji;

	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "ENV port: %d", portnumber);

	if(server.init(portnumber))
	{
		return JNI_TRUE;
	}

	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "server INIT failed");
	return JNI_FALSE;
}

bool bserver_running = false;

JNIEXPORT void cf_run_server(JNIEnv *env, jobject obj, jlong pointer)
{
	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "server RUNNING ...");

	bserver_running = true;
	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "server RUNNING 2");

	while(server.run_server());

	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "server not running");
	bserver_running = false;

	server.cleanup();

	std::cerr.rdbuf(pcerrbuf);
	std::clog.rdbuf(pclogbuf);
	std::cout.rdbuf(pcoutbuf);

	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "server NOT RUNNING");
}

JNIEXPORT void cf_stop_server(JNIEnv *env, jobject obj)
{
	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "server STOP");
	server.stop();
}

JNIEXPORT jboolean native_open_archive(JNIEnv *env, jobject obj, jstring jstr_fn)
{
	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "Open archive file");

	if(NULL != jstr_fn)
	{
		const char *cstr = env->GetStringUTFChars(jstr_fn, NULL); //Ez utan kell Release!? Mert itt nem tudja mikor lehet felszabadítani.

		std::string filename(cstr);

		env->ReleaseStringUTFChars(jstr_fn, cstr);

		if(server.open_archive(filename))
		{
			return JNI_TRUE;
		}
	}	
	return JNI_FALSE;
}

JNIEXPORT jstring native_getfilename(JNIEnv *env, jobject obj)
{
	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "native_getfilename");
	if(server.archive.get_filename().c_str())
		return env->NewStringUTF(server.archive.get_filename().c_str());
	else return 0;
}

JNIEXPORT jboolean native_is_server_running(JNIEnv *env, jobject obj)
{
	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "native_is_server_running");

	if(bserver_running) return JNI_TRUE;
	else return JNI_FALSE;
}

JNIEXPORT jint native_getport(JNIEnv *env, jobject obj)
{
	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "native_getport");

	return server.get_port();
}

/*
SOOOOOOOOOOO IMPORTANT
HAVE TO FOLLOW CHANGES OF FUNCTION LIST
*/

const int method_table_size = 7; //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//these can be called from java code + need declaration in java code
static JNINativeMethod method_table[] = {
	{ "cf_run_server", "()V", (void *)cf_run_server},
	{ "cf_stop_server", "()V", (void *)cf_stop_server},
//	{ "myJNICallJavaFunc", "()V", (void *)myJNICallJavaFunc }, nem kell ez csak megkeresi a fuggvenyt a javaban es meghivja, nem kell regisztralni
	{ "cf_init_server", "(I)Z", (bool *)cf_init_server}, //(String zip_fn, int nport);
	{ "native_open_archive", "(Ljava/lang/String;)Z", (bool *)native_open_archive}, //(String zip_fn, int nport);
	{ "native_getfilename", "()Ljava/lang/String;", (jstring *)native_getfilename},
	{ "native_is_server_running", "()Z", (bool *)native_is_server_running},
	{ "native_getport", "()I", (jint *)native_getport}
};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "CALL JNI_onLoad");

    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
    {
//		LOGI("JNI INIT");
		return JNI_ERR;
	}else{
		jclass clazz = (env)->FindClass("hu/bigplayer/zipservapp/MyService");
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

