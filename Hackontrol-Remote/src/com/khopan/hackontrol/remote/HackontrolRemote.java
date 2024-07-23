package com.khopan.hackontrol.remote;

import java.awt.BorderLayout;
import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;

import javax.swing.JFrame;
import javax.swing.UIManager;
import javax.swing.WindowConstants;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.khopan.hackontrol.remote.component.StreamView;
import com.khopan.hackontrol.remote.network.Packet;

public class HackontrolRemote {
	public static final String NAME = "Hackontrol Remote";
	public static final Logger LOGGER = LoggerFactory.getLogger(HackontrolRemote.NAME);

	private static HackontrolRemote INSTANCE;

	private HackontrolRemote() {
		HackontrolRemote.INSTANCE = this;
		/*JFrame frame = new JFrame();
		frame.setTitle(HackontrolRemote.NAME);
		frame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
		frame.setLayout(new BorderLayout());
		JPanel panel = new JPanel();
		panel.setBorder(new TitledBorder("Connected Devices"));
		panel.setLayout(new BorderLayout());
		JList<RemoteSession> list = new JList<>() {
			private static final long serialVersionUID = -2764133493352244595L;

			@Override
			public int locationToIndex(Point location) {
				int index = super.locationToIndex(location);

				if(index != -1 && !this.getCellBounds(index, index).contains(location)) {
					return -1;
				}

				return index;
			}

			@Override
			protected void processMouseEvent(MouseEvent Event) {
				int identifier = Event.getID();

				if(this.locationToIndex(Event.getPoint()) != -1) {
					super.processMouseEvent(Event);
				} else if(identifier != MouseEvent.MOUSE_RELEASED && identifier != MouseEvent.MOUSE_ENTERED && identifier != MouseEvent.MOUSE_EXITED) {
					this.clearSelection();
				}
			}

			@Override
			protected void processMouseMotionEvent(MouseEvent Event) {
				if(Event.getID() != MouseEvent.MOUSE_DRAGGED) {
					super.processMouseMotionEvent(Event);
				}
			}
		};

		DefaultListModel<RemoteSession> model = new DefaultListModel<>();
		list.setModel(model);
		list.setFocusable(false);
		list.addMouseListener(new ListListener(frame, list));
		list.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		list.setFixedCellHeight(30);
		panel.add(new JScrollPane(list), BorderLayout.CENTER);
		frame.add(panel, BorderLayout.CENTER);
		frame.setSize(400, 600);
		frame.setLocationRelativeTo(null);
		frame.setAlwaysOnTop(true);
		frame.setVisible(true);
		HRSPServer.start(model, () -> frame.setVisible(true));*/
		JFrame frame = new JFrame();
		frame.setTitle(HackontrolRemote.NAME);
		frame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
		frame.setLayout(new BorderLayout());
		StreamView streamView = new StreamView(1366, 768);
		frame.add(streamView, BorderLayout.CENTER);
		streamView.setVisible(true);
		frame.setSize(600, 400);
		frame.setLocationRelativeTo(null);
		frame.setAlwaysOnTop(true);
		frame.setVisible(true);

		try {
			HackontrolRemote.LOGGER.info("Wait for incoming connection...");
			ServerSocket server = new ServerSocket(42485);
			Socket socket = server.accept();
			HackontrolRemote.LOGGER.info("Client connected: {}", socket.getInetAddress().getHostAddress());
			InputStream inputStream = socket.getInputStream();
			String response = new String(inputStream.readNBytes(16), StandardCharsets.UTF_8);

			if(!"HRSP 1.0 CONNECT".equals(response)) {
				socket.close();
				server.close();
				return;
			}

			OutputStream outputStream = socket.getOutputStream();
			outputStream.write("HRSP 1.0 OK".getBytes(StandardCharsets.UTF_8));
			outputStream.flush();
			Packet packet = Packet.readPacket(inputStream);

			if(packet.getType() != Packet.PACKET_TYPE_INFORMATION) {
				server.close();
				throw new IllegalArgumentException("Invalid packet type, the first packet sent must be PACKET_TYPE_INFORMATION");
			}

			ByteArrayInputStream informationStream = new ByteArrayInputStream(packet.getData());
			int width = ((informationStream.read() & 0xFF) << 24) | ((informationStream.read() & 0xFF) << 16) | ((informationStream.read() & 0xFF) << 8) | (informationStream.read() & 0xFF);
			int height = ((informationStream.read() & 0xFF) << 24) | ((informationStream.read() & 0xFF) << 16) | ((informationStream.read() & 0xFF) << 8) | (informationStream.read() & 0xFF);
			String username = new String(informationStream.readAllBytes(), StandardCharsets.UTF_8);
			HackontrolRemote.LOGGER.info("Width: {} Height: {} Username: {}", width, height, username);
			Packet.writePacket(outputStream, Packet.of(new byte[] {0b00000001}, Packet.PACKET_TYPE_STREAM_FRAME));

			while(true) {
				packet = Packet.readPacket(inputStream);

				if(packet.getType() != Packet.PACKET_TYPE_STREAM_FRAME) {
					break;
				}

				streamView.decode(packet.getData());
			}

			server.close();
		} catch(Throwable Errors) {
			throw new RuntimeException(Errors);
		}
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

	/*private static class ListListener extends MouseAdapter {
		private final JFrame frame;
		private final JList<RemoteSession> list;

		private ListListener(JFrame frame, JList<RemoteSession> list) {
			this.frame = frame;
			this.list = list;
		}

		@Override
		public void mouseClicked(MouseEvent Event) {
			Point point = Event.getPoint();
			int index = this.list.locationToIndex(point);

			if(index == -1) {
				this.list.clearSelection();
				return;
			}

			this.list.setSelectedIndex(index);
			RemoteSession session = this.list.getSelectedValue();

			if(SwingUtilities.isLeftMouseButton(Event) && Event.getClickCount() >= 2) {
				this.frame.dispose();
				session.open();
				return;
			}

			if(!SwingUtilities.isRightMouseButton(Event)) {
				return;
			}

			JPopupMenu popupMenu = new JPopupMenu();
			JMenuItem openItem = new JMenuItem("Open");
			openItem.addActionListener(action -> {
				this.frame.dispose();
				session.open();
			});

			popupMenu.add(openItem);
			JMenuItem disconnectItem = new JMenuItem("Disconnect");
			disconnectItem.addActionListener(action -> session.disconnect());
			popupMenu.add(disconnectItem);
			popupMenu.show(this.list, point.x, point.y);
		}
	}*/
}
