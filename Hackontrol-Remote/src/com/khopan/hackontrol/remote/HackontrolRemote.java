package com.khopan.hackontrol.remote;

import java.awt.BorderLayout;
import java.awt.Point;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.DefaultListModel;
import javax.swing.JFrame;
import javax.swing.JList;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.ListSelectionModel;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.WindowConstants;
import javax.swing.border.TitledBorder;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.khopan.hackontrol.remote.network.HRSPServer;
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
		list.setFocusable(false);
		list.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		list.addMouseListener(new ListListener(list));
		panel.add(new JScrollPane(list), BorderLayout.CENTER);
		frame.add(panel, BorderLayout.CENTER);
		frame.setSize(400, 600);
		frame.setLocationRelativeTo(null);
		frame.setVisible(true);
		HRSPServer.start(model);
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

	private static class ListListener extends MouseAdapter {
		private final JList<RemoteSession> list;

		private ListListener(JList<RemoteSession> list) {
			this.list = list;
		}

		@Override
		public void mousePressed(MouseEvent Event) {
			Point point = Event.getPoint();

			for(int i = 0; i < this.list.getModel().getSize(); i++) {
				if(this.list.getCellBounds(i, i).contains(point)) {
					return;
				}
			}

			this.list.clearSelection();
		}

		@Override
		public void mouseClicked(MouseEvent Event) {
			if(!SwingUtilities.isRightMouseButton(Event)) {
				return;
			}

			Point point = Event.getPoint();

			for(int i = 0; i < this.list.getModel().getSize(); i++) {
				if(this.list.getCellBounds(i, i).contains(point)) {
					this.list.setSelectedIndex(i);
					JPopupMenu menu = new JPopupMenu();
					menu.add(new JMenuItem("Open"));
					menu.add(new JMenuItem("Disconnect"));
					menu.show(this.list, point.x, point.y);
				}
			}
		}
	}
}
