package com.khopan.hackontrol.source;

public interface CommandSource {
	String getMachineId();
	void sendMessage(String message);
	void sendCodeMessage(String message);
	void sendBoldCodeMessage(String message);
}
