package com.example.sari.touchpad;

import android.app.AlertDialog;
import android.content.Context;

/**
 * on 6/26/2015.
 */
public class ShowOnceDialog {
    public class ShowOnceDialog extends AlertDialog {

        public ShowOnceDialog(Context context) {
            super(context);
            // TODO Auto-generated constructor stub

        }

        public ShowOnceDialog(Context context, int theme) {
            super(context, theme);
            // TODO Auto-generated constructor stub
        }

        public ShowOnceDialog(Context context, boolean cancelable,
                              OnCancelListener cancelListener) {
            super(context, cancelable, cancelListener);
            // TODO Auto-generated constructor stub
        }
    }
}

