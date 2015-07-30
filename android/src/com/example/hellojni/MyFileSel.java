package com.example.hellojni;

import android.app.ListActivity;
import android.os.Bundle;
import android.content.Intent;
import android.widget.ArrayAdapter;

import java.io.File;
import java.io.FilenameFilter;
import android.util.Log;
import android.os.Environment;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import android.view.View;
import android.widget.ListView;
import android.widget.TextView;
//import android.widget.Button;

public class MyFileSel extends ListActivity
{	
	private static String TAG = "MyFileSel";

	private	List<String> mFileList;
	private String mPath = Environment.getExternalStorageDirectory().getPath(); // "/"; //actual path to filenames
	private	int selected_file_index = -1;
	private ArrayAdapter<String> adapter;
	private final String str_FTYPE = ""; //".zip"; //extension filter

	public void onCreate(Bundle bundle)
	{
		super.onCreate(bundle);
		Log.d(TAG, "onCreate 1");
		setContentView(R.layout.mylist);

		Log.d(TAG, "2");
		mFileList = new ArrayList<String>(); //contains file names, found filenames copied here

		Log.d(TAG, "3");
		//adapter initialization
		adapter = new ArrayAdapter(this, android.R.layout.simple_list_item_1, mFileList);
		setListAdapter(adapter);

		Log.d(TAG, "4");
		loadFileList(mPath); //list mPath and copies filenames to mFileList

		Log.d(TAG, "5");
		((TextView)findViewById(R.id.tv_cim)).setText(mPath);
	}

	@Override
	public void finish()
	{
		Log.d(TAG, "START finish()");

		if(selected_file_index != -1)
		{
			Log.d(TAG, "* * * finish, van kivalsztas" );

			Intent i = new Intent();

			i.putExtra("SelFile", mPath + "/" + mFileList.get(selected_file_index));

			setResult(RESULT_OK, i);
		}else{
			Log.d(TAG, "* * * finish, nincs kivalsztas" );

			setResult(RESULT_CANCELED);
		}
		super.finish();
	}

	@Override
	public void onListItemClick(ListView l, View v, int position, long id)
	{
		super.onListItemClick(l, v, position, id);

		Log.d(TAG, "START OnListItemClick choosen file: " + mFileList.get((int)id));


		if((new File(mPath + "/" + mFileList.get((int)id))).isDirectory())
		{
			Log.d(TAG, "It is a DIR");

			if(mPath.endsWith("/")) loadFileList(mPath + mFileList.get((int)id)); //ha /-rel végződik nem kell beletoldani
			else loadFileList(mPath + "/" + mFileList.get((int)id));

			selected_file_index = -1;
			((TextView)findViewById(R.id.tv_cim)).setText(mPath);
	//		((Button)findViewById(R.id.btn_select)).setEnabled(false);
			((TextView)findViewById(R.id.btn_select)).setText("Cancel");

		}else{
			selected_file_index = (int)id;
			Log.d(TAG, "It is NOT a DIR");
			((TextView)findViewById(R.id.tv_cim)).setText(mPath + "/" + mFileList.get(selected_file_index));
//			((Button)findViewById(R.id.btn_select)).setEnabled(true);
			((TextView)findViewById(R.id.btn_select)).setText("Select");
		}

		Log.d(TAG, "END OnListItemClick");
	}

	public void onUP(View v)
	{
		File f = new File(mPath);
		String p;
		if(null != (p = f.getParent()))
		{
			loadFileList(p);
			((TextView)findViewById(R.id.tv_cim)).setText(mPath);
		//	((Button)findViewById(R.id.btn_select)).setEnabled(false);
			((TextView)findViewById(R.id.btn_select)).setText("Cancel");
		}
	}

	public void onSelect(View v)
	{
		finish();	
	}

	private void loadFileList(String path)
	{
		Log.d(TAG, "START loadFileList: " + path);

		File f = new File(path);
		if(f.exists() && f.canRead() && f.isDirectory())
		{
			FilenameFilter filter = new FilenameFilter() {

				@Override
					public boolean accept(File dir, String filename) {
						File sel = new File(dir, filename);
						return filename.contains(str_FTYPE) || sel.isDirectory(); //meg nem aktivalt filter
					}
			};

			mPath = path;

			//changes here effects adapter!!! adapter.clear(), add(), addAll() etc. causes exception!
			mFileList.clear();
			mFileList.addAll(Arrays.asList((new File(path)).list(filter)));
			adapter.notifyDataSetChanged(); //that is the way

			for(String fn : mFileList) Log.d(TAG, fn); // just feedback
		}

		Log.d(TAG, "END loadFileList");
	}

}


