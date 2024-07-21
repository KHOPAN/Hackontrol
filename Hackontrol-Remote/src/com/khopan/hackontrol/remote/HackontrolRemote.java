package com.khopan.hackontrol.remote;

import java.awt.BorderLayout;

import javax.swing.DefaultListModel;
import javax.swing.JFrame;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.UIManager;
import javax.swing.WindowConstants;
import javax.swing.border.TitledBorder;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.khopan.hackontrol.remote.session.RemoteSession;

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
		JPanel panel = new JPanel();
		panel.setBorder(new TitledBorder("Connected Devices"));
		panel.setLayout(new BorderLayout());
		JList<RemoteSession> list = new JList<>();
		DefaultListModel<RemoteSession> model = new DefaultListModel<>();
		list.setModel(model);
		panel.add(new JScrollPane(list), BorderLayout.CENTER);
		frame.add(panel, BorderLayout.CENTER);
		frame.setSize(400, 600);
		frame.setLocationRelativeTo(null);
		frame.setVisible(true);
		//HRSPServer.start(MultiPacketProcessor.of(streamView));
	}

	public static void main(String[] args) throws Throwable {
		UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
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
