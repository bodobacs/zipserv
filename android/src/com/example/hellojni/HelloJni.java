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
//import android.widget.TextView;
import android.os.Bundle;
import android.util.Log;

//import android.app.PendingIntent;
import android.content.Intent;
import android.view.View;
import android.os.Bundle;
//import android.support.v4.app.NotificationCompat; // STATIKUS KONYVTÁR, be kell másolni az sdk-ból!
//import android.app.NotificationManager; 
//import android.app.Notification; 
//import android.util.Log; 
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

        /* Create a TextView and set its content.
         * the text is retrieved by calling a native
         * function.
         */
/*		Log.d(TAG,"-------------- onCreate 1 ------------");
        TextView  tv = new TextView(this);
		String message;
		message = stringFromJNI() + myJNIFunc();

        tv.setText(message);

		Log.d(TAG,"-------------- onCreate 2 ------------");*/
    }

	public void onBumm(View v)
	{
		Log.d(TAG, "bumm start");

		startService(new Intent(this, MyService.class));

		Log.d(TAG, "bumm end");

	}

	public void switchService(View v)
	{
		Log.d(TAG, "switchService eleje");

		Button b = (Button)findViewById(R.id.btn_switch);
		b.setText("Gommba");

		Log.d(TAG, "switchService vége");
	}

}

