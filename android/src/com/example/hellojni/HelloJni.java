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
package com.example.hellojni;

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
import android.os.IBinder;

public class HelloJni extends Activity
{
	String TAG = "HelloJni";
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.main);

//		((TextView)findViewById(R.id.et_portnumber)).setText(portnumber);
    }

	public void onBumm(View v)
	{
		portnumber = Integer.parseInt(((EditText)findViewById(R.id.et_portnumber)).getText().toString());

		Log.d(TAG, "bumm start");

		Intent i = new Intent(this, MyService.class);
		i.putExtra("SelFile", filename_selected);
		i.putExtra("PortNum", (int)portnumber);

		startService(i);

		Log.d(TAG, "bumm end");

	}

	String localhost = "http://localhost:";
	int portnumber = 19000;

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
	String filename_selected;

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

