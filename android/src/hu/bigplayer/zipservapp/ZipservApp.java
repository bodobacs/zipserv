/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package hu.bigplayer.zipservapp;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import android.content.Intent;
import android.view.View;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.EditText;
import android.widget.Button;

import android.app.Service;

//binding to service
import android.content.ComponentName;
import android.content.Context;
import android.os.IBinder;
import android.content.ServiceConnection;

public class ZipservApp extends Activity
{
	String TAG = "ZipservApp";

	public MyService mService;
	boolean mBound = false;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.main);

		//bind service if it is already runing
	//	Intent i = new Intent(this, MyService.class);
	//	bindService(i, mConnection, Context.BIND_AUTO_CREATE);


//		((TextView)findViewById(R.id.et_portnumber)).setText(portnumber);
    }

	@Override
	public void onStart()
	{
		super.onStart();

		Intent i = new Intent(this, MyService.class);
		bindService(i, mConnection, 0);
		Log.d(TAG, "onStart");
	}

	@Override
	public void onStop() //this activity stops, but the service can run
	{
		super.onStop();
		Log.d(TAG, "onStop");
		
		if(mBound)
		{
			unbindService(mConnection);
			mBound = false;
		}

		Log.d(TAG, "unBind");

	}

	MyService.LocalBinder mBinder;

	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName className, IBinder service) {
			// We've bound to LocalService, cast the IBinder and get LocalService instance
			mBinder = (MyService.LocalBinder) service;
			mService = mBinder.getService();
			Log.d(TAG, "onServiceConnected");
//			mService.sendNotification("ServiceConnection", "Binded!");
			mBound = true;

			filename_selected = mService.str_selfn;
			portnumber = mService.portnumber;
			
//HERE should update the buttons disabled/enabled state

			//disable/enable corresponding buttons
			//((Button) findViewById(R.id.btn_startserver)).setEnabled(false);
			//((Button) findViewById(R.id.btn_stopserver)).setEnabled(true);
			//((EditText) findViewById(R.id.et_portnumber)).setEnabled(false);
		}

		@Override
		public void onServiceDisconnected(ComponentName arg0) {
			Log.d(TAG, "onServiceDisconnected");
			mBound = false;
		}
	};

	//start service
	public void onStartServer(View v)
	{
		Log.d(TAG, "onStartServer ->");

		if(!mBound && !filename_selected.isEmpty())
		{
			Log.d(TAG, "filename_selected: " + filename_selected);

			portnumber = Integer.parseInt(((EditText)findViewById(R.id.et_portnumber)).getText().toString());

			Intent i = new Intent(this, MyService.class);
			i.putExtra("SelFile", filename_selected);
			i.putExtra("PortNum", (int)portnumber);

			startService(i);
			
			//bind service, return checked by serviceconnection callbacks
			bindService(i, mConnection, 0);

		}else{//not bound
			Log.d(TAG, "File not selected or service not bound!");
		}
	}

	public void onStopServer(View v)
	{
		if(mBound)
		{
			mService.stop_server();
		}
	}

	String localhost = "http://localhost:";
	int portnumber = 19000;

	//open site in browser
	public void onOpenSite(View v)
	{
		
		Intent browserIntent = new Intent(Intent.ACTION_VIEW, android.net.Uri.parse("http://localhost:" + portnumber));
		startActivity(browserIntent);
	}

	public void switchService(View v)
	{
/*		Log.d(TAG, "switchService eleje");

		Button b = (Button)findViewById(R.id.btn_switch);
		b.setText("Gommba");

		Log.d(TAG, "switchService vége");
*/	}

	final int REQUEST_FILESELECTOR = 99;
	String filename_selected = "";

	public void onFileChoose(View v)
	{// start file chooser activity
		Intent intent = new Intent(this, MyFileSel.class);
		startActivityForResult(intent, REQUEST_FILESELECTOR);
	}

	@Override
	protected void onActivityResult(int req, int res, Intent i)//req: kérés azonosító, res: visszatérési kód
	{
		Log.d(TAG, "- - - onActivityResult " + "request: " + req + " result: " + res + " RESULT_OK: " + RESULT_OK);

		if(RESULT_OK == res && REQUEST_FILESELECTOR == req)
		{
			if(i.hasExtra("SelFile")) //a file kivalsztasa utan megszerzi a file nevet
			{
				filename_selected = i.getExtras().getString("SelFile");
				Log.d(TAG, "- - - Returned filename: " + filename_selected);
				
				((TextView)findViewById(R.id.tv_selectedfile)).setText(filename_selected);

				
			}
		}else{
			//help.zip lehetne a backup megoldás
			((TextView)findViewById(R.id.tv_selectedfile)).setText("No file");
		}
	}

}

