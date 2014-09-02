package com.note4me.arduinopad;

import java.util.ArrayList;
import java.util.Arrays;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.*;
import android.widget.SeekBar.OnSeekBarChangeListener;

public class DeviceControlActivity extends Activity
{
    private static final String TAG = "DeviceControlActivity";
    private static final boolean D = true;

    final Context context = this;

    private boolean isSettingsMode = false;
    private String connectedDeviceName;
    private String connectedDeviceAddress;
    
    private DeviceConnector connector;
    private BluetoothAdapter btAdapter;

    // Message types sent from the DeviceConnector Handler
    public static final int MESSAGE_STATE_CHANGE = 1;
    public static final int MESSAGE_READ = 2;
    public static final int MESSAGE_WRITE = 3;
    public static final int MESSAGE_DEVICE_NAME = 4;
    public static final int MESSAGE_TOAST = 5;
    
    public static final String TOAST = "toast";
    public static final char SPEED_PREFIX = 'G';
    public static final char BRIGHTNESS_PREFIX = 'Q';
    
    public static final String NOT_SET_TEXT = "[not set]";

    private ArrayList<Button> padButtons = new ArrayList<Button>();
    private ArrayList<Integer> padButtonsIds = new ArrayList<Integer>();
    
    private ArrayList<LogData> logData = new ArrayList<LogData>();
    private String incomingBuffer = "";
    private LogViewDialog logDialog;
    
    private EditText speedText;
    private EditText brightnessText;
    private SeekBar speedBar;
    private SeekBar brightnessBar;

    private View separator1;
    private View separator2;
    private TextView textView1;
    private TextView textView2;
    private LinearLayout speedPanel;
    private LinearLayout brightnessPanel;
    private Button buttonSaveSettings;
    
