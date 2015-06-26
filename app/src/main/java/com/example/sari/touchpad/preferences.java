package com.example.sari.touchpad;

import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;


public class preferences extends PreferenceActivity {

    @Override
    public void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.xml.preferences);
        PreferenceManager.setDefaultValues(this, R.xml.preferences);
    }
}
