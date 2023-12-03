package com.khopan.hackontrol.source;

import java.awt.Image;

import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;

public interface CommandSource {
	String getMachineId();
	String getName();
	boolean isSelected();
	String getNickname();
	MessageChannel getChannel();
	void setSelected(boolean selected);
	void setNickname(String nickname);
	void sendMessage(String message);
	void sendCodeMessage(String message);
	void sendBoldCodeMessage(String message);
	void sendImage(Image image);
	void sendImage(Image image, String message);
	void message(String message);
}
