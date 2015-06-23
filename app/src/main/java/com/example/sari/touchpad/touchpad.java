package com.example.sari.touchpad;

import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;


public class touchpad extends ActionBarActivity {
    static final private String LOG_TAG = "touchpad";

    static final private int CANCEL_ID = Menu.FIRST;

    // Options menu.
    static final private int CONNECT_ID = CANCEL_ID + 1;
    static final private int DISCONNECT_ID = CONNECT_ID + 1;
    static final private int ADD_FAVORITE_ID = DISCONNECT_ID + 1;
    static final private int PREFERENCES_ID = ADD_FAVORITE_ID + 1;
    static final private int EXIT_ID = PREFERENCES_ID + 1;

    // Context (server) menu.
    static final private int FAVORITES_ID = CANCEL_ID + 1;
    static final private int FIND_SERVERS_ID = FAVORITES_ID + 1;
    static final private int SERVER_CUSTOM_ID = FIND_SERVERS_ID + 1;
    static final private int SERVER_FOUND_ID = Menu.FIRST;
    static final private int SERVER_FAVORITE_ID = Menu.FIRST + 1;

    static final protected int KeepAlive = 2000;
    static final private int DefaultPort = 2999;
    static final private int MaxServers = 9;

    // Current preferences.
    protected short Port;
    protected float Sensitivity;
    protected int MultitouchMode;
    protected int Timeout;
    protected boolean EnableScrollBar;
    protected int ScrollBarWidth;
    protected boolean EnableSystem;

    // State.
    protected Handler timer = new Handler();
    protected Socket server = null;
    protected ImageView touchpad;
    protected View mousebuttons;
    protected View keyboard, modifiers;
    protected View media, browser;
    protected ToggleButton[] button = { null, null };
    protected ToggleButton key_shift, key_ctrl, key_alt;

    public Touchpad() {
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_touchpad);
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_touchpad, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
