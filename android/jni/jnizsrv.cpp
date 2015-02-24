#include <iostream>
#include <streambuf>
#include <cstdio>
#include <jni.h>
#include <android/log.h>

#include "zsrv.h"

//cd android/src
//javah -classpath ~/android-develop/sdk/platforms/android-11/android.jar:../bin/classes com.example.hellojni.MyService

const std::string TAG("czsrv_jni");

class cmybuf : public std::streambuf
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


class czsrv_jni : public czsrv
{
	std::streambuf *psysbuf;
	cmybuf mb;

public:
	czsrv_jni()
	{
		__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "!! 1. Constructor");
//		zipname = "/sdcard/cuccok/abs-guide.zip";	
		__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "!! 2. Constructor");
//		listen_port = 19000;
		__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "!! 3. Constructor");
/*
		psysbuf = std::cerr.rdbuf();
		std::cerr.rdbuf(&mb);*/
	}

/*	void init(std::string fn, int p, JNIEnv* env, jobject jo)
	{
		mEnv = env;
		mJobj = jo;

	}
*/
	~czsrv_jni()
	{
//		std::cerr.rdbuf(psysbuf);
	}

protected:
//	jobject mJobj;
//	JNIEnv* mEnv;

private:

};

//extern "C" {


JNIEXPORT void myJNIFunc(JNIEnv* env, jobject obj)
{
	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "START myJNIFunc");
}

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

JNIEXPORT void cf_init_zipserver(JNIEnv *env, jobject obj, jstring jstr_fn, jint ji)
{
	__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "Zip-Server initialize");

	if(NULL != jstr_fn)
	{
		const char *cstr = env->GetStringUTFChars(jstr_fn, NULL); //Ez utan kell Release!? Mert itt nem tudja mikor lehet felszabadítani.
		__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "zipname from ENV: %s", cstr);

		cmybuf mb;
		std::streambuf *pcerrbuf = std::cerr.rdbuf(), *pcoutbuf = std::cout.rdbuf(), *pclogbuf = std::clog.rdbuf();
		std::cerr.rdbuf(&mb);
		std::cout.rdbuf(&mb);
		std::clog.rdbuf(&mb);

		std::cerr << "cout REDIRECTED" << std::endl;

		__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "server CREATE");

		czsrv server(cstr, 19000);

		env->ReleaseStringUTFChars(jstr_fn, cstr);

	//	server.init("/sdcard/cuccok/abs-guide.zip", 19000, env, obj);

		__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "server START");

		if(server.open_zipfile())
		{
			server.run_server();
			server.close_zipfile();
		}

		__android_log_print(ANDROID_LOG_VERBOSE, TAG.c_str(), "server END");

		std::cerr.rdbuf(pcerrbuf);
		std::clog.rdbuf(pcerrbuf);
		std::cout.rdbuf(pcerrbuf);
	}
}

const int method_table_size = 3;

//these can be called from java code + need declaration in java code
static JNINativeMethod method_table[] = {
	{ "myJNIFunc", "()V", (void *)myJNIFunc },
	{ "myJNICallJavaFunc", "()V", (void *)myJNICallJavaFunc },
	{ "cf_init_zipserver", "(Ljava/lang/String;I)V", (void *)cf_init_zipserver} //(String zip_fn, int nport);
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

//}//extern "C"


