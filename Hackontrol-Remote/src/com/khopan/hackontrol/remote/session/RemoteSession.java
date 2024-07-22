package com.khopan.hackontrol.remote.session;

import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.WindowConstants;
import javax.swing.border.TitledBorder;

import com.khopan.hackontrol.remote.HackontrolRemote;
import com.khopan.hackontrol.remote.component.StreamView;
import com.khopan.hackontrol.remote.network.Packet;

public class RemoteSession {
	private final Socket socket;
	private final InputStream inputStream;
	private final OutputStream outputStream;
	private final int screenWidth;
	private final int screenHeight;
	private final String displayName;
	private final JFrame frame;

	private StreamView streamView;

	public RemoteSession(Socket socket, InputStream inputStream, OutputStream outputStream, Runnable onClose, int width, int height, String username) {
		this.socket = socket;
		this.inputStream = inputStream;
		this.outputStream = outputStream;
		this.screenWidth = width;
		this.screenHeight = height;
		String address = this.socket.getInetAddress().getHostAddress();
		this.displayName = username + " (" + address + ')';
		this.frame = new JFrame();
		this.frame.setTitle(HackontrolRemote.NAME + " - " + username + ' ' + address);
		this.frame.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
		this.frame.setLayout(new BorderLayout());
		this.frame.setLayout(new GridLayout(1, 2));
		JPanel streamPanel = new JPanel();
		streamPanel.setBorder(new TitledBorder("Stream"));
		streamPanel.setLayout(new GridLayout(2, 1));
		this.streamView = new StreamView();
		streamPanel.add(this.streamView);
		this.frame.add(streamPanel);
		this.frame.add(new JButton("Button"));
		this.frame.setSize(600, 400);
		this.frame.setLocationRelativeTo(null);
		this.frame.addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent Event) {
				onClose.run();
			}
		});
	}

	public void start() throws IOException {
		while(true) {
			this.streamView.processPacket(Packet.readPacket(this.inputStream));
		}
	}

	public void open() {
		this.frame.setVisible(true);
	}

	@Override
	public String toString() {
		return this.displayName;
	}
}
