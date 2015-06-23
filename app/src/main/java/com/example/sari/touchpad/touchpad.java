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
        
        // Set touchpad events.
        touchpad = (ImageView) findViewById(R.id.touchpad);
        touchpad.setOnTouchListener(mTouchListener);

        // Get button containers.
        LinearLayout buttons = (LinearLayout) findViewById(R.id.buttons);

        // Set keyboard events.
        keyboard = (View) buttons.findViewById(R.id.keyboard);
        keyboard.setOnClickListener(mKeyboardListener);
        keyboard.setOnKeyListener(mKeyListener);

        // Keyboard modifiers.
        modifiers = (View) buttons.findViewById(R.id.modifiers);

        // Set mouse button events.
        mousebuttons = (LinearLayout) findViewById(R.id.mousebuttons);

        button[0] = (ToggleButton) mousebuttons.findViewById(R.id.button0);
        button[0].setOnCheckedChangeListener(mButton0ToggleListener);
        button[0].setOnLongClickListener(mButton0ClickListener);

        button[1] = (ToggleButton) mousebuttons.findViewById(R.id.button1);
        button[1].setOnCheckedChangeListener(mButton1ToggleListener);
        button[1].setOnLongClickListener(mButton1ClickListener);


        // Set media button events.
        media = (LinearLayout) findViewById(R.id.media);

        View playpause = media.findViewById(R.id.playpause);
        playpause.setOnClickListener(new OnClickListener() {
            public void onClick(View v) { sendKeyPress((short) KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE, (short) 0); }
        });

        View stop = media.findViewById(R.id.stop);
        stop.setOnClickListener(new OnClickListener() {
            public void onClick(View v) { sendKeyPress((short) KeyEvent.KEYCODE_MEDIA_STOP, (short) 0); }
        });

        // Set browser button events.
        browser = (LinearLayout) findViewById(R.id.browser);

        // Set up preferences.
        PreferenceManager.setDefaultValues(this, R.xml.preferences, false);

        // If there is no server to reconnect, set the background to bad.
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
        String to = preferences.getString("Server", null);
        if (to == null)
            touchpad.setImageResource(R.drawable.background_bad);
    }

    @Override
    protected void onResume() {
        super.onResume();

        // Restore settings.
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);

        try { Port = (short) Integer.parseInt(preferences.getString("Port", Integer.toString(DefaultPort))); } catch(NumberFormatException ex) { Port = DefaultPort; }
        Sensitivity = (float) preferences.getInt("Sensitivity", 50) / 25.0f + 0.1f;
        try { MultitouchMode = Integer.parseInt(preferences.getString("MultitouchMode", "0")); } catch(NumberFormatException ex) { MultitouchMode = 0; }
        Timeout = preferences.getInt("Timeout", 500) + 1;
        EnableScrollBar = preferences.getBoolean("EnableScrollBar", preferences.getBoolean("EnableScroll", true));
        ScrollBarWidth = preferences.getInt("ScrollBarWidth", 20);
        EnableSystem = preferences.getBoolean("EnableSystem", true);

        boolean EnableMouseButtons = preferences.getBoolean("EnableMouseButtons", false);
        boolean EnableModifiers = preferences.getBoolean("EnableModifiers", false);
        int Toolbar = 0;
        try { Toolbar = Integer.parseInt(preferences.getString("Toolbar", "0")); } catch(NumberFormatException ex) { }

        // Show/hide the mouse buttons.
        if(EnableMouseButtons) mousebuttons.setVisibility(View.VISIBLE);
        else mousebuttons.setVisibility(View.GONE);

        // Show/hide the modifier keys.
        if(EnableModifiers) modifiers.setVisibility(View.VISIBLE);
        else modifiers.setVisibility(View.GONE);

        // Show/hide media toolbar.
        if(Toolbar == 1) media.setVisibility(View.VISIBLE);
        else media.setVisibility(View.GONE);

        // Show/hide browser toolbar.
        if(Toolbar == 2) browser.setVisibility(View.VISIBLE);
        else browser.setVisibility(View.GONE);

        timer.postDelayed(mKeepAliveListener, KeepAlive);
    }

    @Override
    protected void onPause() {
        timer.removeCallbacks(mKeepAliveListener);
        disconnect(true);
        super.onPause();
    }

    // Mouse actions.
    protected class Action {
        protected float downX, downY;
        protected float oldX, oldY;
        protected int pointerId;
        protected long downTime;
        protected boolean moving;

        public boolean isClick(MotionEvent e) {
            ViewConfiguration vc = ViewConfiguration.get(touchpad.getContext());

            int index = e.findPointerIndex(pointerId);
            return	Math.abs(e.getX(index) - downX) < vc.getScaledTouchSlop() &&
                    Math.abs(e.getY(index) - downY) < vc.getScaledTouchSlop() &&
                    e.getEventTime() - downTime < vc.getTapTimeout();
        }

        public boolean onDown(MotionEvent e) {
            pointerId = e.getPointerId(0);
            int index = e.findPointerIndex(pointerId);
            oldX = downX = e.getX(index);
            oldY = downY = e.getY(index);
            downTime = e.getEventTime();
            moving = false;
            return true;
        }
        public boolean onUp(MotionEvent e) {
            if (isClick(e))
                onClick();
            return true;
        }

        public boolean acceptMove(MotionEvent e) {
            return true;
        }

        public boolean onMove(MotionEvent e) {
            if(!acceptMove(e))
                return false;

            int index = e.findPointerIndex(pointerId);

            float X = e.getX(index);
            float Y = e.getY(index);
            if(moving)
                onMoveDelta((X - oldX) * Sensitivity, (Y - oldY) * Sensitivity);
            else
                moving = true;
            oldX = X;
            oldY = Y;
            return true;
        }
        public boolean cancel(MotionEvent e) {
            return false;
        }

        public void onMoveDelta(float dx, float dy) { }
        public void onClick() { }
    };

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
