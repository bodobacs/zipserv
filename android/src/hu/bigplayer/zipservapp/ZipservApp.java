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

	TextView tv_myserv_output = null;
	TextView tv_selected_file = null;
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

		tv_myserv_output = (TextView)findViewById(R.id.tv_myserv_output);
		tv_selected_file = (TextView)findViewById(R.id.tv_selectedfile);
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

		et_port_number.addTextChangedListener(new TextWatcher()
		{
			@Override
			public void afterTextChanged(Editable s)
			{
				portnumber = Integer.parseInt(et_port_number.getText().toString());
				Log.d(TAG, "portnumber changed");
			}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}
		});

		mReceiver = new MyResultReceiver(new Handler());

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

				if(mnewfilechoosen)
				{
					mService.open_archive(filename_selected);
					mnewfilechoosen = false;

					open_site();
				}else{
					tv_selected_file.setText(mService.fn_opened);
				}

				//disable/enable corresponding buttons
			} catch (Exception e) {
				Log.d(TAG, "exception", e);
			}
				Log.d(TAG, "onServiceConnected end");
		}

		@Override
		public void onServiceDisconnected(ComponentName arg0) {

	//		buttons_step_1();

			Log.d(TAG, "onServiceDisconnected");
			mBound = false;
		}
	};

	public void start_server_service()
	{//starts the MyService but the real server thread

		Log.d(TAG, "onStartServer ->");

		if(!mBound)
		{//not bound
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
		if(mBound)
		{ 
			mService.stop_server();
			et_port_number.setEnabled(true);
		}
	}

	String localhost = "http://localhost:";
	int portnumber = 19000;

	public void open_site()
	{
		Intent browserIntent = new Intent(Intent.ACTION_VIEW, android.net.Uri.parse("http://localhost:" + portnumber));
		startActivity(browserIntent);
	}

	//open site in browser
	public void onOpenSite(View v)
	{
		start_server_service();
		open_site();
	}

	final int REQUEST_FILESELECTOR = 99;
	String filename_selected;

	public void onFileChoose(View v)
	{// start file chooser activity
		Intent intent = new Intent(this, MyFileSel.class);
		startActivityForResult(intent, REQUEST_FILESELECTOR);
	}

	@Override
	protected void onActivityResult(int req, int res, Intent i)//req: kérés azonosító, res: visszatérési kód
	{
		Log.d(TAG, "onActivityResult " + "request: " + req + " result: " + res + " RESULT_OK: " + RESULT_OK);

		if(RESULT_OK == res && REQUEST_FILESELECTOR == req)
		{
			if(i.hasExtra("SelFile")) //a file kivalsztasa utan megszerzi a file nevet
			{
				filename_selected = i.getExtras().getString("SelFile");
				tv_selected_file.setText(filename_selected);
				mnewfilechoosen = true;
			}
		}else{
			//help.zip lehetne a backup megoldás
		//	tv_selected_file.setText("No file");
		}
	}


	public class MyResultReceiver extends ResultReceiver {

		public MyResultReceiver(Handler handler){
		   super(handler);
		}

		@Override
		protected void onReceiveResult(int resultCode, Bundle resultData) {
			// Anybody interested in the results? Well, then feel free to take them.
			if(!bpause)
			{
				switch(resultCode)
				{
					case 1:
					//service tried to open archie but failed
						filename_selected = "";
						tv_selected_file.setText("File read error");
						break;
					default:
						set_server_status(resultData.getString("msg"));
						
				}
			}
		}

		private boolean bpause = false;

		public void pause(boolean b)
		{
			bpause = b;
		}
	}

	public void set_server_status(String s)
	{
		Log.d(TAG, "set_server_status: " + s);
		tv_myserv_output.setText(s); 
	}


}
