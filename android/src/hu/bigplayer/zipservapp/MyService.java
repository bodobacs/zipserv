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
	private int notifId = 1;
	private final int UNIQE_NOTI_ID = 13;

	public ResultReceiver mReceiver_activity;

//	private ConditionVariable mCond;
	String str_selfn = "nofileselected";
	int portnumber = 19000;

	public native boolean cf_init_server(String zip_fn, int nport);
	public native void cf_stop_server();
	public native void cf_run_server();

	public void stop_server()
	{
		cf_stop_server();
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
		Intent intent = new Intent();//this, ActNotif.class); //saját magát nyitja meg
		PendingIntent pIntent = PendingIntent.getActivity(this, 0, intent, 0);


		NotificationCompat.Builder mBuilder = new NotificationCompat.Builder(this)
//		Notification noti = new NotificationCompat.Builder(this)
												.setAutoCancel(true)
												.setContentTitle("ZReader")
												.setContentText(sText)
												.setSmallIcon(R.drawable.evilicon)
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
		Log.d(TAG, "onCreate");
		super.onCreate();

		Thread serverThread = new Thread(null, mServerRunnable, "MyService");

		serverThread.start();

	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId)
	{
		// itt kell kitalálni mi van az átadott intent-el
		str_selfn = intent.getExtras().getString("SelFile");
		portnumber = intent.getExtras().getInt("PortNum");

		if(intent.hasExtra("SelFile")) Log.d(TAG, "Found extra data - SelFile: " + str_selfn);
		else Log.d(TAG, "Not found Extra data!");

		Intent notiIntent = new Intent(this, ZipservApp.class);
		PendingIntent pIntent = PendingIntent.getActivity(this, 0, notiIntent, 0);

		NotificationCompat.Builder mBuilder = new NotificationCompat.Builder(this)
//		Notification noti = new NotificationCompat.Builder(this)
												.setAutoCancel(true)
												.setContentTitle("Zipserv")
												.setContentText("Running")
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

			inform_activity(0, "Server starting");
			if(true == cf_init_server(str_selfn, portnumber))
			{
				inform_activity(0, "Server running, open your book in browser!");
				cf_run_server();
				inform_activity(0, "Server stopped");
			}else{
				inform_activity(1, "Server initialization failed");
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

	public void inform_activity(int r, String s)
	{
		if(mReceiver_activity != null)
		{
			Bundle b = new Bundle(); b.putString("msg", s);
			mReceiver_activity.send(r, b);

			Log.d(TAG, "MSG to Activity: " + s);
		}
	}
}


