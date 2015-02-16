package com.example.hellojni;

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

import android.os.ConditionVariable;

public class MyService extends Service
{
	static String TAG = "HelloJni";
	private int notifId = 1;
	private final int UNIQE_NOTI_ID = 13;

	private ConditionVariable mCond;
	String str_selfn = "nofileselected";

	public void sendNotification(String sTitle, String sText)
	{
		Intent intent = new Intent();//this, ActNotif.class); //saját magát nyitja meg
		PendingIntent pIntent = PendingIntent.getActivity(this, 0, intent, 0);


		NotificationCompat.Builder mBuilder = new NotificationCompat.Builder(this)
//		Notification noti = new NotificationCompat.Builder(this)
												.setAutoCancel(true)
												.setContentTitle("My Foreground Service")
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
//		nM = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);

		Thread serverThread = new Thread(null, mServerRunnable, "MyService");

		mCond = new ConditionVariable(false);//késleltetés az üzenetekhez
		serverThread.start();

	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId)
	{
		// itt kell kitalálni mi van az átadott intent-el
		str_selfn = intent.getExtras().getString("SelFile");

		if(intent.hasExtra("SelFile")) Log.d(TAG, "Selected ZIP: " + str_selfn);
		else Log.d(TAG, "NO EXTRA");

		Intent notiIntent = new Intent();
		PendingIntent pIntent = PendingIntent.getActivity(this, 0, notiIntent, 0);

		NotificationCompat.Builder mBuilder = new NotificationCompat.Builder(this)
//		Notification noti = new NotificationCompat.Builder(this)
												.setAutoCancel(true)
												.setContentTitle("ForegroundTitle")
												.setContentText("ServiceText")
												.setSmallIcon(R.drawable.evilicon)
												.setContentIntent(pIntent);
											//	.build();

		Notification noti = mBuilder.build();

		startForeground(UNIQE_NOTI_ID, noti);

    	return START_STICKY;
	}

	@Override
	public void onDestroy()
	{
		mCond.open();
	}

	@Override //kotelezo mert absztrakt fuggvény
	public IBinder onBind(Intent intent)
	{
		return null;
	}

	//ebben a Runnable-ben fog futni az zipserver
	private Runnable mServerRunnable = new Runnable()
	{
		public void run()
		{
			Log.d(TAG, "--------------- Thread started --------------------");

//			for(int i = 0; i < 180; i++)
//			{
//				MyService.this.sendNotification("MyService", "cycle: " + i);

//				mCond.block(1000);//3*
//			}
				
				myJNIFunc();

				myJNICallJavaFunc();

				cf_init_zipserver(str_selfn, 1);


			Log.d(TAG, "Thread topSelf");
			MyService.this.sendNotification("MyService", "Cycles ended.");

			MyService.this.stopForeground(false); //java ...
			
			MyService.this.stopSelf(); //java ...

			MyService.this.sendNotification("MyService", "Very END.");

			Log.d(TAG, "--------------- Thread end --------------------");
		}
	};


	String natStrCout = "kocsog ";
	
	public void addCout()
	{
		natStrCout += "fasz javasok";
		Log.d(TAG, natStrCout);
	}
	
	public static void funcFromC(String s)
	{
		Log.d(TAG, "Message from C++: " + s);
	}

	public static native void cf_init_zipserver(String zip_fn, int nport);
	public static native void myJNIFunc();
	public static native void myJNICallJavaFunc();

	static {
		System.loadLibrary("jnizsrv");
	}
}

