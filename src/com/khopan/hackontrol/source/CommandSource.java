package com.khopan.hackontrol.source;

import java.awt.Image;

public interface CommandSource {
	String getMachineId();
	boolean isSelected();
	void setSelected(boolean selected);
	void sendMessage(String message);
	void sendCodeMessage(String message);
	void sendBoldCodeMessage(String message);
	void sendImage(Image image);
	void sendImage(Image image, String message);
}
