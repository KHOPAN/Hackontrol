package com.khopan.hackontrol.remote;

import java.awt.BorderLayout;

import javax.swing.JFrame;
import javax.swing.WindowConstants;

import com.khopan.hackontrol.remote.component.StreamView;
import com.khopan.hackontrol.remote.network.HRSPConnection;
import com.khopan.hackontrol.remote.network.MultiPacketProcessor;

public class HackontrolRemote {
	private static HackontrolRemote INSTANCE;

	private HackontrolRemote() {
		HackontrolRemote.INSTANCE = this;
		JFrame frame = new JFrame();
		frame.setTitle("Hackontrol Remote");
		frame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
		frame.setLayout(new BorderLayout());
		StreamView streamView = new StreamView();
		frame.add(streamView, BorderLayout.CENTER);
		frame.setSize(600, 400);
		frame.setLocationRelativeTo(null);
		frame.setAlwaysOnTop(true);
		frame.setVisible(true);
		new HRSPConnection(MultiPacketProcessor.of(streamView));
	}

	public static void main(String[] args) {
		HackontrolRemote.getInstance();
	}

	public static HackontrolRemote getInstance() {
		if(HackontrolRemote.INSTANCE == null) {
			HackontrolRemote.INSTANCE = new HackontrolRemote();
		}

		return HackontrolRemote.INSTANCE;
	}
}
