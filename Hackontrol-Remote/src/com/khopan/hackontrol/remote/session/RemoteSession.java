package com.khopan.hackontrol.remote.session;

import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.WindowConstants;

import com.khopan.hackontrol.remote.HackontrolRemote;
import com.khopan.hackontrol.remote.network.Packet;

public class RemoteSession {
	private final Socket socket;
	private final InputStream inputStream;
	private final OutputStream outputStream;
	private final DefaultListModel<RemoteSession> model;
	private final String displayName;
	private final JFrame frame;
	private final StreamPanel streamPanel;

	public RemoteSession(Socket socket, InputStream inputStream, OutputStream outputStream, DefaultListModel<RemoteSession> model, Runnable onClose, int width, int height, String username) {
		this.socket = socket;
		this.inputStream = inputStream;
		this.outputStream = outputStream;
		this.model = model;
		String address = this.socket.getInetAddress().getHostAddress();
		this.displayName = username + " (" + address + ')';
		this.frame = new JFrame();
		this.frame.setTitle(HackontrolRemote.NAME + " - " + username + ' ' + address);
		this.frame.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
		this.frame.setLayout(new BorderLayout());
		this.frame.setLayout(new GridLayout(1, 2));
		this.streamPanel = new StreamPanel(this.outputStream, width, height);
		this.frame.add(this.streamPanel);
		this.frame.add(new JButton("Button"));
		this.frame.setSize(600, 400);
		this.frame.setLocationRelativeTo(null);
		this.frame.addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent Event) {
				RemoteSession.this.streamPanel.streamView.hideWindow();
				onClose.run();
			}
		});
	}

	public void start() throws IOException {
		while(true) {
			Packet packet = Packet.readPacket(this.inputStream);

			if(packet.getType() == Packet.PACKET_TYPE_STREAM_FRAME) {
				this.streamPanel.streamView.decode(packet.getData());
			}
		}
	}

	public void open() {
		this.frame.setVisible(true);
		RemoteSession.this.streamPanel.streamView.showWindow();
	}

	public void disconnect() {
		try {
			Packet.writePacket(this.outputStream, Packet.of(null, Packet.PACKET_TYPE_INFORMATION));
			this.model.removeElement(this);
			this.frame.dispose();
		} catch(Throwable Errors) {
			throw new RuntimeException(Errors);
		}
	}

	@Override
	public String toString() {
		return this.displayName;
	}
}
