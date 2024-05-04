package com.khopan.hackontrol;

public class CameraDevice {
	public final String deviceName;
	public final String symbolicLink;

	public CameraDevice(String deviceName, String symbolicLink) {
		this.deviceName = deviceName;
		this.symbolicLink = symbolicLink;
	}

	public String getDeviceName() {
		return this.deviceName;
	}

	public String getSymbolicLink() {
		return this.symbolicLink;
	}
}
