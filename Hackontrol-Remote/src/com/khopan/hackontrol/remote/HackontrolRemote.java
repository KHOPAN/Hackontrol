package com.khopan.hackontrol.remote;

import java.awt.BorderLayout;

import javax.swing.JFrame;
import javax.swing.WindowConstants;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.khopan.hackontrol.remote.component.StreamView;
import com.khopan.hackontrol.remote.network.HRSPServer;
import com.khopan.hackontrol.remote.network.MultiPacketProcessor;

public class HackontrolRemote {
	public static final String NAME = "Hackontrol Remote";
	public static final Logger LOGGER = LoggerFactory.getLogger(HackontrolRemote.NAME);

	private static HackontrolRemote INSTANCE;

	private HackontrolRemote() {
		HackontrolRemote.INSTANCE = this;
		JFrame frame = new JFrame();
		frame.setTitle(HackontrolRemote.NAME);
		frame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
		frame.setLayout(new BorderLayout());
		StreamView streamView = new StreamView(text -> frame.setTitle(HackontrolRemote.NAME + " " + text));
		frame.add(streamView, BorderLayout.CENTER);
		frame.setSize(600, 400);
		frame.setLocationRelativeTo(null);
		frame.setAlwaysOnTop(true);
		frame.setVisible(true);
		HRSPServer.start(MultiPacketProcessor.of(streamView));
	}

	public static void main(String[] args) {
		HackontrolRemote.LOGGER.info("Initializing");
		HackontrolRemote.getInstance();
	}

	public static HackontrolRemote getInstance() {
		if(HackontrolRemote.INSTANCE == null) {
			HackontrolRemote.INSTANCE = new HackontrolRemote();
		}

		return HackontrolRemote.INSTANCE;
	}
}