    @Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);	
		if (D) Log.d(TAG, "+++ ON CREATE +++");

		setContentView(R.layout.device_controller);

		Intent intent = getIntent();
		connectedDeviceName = intent.getStringExtra(MainActivity.DEVICE_NAME);
		connectedDeviceAddress = intent.getStringExtra(MainActivity.DEVICE_ADDRESS);

		Integer ids[] = {R.id.button0, R.id.button1, R.id.button2, R.id.button3, R.id.button4,
				R.id.button5, R.id.button6, R.id.button7, R.id.button8, R.id.button9, 
				R.id.buttonLeft, R.id.buttonRight};
		
		padButtonsIds.addAll(Arrays.asList(ids));
		
		setTitle(connectedDeviceName + " not connected");
		
		btAdapter = BluetoothAdapter.getDefaultAdapter();
		
		setupControls();
		enableControls();

		if (connector != null)
        {
        	if (D) Log.d(TAG, "+++ ON CREATE +++, connector state " + connector.getState());
        }
	}

    @Override
    public void onStart()
    {
        super.onStart();
        if (D) Log.d(TAG, "++ ON START ++");

        if (!btAdapter.isEnabled())
        {
        	if (D) Log.d(TAG, "++ ON START BT disabled ++");
            Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableIntent, MainActivity.REQUEST_ENABLE_BT);        
        }
        else // Otherwise, setup the chat session
        {
            if (D) Log.d(TAG, "++ ON START BT enabled ++");
            if (connector == null)
            {
            	if (D) Log.d(TAG, "++ ON START setupConnector() ++");
            	setupConnector();
            }
            else
            {
            	if (D) Log.d(TAG, "++ ON START ++, connector state " + connector.getState());
            }
        }
    }

    @Override
    public synchronized void onResume()
    {
        super.onResume();
        if (D) Log.d(TAG, "+ ON RESUME +");

        if (connector != null)
        {
        	if (D) Log.d(TAG, "+ ON RESUME +, connector state " + connector.getState());
        }
    }

    @Override
    protected void onDestroy()
    {
        super.onDestroy();
        if (D) Log.d(TAG, "--- ON DESTROY ---");

        if (connector != null)
        {
        	if (D) Log.d(TAG, "--- ON DESTROY ---, connector state " + connector.getState());
        	connector.stop();
        	connector = null;
        }
    }

    @Override
    public synchronized void onPause()
    {
        super.onPause();
        if (D) Log.d(TAG, "- ON PAUSE -");
        
        if (connector != null)
        {
        	if (D) Log.d(TAG, "- ON PAUSE -, connector state " + connector.getState());
        }
    }

    @Override
    public void onStop()
    {
        super.onStop();
        if (D) Log.d(TAG, "-- ON STOP --");

        if (connector != null)
        {
        	if (D) Log.d(TAG, "-- ON STOP --, connector state " + connector.getState());
        }
    }

	@Override
	protected void onActivityResult (int requestCode, int resultCode, Intent data)
	{
		super.onActivityResult(requestCode, resultCode, data);
		if (D) Log.d(TAG, "onActivityResult " + resultCode);

		if (requestCode == MainActivity.REQUEST_ENABLE_BT)
		{
			if (resultCode == RESULT_OK)
			{
				setupConnector();
			}
			else
			{
                Toast.makeText(this, R.string.bt_not_enabled_leaving, Toast.LENGTH_SHORT).show();
                finish();
			}
		}
	}
	
	private void setupConnector()
	{
		if (D) Log.d(TAG, "setupConnector");
		
		if (connector != null)
		{
			if (D) Log.d(TAG, "setupConnector connector.stop(), state " + connector.getState());
			connector.stop();
			connector = null;
		}
		
		BluetoothDevice connectedDevice = btAdapter.getRemoteDevice(connectedDeviceAddress);
		String emptyName = getResources().getString(R.string.empty_device_name);

		DeviceData data = new DeviceData(connectedDevice, emptyName);
		
		connector = new DeviceConnector(data, mHandler);
		connector.connect();
	}

    private void setupControls()
    {    	
		for (int id : padButtonsIds)
		{
			Button btn = (Button)findViewById(id);
			btn.setOnClickListener(btnControlClick);
			padButtons.add(btn);
		}
		
		for (int i = 0; i < 12; i++)
		{
			Button btn = padButtons.get(i);
			String cmd = MainActivity.buttonCommands.get(i);
			btn.setText(cmd);
		}
		
		checkButtonLabels();

		buttonSaveSettings = (Button)findViewById(R.id.buttonSaveSettings);

		speedText = (EditText)findViewById(R.id.speedText);
		brightnessText = (EditText)findViewById(R.id.brightnessText);
		speedBar = (SeekBar)findViewById(R.id.speedBar);
		brightnessBar = (SeekBar)findViewById(R.id.brightnessBar);

        separator1 = findViewById(R.id.separator1);
        separator2 = findViewById(R.id.separator2);
        textView1 = (TextView)findViewById(R.id.textview1);
        textView2 = (TextView)findViewById(R.id.textview2);
        speedPanel = (LinearLayout)findViewById(R.id.speedPanel);
        brightnessPanel = (LinearLayout)findViewById(R.id.brightnessPanel);
		
		speedText.setEnabled(false);
		brightnessText.setEnabled(false);
		speedText.setFocusable(false);
		brightnessText.setFocusable(false);
		
		buttonSaveSettings.setOnClickListener(btnSaveSettingsClick);
		buttonSaveSettings.setVisibility(View.INVISIBLE);
		
		speedBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener()
		{
			public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser)
			{
				speedText.setText(Integer.toString(progress));
				
    			if (connector != null)
    			{
    				char dataToSend = (char)(SPEED_PREFIX + progress); 
    				String data = String.valueOf(dataToSend);
    				connector.write(data);
    				Log.d(TAG, data);
    			}
			}

			public void onStartTrackingTouch(SeekBar seekBar)
			{
			}

			public void onStopTrackingTouch(SeekBar seekBar)
			{
			}
		});

		brightnessBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener()
		{
			public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser)
			{
				brightnessText.setText(Integer.toString(progress));

    			if (connector != null)
    			{
    				char dataToSend = (char)(BRIGHTNESS_PREFIX + progress); 
    				String data = String.valueOf(dataToSend);
    				connector.write(data);
    				Log.d(TAG, data);
    			}
			}

			public void onStartTrackingTouch(SeekBar seekBar)
			{
			}

			public void onStopTrackingTouch(SeekBar seekBar)
			{
			}
		});
    }
    
    private void checkButtonLabels()
    {
    	for (Button btn : padButtons)
    	{
   			btn.setEnabled(!btn.getText().equals(NOT_SET_TEXT) || isSettingsMode);
    	}
    }

    private OnClickListener btnControlClick = new OnClickListener()
    {
    	@Override
    	public void onClick(View v)
    	{
    		Button btn = (Button)v;
    		if (isSettingsMode)
    		{
    			showButtonActionDialog(btn);
    		}
    		else
    		{
    			if (connector != null)
    			{
    				String data = btn.getText().toString();
    				connector.write(data);
    			}
    		}
    	}
    };
    
    private void showButtonActionDialog(Button btn)
    {    	
    	ButtonSetupDialog newFragment = ButtonSetupDialog.newInstance(btn.getId(), btn.getText().toString());
        newFragment.show(getFragmentManager(), "ButtonSetupDialog");
    }

    private OnClickListener btnSaveSettingsClick = new OnClickListener()
    {
    	@Override
    	public void onClick(View v)
    	{
    		setSettingsMode(false);
    		checkButtonLabels();
    		
    		for (int i = 0; i < 12; i++)
    		{
    			Button btn = padButtons.get(i);
    			String cmd = btn.getText().toString();
    			
    			MainActivity.buttonCommands.set(i, cmd);
    		}
    		
    		saveSettings();
    	}
    };

	private void saveSettings()
	{
		SharedPreferences settings = getSharedPreferences(MainActivity.PREFS_NAME, 0);
		SharedPreferences.Editor editor = settings.edit();
		
		for (int i = 0; i < 12; i++)
		{
			String cmd = MainActivity.buttonCommands.get(i);
			editor.putString(MainActivity.PREFS_KEY_COMMAND + i, cmd);
		}
		
		editor.commit();
	}
    
	@Override
	public boolean onPrepareOptionsMenu(Menu menu)
	{
		super.onPrepareOptionsMenu(menu);		
		if (D) Log.d(TAG, "onPrepareOptionsMenu");
		
		int state = DeviceConnector.STATE_NONE;
		if (connector != null)
		{
			state = connector.getState();
		}

		menu.findItem(R.id.menu_settings).setEnabled(!isSettingsMode && (state == DeviceConnector.STATE_CONNECTED));
		menu.findItem(R.id.menu_show_log).setEnabled(!isSettingsMode && (state == DeviceConnector.STATE_CONNECTED));
		menu.findItem(R.id.menu_connect).setEnabled(!isSettingsMode && (state == DeviceConnector.STATE_NONE));
		menu.findItem(R.id.menu_disconnect).setEnabled(!isSettingsMode && (state != DeviceConnector.STATE_NONE));

		return true;
	}
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
    	super.onCreateOptionsMenu(menu);
    	if (D) Log.d(TAG, "onCreateOptionsMenu");

    	MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.activity_device, menu);

        return true;
    }

    private void setSettingsMode(boolean mode)
    {
        isSettingsMode = mode;

        separator1.setVisibility(isSettingsMode ? View.INVISIBLE : View.VISIBLE);
        separator2.setVisibility(isSettingsMode ? View.INVISIBLE : View.VISIBLE);
        textView1.setVisibility(isSettingsMode ? View.INVISIBLE : View.VISIBLE);
        textView2.setVisibility(isSettingsMode ? View.INVISIBLE : View.VISIBLE);

        speedPanel.setVisibility(isSettingsMode ? View.INVISIBLE : View.VISIBLE);
        brightnessPanel.setVisibility(isSettingsMode ? View.INVISIBLE : View.VISIBLE);
        
        buttonSaveSettings.setVisibility(isSettingsMode ? View.VISIBLE : View.INVISIBLE);
    }
    
    private void showLogDialog()
    {
    	logDialog = LogViewDialog.newInstance();
    	logDialog.show(getFragmentManager(), "LogViewDialog");
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
        switch (item.getItemId())
        {
            case R.id.menu_settings:
                setSettingsMode(true);
                checkButtonLabels();
                return true;
            
            case R.id.menu_connect:
            	setupConnector();
                return true;

            case R.id.menu_show_log:
            	showLogDialog();
                return true;
                
            case R.id.menu_disconnect:
        		if (connector != null)
        		{
        			connector.stop();
        		}
                return true;

            default:
                return super.onOptionsItemSelected(item);
        }
    }

    public void updateButtonText(int btnId, String text)
    {
    	for (Button btn : padButtons)
    	{
    		if (btn.getId() == btnId)
    		{
    			btn.setText(text);
    			return;
    		}
    	}
    }
    
	private void enableControls()
	{
		boolean enable = false;
		if (connector != null)
		{
			enable = connector.getState() == DeviceConnector.STATE_CONNECTED;
		}
		
		speedBar.setEnabled(enable);
		brightnessBar.setEnabled(enable);

    	for (Button btn : padButtons)
    	{
    		if (!enable)
    		{
    			btn.setEnabled(false);
    			continue;
    		}
   			btn.setEnabled(!btn.getText().equals(NOT_SET_TEXT) || isSettingsMode);
    	}
	}
	
	private void fillLogView()
	{
		while (logData.size() > 5)
		{
			logData.remove(0);
		}
		
		ArrayList<LogData> inverted = new ArrayList<LogData>();
		for (int i = logData.size() - 1; i >= 0; i--)
		{
			inverted.add(logData.get(i));
		}
	}

	private boolean getMessage(IncomingMessageData data)
	{
		data.setMessage("");
		
		int startSignaturePos = incomingBuffer.indexOf(IncomingMessageData.START_SIGNATURE);
		int endSinaturePos = incomingBuffer.indexOf(IncomingMessageData.END_SIGNATURE);
		
		if (startSignaturePos != 0 || endSinaturePos == -1)
			return false;
		
		String message = incomingBuffer.substring(0, endSinaturePos + IncomingMessageData.END_SIGNATURE.length());
		
		//message now ends with ***, and starts with ###
		//but following message may happens:
		//###195-145-95-45-1###200-###200-150-100-50-5-55***
		//###200###200-250-210-160-105-55***
		//###245-195-145-95-40-10**####255-205-155-105-50-0***
		//######80-130-180-230-225-175***
		//so will find last occurrence of ###
		
		startSignaturePos = message.lastIndexOf(IncomingMessageData.START_SIGNATURE);
		if (startSignaturePos > 0)
		{
			message = message.substring(startSignaturePos);
		}

		data.setMessage(message);
		
		incomingBuffer = incomingBuffer.substring(endSinaturePos + IncomingMessageData.END_SIGNATURE.length());
		
		return true;
	}
	
	private void shrinkBuffer()
	{
		int startSignaturePos = incomingBuffer.indexOf(IncomingMessageData.START_SIGNATURE);
		if (startSignaturePos == 0 || startSignaturePos == -1)
		{
			return;
		}
		
		incomingBuffer = incomingBuffer.substring(startSignaturePos);
	}
	
	private void sendMessageToLog(String txt)
	{
		if (logDialog == null)
		{
			return;
		}
		
        Message msg = logDialog.getHandler().obtainMessage(LogViewDialog.MESSAGE_LOG_ADDED);
        Bundle bundle = new Bundle();
        bundle.putString(LogViewDialog.MESSAGE_TEXT, txt);
        msg.setData(bundle);
        
        logDialog.getHandler().sendMessage(msg);        
	}
	
	private void appendIncomingMessage(String message)
	{
		incomingBuffer += message;
		
		shrinkBuffer();
		
		IncomingMessageData data = new IncomingMessageData();
		while (getMessage(data))
		{
			String msg = data.getMessage();
			if (!msg.isEmpty())
			{
				sendMessageToLog(msg);
				
				LogData ld = new LogData("", msg);
				logData.add(ld);
			}
		}

		fillLogView();
	}

	private void appendOutgoingMessage(String message)
	{
		LogData data = new LogData("", message);
		logData.add(data);
		fillLogView();
	}
	
    private final Handler mHandler = new Handler()
    {
        @Override
        public void handleMessage(Message msg)
        {
            switch (msg.what)
            {
	            case MESSAGE_STATE_CHANGE:
	            	
	            	if(D) Log.d(TAG, "MESSAGE_STATE_CHANGE: " + msg.arg1);
	            	String messageText = "";
	
	            	switch (msg.arg1)
	                {
		                case DeviceConnector.STATE_CONNECTED:
		                	messageText = "Connected to " + connectedDeviceName;
		                	setTitle(messageText);
		                    break;
		                case DeviceConnector.STATE_CONNECTING:
		                	messageText = "Connecting to " + connectedDeviceName;
		                	setTitle(messageText);
		                    break;
		                case DeviceConnector.STATE_NONE:
		                	messageText = connectedDeviceName + " is not connected";
		                	setTitle(messageText);
		                    break;
	                }

	            	enableControls();
	            	invalidateOptionsMenu();
	            	appendOutgoingMessage(messageText);
	                break;
	            
	            case MESSAGE_DEVICE_NAME:
	                Toast.makeText(getApplicationContext(), "Successfully connected to " + connectedDeviceName, 
	                		Toast.LENGTH_SHORT).show();
	                break;

	            case MESSAGE_WRITE:
	                byte[] writeBuf = (byte[]) msg.obj;
	                // construct a string from the buffer
	                String writeMessage = new String(writeBuf);
	                appendOutgoingMessage(writeMessage);
	                break;
	                
	            case MESSAGE_READ:
	                byte[] readBuf = (byte[]) msg.obj;
	                String readMessage = new String(readBuf, 0, msg.arg1);
	                appendIncomingMessage(readMessage);
	                break;
	                
	            case MESSAGE_TOAST:
	                Toast.makeText(getApplicationContext(), msg.getData().getString(TOAST),
	                        Toast.LENGTH_SHORT).show();
	                break;
            }
        }
    };
}

