package hu.bigplayer.zipservapp;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.Intent;
import android.view.View;
import android.os.Bundle;

import android.support.v4.app.NotificationCompat; // STATIKUS KONYVTÁR, be kell másolni az sdk-ból!
import android.app.NotificationManager; 
import android.app.Notification; 

import android.util.Log; 

import android.app.Service;
import android.os.IBinder;
import android.os.Binder;

import android.os.ConditionVariable;

import android.os.ResultReceiver; //to connect service with activity

public class MyService extends Service
{
	static String TAG = "MyService";
	private final int UNIQE_NOTI_ID = 13;

	public ResultReceiver mReceiver_activity;

//	private ConditionVariable mCond;
	String fn_opened; //successfully opened archive file
	public int portnumber;
	public boolean bthreadrunning = false;

	public native boolean cf_init_server(int nport);
	public native void cf_stop_server();
	public native void cf_run_server();
	public native boolean native_open_archive(String jstr_fn);

	public boolean mopensuccess = false;

	public void stop_server()
	{
		cf_stop_server();
	}

	public boolean open_archive(String s)
	{
		if(!s.isEmpty() && native_open_archive(s))
		{
			fn_opened = s;
			feedback_string(0, "Archive opened successfully");
			sendNotification(s, "http://localhost:" + portnumber);
			mopensuccess = true;

			return true;
		}

		sendNotification(s, "Archive not opened.");
		feedback_string(0, "Archive not opened");
		mopensuccess = false;

		return false;
	}

	public void start_to_serve()
	{//starts the real separate thread running server cycle
		if(!bthreadrunning)
		{
			Thread serverThread = new Thread(null, mServerRunnable, "MyService");
			serverThread.start();
		}
	}

	static
	{
		try {
				System.loadLibrary("modchmlib");
				System.loadLibrary("unzip");
				System.loadLibrary("jnizsrv");
				Log.d(TAG, "Myservice constructor complete, libraries loaded.");
        }
        catch (UnsatisfiedLinkError e) {
				Log.d(TAG, "Cannot load shared libraries.");
        }
	}
	
	public void sendNotification(String sTitle, String sText)
	{
		Intent notiIntent = new Intent(this, ZipservApp.class);
		PendingIntent pIntent = PendingIntent.getActivity(this, 0, notiIntent, PendingIntent.FLAG_UPDATE_CURRENT);

	//	Intent intent = new Intent();//this, ActNotif.class); //saját magát nyitja meg
	//	PendingIntent pIntent = PendingIntent.getActivity(this, 0, intent, 0);


		NotificationCompat.Builder mBuilder = new NotificationCompat.Builder(this)
//		Notification noti = new NotificationCompat.Builder(this)
												.setAutoCancel(true)
												.setContentTitle(sTitle)
												.setContentText(sText)
												.setSmallIcon(R.drawable.zserv_icon_bar)
												.setContentIntent(pIntent);
											//	.build();

		Notification noti = mBuilder.build();

		NotificationManager mNotMngr = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
		mNotMngr.notify(UNIQE_NOTI_ID, noti);
	}

//		private NotificationManager nM;

	@Override
	public void onCreate()
	{
		super.onCreate();

		Log.d(TAG, "onCreate");

		start_to_serve();
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId)
	{
		// itt kell kitalálni mi van az átadott intent-el
		portnumber = intent.getExtras().getInt("portnum");

		Intent notiIntent = new Intent(this, ZipservApp.class);
		PendingIntent pIntent = PendingIntent.getActivity(this, 0, notiIntent, PendingIntent.FLAG_UPDATE_CURRENT);

		NotificationCompat.Builder mBuilder = new NotificationCompat.Builder(this)
//		Notification noti = new NotificationCompat.Builder(this)
												.setAutoCancel(true)
												.setContentTitle(this.getString(R.string.app_name))
												.setContentText("http://localhost" + portnumber)
												.setSmallIcon(R.drawable.zserv_icon_bar)
												.setContentIntent(pIntent);
											//	.build();

		Notification noti = mBuilder.build();
		startForeground(UNIQE_NOTI_ID, noti);

    	return START_STICKY;
	}

	@Override
	public void onDestroy()
	{
		super.onDestroy();
	}

	private final IBinder mBinder = new LocalBinder();

	public class LocalBinder extends Binder {
        MyService getService() {
            // Return this instance of LocalService so clients can call public methods
            return MyService.this;
        }
    }

	@Override //kotelezo mert absztrakt fuggvény
	public IBinder onBind(Intent intent)
	{
		return mBinder;
	}

	//ebben a Runnable-ben fog futni az zipserver
	private Runnable mServerRunnable = new Runnable()
	{
		public void run()
		{
			try {

			Log.d(TAG, "--------------- Thread started --------------------");

			feedback_string(0, "Server starting");
			if(true == cf_init_server(portnumber))
			{
				feedback_string(0, "Server running, open your book in browser!");

				bthreadrunning = true; 
				cf_run_server();

				bthreadrunning = false; 

				feedback_status(2);

				feedback_string(0, "Server stopped");
			}else{
				feedback_string(1, "Server initialization failed");
			}

			MyService.this.stopForeground(false); //java ...
			MyService.this.stopSelf(); //java ...

			Log.d(TAG, "----------------- Thread end ----------------------");

			} catch (Exception e) {
				Log.d(TAG, "exception", e);
			}
		}
	};

	public void funcFromC(String s)
	{

			Log.d(TAG, "Message from C++ lib" + s);
	}
	
	public int fromjni_status(int i)
	{
			Log.d(TAG, "Status from jni: " + i);
			return i+1;
	}

	public void feedback_string(int r, String s)
	{
		if(mReceiver_activity != null)
		{
			Bundle b = new Bundle(); b.putString("m", s);
			mReceiver_activity.send(r, b);
		}
	}

	public void feedback_int(int r, int i)
	{
		if(mReceiver_activity != null)
		{
			Bundle b = new Bundle(); b.putInt("m", i);
			mReceiver_activity.send(r, b);
		}
	}

	public void feedback_status(int r)
	{
		if(mReceiver_activity != null) mReceiver_activity.send(r, null);
	}
}


