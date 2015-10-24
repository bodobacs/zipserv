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
import android.text.TextWatcher;
import android.text.Editable;

import android.app.Service;

//binding to service
import android.content.ComponentName;
import android.content.Context;
import android.os.IBinder;
import android.content.ServiceConnection;

import android.os.ResultReceiver; //to connect service with activity
import android.os.Handler;

//Ad stuff
import com.google.android.gms.ads.AdRequest.Builder;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdView;

public class ZipservApp extends Activity
{
	String TAG = "ZipservApp";

	public MyService mService;
	boolean mBound = false;
	boolean mnewfilechoosen = false;

	MyResultReceiver mReceiver = null;

	Button btn_select_file = null;
	Button btn_start_server = null;
	Button btn_stop_server = null;
	EditText et_port_number = null;
	Button btn_open_site = null;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

		setContentView(R.layout.main);

		btn_select_file = (Button)findViewById(R.id.btn_fileselect);
		btn_start_server = (Button)findViewById(R.id.btn_startserver);
		btn_stop_server = (Button) findViewById(R.id.btn_stopserver);
		et_port_number = (EditText)findViewById(R.id.et_portnumber);
		btn_open_site = (Button) findViewById(R.id.btn_opensite);


//AD's init

		AdView adView = (AdView)this.findViewById(R.id.adView);
		AdRequest adRequest = new AdRequest.Builder()
		.addTestDevice(AdRequest.DEVICE_ID_EMULATOR)
		.addTestDevice("6206E352DF65787684994AF229F64E18") //my nexus
		.build();
		adView.loadAd(adRequest);

//Ad's ^

		mReceiver = new MyResultReceiver(new Handler());

		if(savedInstanceState != null)
		{
		//	mopensuccess = savedInstanceState.getBoolean("a");
		//	mnewfilechoosen = savedInstanceState.getBoolean("b");
		}else{
		//	btn_open_site.setEnabled(false);
		//	btn_select_file.setEnabled(true);
		}

		//buttons default enabled state	
		btn_open_site.setEnabled(false);
		btn_stop_server.setEnabled(false);
    }

	@Override
	public void onSaveInstanceState(Bundle si)
	{
		super.onSaveInstanceState(si);

	//	si.putBoolean("a", mopensuccess);
	//	si.putBoolean("rba", mnewfilechoosen);

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

	@Override
	public void onResume() //this activity stops, but the service can run
	{
		super.onResume();
		mReceiver.pause(false);
	}

	@Override
	public void onPause() //this activity stops, but the service can run
	{
		super.onPause();
		mReceiver.pause(true);
	}

	MyService.LocalBinder mBinder;

	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName className, IBinder service)
		{
				Log.d(TAG, "onServiceConnected");

			try{
				// We've bound to LocalService, cast the IBinder and get LocalService instance
				mBinder = (MyService.LocalBinder) service;
				mService = mBinder.getService();

				mService.mReceiver_activity = mReceiver;

				mBound = true;

				//just now returned from file chooser and tries to open that new file
				et_port_number.setEnabled(false);
				portnumber = mService.portnumber;
				et_port_number.setText("" + portnumber);

				Log.d(TAG, "" + portnumber);

				btn_start_server.setEnabled(false);
				btn_stop_server.setEnabled(true);

				//new file choosen try to open and enable opening as a site
				if(mnewfilechoosen)
				{
					mnewfilechoosen = false;
					if(mService.open_archive(filename_selected))
					{
						btn_select_file.setText(mService.native_getfilename());
						btn_open_site.setEnabled(true);
					}else{
						btn_select_file.setText("Broken file:" + filename_selected);
					}
				}else{
					btn_select_file.setText(mService.native_getfilename());
				}

				if(mService.native_is_server_running())
				{
					btn_open_site.setEnabled(true);
				}else{
					btn_open_site.setEnabled(false);
				}

				//disable/enable corresponding buttons
			} catch (Exception e) {
				Log.d(TAG, "exception", e);
			}
				Log.d(TAG, "onServiceConnected end");
		}

		@Override
		public void onServiceDisconnected(ComponentName arg0) {

			Log.d(TAG, "onServiceDisconnected");
			mBound = false;
		}
	};

	public void onbtnWebsite(View v)
	{
		Intent browserIntent = new Intent(Intent.ACTION_VIEW, android.net.Uri.parse("https://sites.google.com/site/zservreader/"));
		startActivity(browserIntent);
	}

	public void start_server_service()
	{//starts the MyService but the real server thread

		Log.d(TAG, "Starting server");

		if(!mBound)
		{//not bound
			portnumber = Integer.parseInt(et_port_number.getText().toString());
			Intent i = new Intent(this, MyService.class);
			i.putExtra("portnum", (int)portnumber);
			startService(i);
			
			//bind service, return checked by serviceconnection callbacks
			bindService(i, mConnection, 0);
		}
	}

	//start service
	public void onStartServer(View v)
	{
		start_server_service();
	}

	public void onStopServer(View v)
	{
		Log.d(TAG, "onStopServer");
		if(mBound)
		{
			mService.stop_server();
			btn_open_site.setEnabled(false); 
		}
	}

	String localhost = "http://localhost:";
	int portnumber;

	public void open_site()
	{
		Intent browserIntent = new Intent(Intent.ACTION_VIEW, android.net.Uri.parse("http://localhost:" + portnumber));
		startActivity(browserIntent);
	}

	//open site in browser
	public void onOpenSite(View v)
	{
		Log.d(TAG, "portnumber: " + portnumber);
		open_site();
	}

	final int REQUEST_FILESELECTOR = 99;
	String filename_selected;

	public void onFileChoose(View v)
	{// start file chooser activity
		Intent intent = new Intent(this, MyFileSel.class);
		startActivityForResult(intent, REQUEST_FILESELECTOR);
	}

	//Choose archive activity
	@Override
	protected void onActivityResult(int req, int res, Intent i)//req: kérés azonosító, res: visszatérési kód
	{
		Log.d(TAG, "onActivityResult " + "request: " + req + " result: " + res + " RESULT_OK: " + RESULT_OK);

		if(RESULT_OK == res && REQUEST_FILESELECTOR == req) 
		{
			if(i.hasExtra("SelFile")) //a file kivalsztasa utan megszerzi a file nevet
			{
				filename_selected = i.getExtras().getString("SelFile");
				btn_select_file.setText(filename_selected);
				mnewfilechoosen = true;
				start_server_service();
			}
		}else{
				//help.zip lehetne a backup megoldás
		}
	}


	public class MyResultReceiver extends ResultReceiver {

		public MyResultReceiver(Handler handler){
		   super(handler);
		}

		private boolean bpause = false;
		public void pause(boolean b) { bpause = b; }

		//MyService feedback
		@Override
		protected void onReceiveResult(int resultCode, Bundle resultData) {
			// Anybody interested in the results? Well, then feel free to take them.
			if(!bpause)
			{
				switch(resultCode)
				{
					case 3:
						//service about to start
						btn_open_site.setEnabled(true);
						break;
					case 2:
					//service selfstopping
					//	btn_open_site.setEnabled(false); //no open site
						btn_stop_server.setEnabled(false); //no stop btn
						btn_start_server.setEnabled(true); //start btn ok
						et_port_number.setEnabled(true);
						break;
					case 1:
					//broken archive
						btn_select_file.setText("Choose another archive!");
						btn_open_site.setEnabled(false);
						break;
					case 0:
					default:
					//server status info
						break;
						
				}
			}
		}

	}
}

