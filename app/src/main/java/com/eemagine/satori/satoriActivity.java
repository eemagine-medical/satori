package com.eemagine.satori;


import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.util.Log;

import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;

public class satoriActivity extends android.app.NativeActivity {
  static {
    System.loadLibrary("usb");
    System.loadLibrary("eego-SDK");
  }

  private static String TAG = "satori";

  public satoriActivity() {
    super();
    Log.v(TAG, "constructing satoriActivity");
  }

  private UsbManager mUsbManager;
  private PendingIntent mPermissionIntent;
  private static final String ACTION_USB_PERMISSION = "com.eemagine.satori.USB_PERMISSION";
  private HashMap<UsbDevice, UsbDeviceConnection> mDeviceMap = new HashMap<UsbDevice, UsbDeviceConnection>();
  private HashMap<Integer, Integer> mDeviceIdFd = new HashMap<Integer, Integer>();

  @Override
  protected void onCreate (Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    mUsbManager         = (UsbManager) getSystemService( Context.USB_SERVICE );

    mPermissionIntent = PendingIntent.getBroadcast(this, 0, new Intent( ACTION_USB_PERMISSION ), 0 );
    registerReceiver(mUsbAttachReceiver, new IntentFilter(UsbManager.ACTION_USB_DEVICE_ATTACHED));
    registerReceiver(mUsbDetachReceiver, new IntentFilter(UsbManager.ACTION_USB_DEVICE_DETACHED));
    registerReceiver(mUsbPermissionReceiver, new IntentFilter(ACTION_USB_PERMISSION) );

    Log.v(TAG, "java usb: onCreate finished");

    HashMap< String, UsbDevice > stringDeviceMap    = mUsbManager.getDeviceList();
    Collection< UsbDevice > usbDevices              = stringDeviceMap.values();
    Iterator< UsbDevice > usbDeviceIter             = usbDevices.iterator();

    while( usbDeviceIter.hasNext() )
    {
	    UsbDevice device = usbDeviceIter.next();

        if ( device != null )
        {
            openDevice(device);
        }
    }
  }

  private void openDevice(UsbDevice device) {
    // Request permission to access the device.
    // mUsbManager.requestPermission( device, mPermissionIntent );
    
    if(mUsbManager.hasPermission(device)) {
        // Open the device.
        UsbDeviceConnection connection = mUsbManager.openDevice( device );
        // connection.claimInterface(device.getInterface(0), true);
        int fd = connection.getFileDescriptor();
        mDeviceMap.put(device, connection);
        mDeviceIdFd.put(device.getDeviceId(), fd);
        // Log.v(TAG, "java usb: pre-attach, fd=" + fd);
        com.eemagine.eego.sdk.jni.attach_fd(fd);
        // Log.v(TAG, "java usb: post-attach");
    } else {
        Log.v(TAG, "java usb: no permission for device, requesting!");
        mUsbManager.requestPermission(device, mPermissionIntent);
    }
  }

  private void closeDevice(UsbDevice device) {
    mDeviceMap.remove(device);
    mDeviceIdFd.remove(device.getDeviceId());
  }

  private BroadcastReceiver mUsbDetachReceiver = new BroadcastReceiver() {
    public void onReceive(Context context, Intent intent) {
      String action = intent.getAction(); 

      if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action)) {
        Log.v(TAG, "java usb: detach");
              synchronized (this) 
              {
                  UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                  int fd = mDeviceIdFd.get(device.getDeviceId());
                  // Log.v(TAG, "java usb: pre-detach, fd=" + fd);
                  com.eemagine.eego.sdk.jni.detach_fd(fd);
                  // Log.v(TAG, "java usb: post-detach");

                  mDeviceIdFd.remove(device.getDeviceId());
              }
/*
        UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
        if (device != null) {
        }
*/
      }
    }
  };

  private BroadcastReceiver mUsbAttachReceiver = new BroadcastReceiver() {
    public void onReceive(Context context, Intent intent) {
      String action = intent.getAction(); 
      if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action)) {
        Log.v(TAG, "java usb: attach");
              synchronized (this) 
              {
              }
      }
    }
  };

  private BroadcastReceiver mUsbPermissionReceiver = new BroadcastReceiver() 
  {
      public void onReceive(Context context, Intent intent)
      {
          String action = intent.getAction();
          if (ACTION_USB_PERMISSION.equals(action)) {
              Log.v(TAG, "java usb: permission");
              synchronized (this) 
              {
                  UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);

                  if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) 
                  {
                      if(device != null)
                      {
                          // openDevice(device);
  /*
                          if (device != null && !mUsbManager.hasPermission(device)) {
                              Log.v(TAG, "java usb: no permission for device, requesting!");
                              mUsbManager.requestPermission(device, mPermissionIntent);
                              return;
                          }

                          UsbDeviceConnection connection = mUsbManager.openDevice( device );
                          int fd = connection.getFileDescriptor();
                          attach_fd(fd);
                          Log.v(TAG, "java usb: " + device.getDeviceName() + " ID: " + device.getDeviceId());
  */
                      }
                  } 
                  else 
                  {
                      Log.d( "USB", "permission denied for device " + device);
                  }
              }
          }
      }
  };

}
