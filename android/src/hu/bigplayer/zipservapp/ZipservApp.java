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

import android.os.ResultReceiver; //to connect service with activity
import android.os.Handler;

public class ZipservApp extends Activity
{
	String TAG = "ZipservApp";

	public MyService mService;
	boolean mBound = false;
	MyResultReceiver mReceiver = null;

	TextView tv_myserv_output = null;
	TextView tv_selected_file = null;
	Button btn_file_select = null;
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
		btn_file_select = (Button)findViewById(R.id.btn_fileselect);
		btn_start_server = (Button)findViewById(R.id.btn_startserver);
		btn_stop_server = (Button) findViewById(R.id.btn_stopserver);
		et_port_number = (EditText)findViewById(R.id.et_portnumber);
		btn_open_site = (Button) findViewById(R.id.btn_opensite);

		buttons_step_1();


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

	//			mService.sendNotification("ServiceConnection", "Binded!");
				mBound = true;

				tv_selected_file.setText(mService.str_selfn);
		//		et_port_number.setText(mService.portnumber);

				Log.d(TAG, "mService.str_selfn" + mService.str_selfn + " mService.portnumber: " + mService.portnumber);

				buttons_step_3();

				//disable/enable corresponding buttons
			} catch (Exception e) {
				Log.d(TAG, "exception", e);
			}
				Log.d(TAG, "onServiceConnected end");
		}

		@Override
		public void onServiceDisconnected(ComponentName arg0) {

			buttons_step_1();

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

			portnumber = Integer.parseInt(et_port_number.getText().toString());

			Intent i = new Intent(this, MyService.class);
			i.putExtra("SelFile", filename_selected);
			i.putExtra("PortNum", (int)portnumber);

			startService(i);
			
			//bind service, return checked by serviceconnection callbacks
			bindService(i, mConnection, 0);

		}else{//not bound
			Log.d(TAG, "File not selected or service already bound!");
		}
	}

	public void onStopServer(View v)
	{
		if(mBound) mService.stop_server();
	}

	//BUTTONS ENABLED/DISABLED SETS
	public void buttons_step_1() //file not yet selected
	{
		btn_file_select.setEnabled(true);
		btn_start_server.setEnabled(false);
		btn_stop_server.setEnabled(false);
		et_port_number.setEnabled(true);
		btn_open_site.setEnabled(false);

		if(!filename_selected.isEmpty())
		{
			btn_start_server.setEnabled(true);
			tv_selected_file.setText(filename_selected);
		}
		
		Log.d(TAG, "buttons_step_1");
	}

//there is a step 2 but, no need for it really

	public void buttons_step_3() //server running
	{
		//HERE should update the buttons disabled/enabled state
		btn_file_select.setEnabled(false);
		btn_start_server.setEnabled(false);
		btn_stop_server.setEnabled(true);
		et_port_number.setEnabled(false);
		btn_open_site.setEnabled(true);

		Log.d(TAG, "buttons_step_3");
	}

	String localhost = "http://localhost:";
	int portnumber = 19000;

	//open site in browser
	public void onOpenSite(View v)
	{
		
		Intent browserIntent = new Intent(Intent.ACTION_VIEW, android.net.Uri.parse("http://localhost:" + portnumber));
		startActivity(browserIntent);
	}

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
		Log.d(TAG, "onActivityResult " + "request: " + req + " result: " + res + " RESULT_OK: " + RESULT_OK);

		if(RESULT_OK == res && REQUEST_FILESELECTOR == req)
		{
			if(i.hasExtra("SelFile")) //a file kivalsztasa utan megszerzi a file nevet
			{
				filename_selected = i.getExtras().getString("SelFile");
				Log.d(TAG, "Selected filename: " + filename_selected);
				
				buttons_step_1();
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
						filename_selected = "";
						tv_selected_file.setText("File read error (file error or password protected)");
						buttons_step_1();
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

